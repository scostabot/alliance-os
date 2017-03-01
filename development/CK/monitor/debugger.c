/*
 * debugger.c:  The Alliance CK debugger
 *
 * This is not a full-blown debugger, like GDB.  This file is meant to
 * contain all the code I need to debug the Alliance CK, using the 386
 * hardware debugging functionality.  Things will pop up in here more or
 * less as I need them; probably not earlier.
 * I use this debugger in combination with objdump:  ie, set a breakpoint,
 * recompile, reboot, boot kernel, write down breakpoint information,
 * reboot into my development platform, get a dissasembly dump of the
 * kernel using `objdump --disassemble' and resolve the virtual addresses
 * manually.  Primitive, but effective all the same.
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 21/01/99  ramon       1.0    First release
 * 07/02/99  ramon       1.1    Added watchpoints
 * 19/08/99  ramon       1.2    Moved stubs specific to this file here
 *                              so we can seamlessly exchange it with GDB
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
#include "sys/sys.h"
#include "sys/sysdefs.h"
#include "sys/regs.h"
#include "sys/gdt.h"
#include "sched.h"


/* Macros to manipulate the debug registers */
#define GETDEBUG(x,y)  asm("movl %%db" #x ",%0" : "=d" (y))
#define SETDEBUG(x,y)  asm("movl %0,%%db" #x :  : "d" (y))


UWORD32 db6, db7;    /* The contents of debug control and status regs   */


LOCAL struct {       /* This structure defines the debugger's breakpts  */
    enum {           /* The breakpoint type (CK debugger behavior)      */
        UNUSED,      /* --->  No event handler installed                */
        WATCHPOINT,  /* --->  A data watchpoint (R/W, depends on flags) */
        BREAKPOINT   /* --->  A code breakpoint (X watchpoint)          */
    } type;
    enum {           /* The scope of the breakpoint                     */
        QUANTUM,     /* --->  Local scope (only in the current quantum) */
        GLOBAL       /* --->  Global scope                              */
    } scope;
    BYTE length;     /* Length of the watched data structure (1, 2, 4)  */
    ADDR address;    /* Virtual address of breakpoint                   */
    BYTE flags;      /* Breakpoint flags                                */
    STRING *name;    /* Name of this breakpoint                         */
} breakpoint[4];


/* The stack frame of the debugger interrupt */
struct frame {
    UWORD32 ds;
    struct regs regs;
    UWORD32 eip;
    UWORD32 cs;
    UWORD32 eflags;
    UWORD32 esp;
    UWORD32 ss;
};


/* Predeclarations */
LOCAL VOID CKdebugDispatch(UDATA event, struct frame *stack);
LOCAL VOID CKwatchPoint(UDATA breakpt, struct frame *stack);
LOCAL VOID CKbreakPoint(UDATA breakpt, struct frame *stack);


PUBLIC VOID CKdebugEvent(UWORD32 ds, struct regs regs,
    UWORD32 eip, UWORD32 cs, UWORD32 eflags, UWORD32 esp, UWORD32 ss)
/*
 * This is the handler that catches all 386 hardware debugger events.
 * It is hooked to int $0x1.  Designed to run with interrupts as off as
 * possible :)
 *
 * INPUT:
 *     parameters pushed onto the stack by the x86
 *
 * RETURNS:
 *     none
 */
{
    GETDEBUG(6,db6);    /* Fetch the debug status register contents     */
    GETDEBUG(7,db7);    /* Fetch the debug control register contents    */
    SETDEBUG(7,0);      /* Clear all breakpoints while inside debugger  */

    /* Dispatch event handlers for any of the breakpoints */
    if(db6 & (1 << 0))  CKdebugDispatch(0,(struct frame *) &ds);
    if(db6 & (1 << 1))  CKdebugDispatch(1,(struct frame *) &ds);
    if(db6 & (1 << 2))  CKdebugDispatch(2,(struct frame *) &ds);
    if(db6 & (1 << 3))  CKdebugDispatch(3,(struct frame *) &ds);

    /* Dispatch event handlers for the other debug events */
    if(db6 & (1 << 13)) CKdebugDispatch(13,(struct frame *) &ds);
    if(db6 & (1 << 14)) CKdebugDispatch(14,(struct frame *) &ds);
    if(db6 & (1 << 15)) CKdebugDispatch(15,(struct frame *) &ds);

    SETDEBUG(7,db7);    /* Set the new debug control word               */
    SETDEBUG(6,0);      /* Clear the debug status register              */
}


