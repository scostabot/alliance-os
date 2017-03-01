/*
 * pri_heap.c:  Functions to manage the scheduler's priority heaps
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 30/01/99  ramon       1.2    Added CKprioritise()
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <typewrappers.h>
#include "sys/arith.h"
#include "ckobjects.h"
#include "pri_heap.h"

PUBLIC HeapSelector heaps[ht_maxheaptype];  /* The priority heaps       */


PUBLIC VOID CKsetupHeaps(VOID)
/*
 * Initialise the (empty) priority heaps
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    UBYTE c;

    for(c=0;c<ht_maxheaptype;c++) {
        heaps[c].heapLast=heaps[c].heap-1;   /* The heap is empty       */
    }
}

/* The priority heap is a binary tree (see the documentation)
 * We use pointers instead of indices to access it, though, for
 * performance reasons.
 * Here, we define operations to retreive the parent/children of a node
 *
 * Explanation of the math involved:
 * 
 * A binary tree (such as the priority heap) is usually represented as
 * an array heap[].  For any array element heap[N], its parent it
 * represented by heap[(N-1)/2] and its children are heap[2*N+1] and
 * heap[2*N+2].
 *
 * The address A represented by heap[N] is A = (heap + ADDRSIZE * N);
 * Thus, we can write that the address of the left child of heap[N]
 * is CA = (heap + ADDRSIZE * (2*N+1))  (right child is with 2*N+2).
 * Now, we can express N in terms of A using the first formula, which
 * makes N = (A - heap)/ADDRSIZE;  We fill this in in the second formula
 * to achieve the result CA = 2*A - heap + ADDRSIZE;  similarly, the
 * right child can be expressed as CA = 2*A - heap + 2*ADDRSIZE;
 *
 * We can write the parent of heap[N] as P = (heap + ADDRSIZE * ((N-1)/2));
 * As N is an integer, we cannot simply reverse the order of the
 * multiplication and division involved, but need to look at two cases,
 * one if N is odd and the other if N is even.
 *
 * If N is odd,  (N-1) is even, so (N-1)&1 = 0;  thus, we can safely
 * reverse the order of calculation, which gives the formula
 * P = (heap + Q*N - Q) where Q = ADDRSIZE/2;  substituting the relation
 * N = (A - heap)/(2*Q)  (we assume ADDRSIZE is a power of 2, so even) we
 * get P = (A + heap - 2*Q)/2;
 *
 * If N is even, (N-1) is odd, so (N-1)&1 = 1;  we lose a bit when
 * dividing by 2, so we can't simply reverse the order;  in order to be
 * able to simplify the formula we note that in the case N is even, 
 * (N-1)/2 = (N-2)/2, and (N-2) is even.  Continuing like above, we get
 * P = (heap + Q*N - 2*Q) where Q = ADDRSIZE/2;  using the formula derived
 * above we get P = (heap + Q*N - Q) - Q = (A + heap - 2*Q)/2 - Q;
 *
 * In order to make our final simplification, we assume that the heap,
 * and thus all its elements, are aligned to ADDRSIZE boundary; in this
 * case, the lowest Q bits of each heap element are always zero.  We note
 * that for both the N is even and odd cases, P can be expressed as
 * (A + heap - 2*Q), except that if N is even P is Q too big.  The
 * difference Q is in the aligned part of the address, though, so we can
 * remove the Q by doing P&~Q.  This works also for the N is odd case
 * because the aligned bits are zero there anyway.  Thus we have derived
 * the generic form for the parent address of an address in the heap,
 * P = (A + heap - ADDRSIZE)/2 & ~(ADDRSIZE/2);
 *
 * NOTE:  The divisions in the formula for the parent are signed !!!
 * As we use shifts for speed, we get problems because the heap is
 * in the "negative" part of the memory.  In order to do the calculation
 * correctly we need to set the high bit of the address after shift.
 * (In order to see why, write down all addresses as if they were moved
 * up from low memory:
 * P + 0xc0000000 = (A + heap - ADDRSIZE + 2*0xc0000000)/2 & ~(ADDRSIZE/2);
 * Now 2*0xc0000000=0x180000000, which is truncated to 0x80000000.  So the
 * division by two (which is a shift) actually misses the high bit, which
 * was truncated.  We put it back manually using a bitwise-or.)
 */

#define LCHILD(x) ((EVENT **)((((ADDR)(x))<<1)-((ADDR)curHeaps->heap)+ADDRSIZE))
#define RCHILD(x) ((EVENT **)((((ADDR)(x))<<1)-((ADDR)curHeaps->heap)+2*ADDRSIZE))
#define PARENT(x) ((EVENT **)(((((ADDR)(x)+((ADDR)curHeaps->heap)-ADDRSIZE)>>1)&~(ADDRSIZE>>1))|0x80000000))

/* we use MINCHILD to select which child has the earliest execution time */

#define MINCHILD(x) (((LCHILD(x) < curHeaps->heapLast) && \
                       CLSE(RCHILD(x)[0]->time,LCHILD(x)[0]->time)) ? \
                       RCHILD(x) : LCHILD(x))


