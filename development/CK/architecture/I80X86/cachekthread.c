/*
 * cacheinitthread.c:  CK code to cache an initial thread
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
#include "sys/memory.h"
#include "ckobjects.h"
#include "ckkernelcalls.h"
#include "pri_heap.h"


PUBLIC DATA CKcacheUserThread(ADDR entry, ADDR stack,
                              ThreadObject *eventblk, DESCRIPTOR addrspc)
/*
 * Fill in the thread object for a new thread, and cache it
 *
 * INPUT:
 *     entry:     Address of the thread's entry point
 *     stack:     The thread's stack
 *     eventblk:  Address of memory allocated to the thread's event block
 *     addrspc:   Descriptor to the thread's address space
 *
 * RETURNS:
 *     none
 */
{
    CKThreadObject *thread;
    DATA error;

    eventblk->quantum        = 0x1000;      /* Time quantum          */
    eventblk->defer          = 0x2000;      /* Defer                 */

    eventblk->userState.eip  = entry;       /* Entry point           */
    eventblk->userState.esp  = stack;       /* Application stack     */

    /* For now:  everything DPL3 */

    eventblk->userState.cs   = USERCODE;    /* Selectors:  Code      */
    eventblk->userState.ss   = USERDATA;    /* Stack                 */
    eventblk->userState.ds   = USERDATA;    /* Data                  */
    eventblk->userState.es   = USERDATA;    /* Data                  */
    eventblk->userState.fs   = USERDATA;    /* Data                  */
    eventblk->userState.gs   = USERDATA;    /* Data                  */

    eventblk->userState.ldt  = 0;           /* LDT for this task NYI */
    eventblk->userState.trap = 0;           /* Debug flag, is off    */
    eventblk->userState.io   = 0;           /* No I/O maps for now   */

    eventblk->userState.esp1 = 0;           /* Stack data for parent */
    eventblk->userState.esp2 = 0;           /* Kernel of this thread */
    eventblk->userState.ss1  = 0;           /* NYI                   */
    eventblk->userState.ss2  = 0;

    eventblk->addressSpace   = addrspc;     /* The address space     */
    eventblk->parentKernel   = NIL;         /* No parent kernel      */

    /* Now we cache the thread into the CK */
    if((error = CKcacheObjectKernelCode((ADDR)eventblk,SIGTHREAD,sizeof(ThreadObject),0)))
        return error;

    /* ... and we make it running */
    thread = (CKThreadObject *) eventblk->self;
    if(CKisHeapEmpty(ht_timesharing)) {
        thread->time   = 0;
        eventblk->time = 0;
    } else {
        thread->time   = CKgetNextTime(ht_timesharing);
        eventblk->time = CKgetNextTime(ht_timesharing);
    }
    thread->ts = ts_Running;
    CKsiftup(ht_timesharing, thread);
 
    return 0;  /* success */
}


extern struct AddressSpace kernelAddrSpc;

PUBLIC DATA CKcacheKernelThread(VOID *entry, UBYTE *stack, EVENT *eventblk, ThreadObject *thread)
/*
 * Fill in the thread object for a new thread, and cache it
 *
 * INPUT:
 *     entry:     Address of the thread's entry point
 *     stack:     The thread's stack
 *     eventblk:  Address of memory allocated to the thread's event block
 *
 * RETURNS:
 *     none
 */
{
    eventblk->quantum        = 0x1000;     /* Time quantum          */
    eventblk->defer          = 0x2000;     /* Defer                 */

    eventblk->taskState.eip  = (UWORD32)entry; /* Entry point        */
    eventblk->taskState.esp  = (UWORD32)stack; /* Application stack  */

    eventblk->taskState.cs   = KCODE;       /* Selectors:  Code      */
    eventblk->taskState.ss   = KDATA;       /* Stack                 */
    eventblk->taskState.ds   = KDATA;       /* Data                  */
    eventblk->taskState.es   = KDATA;       /* Data                  */
    eventblk->taskState.fs   = KDATA;       /* Data                  */
    eventblk->taskState.gs   = KDATA;       /* Data                  */

    eventblk->taskState.ldt  = 0;           /* LDT for this task NYI */
    eventblk->taskState.trap = 0;           /* Debug flag, is off    */
    eventblk->taskState.io   = 0;           /* No I/O maps for now   */

    eventblk->taskState.esp0 = 0;
    eventblk->taskState.esp1 = 0;
    eventblk->taskState.esp2 = 0;
    eventblk->taskState.ss0  = 0;
    eventblk->taskState.ss1  = 0;
    eventblk->taskState.ss2  = 0;

    eventblk->taskState.cr3  = ((ADDR)kernelAddrSpc.pageDir)&0xfffff000;

    eventblk->taskState.i387.used  = 0;
    eventblk->taskState.back       = 0;

    eventblk->signals              = NIL;

    eventblk->head.objectSignature = SIGTHREAD;
    eventblk->head.objectData      = (ADDR) thread;
    eventblk->head.flags           = 0;

    thread->interruptable          = 0;

    if(CKisHeapEmpty(ht_timesharing)) {
        eventblk->time = 0;
        thread->time   = 0;
    } else {
        eventblk->time = CKgetNextTime(ht_timesharing);
        thread->time   = CKgetNextTime(ht_timesharing);
    }
    eventblk->ts = ts_Running;
    CKsiftup(ht_timesharing, eventblk);

    return 0; /* success */
}

