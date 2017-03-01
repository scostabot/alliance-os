/*
 * sched.c:  Main scheduler code and timer interrupt handler
 *
 * NOTE:  modified 8/3/99 for the scheduler simulation [Ramon]
 *
 * Note:  The original idea for this type of scheduler comes from 
 *        John Fine <johnfine@erols.com> who implemented it as an
 *        assembler example.  This is a complete reimplementation
 *        of this kind of scheduler, combining John's timesharing
 *        scheduler design as well as John's realtime scheduler
 *        design, together with some of our own extensions.
 *        I'd like to thank John for his help here.      -- Ramon
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 16/12/98  jens        1.2    Added system dependant include
 * 21/12/98  ramon       1.3    Added thread block/unblock/idle functions
 * 02/01/99  ramon       1.4    Some small optimisations
 * 15/01/99  ramon       1.5    Added signal handling
 * 04/02/99  ramon       1.6    Now scheduler runs with ints on
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
#include "sys/sysdefs.h"
#include "sys/tasks.h"
#include "sys/arith.h"
#include "sys/hwkernel.h"
#include "ckobjects.h"
#include "pri_heap.h"


/* currtask is the task that has been running just before the           */
/* scheduler was triggered (by the timer.)                              */

PUBLIC EVENT *currtask = NIL;


/* last_time contains the execution time of the thread that has just    */
/* been running.  We use this to determine whether the thread was an    */
/* RT thread                                                            */

LOCAL TIME lasttime;


/* The realtime clock count (PUBLIC for now) */

PUBLIC TIME realtime;


/* The idle loop */

extern EVENT idleLoop;


/* doingRealtime is nonzero is the currently running task is realtime */

PUBLIC UBYTE doingRealtime;


/*
 * Now we get the scheduler code itself (this stuff is hooked to
 * the timer interrupt.)
 */

LOCAL inline VOID CKflushPrevTask(VOID)
/*
 * Flush running task down the correct heap with the right time values
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKlock();   /* Keep us from interruptions while modifying the heaps */

    /* Handle flushing the previous task down the right heap            */
    switch(currtask->ts) {
        case ts_Running:                         /* Task was TS         */
            if(!CLESS(lasttime,currtask->time)) {/* Still timesharing   */
                currtask->time+=currtask->defer; /* Advance time value  */
                CKsiftdown(ht_timesharing);      /* Siftdown heap       */
            } else {                             /* Change to realtime  */
                currtask->ts=ts_RealTime;
                CKremoveEvent(ht_timesharing,currtask->heap); /* Remove */
                CKsiftup(ht_realtime,currtask);  /* add to the RT heap  */
            }
            break;
        case ts_RealTime:                        /* Task was RT         */
            doingRealtime = 0;                   /* Exit from RT task   */
            if(CLESS(lasttime,currtask->time)) { /* Still realtime      */
                CKsiftdown(ht_realtime);         /* Siftdown RT priheap */
            } else {                             /* It is now TS        */
                currtask->ts=ts_Running;
                currtask->time=CKgetNextTime(ht_timesharing);
                CKremoveEvent(ht_realtime,currtask->heap); /* Remove    */
                CKsiftup(ht_timesharing,currtask); /* Add it to TS heap */
            }
            break;
        case ts_Idle:                            /* It was an idle task */
            if(currtask != &idleLoop) {          /* If wasn't idle loop */
                currtask->time+=currtask->defer; /* Advance time value  */
                CKsiftdown(ht_idle);             /* Siftdown heap       */
            }
            break;
        case ts_Blocked:                         /* Thread blocked      */
            break;
        default:                                 /* Something's wrong   */
            break;                               /* Oh well, never mind */
    } 

    CKunlock(); /* Other interrupts may occur now                       */
}