LOCAL VOID CKdebugDispatch(UDATA event, struct frame *stack)
/*
 * Dispatch the correct debugger event handler
 *
 * INPUT:
 *     event:  event to be handled
 *     stack:  the stack frame of the event
 *
 * RETURNS:
 *     none
 */
{
    switch(event) {
        case 0:      /* Debug register 0 triggered event                */
        case 1:      /* Debug register 1 triggered event                */
        case 2:      /* Debug register 2 triggered event                */
        case 3:      /* Debug register 3 triggered event                */

            switch(breakpoint[event].type) {
                case UNUSED:                   /* No event handler inst.*/
                    db7 &= ~(3 << (2*event));  /* Turn off the breakpt. */
                    break;
                case WATCHPOINT:               /* A data watchpoint     */
                    CKwatchPoint(event,stack); /* Invoke the handler    */
                    break;
                case BREAKPOINT:               /* A code breakpoint     */
                    CKbreakPoint(event,stack); /* Invoke the handler    */
                    break;
                default:                       /* Oops... error         */
                    breakpoint[event].type = UNUSED;
                    db7 &= ~(3 << (2*event));  /* Turn off the breakpt. */
                    break;
            }
            break;

        case 14:     /* Single step debugging (TF-flag event)           */ 
        case 15:     /* Task switch occured (T-bit in TSS event)        */
            break;

        case 13:     /* Error:  ICE-386 is using the debug registers    */
        default:     /* Error:  Bug in debugger                         */
            break;
    }
}


LOCAL VOID CKwatchPoint(UDATA breakpt, struct frame *stack)
/*
 * Debug event handler for a data watchpoint
 *
 * INPUT:
 *     break:  breakpoint that caused event
 *     stack:  the stack frame of the event
 *
 * RETURNS:
 *     none
 */
{
    UWORD32  sp = ((stack->cs == KCODE) ? (UWORD32) &stack->esp : stack->esp);

    CKdebugPrint("CKDebug:  Thread 0x%x wrote to watchpoint %s\n",currtask,breakpoint[breakpt].name);
    CKdebugPrint("CKDebug:  before %eip = 0x%x, %esp = 0x%x\n",stack->eip,sp);

    if(breakpoint[breakpt].flags & 1) {
        CKdebugPrint("CKDebug:  Halting...\n");
        for(;;);
    }
}


