/*
 * cachethread.c:  Initialise the thread obj for a new thread, and cache it
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 28/11/98  ramon       1.2    Updated to use object caching
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
#include "sys/taskstate.h"
#include "ckobjects.h"
#include "cksyscalls.h"

PUBLIC DATA cacheThread(VOID *entry, UBYTE *stack, TIME quantum,
                        TIME defer, ThreadObject *eventblk, DESCRIPTOR addrspc,
                        DESCRIPTOR kernel, UBYTE *kstack)
/*
 * Fill in the thread object for a new thread, and cache it
 *
 * INPUT:
 *     entry:     Address of the thread's entry point
 *     stack:     The thread's stack
 *     quantum:   The thread's quantum
 *     defer:     The thread's defer
 *     eventblk:  Address of memory allocated to the thread's event block
 *     addrspc:   Descriptor to the thread's address space
 *
 * RETURNS:
 *     none
 */
{
    DATA error;

    eventblk->quantum        = quantum;     /* Time quantum          */
    eventblk->defer          = defer;       /* Defer                 */

    eventblk->userState.eip  = (UWORD32)entry; /* Entry point        */
    eventblk->userState.esp  = (UWORD32)stack; /* Application stack  */

    eventblk->userState.cs   = USERCODE;    /* Selectors:  Code      */
    eventblk->userState.ss   = USERDATA;    /* Stack                 */
    eventblk->userState.ds   = USERDATA;    /* Data                  */
    eventblk->userState.es   = USERDATA;    /* Data                  */
    eventblk->userState.fs   = USERDATA;    /* Data                  */
    eventblk->userState.gs   = USERDATA;    /* Data                  */

    eventblk->userState.ldt  = 0;           /* LDT for this task NYI */
    eventblk->userState.trap = 0;           /* Debug flag, is off    */
    eventblk->userState.io   = 0;           /* No I/O maps for now   */

    eventblk->userState.esp1 = (UWORD32)kstack;  /* Stack for parent */
    eventblk->userState.ss1  = EKDATA;      /* kernel of this thread */

    eventblk->userState.esp2 = 0;           /* These are unused      */
    eventblk->userState.ss2  = 0;

    eventblk->addressSpace   = addrspc;     /* The address space     */
    eventblk->parentKernel   = kernel;      /* The parent kernel     */

    /* Now we cache the thread into the CK */
    if((error = CKcacheObject((ADDR)eventblk,SIGTHREAD,sizeof(ThreadObject),0)))
        return error;

    /* ... and we make it running */
    CKsetThreadState(eventblk->self,ts_Running);

    return 0;  /* success */
}
