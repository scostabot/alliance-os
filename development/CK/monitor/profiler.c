/*
 * profiler.c:  The GNU profiler support code
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 03/02/99  ramon       1.0    First release
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

/*
 * Copyright (c) 1998 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

/*-
 * Copyright (c) 1983, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <typewrappers.h>
#include "sys/sys.h"
#include "sys/ioport.h"
#include "sys/gdt.h"
#include "sys/hwkernel.h"
#include "memory.h"
#include "profiler.h"


#define PROFHZ  8192          /* The histogram collection frequency */


/* Data about the CK ELF executable */
extern UADDR _start;                     /* The ELF entry point     */
extern UADDR end;                        /* End of ELF data segment */
extern UADDR etext;                      /* End of ELF text segment */

extern UADDR __profil;                   /* Histogram collecor stub */


/* The profiler main data structure */
struct gmonparam _gmonparam = { GMON_PROF_OFF };


/********************************************************************/
/* Histogram generation code                                        */
/********************************************************************/

/*
 * The following routines are responsible for collection of the histogram
 * data.  From the gprof texinfo:
 *
 *    Profiling also involves watching your program as it runs, and
 *    keeping a histogram of where the program counter happens to be every
 *    now and then.  Typically the program counter is looked at around 100
 *    times per second of run time, but the exact frequency may vary from
 *    system to system.
 *
 * This implementation of the histogram code uses the Motorola MC146818 RTC
 * (real-time clock), which is found in all PCs, to generate interrupts
 * periodically at a fixed frequency.  On every profiling interrupt, the
 * function CKprofilSampler() is called to collect the histogram data.
 *
 * The interface to configuring the histogram collection code is done
 * through the CKprofil() call.  Note:  CKprofil() is NOT POSIX.1
 * compliant !
 *
 * The histogram itself is in the array _gmonparam.kcount[].  It would
 * take a bit too much memory to keep a histogram for every memory address
 * separately, so the histogram accounts blocks of memory, defined by
 * HISTFRACTION (see profile.h.)
 */

VOID CKprofilSampler(struct trap_state *ts)
/*
 * The profiler histogram data collection routine.  This is hooked
 * to IRQ 8 (the MC146818 RTC).  Designed to run with interrupts off.
 *
 * INPUT:
 *     parameters pushed onto the stack by the x86
 *
 * RETURNS:
 *     none
 */
{
    UDATA i, save, savedstate;
    struct gmonparam *p;

    save = dinportb(0x70);      /* Save the RTC status                  */

    p = &_gmonparam;            /* Fetch the profiler data structure    */
    savedstate = p->state;      /* Save the profiler state              */

    p->state = GMON_PROF_BUSY;  /* We're going to be busy for a while   */

    if(savedstate == GMON_PROF_ON) {   /* If we're actually profiling   */
        i = eip - p->lowpc;            /* If we're in the bounds of the */
        if (i < p->textsize) {         /* code to be profiled           */
            i /= HISTFRACTION * sizeof(*p->kcount); /* Calc hist entry  */
            p->kcount[i]++;                         /* Charge the entry */
        }
    }

    doutportb(0x70, 0x0c);      /* Acknowledge the RTC                  */
    dinportb(0x71);

    doutportb(0x70, save);      /* Restore saved RTC status             */
    p->state = savedstate;      /* Restore saved profiler status        */
}


