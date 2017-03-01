/*
 * sched.c:  Main scheduler code
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
 * 11/03/99  ramon       1.7    Added fudge calibration
 * 12/03/99  ramon       1.8    Several bugfixes to the main scheduler code
 * 15/05/99  jens        1.9    Removed kludge now seems to work without it
 * 07/09/99  jens        1.10   Added CKsetThreadState and CKthreadtunnel
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
 * Note:  The original idea for this type of scheduler comes from 
 *        John Fine <johnfine@erols.com> who implemented it as an
 *        assembler example.  This is a complete reimplementation
 *        of this kind of scheduler, combining John's timesharing
 *        scheduler design as well as John's realtime scheduler
 *        design, together with some of our own extensions.
 *        I'd like to thank John for his help here.      -- Ramon
 */

#include <typewrappers.h>
#include "sys/sys.h"
#include "sys/sysdefs.h"
#include "sys/tasks.h"
#include "sys/arith.h"
#include "sys/hwkernel.h"
#include "sys/realtime.h"
#include "ckobjects.h"
#include "ckerror.h"
#include "pri_heap.h"
#include "memory.h"


/* currtask is the task that has been running just before the           */
/* scheduler was triggered (by the timer.)                              */

PUBLIC EVENT *currtask = NIL;
PUBLIC EVENT zombietask;


/* last_time contains the execution time of the thread that has just    */
/* been running.  We use this to determine whether the thread was an    */
/* RT thread                                                            */

LOCAL TIME lasttime;


/* The realtime clock count */

PUBLIC TIME realtime;


/* The scheduler fudge factor:  used for high-precision timing */

