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
 * 08/03/99  ramon       1.4    Completely redone using Limes
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
#include "sys/sysdefs.h"
#include "ckobjects.h"
#include "sched.h"


TIME nexttime;   /* Time until which next task executes */
DATA *exctime;
void (*nexttask)(TIME finish, DATA *timevar, TIME next);


LOCAL inline VOID CKcontextSwitch(EVENT *event)
/*
 * Perform a context switch
 *
 * INPUT:
 *     event:  Event structure of task to switch to
 *
 * RETURNS:
 *     none
 */
{
    exctime  = &event->time;
    nexttask = event->taskState;
}


LOCAL inline VOID CKsetTimerPeriod(TIME quantum)
/*
 * Set the quantum of the next task
 *
 * INPUT:
 *     quantum:  Value to load into the counter register
 *
 * RETURNS:
 *     none
 */
{
    nexttime = realtime + quantum;
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

    CLOCK(t_value)
    realtime = t_value;
}

#endif
