/*
 * irq.c:  IRQ functions
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 11/01/99  ramon       1.1    Added IRQ masking functions
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
#include "sys/ioport.h"


/* The IRQ masks contain which IRQ's are enabled.  Initially, none. */
LOCAL UBYTE IRQMaskMaster = 0xfb;
LOCAL UBYTE IRQMaskSlave  = 0xff;


PUBLIC VOID CKdisableIRQ(UDATA irq)
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


PUBLIC VOID CKenableIRQ(UDATA irq)
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


PUBLIC VOID CKinit8259(VOID)
/*
 * Initialise the 8259 Programmable Interrupt Controller
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    outportb(M_IMR, IRQMaskMaster);  /* Mask all interrupts          */
    outportb(S_IMR, IRQMaskSlave);
}
