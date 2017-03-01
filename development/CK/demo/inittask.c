/*
 * inittask.c:  The init task
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author       Rev    Notes
 * 07/11/98  ramon/jens   1.0    First full internal release
 * 16/11/98  ramon        1.1    Updated to use generic ckobjects.h
 * 26/11/98  jens         1.2    Added RT external event sync task (VBL)
 * 28/11/98  ramon        1.3    Added address space object
 * 29/11/98  ramon        1.4    Added init task
 * 19/12/98  ramon        1.5    Made calc for next exec time more accurate
 * 23/12/98  ramon        1.6    Added kernel object stuff
 * 31/12/98  ramon        1.7    Added resource allocation (just demo)
 * 01/01/99  ramon        1.8    Printing now works through trapping to kernel
 * 02/01/99  ramon        1.9    Added the two FPU test tasks
 * 14/01/99  haggai       1.10   Added reset task
 * 30/01/99  ramon        1.11   Added global interrupt allocation
 * 31/01/99  haggai/ramon 1.12   DemoKernel keyboard driver
 * 04/02/99  haggai       1.13   Added keyboard buffer and first DemoShell
 * 21/02/99  jens         1.14   Implemented VBL synchronized Keyboard driver
 * 19/03/99  jens         1.15   Removed preallocated page directory
 * 15/04/99  ramon        1.16   Separated demo system into demo/ tree
 * 23/07/99  ramon        1.17   Modified to be loaded by ABL
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

/**************************************************************/
/* NOTE:  This file contains platform-dependant code for i386 */
/**************************************************************/

/* This file represents 'user programs', and is not done */
/* in Alliance Coding Style                              */

#include <typewrappers.h>
#include <klibs.h>
#include "sys/sys.h"
#include "sys/colors.h"
#include "sys/ioport.h"
#include "sys/realtime.h"
#include "sys/memory.h"
#include "ckobjects.h"
#include "cksyscalls.h"
#include "cklib.h"
#include "demo.h"


/******************************************************************/
/* The address space object                                       */
/******************************************************************/

AddrSpcObject *demoASpace;


/******************************************************************/
/* The thread object                                              */
/******************************************************************/

ThreadObject *ITASK;


/******************************************************************/
/* The application kernel                                         */
/******************************************************************/

KernelObject demoKernel = { NIL, NIL, 0, NIL };


/******************************************************************/
/* Shared memory object                                           */
/******************************************************************/

SHMemObject shMemObj = { 0 };


/******************************************************************/
/* The resource loader structure                                  */
/******************************************************************/

/* These are used by INIT to test the resource allocation system  */

IOregionAlloc IOregion  = { SIGIOREGION, 0x300, 0x310 };
IOregionAlloc IOregion2 = { SIGIOREGION, 0x305, 0x315 };


/******************************************************************/
/* The interrupt allocation resource                              */
/******************************************************************/

/* This is used by INIT to allocate a global interrupt to DemoKernel */

TrapAlloc DemoTrap = { SIGTRAP, 0x69, NIL, 0x000 };


/******************************************************************/
/* The entry point                                                */
/******************************************************************/

UBYTE initstack[4096];

asm (
    ".text; .globl _start       \n"
    "_start:                    \n"
    "    movl $initstack,%esp   \n"
    "    addl $4096,%esp        \n"
    "    jmp inittask           \n"
);


/******************************************************************/
/* The init task                                                  */
/******************************************************************/

void inittask(void)
{
    setTextAttr(T_FG_GREEN + T_BOLD);

    demoASpace = (AddrSpcObject *) ((ADDR)_start - PAGESIZE);
    ITASK      = (ThreadObject *) (demoASpace + 1);

    /* Map video memory into our address space, so we can print ! */
    CKloadMapping(demoASpace->self,0xb8000,0x60000000,16*PAGESIZE-1,PTWRITE);

    print("Init:  Setting up parent kernel\n");
    demoKernel.addrSpace  = demoASpace->self;
    demoKernel.sigHandler = (ADDR) AKsigHandlStub;
    CKcacheObject((ADDR)&demoKernel,SIGKERNEL,sizeof(demoKernel),0);

    print("Init:  Caching four timesharing tasks\n");
    cacheThread(tstask1,TSTACK1+4096,0x1000,0x2000, &TSTASK1,demoASpace->self,demoKernel.self,TKSTACK1+4096);
    cacheThread(tstask2,TSTACK2+4096,0x1000,0x4000, &TSTASK2,demoASpace->self,demoKernel.self,TKSTACK2+4096);
    cacheThread(tstask3,TSTACK3+4096,0x1000,0x8000, &TSTASK3,demoASpace->self,demoKernel.self,TKSTACK3+4096);
    cacheThread(tstask4,TSTACK4+4096,0x1000,0x10000,&TSTASK4,demoASpace->self,demoKernel.self,TKSTACK4+4096);

    print("Init:  Caching two realtime tasks\n");
    cacheThread(rttask1,RSTACK1+4096,0x1000,0x2000,&RTTASK1,demoASpace->self,demoKernel.self,RKSTACK1+4096);
    cacheThread(rttask2,RSTACK2+4096,0x1000,0x2000,&RTTASK2,demoASpace->self,demoKernel.self,RKSTACK2+4096);

    print("Init:  Caching GRSK VBL-synchronised RT task\n");
    cacheThread(rttask3,RSTACK3+4096,0xffff,0x2000,&RTTASK3,demoASpace->self,demoKernel.self,RKSTACK3+4096);

    print("Init:  Caching two i387 FPU test tasks\n");
    cacheThread(fptask1,FSTACK1+4096,0x1000,0x2000,&FPTASK1,demoASpace->self,demoKernel.self,FKSTACK1+4096);
    cacheThread(fptask2,FSTACK2+4096,0x1000,0x2000,&FPTASK2,demoASpace->self,demoKernel.self,FKSTACK2+4096);

    print("Init:  Caching DemoShell\n");
    cacheThread(demoshell,DSSTACK+4096,0x1000,0x2000,&DSTASK,demoASpace->self,demoKernel.self,DSKSTACK+4096);

    print("Init:  Allocating I/O region 0x300-0x310 (demo)");
    if(CKresourceAlloc(&IOregion, DSTASK.self, 0))
        print(" ... error\n");
    else if(!CKresourceAlloc(&IOregion2, DSTASK.self, 0))
        print(" ... error\n");
    else
        print("\n");

    print("Init:  Allocating keyboard IRQ to kernel");
    DemoTrap.sigThread = (EVENT *) DSTASK.self;
    if(CKresourceAlloc(&DemoTrap, DSTASK.self, 0))
        print(" ... error\n");
    else {
        print("\n");
        while(inportb(0x64) & 2);
    }

    print("Init:  Caching shared memory object\n");
    CKcacheObject((ADDR)&shMemObj,SIGSHMEM,sizeof(shMemObj),0);

    CKshMemSetRange(shMemObj.self,0x60000000,PAGESIZE);
    CKshMemInvite(shMemObj.self,(EVENT *)TSTASK1.self);
    CKshMemInvite(shMemObj.self,(EVENT *)TSTASK2.self);

#if 0
    print("Init:  Uncaching self\n");
    CKunCacheObject(ITASK->self);
#else
    print("Init:  Blocking self\n");
    CKsetThreadState(ITASK->self,ts_Blocked);
#endif

    quantumDone();

    /* Shouldn't reach this:  hang, just in case something is broken */
    print("Init:  Oops:  Should never reach this point\n");
    for(;;);
}
