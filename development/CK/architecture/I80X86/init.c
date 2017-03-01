/*
 * init.c:  Hardware initialisation functions
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 21/11/98  jens        1.1    Fixed bug that prevented paging from working
 * 26/12/98  ramon       1.2    Made stack dump more accurate
 * 07/05/99  ramon       1.3    Added dumping of original stack to stack dump
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
#include "sys/sys.h"           /* Function prototypes                   */
#include "sys/8259.h"          /* 8259 interrupt controller definitions */
#include "sys/gdt.h"           /* Descriptor definitions                */
#include "sys/sysdefs.h"
#include "sys/ioport.h"
#include "sys/regs.h"
#include "sys/jump.h"
#include "sys/exceptions.h"


extern VOID CKinit8259(VOID);

extern union DTEntry IDT[];

PUBLIC struct
{ 
    UWORD16       limit __attribute__ ((packed));
    union DTEntry *idt  __attribute__ ((packed)); 
} loadidt= { (256 * sizeof(union DTEntry) - 1), IDT };


PUBLIC VOID CKunImplIntr(UWORD32 ds, struct regs regs, UWORD32 intr,
    UWORD32 error, UWORD32 eip, UWORD32 cs, UWORD32 eflags, UWORD32 oesp, UWORD32 oss)
/*
 * The handler for unimplemented exceptions
 * Does a stack dumplet and hangs the computer
 *
 * INPUT:
 *     intr:  Interrupt number
 *     (the other parameters are pushed onto the stack by the x86)
 *
 * RETURNS:
 *     none
 */
{
    UWORD32 *esp = 0, ss = 0;

    /* Find the correct stack location */
    if(cs == KCODE) {
        esp = &oesp;
        asm( "movw %%ss,%0" : "=r" ((UWORD16)ss) );
    } else {
        esp = (UWORD32 *) oesp;
        ss  = oss;
    }

    /* Print exception type */
    CKprint("\nUnimplemented exception:  0x%x", intr);
    switch(intr) {
        case 0x00:
            CKprint(" (divide by zero)");
            break;
        case 0x01:
            CKprint(" (debug exception)");
            break;
        case 0x02:
            CKprint(" (NMI)");
            break;
        case 0x03:
            CKprint(" (breakpoint)");
            break;
        case 0x04:
            CKprint(" (INTO overflow)");
            break;
        case 0x05:
            CKprint(" (BOUND limit exceeded)");
            break;
        case 0x06:
            CKprint(" (invalid opcode)");
            break;
        case 0x08:
            CKprint(" (double fault)");
            break;
        case 0x09:
            CKprint(" (coprocessor segment overrun)");
            break;
        case 0x0a:
            CKprint(" (invalid TSS)\n");
            CKprint("Error 0x%p", error);
            break;
        case 0x0b:
            CKprint(" (segment not present)\n");
            CKprint("Error 0x%p", error);
            break;
        case 0x0c:
            CKprint(" (stack exception)\n");
            CKprint("Error 0x%p", error);
            break;
        case 0x0d:
            CKprint(" (general protection exception)\n");
            CKprint("Error 0x%p", error);
            break;
        case 0x0e:
            CKprint(" (page fault)\n");
            CKprint("Error 0x%p", error);
            break;
        case 0x0f:
            CKprint(" (coprocessor error)");
            break;
        default:
            break;
    }

    /* Print exception location */
    CKprint(" at %%eip = 0x%p", eip);

    /* On page fault, print page fault linear address */
    if(intr == 0xe) {  /* Page fault */
        UWORD32 pflinear;
        asm("movl %%cr2,%0":"=g" (pflinear));
        CKprint(", page fault at linear 0x%p", pflinear);
    }
    CKprint("\n");

    /* Print register dump */
    CKprint("%%eax = 0x%p  %%ebx = 0x%p  %%ecx = 0x%p  %%edx = 0x%p\n",
            regs.eax, regs.ebx, regs.ecx, regs.edx);
    CKprint("%%esp = 0x%p  %%ebp = 0x%p  %%esi = 0x%p  %%edi = 0x%p\n",
            esp, regs.ebp, regs.esi, regs.edi);
    CKprint("%%cs  = 0x%p  %%ds  = 0x%p  %%ss  = 0x%p  %%eflags = 0x%p\n",(cs&0xffff),ds,(ss&0xffff),eflags);
#if 0
    /* Dump original stack */
    CKprint("\nOriginal stack:\n");
    CKprint("0x00(%esp)    0x%p   0x%p   0x%p   0x%p   0x%p\n", esp[0], esp[1], esp[2], esp[3], esp[4]);
    CKprint("0x14(%esp)    0x%p   0x%p   0x%p   0x%p   0x%p\n", esp[5], esp[6], esp[7], esp[8], esp[9]);
    CKprint("0x28(%esp)    0x%p   0x%p   0x%p   0x%p   0x%p\n", esp[10], esp[11], esp[12], esp[13], esp[14]);
    CKprint("0x3c(%esp)    0x%p   0x%p   0x%p   0x%p   0x%p\n", esp[15], esp[16], esp[17], esp[18], esp[19]);
    CKprint("0x50(%esp)    0x%p   0x%p   0x%p   0x%p   0x%p\n", esp[20], esp[21], esp[22], esp[23], esp[24]);
#endif
    CKprint("\nFatal:  halting.  Please reboot...");

    for(;;);   /* Hang the computer (ints are off) */
}


LOCAL VOID CKinitIDT(VOID)
/*
 * Initialise the IDT with the standard interrupt handlers
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKinitExceptionStubs();
    CKinitLocalForwards();

    asm (
        "lidt loadidt              \n"   /* Load the IDT              */
        "pushfl                    \n"   /* Clear the NT flag         */
        "andl $0xffffbfff,(%esp)   \n"
        "popfl                     \n"
    );
}


LOCAL VOID CKhwInitIntr(VOID)
/*
 * Initialise the interrupt subsystem
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKinit8259();                   /* Init the 8259 intr controller    */
    CKinitIDT();                    /* Initialize the interrupts        */
}


PUBLIC VOID CKinitHardware(VOID)
/*
 * Initialise the system hardware
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKhwInitMem();                  /* Initialise paging                */
    CKhwInitIntr();                 /* Initialise the interrupt hardwr  */
    CKinitCons();                   /* Init the VGA console             */
    CKinitDebugger();               /* Initialise the CK debugger       */
}

