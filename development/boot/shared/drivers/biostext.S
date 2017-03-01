/*
 * biostext.S:  BIOS keyboard driver for GRUB and the ABL
 *
 * Mostly (C) 1996 Erich Boleyn <erich@uruk.org>
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
 * getkey()
 * BIOS call "INT 16H Function 00H" to read character from keyboard
 *      Call with       %ah = 0x0
 *      Return:         %ah = keyboard scan code
 *                      %al = ASCII character
 */

ENTRY(getkey)
        push    %ebp
        push    %ebx                    /* save %ebx */

        call    prot_to_real
        .code16

        int     $0x16

        movw    %ax, %bx                /* real_to_prot uses %eax */

        data32
        call    real_to_prot
        .code32

        movw    %bx, %ax

        pop     %ebx
        pop     %ebp
        ret


/*
 * checkkey()
 *      if there is a character pending, return it; otherwise return -1
 * BIOS call "INT 16H Function 01H" to check whether a character is pending
 *      Call with       %ah = 0x1
 *      Return:
 *              If key waiting to be input:
 *                      %ah = keyboard scan code
 *                      %al = ASCII character
 *                      Zero flag = clear
 *              else
 *                      Zero flag = set
 */
ENTRY(checkkey)
        push    %ebp
        push    %ebx

        xorl    %ebx, %ebx

        call    prot_to_real            /* enter real mode */
        .code16

        movb    $0x1, %ah
        int     $0x16

        data32
        jz      notpending
        movw    %ax, %bx
        data32
        jmp     pending

notpending:
        movl    $0xFFFFFFFF, %ebx

pending:
        data32
        call    real_to_prot
        .code32

        mov     %ebx, %eax

        pop     %ebx
        pop     %ebp
        ret