LOCAL VOID CKbreakPoint(UDATA breakpt, struct frame *stack)
/*
 * Debug event handler for a code breakpoint
 *
 * INPUT:
 *     break:  breakpoint that caused event
 *     stack:  the stack frame of the event
 *
 * RETURNS:
 *     none
 */
{
    UWORD32  sp = ((stack->cs == KCODE) ? (UWORD32) &stack->esp : stack->esp);

    CKdebugPrint("CKDebug:  Thread 0x%x reached breakpoint %s\n",currtask,breakpoint[breakpt].name);
    CKdebugPrint("CKDebug:  %eip = 0x%p, %esp = 0x%p, %cs  = 0x%p\n",stack->eip,sp,stack->cs);
    CKdebugPrint("CKDebug:  %ds  = 0x%p, %ss  = 0x%p, %ebp = 0x%p\n",stack->ds,stack->ss,stack->regs.ebp);
    CKdebugPrint("CKDebug:  %eax = 0x%p, %ebx = 0x%p, %ecx = 0x%p\n",stack->regs.eax, stack->regs.ebx, stack->regs.ecx);
    CKdebugPrint("CKDebug:  %edx = 0x%p, %esi = 0x%p, %edi = 0x%p\n",stack->regs.edx,stack->regs.esi, stack->regs.edi);
    CKdebugPrint("CKDebug:  %eflags = 0x%p\n",stack->eflags);

    if(breakpoint[breakpt].flags & 2) {
        DATA i;
        CKdebugPrint("CKDebug:\nCKDebug:\t\tCode dump\tStack dump\n");
        for(i=0;i<4;i++) {
            CKdebugPrint("CKDebug:\t0x%x\t0x%p\t0x%p\n",i,((UWORD32 *)stack->eip)[i], ((UWORD32 *)sp)[i]);
        }
    }

    if(breakpoint[breakpt].flags & 1) {
        CKdebugPrint("CKDebug:  Halting...\n");
        for(;;);
    }
}


/*
 * Debug resource allocation
 */

PUBLIC DATA CKdebugWatchData(STRING *name, ADDR address, DATA flags, DATA len)
/*
 * Setup a data watchpoint
 *
 * INPUT:
 *     name:     name of the data to watch
 *     address:  virtual address of the data to watch
 *     flags:    breakpoint flags
 *     len:      length of the data to watch
 *
 * Breakpoint flag masks for data watchpoints:
 *     1:  Halt on write
 *
 * RETURNS:
 *     -1 on error, otherwise allocated breakpoint number.
 */
{
    DATA i;

    /* Check whether length is valid */
    if(!len || len > 4 || len == 3)
        return -1;

    /* Find a free breakpoint */
    for(i=0; i < 4; i++)
        if(breakpoint[i].type == UNUSED) break;

    /* Check for no more free breakpoints */
    if(i == 4)
        return -1;

    /* Fill in basic breakpoint data */
    breakpoint[i].type    = WATCHPOINT;
    breakpoint[i].scope   = GLOBAL;
    breakpoint[i].name    = name;
    breakpoint[i].address = address;
    breakpoint[i].flags   = flags;
    breakpoint[i].length  = len;

    /* Load the watchpoint address into the free debug register */
    switch(i) {
        case 0:
            SETDEBUG(0,address);
            break;
        case 1:
            SETDEBUG(1,address);
            break;
        case 2:
            SETDEBUG(2,address);
            break;
        case 3:
            SETDEBUG(3,address);
            break;
    }

    /* Setup the debug control register */
    GETDEBUG(7,db7);

    db7 &= ~(1 << (2*i));         /* Global scope                */
    db7 |=  (2 << (2*i));

    db7 |=  0x0200;               /* Enable breakpoint detection */

    if(len == 1) {
        db7 &= ~(15 << (4*i + 16));   /* Set break-on-write          */
        db7 |=  (1  << (4*i + 16));   /* and 1-byte size             */
    } else {
        db7 &= ~(15 << (4*i + 16));   /* Set break-on-write          */
        db7 |=  ((4*len - 3) << (4*i + 16)); /* and the correct size */
    }

    /* Load the debug control register */
    SETDEBUG(7,db7);

    return i;
}


