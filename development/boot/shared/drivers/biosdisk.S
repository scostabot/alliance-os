/*
 * biosdisk.S:  BIOS hard disk/floppy disk driver for GRUB and the ABL
 *
 * Portions (C) 1996 Erich Boleyn <erich@uruk.org>
 * Portions (C) 1999 Gordon Matzigkeit, Free Software Foundation, Inc.
 * Portions (C) 1999 Ramon van Handel, The Alliance Operating System Team
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
 *
 * get_diskinfo(drive):  return a word that represents the
 *      max number of sectors and heads and cylinders for this drive
 *
 */

ENTRY(get_diskinfo)
        pushl   %ebp
        movl    %esp, %ebp
        pushl   %ebx
        pushl   %ecx
        pushl   %edx
        pushl   %edi
        pushl   %esi

        movw    0x8(%ebp), %dx          /* diskinfo(drive #) */

        call    prot_to_real            /* enter real mode   */
        .code16

        movb    %dl, %al
        andb    $0x80, %al
        data32
        jnz     hard_drive

/*
 *  Perform floppy probe!
 */

        movl    $probe_values-1, %esi

probe_loop:
        /* reset floppy controller INT 13h AH=0 */
        xorw    %ax, %ax
        int     $0x13

        incw    %si
        movb    (%si), %cl

        /* if number of sectors is 0, display error and die */
        cmpb    $0, %cl

        data32
        je      probe_failed

        /* perform read */
        movw    $SCRATCHSEG, %ax
        movw    %ax, %es
        xorw    %bx, %bx
        movw    $0x201, %ax
        movb    $0, %ch
        movb    $0, %dh
        int     $0x13

        data32
        jc      probe_loop

        /* %cl is already the correct value! */
        movb    $1, %dh
        movb    $79, %ch
        xorb    %bh, %bh

        data32
        jmp     probe_success

probe_values:
        .byte   36, 18, 15, 9, 0

hard_drive:
        /* Check for LBA. */
        movb    $0x41, %ah
        movw    $0x55aa, %bx
        int     $0x13                   /* int 13 extensions install check */

        data32
        jc      1f                      /* invalid function */
        cmpw    $0xaa55, %bx
        data32
        jne     1f                      /* failed magic */

        andb    $1, %cx
        data32
        jz      1f                      /* LBA not supported */

        /* Wahoo!  Got LBA! */
        movb    $0x1, %bh
        data32
        jmp     2f

1:      xorb    %bh, %bh                /* Too bad, no LBA */
2:      movb    $0x8, %ah               /* ask for disk info */
        int     $0x13

        data32
        jc      probe_failed

        /* es:di = parameter table */
        /* carry = 0 */

probe_success:
        /*
         * form a longword representing all this gunk:
         *       4 bit BIOS extension (0 = none, 1 = LBA)
         *      12 bit cylinder
         *       8 bit head
         *       8 bit sector
         */
        movb    %bh, %ah                /* restore BIOS extensions bits */
        shlw    $2, %ax                 /* << 2 */
        movb    %cl, %al                /* bits 8,9 of cylinder count */
        andb    $0xc0, %al
        shlw    $2, %ax                 /* << 2 */
        movb    %ch, %al                /* Lower 8 bits */
        shll    $16, %eax               /* << 16 */
        movb    %dh, %ah                /* max head */
        andb    $0x3f, %cl              /* mask off cylinder gunk */
        movb    %cl, %al                /* max sector (and # sectors) */
        movl    %eax, %ebx              /* save return value */

        data32
        jmp     got_drive

probe_failed:
        /*
         * Urk.  Call failed.  It is not supported for floppies by old
         * BIOSes, but it should work for all hard drives!!
         *
         * Return a 0 here...  presume there is no drive present.  ????
         */

        movl    $0, %ebx                /* not present value */

got_drive:
        data32
        call    real_to_prot            /* back to protected mode */
        .code32

        /* set up return in correct register */
        movl    %ebx, %eax

        popl    %esi
        popl    %edi
        popl    %edx
        popl    %ecx
        popl    %ebx
        popl    %ebp

        ret


/*
 * biosdisk(subfunc, drive, geometry, sector, nsec, segment)
 *      Read/write "nsec" sectors from disk to real segment "segment"
 *              offset zero
 *
 *   If it will fit into the BIOS geometry, it tries the INT 0x13
 *   AH=<subfunction> interface, else if it is a hard disk, it will
 *   try the hard disk LBA calls.  If these won't work, or otherwise it is
 *   out of the translated area, then it returns BIOS_GEOMETRY_ERROR.
 */

ENTRY(biosdisk)
        push    %ebp
        mov     %esp, %ebp

        push    %ebx
        push    %ecx
        push    %edx
        push    %esi
        push    %edi