LOCAL TIME fudge;


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
         */

        if(CLSE(CKgetNextTime(ht_realtime)-fudge,realtime)) {
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
         */

        if(CKisHeapEmpty(ht_timesharing)) goto idle;

        if(CLSE(CKgetNextTime(ht_realtime)-fudge,CKgetNextQuantum(ht_timesharing)+realtime)) {
            *period=CKgetNextTime(ht_realtime)-realtime-fudge;
            return CKgetNextEvent(ht_timesharing); /* make time for RT  */
        }

        /*
         * Plain ol' timesharing task, no big deal
         */

        goto timesharing;
    }

    /* Check if we are going idle, and if so, for how long */
    if(CKisHeapEmpty(ht_timesharing)) {
        /*
         * We have a look what idle tasks we have.  If we have any,
         * we run the first on the idle heap, otherwise we pick the
         * CK idle loop.  We also look whether we'll be preempted
         * by an RT task anytime soon, and if so, we adjust the quantum.
         */
 
        if(!CKisHeapEmpty(ht_realtime)) {
idle:
            if((CKisHeapEmpty(ht_idle) && CLSE(CKgetNextTime(ht_realtime)-fudge,idleLoop.quantum+realtime)) || 
              (!CKisHeapEmpty(ht_idle) && CLSE(CKgetNextTime(ht_realtime)-fudge,CKgetNextQuantum(ht_idle)+realtime))) {
                *period=CKgetNextTime(ht_realtime)-realtime-fudge;
            } else if(!CKisHeapEmpty(ht_idle)) {
                *period=CKgetNextQuantum(ht_idle);
            } else {
                *period=idleLoop.quantum;
            }
        } else if(!CKisHeapEmpty(ht_idle)) {
            *period=CKgetNextQuantum(ht_idle);
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
    ThreadObject *usertask = (ThreadObject *) currtask->head.objectData;
    TIME period;

    currtask->time = usertask->time; /* Get the next execution time     */
    CKflushPrevTask();               /* Flush task down correct heap    */
    currtask=CKgetNextTask(&period); /* Get next task to run            */
    lasttime = currtask->time;       /* Remember time for RT check      */

    CKsetTimerPeriod(period);        /* Program timer for next quantum  */
    CKcontextSwitch(currtask);       /* Switch to new thread            */

    /*
     * When we get to this point, we have just started a new quantum.
     * The time is ripe to check for signals, and if some are pending,
     * handle them.
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

    usertask->time = currtask->time; /* Set time in user structure      */
    usertask->realtime = realtime;   /* Pass realtime clock phase       */

    /* Check for signals */
    if(!currtask->signals) return 0;

    /* Check whether we are busy */
    if(currtask->signals->signal & SFBUSY || usertask->interruptable)
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
 * Now we get the functions that are used to initialise the scheduler.
 * This includes calibrating the scheduler fudge.
 */

#define CALPERIOD 0x1000

LOCAL VOID CKfudgeCalibThread(VOID);

LOCAL volatile enum { COARSE, FINE } calstage;
LOCAL volatile TIME tactual, texpect, calbit, bfudge = 0;
LOCAL volatile DATA synch;
LOCAL EVENT *calthread;
LOCAL ThreadObject *caluthread;

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
    BYTE *ctstack;

    CKsetupHeaps();                /* Set up the priority heaps         */

    currtask = &idleLoop;          /* We're coming from the idle loop   */
    realtime = 0;                  /* The time is 0                     */
    lasttime = 0;

    /* Setup zombie task structure */
    zombietask.head.objectSignature = SIGTHREAD;
    zombietask.head.objectData      = (ADDR) &zombietask;
    zombietask.head.flags           = 0;
    zombietask.ts                   = ts_Blocked;

    CKhwInitSched();               /* Get the hardware going            */

    /*
     * Now we will calibrate the scheduler fudge.  The scheduler fudge
     * is a machine-dependant constant that is used by the scheduler
     * in order to allow high-precision realtime scheduling.  If the fudge
     * were not taken into account, realtime threads would always gain
     * control 'just' too late because part of the scheduler runs inside
     * the quantum of the realtime thread.  As we want high-precision
     * timing at about microsecond resolution, we need to have the
     * scheduler be invoked 'just' too early.  We do this by substracting
     * the fudge from the interruption time.  We need to calibrate the
     * fudge first because it differs per machine.
     * We do this with a binary approximation using a method similar to
     * the one used by linux to calibrate the BogoMIPS loop.  We have one
     * special realtime thread running for calibration, which determines
     * the time it gains control, and compares it to the time it should
     * have. We keep shifting the fudge to the left until it is too big
     * (the RT task was invoked too early), after which we shift it to the
     * right once and start adding lower powers of two, using the same
     * technique to check for fudge overflow.  This way we can get a
     * good value for the fudge without spending too much time on it on
     * slower computers (on fast computers, fudge is very small so this
     * way of calculating it does not give any real speed benefits.)
     *
     * Note that the invocation time is a sort of deadline:  if we have a
     * choice between having the thread be invoked just a tiny bit early
     * and just a tiny bit late, we prefer having it run early.
     */

    ctstack     = CKmemAlloc(1024);          /* 1024-byte stacklet      */
    calthread   = CKmemAlloc(sizeof(EVENT)); /* Allocate thread struct  */
    caluthread  = CKmemAlloc(sizeof(ThreadObject));

    CKcacheKernelThread(CKfudgeCalibThread,  /* Setup the calibration   */
                        ctstack+1024,        /* thread and load it into */
                        calthread,           /* the scheduler           */
                        caluthread);

    fudge    = 1;                  /* Initialise the fudge              */
    synch    = 0;                  /* Initialise the synch              */
    calstage = COARSE;             /* Set the coarse calibration stage  */

    CKunlock();  quantumDone();    /* Pass control to the calib thread  */

    do {                           /* Stage 1:  Coarse calibration      */
        texpect = calthread->time; /* Cache expected invocation time    */
        synch = 0;                 /* Zero synch so we can wait on cal  */
        while(!synch);             /* Wait until the cal thread has run */
    } while(CLESS(texpect,tactual));  /* If we're not overdue, break    */

    if(texpect == tactual) {       /* Check whether we perhaps already  */
        fudge >>= 1;               /* have the correct fudge.  If so,   */
        goto fudgeok;              /* we just skip the fine calibration */
    }

    fudge >>= 2;                   /* Make fudge 'just' too little      */
    calbit   = fudge;              /* Initialise the bit to calibrate   */
    calstage = FINE;               /* Set the fine calibration stage    */

    synch = 0;                     /* Reinitialise the synch            */
    while(!synch);                 /* Synchronise with the calib thread */

    for(;;) {                      /* Stage 2:  Fine calibration        */
        calbit >>= 1;              /* Next bit to calibrate             */
        if(!calbit) break;         /* If we have done all bits, stop    */

        texpect = calthread->time; /* Cache expected invocation time    */
        synch = 0;                 /* Zero synch so we can wait on cal  */
        while(!synch);             /* Wait until the cal thread has run */
    }

    if(CLESS(texpect,tactual) && bfudge) {    /* If we are overdue, the */
        fudge = bfudge;            /* fudge is too small. Revert to the */
    }                              /* cached fudge value                */

fudgeok:                           /* We're done !  cleanup             */
    CKlock();
    CKremoveEvent(ht_realtime, calthread->heap);
    CKmemFree(ctstack,1024);
    CKmemFree(calthread,sizeof(EVENT));
    CKmemFree(caluthread,sizeof(ThreadObject));
    CKunlock();

    quantumDone();                 /* Get the scheduler kickstarted     */
}

LOCAL VOID CKfudgeCalibThread(VOID)
/*
 * Thread used to calibrate the scheduler fudge
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    for(;;) {
        tactual = CKgetRealtime();           /* Get the invocation time */
        caluthread->time=tactual+CALPERIOD;  /* Set the next deadline   */
        synch++;                             /* Sync with main routine  */

        if(calstage == COARSE) {     /* If this is coarse calibration   */
            fudge <<= 1;             /* try the next power-2 fudge;     */
        } else {                     /* otherwise (fine calibration)    */
            fudge |= calbit;         /* set the calibration bit and try */
            if(!CLESS(texpect,tactual)) {     /* to get a better approx */
                bfudge = fudge;      /* If we're not overdue anymore    */
                fudge &= ~calbit;    /* turn bit back off and try to    */
            }                        /* get a better approx. Keep a     */
        }                            /* backup for if we don't find one */
        quantumDone();               /* We're go until next quantum     */
    }
}


