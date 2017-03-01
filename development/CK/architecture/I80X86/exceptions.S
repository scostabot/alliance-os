/*
 * exceptions.S:  Assembly stubs for the exception handlers
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 22/11/98  ramon       1.1    Rewrite using those funky macros
 * 01/01/99  ramon       1.2    Using SAVEREGS
 * 02/01/99  ramon       1.3    Added FPU DNA stub (0x7) / smaller exceptions
 * 16/01/99  ramon       1.4    Added EOS code / signal handling in timer
 * 20/07/99  ramon       1.5    Made scheduler locking atomic
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

#include "sys/asm.h"

/*
 * The exception stubs were made to provide special support for the
 * traditional error code.  ERREXCEPT() is used with exceptions that do
 * push an error code. NERREXCEPT() explicitly pushes $0 on the stack to
 * 'simulate' the error code.
 */

#define ERREXCEPT(nr,stubsym)                                         \
    .globl stubsym         ;            /* Global entry point   */    \
    stubsym:               ;            /* Start of excp stub   */    \
        pushl $(nr)        ;            /* Push interrupt num   */    \
        jmp __int          ;            /* Handle the exception */

#define NERREXCEPT(nr,stubsym)                                        \
    .globl stubsym         ;            /* Global entry point   */    \
    stubsym:               ;            /* Start of excp stub   */    \
        pushl $0           ;            /* Error substitute     */    \
        pushl $(nr)        ;            /* Push interrupt num   */    \
        jmp __int          ;            /* Handle the exception */


.text

NERREXCEPT(0x00,__int_0x00)   /* Divide by zero           */
NERREXCEPT(0x02,__int_0x02)   /* NMI                      */

/* These exceptions need D_DPL3 */
NERREXCEPT(0x03,__int_0x03)   /* Breakpoint               */
NERREXCEPT(0x04,__int_0x04)   /* Overflow (INTO)          */
NERREXCEPT(0x05,__int_0x05)   /* Bounds check (BOUNDS)    */

/* These don't */
NERREXCEPT(0x06,__int_0x06)   /* Invalid opcode           */
NERREXCEPT(0x08,__int_0x08)   /* Double fault             */
NERREXCEPT(0x09,__int_0x09)   /* Reserved                 */
ERREXCEPT(0x0a, __int_0x0a)   /* Invalid TSS              */
ERREXCEPT(0x0b, __int_0x0b)   /* Segment not present      */
ERREXCEPT(0x0c, __int_0x0c)   /* Stack exception          */
ERREXCEPT(0x0d, __int_0x0d)   /* General protection fault */
ERREXCEPT(0x0e, __int_0x0e)   /* Page fault               */
NERREXCEPT(0x0f,__int_0x0f)   /* Reserved                 */
NERREXCEPT(0x10,__int_0x10)   /* Coprocessor error        */
NERREXCEPT(0x11,__int_0x11)   /* Reserved                 */
NERREXCEPT(0x12,__int_0x12)   /* Reserved                 */
NERREXCEPT(0x13,__int_0x13)   /* Reserved                 */
NERREXCEPT(0x14,__int_0x14)   /* Reserved                 */
NERREXCEPT(0x15,__int_0x15)   /* Reserved                 */
NERREXCEPT(0x16,__int_0x16)   /* Reserved                 */
NERREXCEPT(0x17,__int_0x17)   /* Reserved                 */
NERREXCEPT(0x18,__int_0x18)   /* Reserved                 */
NERREXCEPT(0x19,__int_0x19)   /* Reserved                 */
NERREXCEPT(0x1a,__int_0x1a)   /* Reserved                 */
NERREXCEPT(0x1b,__int_0x1b)   /* Reserved                 */
NERREXCEPT(0x1c,__int_0x1c)   /* Reserved                 */
NERREXCEPT(0x1d,__int_0x1d)   /* Reserved                 */
NERREXCEPT(0x1e,__int_0x1e)   /* Reserved                 */
NERREXCEPT(0x1f,__int_0x1f)   /* Reserved                 */


/*
 * __int is the stub for the unimplemented interrupt handler (stack dump)
 */