/* We define two basic operations on the priority heaps :                */
/*                                                                       */
/* siftup():   This is used to add an event to the list                  */
/* siftdown(): This is used to adjust the heap when an event has changed */
/*                                                                       */
/* NOTE:  Make sure the processor is locked (CKlock()) before use,       */
/*        otherwise a schedule might occur in the middle of one of       */
/*        these, making the system unstable                              */

PUBLIC VOID CKsiftup(HeapType heaptype, EVENT *newEvt)
/*
 * Adjust the heap for an event whose time might be earlier than
 * is appropriate for its position in the heap.
 *
 * INPUT:
 *     heaptype:  The heap to siftup
 *     newEvt:    The address of the EVENT structure to be added to the heap
 *
 * RETURNS:
 *     none
 */
{
    EVENT **next, **parent;
    HeapSelector *curHeaps = &heaps[heaptype];

    curHeaps->heapLast++;         /* Ajust the size of the heap for new evt */
    next = curHeaps->heapLast;    /* Place to start traversing up the tree  */
    parent = PARENT(curHeaps->heapLast); /* ... and its parent              */

    while((next > curHeaps->heap) && CLESS(newEvt->time,parent[0]->time)) {
        next[0] = parent[0];     /* Adjust the heap      */
        next[0]->heap = next;    /* Adjust event backptr */

        next = parent;           /* Advance up the tree  */
        parent = PARENT(next);
    }

    next[0] = newEvt;            /* Add the new event    */
    newEvt->heap = next;         /* Adjust event backptr */
}


PUBLIC VOID CKsiftdown(HeapType heaptype)
/*
 * Adjust the heap for an event whose time might be later than
 * is appropriate for its position in the heap.
 *
 * INPUT:
 *     heaptype:  The heap to siftdown
 *
 * RETURNS:
 *     none
 */
{
    EVENT **next, **minchild, *topEvent;
    HeapSelector *curHeaps = &heaps[heaptype];

    next      = curHeaps->heap;            /* Sift down from the root       */
    minchild  = MINCHILD(curHeaps->heap);  /* Child with earliest exec time */
    topEvent = curHeaps->heap[0];          /* This is the root event        */

    while((minchild <= curHeaps->heapLast) &&
           CLESS(minchild[0]->time,topEvent->time)) {
        next[0] = minchild[0];          /* Adjust the heap              */
        minchild[0]->heap = next;       /* Adjust the event backpointer */
        next = minchild;                /* Traverse through the tree    */
        minchild = MINCHILD(next);
    }

    next[0] = topEvent;                 /* Adjust position for root evt */
    topEvent->heap = next;              /* Adjust the backpointer       */
}


PUBLIC VOID CKremoveEvent(HeapType heaptype, EVENT **evt)
/*
 * Remove an event from a heap
 *
 * INPUT:
 *     heaptype:  The heap to remove an event from
 *     evt:       Pointer into the heap of the event to remove
 *
 * RETURNS:
 *     none
 */
{
    EVENT **minchild, *topEvent;
    HeapSelector *curHeaps = &heaps[heaptype];

    evt[0] = curHeaps->heapLast[0];    /* Remove last element in stead  */
    evt[0]->heap = evt;                /* deleted event and switch them */
    curHeaps->heapLast--;              /* Shorten the heap              */

    minchild  = MINCHILD(evt);         /* Child with earliest exec time */
    topEvent = evt[0];                 /* This is the root event        */

    while((minchild <= curHeaps->heapLast) &&
           CLESS(minchild[0]->time,topEvent->time)) {
        evt[0] = minchild[0];          /* Adjust the heap               */
        minchild[0]->heap = evt;       /* Adjust the event backpointer  */
        evt = minchild;                /* Traverse through the tree     */
        minchild = MINCHILD(evt);
    }

    evt[0] = topEvent;                 /* Adjust position for root evt  */
    topEvent->heap = evt;              /* Adjust the backpointer        */
}


PUBLIC VOID CKprioritise(EVENT *event)
/*
 * Adjust a non-root timesharing event in the heap for an earlier
 * execution time
 *
 * INPUT:
 *     event:     The address of the EVENT structure to be prioritised
 *
 * RETURNS:
 *     none
 */
{
    EVENT **next, **parent;
    HeapSelector *curHeaps;
 
    /* We can only prioritise in the timesharing heap */
    if(event->ts == ts_Running)
        curHeaps = &heaps[ht_timesharing];
    else
        return;

    /* Prioritise by making this the most recent execution time */
    event->time = CKgetNextTime(ht_timesharing);

    next = event->heap;          /* Place to start traversing up the tree   */
    parent = PARENT(next);       /* ... and its parent                      */

    while((next > curHeaps->heap) && CLESS(event->time,parent[0]->time)) {
        next[0] = parent[0];     /* Adjust the heap      */
        next[0]->heap = next;    /* Adjust event backptr */

        next = parent;           /* Advance up the tree  */
        parent = PARENT(next);
    }

    next[0] = event;             /* Add the new event    */
    event->heap = next;          /* Adjust event backptr */
}