/*
 * Now we get all the functions related to scheduling that aren't used
 * by the scheduler itself.
 */

PUBLIC VOID CKblockThread(EVENT *thread)
/*
 * Blocks a thread
 *
 * INPUT:
 *     thread: Thread to block
 *
 * RETURNS:
 *     none
 */
{
    /*
     * TODO:  Security checking on thread->heap, that that position in the
     *        heap really points to the thread.  Also in objmgmt.c
     */

    switch(thread->ts) {
        case ts_Running:
            CKlock();
            CKremoveEvent(ht_timesharing, thread->heap);
            thread->ts = ts_Blocked;
            CKunlock();
            break;
        case ts_RealTime:
            CKlock();
            CKremoveEvent(ht_realtime, thread->heap);
            thread->ts = ts_Blocked;
            CKunlock();
            break;
        case ts_Idle:
            CKlock();
            CKremoveEvent(ht_idle, thread->heap);
            thread->ts = ts_Blocked;
            CKunlock();
            break;
        case ts_Blocked:
        default:
            break;
    }
}


PUBLIC VOID CKunblockThread(EVENT *thread)
/*
 * Unblocks a thread
 *
 * INPUT:
 *     thread: Thread to unblock
 *
 * RETURNS:
 *     none
 */
{
    /*
     * TODO:  Security checking on thread->heap, that that position in the
     *        heap really points to the thread.  Also in objmgmt.c
     */

    switch(thread->ts) {
        case ts_Blocked:
            if(CKisHeapEmpty(ht_timesharing))
                thread->time = 0;
            else
                thread->time = CKgetNextTime(ht_timesharing);
            thread->ts = ts_Running;
            CKlock();
            CKsiftup(ht_timesharing, thread);
            CKunlock();
            break;
        case ts_Running:
        case ts_RealTime:
        case ts_Idle:
        default:
            break;
    }
}


