/*
 * sys.h:  Prototypes of hardware related functions in src/sys
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
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

#ifndef __SYS_SYS_H
#define __SYS_SYS_H

#include <typewrappers.h>
#include "sys/sysdefs.h"
#include "ckobjects.h"
#include "ckresource.h"

VOID CKinitHardware(VOID);          /* Initialise the hardware          */
VOID CKhwInitMem(VOID);
VOID CKinitCons(VOID);
VOID CKinitDebugger(VOID);
VOID CKhwInitSched(VOID);           /* Initialise the timer hardware    */

VOID CKenableIRQ(UDATA irq);
VOID CKdisableIRQ(UDATA irq);

VOID CKwait(VOID);                  /* Wait a short while               */
VOID CKwaitSec(VOID);               /* Wait a second or so              */

VOID CKclrScr(VOID);                /* Clear the screen                 */
DATA CKprint(CONST STRING *formatString, ...);
DATA CKdebugPrint(CONST STRING *formatString, ...);
VOID CKsetTextAttr(DATA attr);
VOID CKbeep(VOID);

VOID CKsetVector(VOID *handler,     /* Sets an interrupt in the IDT     */
                UBYTE interrupt, UWORD16 control_major);

DATA CKpreProcessTaskState(ThreadObject *object, CKThreadObject *ckobject);
VOID CKpostProcessTaskState(ThreadObject *object, CKThreadObject *ckobject);

DATA CKpreProcessAddrSpc(CKAddrSpcObject *object);
VOID CKpostProcessAddrSpc(CKAddrSpcObject *object);
/*
DATA CKpreProcessSHMem(struct SHMemObject *object);
VOID CKpostProcessSHMem(struct SHMemObject *object);
*/
DATA CKpreProcessKernel(KernelObject *object, CKKernelObject *ckobject);
VOID CKpostProcessKernel(KernelObject *object, CKKernelObject *ckobject);

DATA CKpreProcessTrapAlloc(ADDR trap, CKTrapAlloc *ckres, DATA flags);
VOID CKpostProcessTrapAlloc(CKTrapAlloc *res);

VOID CKidle(VOID) __attribute__ ((noreturn));  /* The Idle loop         */

DATA CKcacheUserThread(ADDR entry, ADDR stack, ThreadObject *eventblk, DESCRIPTOR addrspc);
DATA CKcacheKernelThread(VOID *entry, UBYTE *stack, EVENT *eventblk, ThreadObject *thread);

PUBLIC BOOLEAN
CKsetPageMapping(AddressSpace addrSpc, ADDR logicalPageAddr, 
		 ADDR physicalPageAddr, ADDR length, UWORD32 Attr);
PUBLIC VOID
CKmarkPageNotPresent(AddressSpace addrSpc, ADDR logicalPageAddr, ADDR length,
		 UWORD32 Data);

PUBLIC BOOLEAN
CKcopySharedPageMapping(AddressSpace sourceAddrSpc, ADDR sourceLogPageAddr,
                        AddressSpace destAddrSpc, ADDR destLogPageAddr,
                        ADDR length);

PUBLIC VOID
CKpageMarkHardwareDependent(VOID);

/* Debugger calls */
DATA CKdebugWatchMutex(STRING *name, ADDR address, DATA flags);
DATA CKdebugWatchData(STRING *name, ADDR address, DATA flags, DATA len);
DATA CKdebugBreakPoint(STRING *name, ADDR address, DATA flags);
VOID CKdebugFree(DATA bpoint);

/* System/Boot info */
extern sysinfo sysinf;

#endif

