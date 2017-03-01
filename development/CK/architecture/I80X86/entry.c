/*
 * entry.c:  The entey point of the kernel, and all those awful tables
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 18/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 25/11/98  ramon       1.2    Updated GDT
 * 26/11/98  ramon       1.3    Updated GDT again
 * 08/12/98  jens        1.4    Call gates implemented
 * 23/07/99  ramon       1.5    Major segmentation overhaul / ABL loading
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
#include "sys/sys.h"           /* CK function prototypes                */
#include "sys/sysdefs.h"       /* Selectors and such                    */
#include "sys/stubs/callgates.h"  /* Call gates definitions             */
#include "sys/gdt.h"           /* Descriptor definitions                */
#include "sys/taskstate.h"     /* TSS                                   */
#include "sys/memory.h"        /* Paging                                */
#include "ckobjects.h"         /* CK object structures                  */

VOID CKmain(VOID) __attribute__ ((noreturn));

LOCAL UBYTE kernelStack[8192]; /* The initial kernel stack              */
PUBLIC EVENT idleLoop;         /* The idle loop                         */
PUBLIC ThreadObject idleULoop; /* Idle loop user struct (for scheduler) */
PUBLIC sysinfo sysinf;         /* The boot structure from the ABL       */

extern struct AddressSpace kernelAddrSpc;

/************************************************************************/
/* We now get all of those system-dependant tables that HAVE to lurk    */
/* *somewhere*...  The Global Descriptor Table, and the IDT too         */

PUBLIC descTable(GDT, NOGDTENTRIES) {    /* The Global Descriptor Table           */
    {dummy: 0},
    stndDesc(0x00000000,0xfffff,(D_CODE+D_READ+D_BIG+D_BIG_LIM)),
    stndDesc(0x00000000,0xfffff,(D_DATA+D_WRITE+D_BIG+D_BIG_LIM)),
    stndDesc(0x00000000,0xdffff,(D_CODE+D_READ+D_BIG+D_BIG_LIM+D_DPL1)),
    stndDesc(0x00000000,0xdffff,(D_DATA+D_WRITE+D_BIG+D_BIG_LIM+D_DPL1)),
    stndDesc(0x00000000,0xbffff,(D_CODE+D_READ+D_BIG+D_BIG_LIM+D_DPL3)),
    stndDesc(0x00000000,0xbffff,(D_DATA+D_WRITE+D_BIG+D_BIG_LIM+D_DPL3)),
    stndDesc(0x00000000,(sizeof(struct TSS)-1), D_TSS),
    /* Call gates go here */
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0),
    gateDesc(0,KCODE,D_CALL+D_DPL3+0)
};

PUBLIC descTable(IDT, 256) {   /* The empty Interrupt Descriptor Table  */
};

/************************************************************************/

PUBLIC struct                  /* Loading structure for the GDT         */
{ 
    UWORD16 limit       __attribute__ ((packed)); 
    union DTEntry *idt  __attribute__ ((packed)); 
} loadgdt = { (NOGDTENTRIES * sizeof(union DTEntry) - 1), GDT };

/************************************************************************/
/* And now...                                                           */