PUBLIC VOID CKidleThread(EVENT *thread)
/*
 * Makes a blocked thread run on idle
 *
 * INPUT:
 *     thread: Thread to idle
 *
 * RETURNS:
 *     none
 */
{
    /*
     * TODO:  Security checking on thread->heap, that that position in the
     *        heap really points to the thread.  Also in objmgmt.c
     */

    switch(thread->ts) {
        case ts_Blocked:
            if(CKisHeapEmpty(ht_idle))
                thread->time = 0;
            else
                thread->time = CKgetNextTime(ht_idle);
            thread->ts = ts_Idle;
            CKlock();
            CKsiftup(ht_idle, thread);
            CKunlock();
            break;
        case ts_Running:
        case ts_RealTime:
        case ts_Idle:
        default:
            break;
    }
}

PUBLIC DATA inline CKthreadTunnelKernelCode(DESCRIPTOR object,TunnelType type)
/*
 * Surrender the remaining quantum to another thread
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object: Descriptor of thread object to change state of
 *     type: Type wanted
 *
 * RETURNS:
 *     CKthreadTunnel(): 0 for success, or otherwise error code
 */
{
    EVENT *thread         = (EVENT *) object;

    if(!CKvalidThread(object)) return ENOVALIDTHREAD;
    switch(type) {
        case tt_Conservative:
            /* Conservative : just context switch to new thread
             * Ramon : should'nt currtask or usertask be marked to the new thread?
             * 50% Working, is only able to switch first time, stack problems, needs to figure it out.
             * Maybe this is ok?
             */
            if(thread==currtask) {
	            CKcontextSwitch(thread);
            } else {
            	quantumDone();
            }
            break;
        case tt_Swapping:
            /* Swapping : make new thread running, and block the current task
             * Tested : Working
             */

            CKlock();
            switch(currtask->ts) {
                case ts_Running:
                    CKremoveEvent(ht_timesharing, currtask->heap);
                    break;
                case ts_RealTime:
                    CKremoveEvent(ht_realtime, currtask->heap);
                    break;
                case ts_Idle:
                    CKremoveEvent(ht_idle, currtask->heap);
                    break;
                case ts_Blocked:
                default:
                    break;
            }
            currtask->ts = ts_Blocked;

            switch(thread->ts) {
                case ts_Running:
                    break;
                case ts_RealTime:
                    CKremoveEvent(ht_realtime, thread->heap);
                    goto ts_siftup;
                case ts_Idle:
                    CKremoveEvent(ht_idle, thread->heap);
                case ts_Blocked:
ts_siftup:          if(CKisHeapEmpty(ht_timesharing)) {
                        thread->time = 0;
                    } else {
                        thread->time = CKgetNextTime(ht_timesharing);
					}
                    thread->ts = ts_Running;
                    CKsiftup(ht_timesharing, thread);
                    break;
            }
            CKunlock();
            CKschedule(); //change to new thread
            break;
        case tt_Unblocking:
            /* Unblocking : make new thread running and switch to it */
            /* Tested : Working */
            CKunblockThread(thread);
            CKschedule(); //change to new thread
            break;
        default:
            return EINVALIDPARAMS;
    }
    return 0;
}