#if 0
        /*
         * This part of biosdisk() checks whether hard-disk LBA calls are
         * supported, and if so they are used to access the disk.  If they
         * aren't supported or don't succeed, it falls through to normal
         * BIOS calls.
         * This code was taken from Gordon Matzigkeit's GNU GRUB.  It
         * doesn't work though.  I'll debug it sometime when I have time;
         * it's not really urgent though (but nice to have for big disks.)
         */

        /* Check whether we have LBA. */
        movb    0x13(%ebp), %al
        andb    $0x10, %al
        jz      disk_compute_args       /* nope. */

        /* set up disk address packet for extended calls (LBA mode) */
        movl    0x14(%ebp), %eax        /* absolute block number */
        movl    %eax, dap_block
        xorl    %eax, %eax
        movl    %eax, dap_block + 4

        movb    0x18(%ebp), %al         /* number of sectors */
        movw    %ax, dap_blocks

        /* set up the buffer address */
        xorl    %eax, %eax
        movw    0x1c(%ebp), %ax         /* segment */
        shll    $16, %eax
        movl    %eax, dap_buffer

        /* compute the address of disk_address_packet */
        movl    $disk_address_packet, %eax
        movw    %ax, %si
        xorw    %ax, %ax
        shrl    $4, %eax
        movw    %ax, %cx                /* save the segment to cx */

        xorb    %bl, %bl
        movb    0x8(%ebp), %bh          /* read=0, write=1 */
        addb    $0x42, %bh              /* convert to extended subfunction */

        movb    0xc(%ebp), %dl          /* drive */

        movl    %ebp, %edi              /* Save the stack frame address */

        call    prot_to_real            /* enter real mode */
        .code16

        movw    %cx, %ds
        movw    %bx, %ax
        int     $0x13                   /* do the operation */

        /* set success return value */
        movb    $0, %bl

        /* did we actually succeed? */
        data32
        jnc     disk_exit_16

        data32
        call    real_to_prot            /* back to protected mode */
        .code32

        movl    %edi, %ebp              /* Restore the stack frame address */
        jmp     disk_compute_args       /* Fallback to standard BIOS use */

        .align  4
disk_address_packet:
        .byte   0x10                    /* length of packet */
        .byte   0                       /* reserved */
dap_blocks:
        .word   0                       /* number of blocks */
dap_buffer:
        .long   0                       /* buffer address */
dap_block:
        .long   0                       /* absolute block number */
        .long   0
#endif
        /* either failed, or a floppy, so try standard BIOS calls */

        /*
         * "geometry" is a longword representing the BIOS geometry:
         *       4 bit BIOS extension (0 = none, 1 = LBA)
         *      12 bit cylinder
         *       8 bit head
         *       8 bit sector
         */

disk_compute_args:
        /* set up original sector number */
        xorl    %edx, %edx
        movl    0x14(%ebp), %eax

        /* get sector offset, place in %ecx */
        xorl    %ebx, %ebx
        movb    0x10(%ebp), %bl
        divl    %ebx
        movl    %edx, %ecx

        /* get track offset (head number) in %edx,
           and cylinder offset in %eax */
        xorl    %edx, %edx
        xorl    %ebx, %ebx
        movb    0x11(%ebp), %bl
        inc     %ebx
        divl    %ebx

        /* check cylinder offset, is there a geometry problem here? */
        movl    0x10(%ebp), %ebx
        shrl    $16, %ebx
        andb    $0xf, %bh               /* mask out bios extension code */
        cmpl    %ebx, %eax

        /* if not, go onto standard read function */
        jle     disk_use_standard_bios

        movl    $BIOSDISK_ERROR_GEOMETRY, %eax
        jmp     disk_exit_32

        /*
         *  This portion implements the BIOS standardized
         *  INT 0x13/AH=<subfunc> interface.
         */
disk_use_standard_bios:

        shll    $8, %edx                /* get head number to %dh */
        xchgl   %eax, %ecx              /* cylinder to %cx, sector to %al */
        /* cylinder; the highest 2 bits of cyl is in %cl */
        xchgb   %ch, %cl
        shlb    $6, %cl
        incb    %al                     /* sector; sec starts from 1, not 0 */
        orb     %al, %cl
        movb    0xc(%ebp), %dl          /* drive */

        /* prot_to_real will set %es to 0, so must set it ourselves
           after it is called */
        movb    0x18(%ebp), %bl         /* number of sectors */
        movb    0x8(%ebp), %bh          /* bios disk subfunction */
        addb    $0x2, %bh               /* convert to subfunction */
        shll    $16, %ebx               /* shift num sect. and subfunction */
        movw    0x1c(%ebp), %bx         /* segment */

        call    prot_to_real            /* enter real mode */
        .code16

        movw    %bx, %es                /* load segment */
        /*
         *  Shift num sectors and subfunction back
         */
        data32
        shrl    $16, %ebx
        pushw   %bx                     /* save num sectors */
        xorw    %bx, %bx

        movw    $3, %si

disk_loop:
        popw    %ax                     /* restore num sectors */
        pushw   %ax                     /* save num sectors */

        /* BIOS call for reading/writing */
        int     $0x13

        /* set success return value */
        movb    $0, %bl

        /* did we actually succeed? */
        data32
        jnc     disk_exit_16

        /* do we try again? */
        decw    %si
        cmpw    $0, %si

        /* if this isn't the third try, go again */
        data32
        jne     disk_loop

        /* save return value */
        movb    %ah, %bl

disk_exit_16:
        data32
        call    real_to_prot            /* back to protected mode */
        .code32

        movb    %bl, %al                /* return value in %eax */

disk_exit_32:
        pop     %edi
        pop     %esi
        pop     %edx
        pop     %ecx
        pop     %ebx
        pop     %ebp

        ret


/*
 * stop_floppy()
 *
 * Stops the floppy drive from spinning, so that other software is
 * jumped to with a known state.
 */
ENTRY(stop_floppy)
        movw    $0x3F2, %dx
        xorb    %al, %al
        outb    %al, %dx
        ret