VOID CKprofil(DATA control)
/*
 * The histogram collection control function
 *
 * INPUT:
 *     control:  0 to turn off profiling, otherwise to turn it on
 *
 * RETURNS:
 *     none
 */
{
    BYTE freqcount;

    /* Check the action to preform */
    if (!control) {
        CKdisableIRQ(8);
        return;
    }

    /* Keep us from being interrupted */
    CKlock();

    /* Set the IRQ 8 (RTC) handler to the profiler histogram collector */
    CKsetVector((VOID *) &__profil, M_VEC+8, (D_PRESENT+D_INT+D_DPL3));

    /*
     * Tell the RTC clock the frequency we need
     *
     * 0x29 = 128Hz  0x28 = 256Hz  0x27 = 512 Hz ... 0x23 = 8192Hz
     */

    switch(PROFHZ)
    {
        case 8192:
            freqcount = 0x23;
            break;
        case 4096:
            freqcount = 0x24;
            break;
        case 2048:
            freqcount = 0x25;
            break;
        case 1024:
            freqcount = 0x26;
            break;
        case 512:
            freqcount = 0x27;
            break;
        case 256:
            freqcount = 0x28;
            break;
        case 128:
            freqcount = 0x29;
            break;
        default:
            freqcount = 0x26;
            break;
    }

    /* Set up the RTC */
    doutportb(0x70, 0x0a);      /* Register 0xa - the frequency counter  */
    doutportb(0x71, freqcount); /* Set the RTC interrupts frequency      */
    doutportb(0x70, 0x0b);      /* Register 0xb - the interrupt selector */
    doutportb(0x71, 0x42);      /* Tell the RTC to generate interrupts   */

    /* Clear any pending interrupts in the RTC */
    CKenableIRQ(8);
    doutportb(0x70, 0x0c);
    dinportb(0x71);
    CKenableIRQ(8);

    /* We can continue normal operation now */
    CKunlock();
}


/********************************************************************/
/* Call graph generation code                                       */
/********************************************************************/

/*
 * The following routines are responsible for collection of the call graph
 * data.  From the gprof texinfo:
 *
 *    Profiling works by changing how every function in your program is
 *    compiled so that when it is called, it will stash away some
 *    information about where it was called from.  From this, the profiler
 *    can figure out what function called it, and can count how many times
 *    it was called. This change is made by the compiler when your program
 *    is compiled with the `-pg' option, which causes every function to
 *    call `mcount' (or `_mcount', or `__mcount', depending on the OS and
 *    compiler) as one of its first operations.
 *
 *    The `mcount' routine, included in the profiling library, is
 *    responsible for recording in an in-memory call graph table both its
 *    parent routine (the child) and its parent's parent.  This is
 *    typically done by examining the stack frame to find both the address
 *    of the child, and the return address in the original parent.  Since
 *    this is a very machine-dependant operation, `mcount' itself is
 *    typically a short assembly-language stub routine that extracts the
 *    required information, and then calls `__mcount_internal' (a normal C
 *    function) with two arguments - `frompc' and `selfpc'. 
 *    `__mcount_internal' is responsible for maintaining the in-memory
 *    call graph, which records `frompc', `selfpc', and the number of
 *    times each of these call arcs was transversed.
 *
 * Our __mcount_internal is called _mcount(), and is called from the
 * mcount() and __mcount() stubs in profiler.S.
 *
 * The profiler call graph is a directed weighted graph that keeps track
 * of control transfers between routines.  An edge in the graph represents
 * a control transfer from its tail vertex to its head vertex (which
 * represent control transfer addresses.)  Its corresponding weight keeps
 * track of the amount of times that specific control transfer has taken
 * place.
 *
 * _mcount updates the data structures that represent traversals of the
 * program's call graph edges.  frompc and selfpc are the return
 * address and function address that represents the given call graph edge.
 * (ie, address frompc contains a piece of code that transfers control
 * to selfpc.  selfpc represents the routine entry point.)
 */

