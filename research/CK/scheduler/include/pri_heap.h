/*
 * pri_heap.h:  Defenitions and basic operations for the priority heaps
 *
 * NOTE:  modified 8/3/99 for the scheduler simulation [Ramon]
 *
 * (C) 1998 Jens Albretsen, Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use the generic ckobjects.h
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

#ifndef __PRI_HEAP_H
#define __PRI_HEAP_H

#include <typewrappers.h>
#include "ckobjects.h"
#include "sched.h"

typedef enum {         /* These are the priority heaps in the scheduler */
    ht_timesharing,    /* The heap for 'normal' timesharing threads     */
    ht_realtime,       /* The heap for realtime threads                 */
    ht_idle,           /* The heap for threads that run on idle system  */
    ht_maxheaptype     /* This is the amount of heaps to allocate       */
} HeapType;

typedef struct {            /* Defenition of the heap selector          */
    EVENT *heap[MAXEVENTS]; /* The heap itself, with max size MAXEVENTS */
    EVENT **heapLast;       /* The last used element of the heap        */
} HeapSelector;

extern HeapSelector heaps[];   /* The heaps are allocated in pri_heap.c */


/* We now define a few macros to get basic information from a           */
/* priority heap:                                                       */
/*                                                                      */
/* CKgetNextEvent():   Gets the address of the top event of the heap    */
/* CKisHeapEmpty():    Returns TRUE is the heap is empty                */
/* CKgetNextQuantum(): Gets the quantum of the next event on the heap   */
/* CKgetNextTime():    Gets the execution time of next event on heap    */

#define CKgetNextEvent(x)   heaps[x].heap[0]
#define CKisHeapEmpty(x)   (heaps[x].heapLast==(heaps[x].heap-1))
#define CKgetNextQuantum(x) CKgetNextEvent(x)->quantum
#define CKgetNextTime(x)    CKgetNextEvent(x)->time

/* And now for the compulsory function prototypes */

VOID CKremoveEvent(HeapType heaptype, EVENT **evt);
VOID CKsiftup(HeapType heaptype, EVENT *newEvt);
VOID CKsiftdown(HeapType heaptype);
VOID CKsetupHeaps(VOID);
VOID CKprioritise(EVENT *event);

#endif
