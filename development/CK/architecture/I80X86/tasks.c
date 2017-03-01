/*
 * tasks.c:  Harware-related parts of the scheduler
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 26/11/98  ramon       1.1    Added thread state pre/postprocessing
 * 19/12/98  ramon       1.2    Made timer int available for DPL3
 * 01/01/99  ramon       1.3    Added FPU stuff
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
#include <klibs.h>
#include "sys/taskstate.h"
#include "sys/tasks.h"
#include "sys/sysdefs.h"
#include "sys/sys.h"
#include "sys/8254.h"
#include "sys/8259.h"
#include "sys/gdt.h"
#include "sys/ioport.h"
#include "sys/memory.h"
#include "sys/hwkernel.h"
#include "ckerror.h"
#include "ckobjects.h"
#include "memory.h"

extern union DTEntry GDT[];
extern EVENT *lastFPUUser;

extern VOID CKtimerService(VOID);  /* The timer service assembly stub */
extern VOID CKfinishSignal(VOID);  /* The EOS stub                    */

PUBLIC VOID CKhwInitSched(VOID)
/*
 * Initialise the scheduler hardware (interrupts, IRQ, timer)
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    /* Set 8254 PIT channel 0 and 1 to mode 0 (int on terminal count) */
    outportb(TMR_CTRL, (TMR_SC0 + TMR_both + TMR_MD0));
    outportb(TMR_CTRL, (TMR_SC1 + TMR_both + TMR_MD0));

    /* Reset channel 1 counter */
    outportb(TMR_CNT1,0);
    outportb(TMR_CNT1,0);

    /* Set up timer service and signal handling */
    CKsetVector(CKtimerService, M_VEC,    (D_TRAP + D_PRESENT + D_DPL3));
    CKsetVector(CKfinishSignal, M_VEC+2,  (D_INT  + D_PRESENT + D_DPL1));

    /* Enable timer IRQ */
    CKenableIRQ(0);
}


PUBLIC DATA CKpreProcessTaskState(ThreadObject *object, CKThreadObject *ckobject)
/*
 * Initialise a thread's TSS and allocate a CK stack
 *
 * INPUT:
 *     object:  Pointer to object to set up TSS for
 *
 * RETURNS:
 *     CKpreProcessTaskState():  Error code
 */
{
    struct TSS *userState = &object->userState;
    struct TSS *taskState = &ckobject->taskState;
    CKAddrSpcObject *aspace = (CKAddrSpcObject *) ckobject->addressSpace;

    KLmemoryCopy(taskState, userState, sizeof(struct TSS));

    taskState->back = 0;

    /* Initialise the thread's address space */
    taskState->cr3 = (UWORD32) PHYS(aspace->addrSpc);
    if(!taskState->cr3) return ENOVIRTMEM;

    /* Check the kernel entry point */
    if(!taskState->eip) return ESEGMENTATION;

    /* Set the flags */
    taskState->eflags = STDFLAGS;

    /* This thread hasn't used the coprocessor yet */
    taskState->i387.used = 0;

    /* Check the code/data segment registers */
    if(!taskState->cs || !taskState->ds || !taskState->ss) return ESEGMENTATION;

    /* Set up the kernel stack */
    /* The other stacks are set up by the parent kernel */
    taskState->ss0  = KDATA;
    if((taskState->esp0 = (UWORD32) CKmemAlloc(KSTACKSIZE)))
        taskState->esp0 += KSTACKSIZE;
    else
        return ENOMEM;
    return 0;  /* Success */
}


PUBLIC VOID CKpostProcessTaskState(ThreadObject *object, CKThreadObject *ckobject)
/*
 * Deallocate CK stack
 *
 * INPUT:
 *     object:  Pointer to object with TSS for postprocessing
 *
 * RETURNS:
 *     none
 */
{
    struct TSS *taskState = &ckobject->taskState;

    /* Make sure we don't try to save the FPU context here later on */
    if(ckobject == lastFPUUser) lastFPUUser = NIL;

    /* Deallocate kernel stack */
    CKmemFree((VOID *) (taskState->esp0 - KSTACKSIZE),KSTACKSIZE);
}
