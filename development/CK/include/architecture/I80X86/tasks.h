/*
 * tasks.h:  Hardware-dependant inlines
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 25/11/98  ramon       1.2    Updated context switch function
 * 01/01/99  ramon       1.3    Context switching now supports FPU
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

#ifndef __SYS_TASKS_H
#define __SYS_TASKS_H

#include <typewrappers.h>
#include "sys/8254.h"
#include "sys/8259.h"
#include "sys/gdt.h"
#include "sys/taskstate.h"
#include "sys/ioport.h"
#include "sys/sysdefs.h"
#include "ckobjects.h"
#include "sched.h"
#include "mutex.h"

extern union DTEntry GDT[];
extern EVENT *lastFPUUser;

PUBLIC CKMutex schedLock;

LOCAL inline VOID CKcontextSwitch(EVENT *event)
/*
 * Perform a context switch using a single TSS descriptor in the GDT
 *
 * INPUT:
 *     event:  Event structure of task to switch to
 *
 * RETURNS:
 *     none
 */
{
    /* Change TSS descriptor in gdt */
    GDT[TSSDSC].desc.base_low  = ((UWORD32)&event->taskState) & 0xffff;
    GDT[TSSDSC].desc.base_med  = ((UWORD32)&event->taskState>>16)&0xff;
    GDT[TSSDSC].desc.base_high = ((UWORD32)&event->taskState>>24)&0xff;

    /* GDT[TSSDSC].desc.access = (D_TSS+D_PRESENT) >> 8;               */

    asm (
        "cli               \n"  /* Avoid race condition on schedLock   */
        "movb $0,schedLock \n"  /* Free up the scheduler               */
        "ljmp %0,$0x0      \n"  /* Do the context switch               */
        :
        : "p" (TSSSEL)          /* The TSS selector                    */
        : "memory"              /* Keep GCC from caching currtask etc. */
    );                          /* They are bound to have changed when */
                                /* we return here, and need reloading  */

    /*
     * Now we check whether the FPU context is valid.  If not,
     * we keep TS set so an int $7 will be generated on the first
     * FPU opcode for this quantum.
     */

    asm (
        "cmpl %0,%1     \n"     /* Is the FPU context valid ?          */
        "jne 1f         \n"     /* No, keep TS set so int $7 will be   */
        "clts           \n"     /* invoked on the first FPU operation  */
        "1:             \n"
        :
        : "r" (currtask), "g" (lastFPUUser)
    );
}


LOCAL inline VOID CKsetTimerPeriod(TIME quantum)
/*
 * Load a value into 8254 counter channel 0
 *
 * INPUT:
 *     quantum:  Value to load into the counter register
 *
 * RETURNS:
 *     none
 */
{
    /* Check for upper bound quantum */
    if((UDATA) quantum > 0xffff)
        quantum = 0xffff;

    /* Program period for 8254 */
    outportb(TMR_CNT0, quantum&0xff);
    outportb(TMR_CNT0, quantum>>8);
}


LOCAL inline TIME CKgetTimerValue(VOID)
/*
 * Get the contents of 8254 counter channel 1
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     CKgetTimerValue():  contents of the channel 1 timer
 */
{
    TIME t_val;

    outportb(TMR_CTRL, TMR_READ-TMR_CNT+TMR_CH1); /* Read 8254 channel 1*/
    t_val  = inportb(TMR_CNT1);                   /* Get timer value    */
    t_val |= (inportb(TMR_CNT1)<<8);

    return t_val;
}


LOCAL inline VOID CKupdateRealtime(VOID)
/*
 * Update the realtime clock count using the hardware timer
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    TIME t_value;

    t_value = CKgetTimerValue();               /* Read 8254 channel 1   */
    t_value = -(t_value+realtime)&0xffff;      /* Calculate ticks since */
                                               /* last schedule         */

    realtime+=t_value;                         /* increment realtime    */
                                               /* with ticks since last */
                                               /* schedule              */
}


LOCAL inline TIME CKgetRealtime(VOID)
/*
 * Get the real time without updating the realtime count
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    TIME t_value;

    outportb(TMR_CTRL, TMR_LATCH+TMR_SC1);
    t_value  = inportb(TMR_CNT1);
    t_value |= (inportb(TMR_CNT1)<<8);
    t_value  = -(t_value + realtime) & 0xffff;

    return (realtime + t_value);
}

#endif