VOID _mcount(UADDR frompc, UADDR selfpc)
/*
 * The call graph maintainance code
 *
 * INPUT:
 *     frompc:  caller's return address
 *     selfpc:  caller's address
 *
 * RETURNS:
 *     none
 */
{
    register UHALF *frompcindex;
    register struct tostruct *top, *prevtop;
    register DATA toindex;
    register struct gmonparam *p = &_gmonparam;

    /* Check that we are profiling and that we aren't recursively invoked */
    if (p->state != GMON_PROF_ON)
        return;

    p->state = GMON_PROF_BUSY;  /* We're going to be busy for a while   */

    /*
     * Check that frompcindex is a reasonable pc value.
     * For example: signal catchers get called from the stack,
     *              not from text space.  too bad.
     */

    frompc -= p->lowpc;         /* If we're outside the bounds of the   */
    if (frompc > p->textsize)   /* code to be profiled, just return     */
        goto done;

    /* Find the position of this edge in the edge hash table (p->froms) */
    frompcindex = &p->froms[frompc / (p->hashfraction * sizeof(*p->froms))];
    toindex = *frompcindex;

    if (!toindex) {                   /* First time traversing this arc */
        toindex = ++p->tos[0].link;   /* Get the index of a new edge    */
        if (toindex >= p->tolimit)    /* Check if another edge was free */
            goto overflow;            /* If not, halt further profiling */

        *frompcindex = toindex;       /* Link it into the hash chain    */
        top = &p->tos[toindex];       /* Fetch the address of new edge  */
        top->selfpc = selfpc;         /* Fill in the edge structure     */
        top->count = 1;
        top->link = 0;
        goto done;                    /* We're done !!!                 */
    }

    /* This edge was traversed before                                   */
    top = &p->tos[toindex];           /* Get the address of the edge    */

    if (top->selfpc == selfpc) {      /* Is this the right one ?        */
        top->count++;                 /* Yup, charge it (usual case)    */
        goto done;                    /* Whoopie !!  We're done !       */
    }

    /*
     * Well, the edge we were looking for was not the first entry in the
     * position we were looking at in the edge hash table, so we have to
     * go looking down chain for it. Top points to what we are looking at,
     * prevtop points to previous top.
     */

    for ( ; ; ) {
        if (top->link == 0) {
            /*
             * Top is the end of the chain and none of the chain had
             * top->selfpc == selfpc.  So we allocate a new tostruct
             * and link it to the head of the chain.
             */
            toindex = ++p->tos[0].link;  /* Get the index of a new edge */
            if (toindex >= p->tolimit)   /* Check if an edge was free   */
                goto overflow;           /* No, halt further profiling  */

            top = &p->tos[toindex];      /* Fetch address of new edge   */
            top->selfpc = selfpc;        /* Fill in the edge structure  */
            top->count = 1;
            top->link = *frompcindex;
            *frompcindex = toindex;      /* Link us into the hash chain */
            goto done;                   /* We're done !!!              */
        }

        /* Otherwise, check the next arc on the chain. */

        prevtop = top;                   /* Travese the hash chain      */
        top = &p->tos[top->link];        /* Get address of next edge    */

        if (top->selfpc == selfpc) {
            /*
             * There it is.  Increment its count and move it to the head
             * of the chain.
             */
            top->count++;                /* Charge the arc              */
            toindex = prevtop->link;     /* Move arc to top of chain    */
            prevtop->link = top->link;   /* (this might save us looking */
            top->link = *frompcindex;    /* for it again later on, if   */
            *frompcindex = toindex;      /* it's called again soon      */
            goto done;                   /* We're finished !!!          */
        }
    }

done:                                    /* We're finished !            */
        p->state = GMON_PROF_ON;         /* We can resume profiling now */
        return;                          /* Bye-bye !!!                 */

overflow:                                /* No more arcs available      */
        p->state = GMON_PROF_ERROR;      /* Oops... halt further        */
        return;                          /* profiling and return.       */
}


/********************************************************************/
/* Profiler control code                                            */
/********************************************************************/