PUBLIC DATA CKdebugBreakPoint(STRING *name, ADDR address, DATA flags)
/*
 * Setup a code breakpoint
 *
 * INPUT:
 *     name:     name of the breakpoint
 *     address:  virtual address of the breakpoint
 *     flags:    breakpoint flags
 *
 * Breakpoint flag masks for code breakpoints:
 *     1:  Halt on breakpoint
 *     2:  Show code/stack dump
 *
 * RETURNS:
 *     -1 on error, otherwise allocated breakpoint number.
 */
{
    DATA i;

    /* Find a free breakpoint */
    for(i=0; i < 4; i++)
        if(breakpoint[i].type == UNUSED) break;

    /* Check for no more free breakpoints */
    if(i == 4)
        return -1;

    /* Fill in basic breakpoint data */
    breakpoint[i].type    = BREAKPOINT;
    breakpoint[i].scope   = GLOBAL;
    breakpoint[i].name    = name;
    breakpoint[i].address = address;
    breakpoint[i].flags   = flags;
    breakpoint[i].length  = 1;

    /* Load the breakpoint address into the free debug register */
    switch(i) {
        case 0:
            SETDEBUG(0,address);
            break;
        case 1:
            SETDEBUG(1,address);
            break;
        case 2:
            SETDEBUG(2,address);
            break;
        case 3:
            SETDEBUG(3,address);
            break;
    }

    /* Setup the debug control register */
    GETDEBUG(7,db7);

    db7 &= ~(1 << (2*i));         /* Global scope                */
    db7 |=  (2 << (2*i));

    db7 |=  0x0200;               /* Enable breakpoint detection */

    db7 &= ~(15 << (4*i + 16));   /* Set break-on-execute        */

    /* Load the debug control register */
    SETDEBUG(7,db7);

    return i;
}


PUBLIC VOID CKdebugFree(DATA bpoint)
/*
 * Free a breakpoint
 *
 * INPUT:
 *     bpoint:  breakpoint to be freed
 *
 * RETURNS:
 *     none
 */
{
    UDATA   i;
    UWORD32 db7;
    BOOLEAN locals = FALSE, globals = FALSE;

    if(bpoint > 3) return;

    breakpoint[bpoint].type = UNUSED;

    GETDEBUG(7,db7);
    db7 &= ~(3 << (2*bpoint));

    for(i=0; i<4; i++) {
        if(breakpoint[i].scope == QUANTUM) locals  = TRUE;
        if(breakpoint[i].scope == GLOBAL)  globals = TRUE;
    }

    if(locals  == FALSE) db7 &= ~0x100;
    if(globals == FALSE) db7 &= ~0x200;

    SETDEBUG(7,db7);
}


/*
 * Initialisation functions
 */

extern VOID __int_0x01(VOID);

PUBLIC VOID CKinitDebugger(VOID)
/*
 * Initialise the debugging subsystem
 *
 * INPUT:
 *     event:  event to be handled
 *
 * RETURNS:
 *     none
 */
{
    DATA i;

    /* Clear all breakpoints */
    for(i=0; i < 4; i++) {
        breakpoint[i].type = UNUSED;
    }

    /* Clear the debug status register, and the debug control register */
    SETDEBUG(6,0);
    SETDEBUG(7,0);

    /* Set the debug interrupt vector */
    CKsetVector(__int_0x01, 0x01, D_PRESENT+D_INT);
}


/*
 * This is the debug trap.  It is hooked to the CK debugger.
 */

asm (
    ".text                       \n"
    ".globl __int_0x01           \n"
    "__int_0x01:                 \n"
    "    pushl %eax              \n"         /* Save the registers */
    "    pushl %edx              \n"
    "    pushl %ecx              \n"
    "    pushl %ebp              \n"
    "    pushl %edi              \n"
    "    pushl %esi              \n"
    "    pushl %ebx              \n"
    "    pushl %ds               \n"         /* Save %ds           */
    "    pushl %ss               \n"         /* Switch to kernel   */
    "    popl %ds                \n"         /* data segment       */
    "    cld                     \n"         /* GCC likes this     */
    "    call CKdebugEvent       \n"         /* Handle the int     */
    "    popl %ds                \n"         /* Restore %ds        */
    "    popl %ebx               \n"         /* Restore the regs   */
    "    popl %esi               \n"
    "    popl %edi               \n"
    "    popl %ebp               \n"
    "    popl %ecx               \n"
    "    popl %edx               \n"
    "    popl %eax               \n"
    "    iret                    \n"         /* Return from intr.  */
);