.align 16,0x90
__int:
    SAVEREGS                            /* Save the registers */
    pushl %ds                           /* Save %ds           */
    pushl %ss                           /* Switch to kernel   */
    popl %ds                            /* data segment       */
    cld                                 /* GCC likes this     */
    call CKunImplIntr                   /* Handle the int     */
    popl %ds                            /* Restore %ds        */
    RESTOREREGS                         /* Restore the regs   */
    addl $8,%esp                        /* Remove intr number */
    iret                                /* Return from intr.  */


/*
 * CKtimerService is the stub for the timer service, which is hooked to
 * the scheduler.  This is int $0x68
 */

.globl CKtimerService
.align 16,0x90
CKtimerService:
    pushl %ds                /* Save the user data segment       */
    SAVECREGS                /* Save the registers               */
    movb $0x20,%al           /* Send EOI to the master PIC so    */
    outb %al,$0x20           /* other IRQs can be triggered      */
    pushl %ss                /* Set up the data segment          */
    popl %ds                 /* Note that %ss is always valid    */
    movl $0x1,%eax           /* Check whether the scheduler busy */
    xchgl %eax,schedLock     /* Is it busy ?                     */
    testl %eax,%eax
    jnz 2f                   /* Yup, ret to previous scheduler   */
    incb schedLock           /* Mark the scheduler busy          */
    cld                      /* GCC likes this                   */
    call CKschedule          /* Call the scheduler               */
    orl %eax,%eax            /* Is there a pending signal ?      */
    jnz 1f                   /* Yes, handle it                   */
2:  RESTORECREGS             /* Restore registers                */
    popl %ds                 /* Restore the user data segment    */
    iret                     /* Exit interrupt                   */
1:  sti                      /* Unlock the processor             */
    SAVEXREGS                /* Save the remaining registers     */
    call CKsignal            /* Handle the signal                */


/*
 * CKfinishSignal is the stub for the signal termination interrupt, which
 * is hooked to CKendOfSignal().  This is int $0x6a
 *
 * CKfinishSignal has a DPL of 1, so we don't have to worry about a buggy
 * user program calling us.  Also, I chose int $0x6a for this int because
 * it was reserved by the CK anyway, but wasn't used (cascade IRQ doesn't
 * actually ever generate any interrupts.)  I don't think this should ever
 * become a problem.
 *
 * NOTE:  the movl 0x10(%esp),%esp; instruction is currently not strictly
 * correct, because our segments don't have the same base linear address.
 * (Conversion with KRNL().) We don't have any problems with it currently,
 * though, because of the identity mapping, and the problem will disappear
 * in the future once the ABL is up and running and the segments have been
 * kicked around once more.
 */

.globl CKfinishSignal
.align 16,0x90
CKfinishSignal:
    pushl %ds                /* Save the user data segment       */
    pushl %ss                /* Set up the data segment          */
    popl %ds                 /* Note that %ss is always valid    */
    SAVECREGS                /* Save the registers               */
    cld                      /* GCC likes this                   */
    call CKendOfSignal       /* Call the EOS function            */
    orl %eax,%eax            /* Do we need to create new frame ? */
    jz 1f                    /* Yes, create one                  */
/* Restore previous frame */
    popl %ecx                /* Restore registers %ecx and %edx  */
    popl %edx                /* %eax was saved on previous stack */
    movl %eax,%esp           /* Move to the previous stack frame */
    popl %eax                /* Restore previous %eax            */
    popl %ds                 /* Restore the user data segment    */
    iret                     /* Exit interrupt                   */
/* Create a new frame */
1:  sti                      /* Unlock the processor             */
    SAVEXREGS                /* Save the remaining registers     */
    call CKsignal            /* Handle the signal                */


/*
 * CKsaveFPUContext is the stub for the FPU context updator;  it is
 * invoked whenever we try to use the FPU while the FPU context does
 * not match the processor context.  This is int $0x07
 */

.globl CKsaveFPUContext
.align 16,0x90
CKsaveFPUContext:
    SAVECREGS                /* Save the registers               */
    pushl %ds                /* Save the user data segment       */
    pushl %ss                /* Set up the data segment          */
    popl %ds                 /* Note that %ss is always valid    */
    cld                      /* GCC likes this                   */
    call CKi387SetupContext  /* Switch the FPU context           */
    popl %ds                 /* Restore the user data segment    */
    RESTORECREGS             /* Restore registers                */
    iret                     /* Exit interrupt                   */
