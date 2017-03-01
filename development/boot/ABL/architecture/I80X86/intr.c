/*
 * intr.c:  Interrupt and IRQ code
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 04/02/99  ramon       1.1    Modified for ABL
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
 */

#include <typewrappers.h>
#include "sys/8259.h"
#include "sys/gdt.h"
#include "sys/ioport.h"


extern VOID ABLnoIntr(VOID);


PUBLIC descTable(IDT, 256) {
};


PUBLIC VOID ABLsetVector(VOID *handler, UBYTE interrupt, UWORD16 control_major)
/*
 * Set up a vector in the IDT
 *
 * INPUT:
 *     handler:        Address of ISR
 *     interrupt:      Interrupt number
 *     control_major:  Major control byte
 *                     usually (D_INT  + D_PRESENT)
 *                     or      (D_TRAP + D_PRESENT)
 *
 * RETURNS:
 *     none
 */
{
    UWORD16 sel;

    asm( "movw %%cs,%%ax" : "=a" (sel) );

    IDT[interrupt].gate.offset_low    = (UWORD16)(((UWORD32)handler)&0xffff);
    IDT[interrupt].gate.selector      = sel;
    IDT[interrupt].gate.access        = control_major;
    IDT[interrupt].gate.offset_high   = (UWORD16)(((UWORD32)handler)>>16);
}


LOCAL struct
{
    UWORD16       limit __attribute__ ((packed));
    union DTEntry *idt  __attribute__ ((packed));
} loadidt= { (256 * sizeof(union DTEntry) - 1), IDT };


/* The IRQ masks contain which IRQ's are enabled.  Initially, none. */
LOCAL UBYTE IRQMaskMaster = 0xfb;
LOCAL UBYTE IRQMaskSlave  = 0xff;


PUBLIC VOID ABLdisableIRQ(UDATA irq)
/*
 * Disable an IRQ from the i8259
 *
 * INPUT:
 *     irq:  IRQ to disable
 *
 * RETURNS:
 *     none
 */
{
    if(irq < 8) {                           /* If IRQ is on master PIC  */
        IRQMaskMaster |= (1 << irq);        /* Set IRQ bit in mask      */
        outportb(M_IMR, IRQMaskMaster);     /* Send mask to master PIC  */
        return;                             /* We're done !             */
    }
                                            /* ... else, IRQ on slave   */
    IRQMaskSlave |= (1 << (irq & 7));       /* Set IRQ bit in mask      */
    outportb(S_IMR, IRQMaskSlave);          /* Send mask to slave PIC   */
}


PUBLIC VOID ABLenableIRQ(UDATA irq)
/*
 * Enable an IRQ from the i8259
 *
 * INPUT:
 *     irq:  IRQ to enable
 *
 * RETURNS:
 *     none
 */
{
    if(irq < 8) {                           /* If IRQ is on master PIC  */
        IRQMaskMaster &= ~(1 << irq);       /* Unset IRQ bit in mask    */
        outportb(M_IMR, IRQMaskMaster);     /* Send mask to master PIC  */
        return;                             /* We're done !             */
    }
                                            /* ... else, IRQ on slave   */
    IRQMaskSlave &= ~(1 << (irq & 7));      /* Unset IRQ bit in mask    */
    outportb(S_IMR, IRQMaskSlave);          /* Send mask to slave PIC   */
}


PUBLIC VOID ABLinitIntr(VOID)
/*
 * Initialise the ABL interrupt system
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    DATA i;

    outportb(M_PIC, ICW1);           /* Start 8259 initialization    */
    outportb(S_PIC, ICW1);

    outportb(M_PIC+1, M_VEC);        /* Set base interrupt vector    */
    outportb(S_PIC+1, S_VEC);

    outportb(M_PIC+1, 1<<2);         /* Bitmask for cascade on IRQ 2 */
    outportb(S_PIC+1, 2);            /* Cascade on IRQ 2             */

    outportb(M_PIC+1, ICW4);         /* Finish 8259 initialization   */
    outportb(S_PIC+1, ICW4);

    outportb(M_IMR, IRQMaskMaster);  /* Mask all interrupts          */
    outportb(S_IMR, IRQMaskSlave);

    for(i=0;i<256;i++)
        ABLsetVector(ABLnoIntr, i, (D_INT+D_PRESENT));

    asm (
        "lidt (%0)                 \n"   /* Load the IDT              */
        "pushfl                    \n"   /* Clear the NT flag         */
        "andl $0xffffbfff,(%%esp)  \n"
        "popfl                     \n"
        "sti                       \n"
        :
        : "r" ((UBYTE *) &loadidt)
    );
}


PUBLIC UDATA missedInts = 0;

asm (
    ".globl ABLnoIntr     \n"
    "ABLnoIntr:           \n"
    "    incl missedInts  \n"
    "    iret             \n"
);
