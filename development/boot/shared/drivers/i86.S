/*
 * i86.S:  Generic ix86 processor support for GRUB and the ABL
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 * Portions (C) 1996 Erich Boleyn <erich@uruk.org>
 * Portions (C) 1999 Gordon Matzigkeit, Free Software Foundation, Inc.
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 19/06/99  ramon       1.0    First release
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

#define ASM_FILE
#include "shared.h"

/* Mark this code so it gets linked and relocated into lower memory */
.section .rmtext, "ax", @progbits 

/*
 *  These next two routines, "real_to_prot" and "prot_to_real" are structured
 *  in a very specific way.  Be very careful when changing them.
 *
 *  NOTE:  Use of either one messes up %eax and %ebp.
 */

ENTRY(real_to_prot)
        .code16
        cli

        /* turn on protected mode */
        movl    %cr0, %eax
        orl     $CR0_PE_ON, %eax
        movl    %eax, %cr0

        /* jump to relocation, flush prefetch queue, and reload %cs */
        data32
        ljmp    $PROT_MODE_CSEG, $protcseg

protcseg:
        .code32

        /* reload other segment registers */
        movw    $PROT_MODE_DSEG, %ax
        movw    %ax, %ds
        movw    %ax, %es
        movw    %ax, %fs
        movw    %ax, %gs
        movw    %ax, %ss

        /* put the return address in a known safe location */
        movl    (%esp), %eax
        movl    %eax, STACKOFF

        /* get protected mode stack */
        movl    protstack, %eax
        movl    %eax, %esp
        movl    %eax, %ebp

        /* Restore the flags from the stack */
        popfl

        /* get return address onto the right stack */
        movl    STACKOFF, %eax
        movl    %eax, (%esp)

        /* zero %eax */
        xorl    %eax, %eax

        /* return on the old (or initialized) stack! */
        ret


ENTRY(prot_to_real)
        /* get the return address */
        movl    (%esp), %eax
        movl    %eax, STACKOFF

        /* Save the flags and clear interrupts */
        pushfl
        cli

        /* save the protected mode stack */
        movl    %esp, %eax
        movl    %eax, protstack

        /* set up new stack */
        movl    $STACKOFF, %eax
        movl    %eax, %esp
        movl    %eax, %ebp

        /* set up segment limits */
        movw    $PSEUDO_RM_DSEG, %ax
        movw    %ax, %ds
        movw    %ax, %es
        movw    %ax, %fs
        movw    %ax, %gs
        movw    %ax, %ss

        /* this might be an extra step */
        ljmp    $PSEUDO_RM_CSEG, $tmpcseg       /* jump to a 16 bit segment */

tmpcseg:
        .code16

        /* clear the PE bit of CR0 */
        movl    %cr0, %eax
        andl    $CR0_PE_OFF, %eax
        movl    %eax, %cr0

        /* flush prefetch queue, reload %cs */
        data32
        ljmp    $0, $realcseg

realcseg:
        /* we are in real mode now
         * set up the real mode segment registers : DS, SS, ES
         */
        /* zero %eax */
        xorl    %eax, %eax

        movw    %ax, %ds
        movw    %ax, %es
        movw    %ax, %fs
        movw    %ax, %gs
        movw    %ax, %ss

        /* restore interrupts */
        sti

        /* return on new stack! */
        data32
        ret

        .code32


/*
 *  This is the area for all of the special variables.
 */

        .p2align        2       /* force 4-byte alignment */

.globl protstack
protstack:
        .long   PROTSTACKINIT-0x4