VOID CKprofilerStartup(UADDR lowpc, UADDR highpc)
/*
 * CK profiler startup
 *
 * INPUT:
 *     lowpc:  lowest address to profile
 *     highpc: highest address to profile
 *
 * RETURNS:
 *     none
 */
{
    BYTE *cp;
    struct gmonparam *p = &_gmonparam;

    if(!lowpc && !highpc) {
        lowpc  = (UADDR) &_start;
        highpc = (UADDR) &etext;
    }

    /*
     * Now we fill in the main profiler data structure.
     *
     * Round lowpc and highpc to multiples of the density we're using
     * so the rest of the scaling (here and in gprof) stays in ints.
     */

    p->lowpc        = ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->highpc       = ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->textsize     = p->highpc - p->lowpc;
    p->kcountsize   = p->textsize / HISTFRACTION;
    p->hashfraction = HASHFRACTION;
    p->fromssize    = p->textsize / HASHFRACTION;
    p->tolimit      = p->textsize * ARCDENSITY / 100;

    if (p->tolimit < MINARCS)
        p->tolimit = MINARCS;
    else if (p->tolimit > MAXARCS)
        p->tolimit = MAXARCS;

    p->tossize = p->tolimit * sizeof(struct tostruct);

    /* Allocate memory for the profiler data structures: hist, callg */
    cp = CKmemAlloc(p->kcountsize + p->fromssize + p->tossize);
    if (cp == NIL) {
        return;
    }

    p->tos = (struct tostruct *)cp;  cp += p->tossize;
    p->kcount = (HISTCOUNTER *)cp;   cp += p->kcountsize;
    p->froms = (u_short *)cp;
    p->tos[0].link = 0;

    CKprofilerControl(1);
}


VOID CKprofilerControl(DATA mode)
/*
 * CK profiler control
 *
 * INPUT:
 *     mode:  profiler mode to set
 *
 * RETURNS:
 *     none
 */
{
    struct gmonparam *p = &_gmonparam;

    if (mode) {
        /* Start */
        CKprofil(1);
        p->state = GMON_PROF_ON;
    } else {
        /* Stop */
        CKprofil(0);
        p->state = GMON_PROF_OFF;
    }
}


VOID CKprofilerCleanup(VOID)
/*
 * CK profiler cleanup:  generates gmon.out data for gprof
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    DATA fromindex, endfrom, toindex;
    UADDR frompc;
    struct rawarc rawarc;
    struct gmonparam *p = &_gmonparam;
    struct gmonhdr gmonhdr, *hdr;

    /* Check for profiling error -- if so, no profiling data is sent */
    if (p->state == GMON_PROF_ERROR)
        return;

    /* Turn off profiling */
    CKprofilerControl(0);

    /* Initialise a file transfer over the serial port */
    CKinitSerialTransfer();

    /* Fill in the profiler data header */
    hdr = (struct gmonhdr *) &gmonhdr;
    hdr->lpc = p->lowpc;
    hdr->hpc = p->highpc;
    hdr->ncnt = p->kcountsize + sizeof(gmonhdr);
    hdr->version = GMONVERSION;
    hdr->profrate = PROFHZ;

    /* Send it to the serial port */
    CKwriteSerial((char *)hdr, sizeof *hdr);
    CKwriteSerial(p->kcount, p->kcountsize);

    /* Loop through the profiler data and write it to the serial port */
    endfrom = p->fromssize / sizeof(*p->froms);
    for (fromindex = 0; fromindex < endfrom; fromindex++) {
        if (p->froms[fromindex] == 0)
            continue;

        frompc = p->lowpc;
        frompc += fromindex * p->hashfraction * sizeof(*p->froms);
        for (toindex = p->froms[fromindex]; toindex != 0;
             toindex = p->tos[toindex].link) {
            rawarc.raw_frompc = frompc;
            rawarc.raw_selfpc = p->tos[toindex].selfpc;
            rawarc.raw_count = p->tos[toindex].count;
            CKwriteSerial(&rawarc, sizeof rawarc);
        }
    }

    /* Terminate the file transfer (finished) */
    CKterminateSerialTransfer();
}