PUBLIC DATA inline CKsetThreadStateKernelCode(DESCRIPTOR object,ThreadState state)
/*
 * Changes the thread state of a thread
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object: Descriptor of thread object to change state of
 *     state: State wanted
 *
 * RETURNS:
 *     CKsetThreadState(): 0 for success, or otherwise error code
 */
{
    EVENT *thread         = (EVENT *) object;
    ThreadObject *uthread = (ThreadObject *) thread->head.objectData;
    DATA error            = 0;

    if(!CKvalidThread(object)) return ENOVALIDTHREAD;

    CKlock();

    switch(state) {
        case ts_Running:
            switch(thread->ts) {
                case ts_Running:
                    break;
                case ts_RealTime:
                    CKremoveEvent(ht_realtime, thread->heap);
                    goto ts_siftup;
                case ts_Idle:
                    CKremoveEvent(ht_idle, thread->heap);
                case ts_Blocked:
ts_siftup:          if(CKisHeapEmpty(ht_timesharing)) {
                        thread->time = 0;
						uthread->time = 0;
                    } else {
                        thread->time = CKgetNextTime(ht_timesharing);
                        uthread->time = CKgetNextTime(ht_timesharing);
					}
                    thread->ts = ts_Running;
                    CKsiftup(ht_timesharing, thread);
                    break;
            }
            break;
        case ts_Blocked:
            switch(thread->ts) {
                case ts_Running:
                    CKremoveEvent(ht_timesharing, thread->heap);
                    thread->ts  = ts_Blocked;
                    break;
                case ts_RealTime:
                    CKremoveEvent(ht_realtime, thread->heap);
                    thread->ts  = ts_Blocked;
                    break;
                case ts_Blocked:
                    break;
                case ts_Idle:
                    CKremoveEvent(ht_idle, thread->heap);
                    thread->ts  = ts_Blocked;
                    break;
            }
            break;
        case ts_Idle:
            switch(thread->ts) {
                case ts_Running:
                    CKremoveEvent(ht_timesharing, thread->heap);
                    goto idle_siftup;
                case ts_RealTime:
                    CKremoveEvent(ht_realtime, thread->heap);
                case ts_Blocked:
idle_siftup:        if(CKisHeapEmpty(ht_idle)) {
                        thread->time = 0;
						uthread->time = 0;
                    } else {
                        thread->time = CKgetNextTime(ht_idle);
                        uthread->time = CKgetNextTime(ht_idle);
					}
                    thread->ts = ts_Idle;
                    CKsiftup(ht_idle, thread);
                    break;
                case ts_Idle:
                    break;
            }
            break;
        default:
            error = EINVALIDPARAMS;
            break;

    }
	CKunlock();
    return error;
}

/* This functions is unused
PUBLIC DATA inline CKswitchThreadStateKernelCode(DESCRIPTOR object)
 */
/*
 * Switches thread state between blocked and timesharing
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object: Descriptor of thread object to change state of
 *
 * RETURNS:
 *     CKswitchThreadState(): 0 for success, or otherwise error code
 */
/*{
    EVENT *thread         = (EVENT *) object;
    ThreadObject *uthread = (ThreadObject *) thread->head.objectData;
    DATA error            = 0;

    if(!CKvalidThread(object)) return ENOVALIDTHREAD;

    CKlock();
*/
    /*
     * TODO:  Security checking on thread->heap, that that position in the
     *        heap really points to the thread.  Also in objmgmt.c
     */
/*
    switch(thread->ts) {
        case ts_Running:
            CKremoveEvent(ht_timesharing, thread->heap);
            thread->ts  = ts_Blocked;
            break;
        case ts_RealTime:
            CKremoveEvent(ht_realtime, thread->heap);
            thread->ts  = ts_Blocked;
            break;
        case ts_Blocked:
            if(CKisHeapEmpty(ht_timesharing)) {
                thread->time  = 0;
                uthread->time = 0;
            } else {
                thread->time  = CKgetNextTime(ht_timesharing);
                uthread->time = CKgetNextTime(ht_timesharing);
            }
            thread->ts = ts_Running;
            CKsiftup(ht_timesharing, thread);
            break;
        case ts_Idle:
        default:
            error = ENOVALIDTHREAD;
            break;
    }

    CKunlock();

    return error;
}
*/
/**************************************************************************/
/* Scheduler Calls (platform dependant)                                   */
/* this function is the same as above, "Code" is just change to "Stub",   */
/* and some platform dependant stuff is done.                             */
/* ---------------------------------------------------------------------- */
/* Include Need to be here, CKswitchThreadStateKernelCode is inline       */
/* expanded in it, can't be linked, won't produce the same result,        */
/* faster code.                                                           */
/**************************************************************************/

#include "sys/stubs/sched.h"
