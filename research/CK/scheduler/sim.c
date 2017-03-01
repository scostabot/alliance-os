/*
 * sim.c:  The scheduler simulation main program and simulated tasks
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 08/03/99  ramon       1.0    First release
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


#include <stdio.h>
#include "ckobjects.h"
#include "sched.h"
#include "pri_heap.h"

#define TS  4
#define RT  2

#define RTLEN   0x50
#define RTFREQ  0x30000

EVENT idleLoop;
EVENT evt[TS+RT];

extern TIME nexttime;
extern DATA *exctime;
extern void (*nexttask)(TIME finish, DATA *timevar, TIME next);

void TSTask(TIME finish, DATA *, TIME);
void RTTask(TIME finish, DATA *, TIME);
void processor(void);

void main(int argc, char*argv[]) 
{
    int i;

    /* Initialise the scheduler data structures */
    CKinitSched();

    printf("Simulated threads:\n");
    printf("Type\tThread\tQuantum\tDefer\n");

    /* Set up the task set */
    for(i=0;i<TS;i++) {
        evt[i].taskState = TSTask;
        evt[i].quantum = 0x2000;
        evt[i].defer = 0x1000 + i*0x1000;
        if(CKisHeapEmpty(ht_timesharing))
            evt[i].time = 0;
        else
            evt[i].time = CKgetNextTime(ht_timesharing);
        evt[i].ts = ts_Running;
        CKsiftup(ht_timesharing, &evt[i]);
        printf("TS\t%d\t%p\t%p\n",i,0x2000,0x1000 + i*0x1000);
    }

    for(i=TS;i<TS+RT;i++) {
        evt[i].taskState = RTTask;
        evt[i].quantum = 0x10000;
        evt[i].defer = 0x1000;
        if(CKisHeapEmpty(ht_timesharing))
            evt[i].time = 0;
        else
            evt[i].time = CKgetNextTime(ht_timesharing);
        evt[i].ts = ts_Running;
        CKsiftup(ht_timesharing, &evt[i]);
        printf("RT\t%d\t%p\t%p\n",i-TS,0x10000,0x1000);
    }

    /* Set up idle loop */
    idleLoop.quantum   = 0x1000;
    idleLoop.taskState = TSTask;

    printf("\nRT tasks run periodically every %p clockticks\n\n",RTFREQ);

    /* Start up all processors */
    processor();
}


void processor(void)
{
    long t;

    /* The main processor loop */
    for(;;) {
        CKschedule();                         /* Schedule the next task */

        /* Print statistics */
        printf("Processor %d:  Running ", 0);
        if((t = ((long)exctime-(long)&evt[0].time)/sizeof(EVENT)) >= TS+RT)
            printf("idle        thread at time %p\t", realtime);
        else if(t >= TS)
            printf("realtime    thread %d at time %p\t", t-TS, realtime);
        else
            printf("timesharing thread %d at time %p\t", t, realtime);

        switch(evt[t].ts) {
            case ts_Running:
                printf("(TS mode)\n");
                break;
            case ts_RealTime:
                printf("(RT mode)\n");
                break;
            case ts_Idle:
                printf("(Idle mode)\n");
                break;
        }

        /*
         * For now, all RT tasks execute periodically every RTFREQ
         * clockticks.  Should probably change this later.
         */

        nexttask(nexttime, exctime, RTFREQ);  /* Run the next task      */
    }
}


void TSTask(TIME finish, DATA *timevar, TIME next)
{
    TIME now;

    for(;;) {
        CLOCK(now)
        if(now-finish >= 0) return;
    }
}

void RTTask(TIME finish, DATA *timevar, TIME next)
{
    TIME now, start;

    *timevar += next;

    CLOCK(start)

    for(;;) {
        CLOCK(now)
        if(now-finish >= 0) return;
        if(now-start >= RTLEN) return;
    }
}
