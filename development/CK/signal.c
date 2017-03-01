/*
 * signal.c:  Code for signal handling
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 16/01/99  ramon       1.0    First release
 * 29/01/99  ramon       1.1    Unblock target thread on signal
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
#include "ckobjects.h"
#include "ckerror.h"
#include "sched.h"
#include "memory.h"


PUBLIC DATA CKsendSignal(EVENT *thread, UDATA signal, UDATA extra, UDATA priority)
/*
 * Send a signal to a kernel running in a specific thread
 *
 * INPUT:
 *     thread:    Thread to signal
 *     signal:    Signal to send
 *     extra:     Signal parameter
 *     priority:  Signal priority (0 is highest)
 *
 * RETURNS:
 *     CKsendSignal():  0 on sucess, otherwise error
 */
{
    SIGNAL *pos;
    SIGNAL *newsig = CKmemAlloc(sizeof(SIGNAL));
    if(!newsig) return ENOMEM;

    /* Fill in the signal structure */
    newsig->sender   = (DESCRIPTOR) currtask; /* Sending thread desc.  */
    newsig->signal   = signal & ~SFBUSY;  /* Not being handled yet     */
    newsig->extra    = extra;             /* Parameter data            */
    newsig->priority = priority;          /* Signal priority           */

    if(!thread->signals) {                /* No pending signals ?      */
        newsig->next    = NIL;            /* Hey, we're the first      */
        thread->signals = newsig;         /* signal, life is easy      */
        if(thread->ts == ts_Blocked)      /* If neccessary, unblock    */
            CKunblockThread(thread);      /* the signalled thread      */
        return 0;                         /* Success !                 */
    }

    /* Check whether we preempt the current signal, and if so set it   */
    if(thread->signals->priority > priority) {
        newsig->next    = thread->signals;
        thread->signals = newsig;
        if(thread->ts == ts_Blocked)
            CKunblockThread(thread);
        return 0;
    }

    /* More signals pending... find the right pos. in the signal queue */
    for(
        pos = thread->signals;
        pos->next != NIL && (pos->next->priority <= priority);
        pos = pos->next
    );

    /* Insert us into the signal queue */
    newsig->next    = pos->next;
    pos->next       = newsig;

    /* Unblock the signalled thread */
    if(thread->ts == ts_Blocked)
        CKunblockThread(thread);

    return 0;   /* Success !!!  */
}