LOCAL inline EVENT *CKgetNextTask(TIME *period)
/*
 * Select next task to run and calculate its period
 * Also update the realtime count
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     CKgetNextTask():  Pointer to the event block of next task to be run
 *     period:           Address to store quantum of next task to be run at
 */
{
    CKupdateRealtime();              /* Update the real time count      */

    /* Check if we have a pending RT task ready to run */
    if(!CKisHeapEmpty(ht_realtime)) {
        /*
         * If the next RT task is overdue, handle it directly.
         * XXX - take fudge into account:  if overdue +/- fudge
         */

        if(CLSE(CKgetNextTime(ht_realtime),realtime)) {
            *period=CKgetNextQuantum(ht_realtime); /* Run next RT task  */
            doingRealtime++;                       /* Mark it realtime  */
            return CKgetNextEvent(ht_realtime);
        }

        /*
         * There are no immediate RT tasks to run.  If there are no
         * timesharing tasks pending, we go to the idle loop.  Otherwise,
         * we want to run the next timesharing task.  We check whether
         * a pending RT task will preempt the TS task we're going to run,
         * and if so we adjust its quantum accordingly.
         * XXX - take fudge factor into account here so RT task is run on time
         */

        if(CKisHeapEmpty(ht_timesharing)) goto idle;

        if(CLSE(CKgetNextTime(ht_realtime),CKgetNextQuantum(ht_timesharing)+realtime)) {
            *period=CKgetNextTime(ht_realtime)-realtime;
            return CKgetNextEvent(ht_timesharing); /* make time for RT  */
        }

        /*
         * Plain ol' timesharing task, no big deal
         */

        goto timesharing;
    }

    /* Check if we are going idle, and if so, for how long */
    if(CKisHeapEmpty(ht_timesharing)) {
idle:
        /*
         * We have a look what idle tasks we have.  If we have any,
         * we run the first on the idle heap, otherwise we pick the
         * CK idle loop.  We also look whether we'll be preempted
         * by an RT task anytime soon, and if so, we adjust the quantum.
         * XXX - take fudge factor into account here so RT task is run on time
         */
 
        if(!CKisHeapEmpty(ht_realtime)) {
            *period=CKgetNextTime(ht_realtime)-realtime;
        } else if(!CKisHeapEmpty(ht_idle)) {
            *period=CKgetNextTime(ht_idle);
        } else {
            *period=idleLoop.quantum;
        }

        if(!CKisHeapEmpty(ht_idle)) {
            return CKgetNextEvent(ht_idle);   /* idle, run idle events  */
        } else {
            return &idleLoop;                 /* idle loop              */
        }
    }

    /*
     * Plain ol' timesharing task:  at last resort, when we have nothing
     * better to do (ie., run a realtime task or a *really* wild idea like
     * that.)
     */

timesharing:
    *period=CKgetNextQuantum(ht_timesharing);  /* 'At last resort': run */
    return CKgetNextEvent(ht_timesharing);     /* the next TS task      */
}


PUBLIC DATA CKschedule(VOID)
/*
 * Main scheduler function, gets run on each timer interrupt
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     CKschedule():  0 if no signals pending, else pending signal
 */
{
    TIME period;

    CKflushPrevTask();               /* Flush task down correct heap    */

    currtask=CKgetNextTask(&period); /* Get next task to run            */
    lasttime=currtask->time;         /* Remember time for RT check      */
    currtask->realtime = realtime;   /* Pass realtime clock phase       */

    CKsetTimerPeriod(period);        /* Program timer for next quantum  */

    CKcontextSwitch(currtask);       /* Switch to new thread            */

    /*
     * When we get to this point, we have just started a new quantum.
     * The time is ripe to check for signals, and if some are pending,
     * handle them.
     *
     * We use the following optimisation:  we do not expect signals most
     * of the time.  In order to cut down the overhead, we first check
     * whether there are no signals pending, and if so, return to the
     * user code.  This way, we get a straight-line code execution path in
     * the usual case, which is nice and easy on the prefetch queue, and
     * therefore fast.
     *
     * We don't have to worry about variables cached in registers by EGCS,
     * because CKcontextSwitch() makes sure EGCS reloads them when needed.
     * This means that currtask points to the task that is currently
     * running, not neccessarily to the value we assigned to it above.
     *
     * From this point on, the interrupts are off (CKcontextSwitch()
     * leaves them off for us.)  This keeps an IRQ from sending us a
     * high-priority signal while we're trying to process the previous
     * one.
     */

    /* Check for signals */
    if(!currtask->signals) return 0;

    /* Check whether we are busy */
    if(currtask->signals->signal & SFBUSY || currtask->interruptable)
        return 0;

    /* Handle the signal */
    currtask->signals->signal |= SFBUSY;
    return 1;

    /*
     * We now return to the stub, which will check for the return code,
     * and if set, make sure the signal is handled correctly.
     */
}


/*
 * Now we get all the functions related to scheduling that aren't used
 * by the scheduler itself.
 */

PUBLIC VOID CKinitSched(VOID)
/*
 * Initialise the scheduling subsystem
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKsetupHeaps();                /* Set up the priority heaps         */

    currtask=&idleLoop;            /* We're coming from the idle loop   */
    realtime=0;                    /* The time is 0                     */
    lasttime=0;
}
