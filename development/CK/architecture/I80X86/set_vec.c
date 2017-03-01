/*
 * set_vec.c  Setup one vector in the IDT
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
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
#include "sys/gdt.h"
#include "sys/sysdefs.h"

extern union DTEntry IDT[];  /* the Interrupt Descriptor Table  */


PUBLIC VOID CKsetVector(VOID *handler, UBYTE interrupt, UWORD16 control_major)
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
    IDT[interrupt].gate.offset_low    = (UWORD16)(((UWORD32)handler)&0xffff);
    IDT[interrupt].gate.selector      = KCODE;
    IDT[interrupt].gate.access        = control_major;
    IDT[interrupt].gate.offset_high   = (UWORD16)(((UWORD32)handler)>>16);
}