PUBLIC VOID _start(VOID)
/*
 * The kernel entry point
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    asm volatile               /* Load the GDT and the kernel stack     */
    (                          /* We assume the loader has cleared ints */
        "movl %%eax,%%ebx  \n" /* Save the boot structure in %ebx       */
        "lgdtl loadgdt     \n" /* Load our own GDT                      */
        "movw %3,%%ax      \n" /* Init data registers with kdata        */
        "movw %%ax,%%ds    \n"
        "movw %%ax,%%es    \n"
        "movw %%ax,%%fs    \n"
        "movw %%ax,%%gs    \n"
        "movw %%ax,%%ss    \n" /* ... and the stack, too                */
        "movl %1,%%esp     \n" /* Set the SP to the end of the stack    */
        "ljmp %2,$1f       \n" /* Flush prefetch queue                  */
        "1: movl %%ebx,%0  \n" /* Save the boot structure in memory     */
        : "=g" (sysinf)
        : "p" (kernelStack+8192), "p" (KCODE), "p" (KDATA)
        /* Don't mark any registers clobbered to avoid using */
        /* the stack before we've had time to initialise it  */
    );

    /* Find linear location of the system info structure */
    sysinf = (sysinfo) LINEAR(sysinf);

    CKinitHardware();           /* Initialise memory/interrupt hardware */

    /* Setup system call gates */
    SETUPCALLGATE(CKCACHEOBJECTDSC,CKcacheObjectGateCode,4);
    SETUPCALLGATE(CKUNCACHEOBJECTDSC,CKunCacheObjectGateCode,1);
    SETUPCALLGATE(CKSETTHREADSTATEDSC,CKsetThreadStateGateCode,2);
    SETUPCALLGATE(CKRESOURCEALLOCDSC,CKresourceAllocGateCode,3);
    SETUPCALLGATE(CKRESOURCEFREEDSC,CKresourceFreeGateCode,1);
    SETUPCALLGATE(CKLOADMAPPINGDSC,CKloadMappingGateCode,5);
    SETUPCALLGATE(CKUNLOADMAPPINGDSC,CKunloadMappingGateCode,3);
    SETUPCALLGATE(CKSHMEMSETRANGEDSC,CKshMemSetRangeGateCode,3);
    SETUPCALLGATE(CKSHMEMINVITEDSC,CKshMemInviteGateCode,2);
    SETUPCALLGATE(CKSHMEMREMAPDSC,CKshMemRemapGateCode,3);
    SETUPCALLGATE(CKSHMEMATTACHDSC,CKshMemAttachGateCode,3);
    SETUPCALLGATE(CKSHMEMDETACHDSC,CKshMemDetachGateCode,1);
    SETUPCALLGATE(CKSHMEMNOTIFYDSC,CKshMemNotifyGateCode,1);
    SETUPCALLGATE(CKTHREADTUNNELDSC,CKthreadTunnelGateCode,2);

    /* Change TSS descriptor in gdt */
    GDT[TSSDSC].desc.base_low  = ((UWORD32)&idleLoop.taskState) & 0xffff;
    GDT[TSSDSC].desc.base_med  = ((UWORD32)&idleLoop.taskState>>16)&0xff;
    GDT[TSSDSC].desc.base_high = ((UWORD32)&idleLoop.taskState>>24)&0xff;

    /* Set up data for the kernel task                                  */
    /* After the initialisation phase, it turns into the idle loop      */

    idleLoop.head.objectSignature = SIGTHREAD;
    idleLoop.head.objectData      = (ADDR) &idleULoop;
    idleLoop.head.flags           = 0;

    idleULoop.interruptable       = 0;

    idleLoop.ts      = ts_Idle;           /* The idle loop is, um, idle */
    idleLoop.quantum = 0x1000;            /* Standard quantum           */

    idleLoop.signals        = NIL;        /* Idle task cannot receive   */

    idleLoop.taskState.cs   = KCODE;      /* Set up all those segments: */
    idleLoop.taskState.ss0  = KDATA;      /* Code, Data, Stack (==data) */
    idleLoop.taskState.ss   = KDATA;
    idleLoop.taskState.ds   = KDATA;
    idleLoop.taskState.es   = KDATA;
    idleLoop.taskState.fs   = KDATA;
    idleLoop.taskState.gs   = KDATA;

    idleLoop.taskState.cr3  = (UWORD32) kernelAddrSpc.pageDir;

    idleLoop.taskState.ldt  = 0;          /* Kernel doesn't have an LDT */
    idleLoop.taskState.trap = 0;          /* NO debugging PLEASE        */
    idleLoop.taskState.io   = 0;          /* PL0, so no IOMap needed    */

    idleLoop.taskState.esp1 = 0;          /* Kernel task never does a   */
    idleLoop.taskState.esp2 = 0;          /* PL switch to higher PL     */
    idleLoop.taskState.ss1  = 0;
    idleLoop.taskState.ss2  = 0;

    /* Load the TSS into the task register, and make it non-busy        */
    asm("ltr %0"::"r" ((UWORD16)(TSSSEL)));
    GDT[TSSDSC].desc.access = (D_TSS+D_PRESENT) >> 8;

    /* Now we're ready to transfer to the platform-independant kernel   */

    CKmain();                   /* Call the main kernel init code       */

    /* we should never, ever return here                                */
    /* just in case (we're used to impossibilities at Alliance) halt    */
    /* the computer                                                     */

    asm("cli;hlt");             /* Halt the computer, preventing harm   */
}

