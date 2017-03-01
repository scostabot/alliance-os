/*
 * entry.S:  Entry code for the ABL
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 09/01/99  ramon       1.0    First release
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
 
.text

/*
 * This is the entry point of the ABL.  It is where the bootloader
 * jumps when it passes control to the kernel.  Our job is to get
 * the system into such a state that we can run C code properly.
 * This code also contains the Multiboot header for the loader.
 *
 * We do not bother with setting up paging or anything of the kind.
 * That will be done by the CK when it initialises.  We set up our
 * stack, clear the flags, and then pass on the multiboot info
 * structure to the main ABL code.  We also relocate the realmode
 * code into lower memory.
 */

#define STACK 4096                   /* The ABL stack size is 4k   */

.globl CK                            /* The CK entry point         */

.type  _start,@function              /* The ABL entry point        */
.globl _start
_start:
    lgdtl (loadgdt)                  /* Load our own GDT           */
    movw $0x10,%dx                   /* Init data segment regs     */
    movw %dx,%ds                     /* with new data segment      */
    movw %dx,%es
    movw %dx,%fs
    movw %dx,%gs
    movw %dx,%ss                     /* ... and the stack seg, too */
    movl $1f,%esp                    /* Set up the stack           */

    ljmp $0x08,$1f                   /* Skip the mulitboot header  */

.p2align 2                           /* The multiboot header:      */
    .long  0x1BADB002                /* Multiboot magic number     */
    .long  0x00000000                /* Multiboot flags            */
    .long  0-0x1BADB002              /* Multiboot checksum         */

.p2align 2                           /* The ABL stack              */
    .fill  STACK                     /* Reserve the room           */

1:
    pushl $2                         /* Clear the flags            */
    popf

    xorl %ebp,%ebp                   /* Clear the stack frame ptr  */

    pushl %ebx                       /* Pass multiboot info and    */
    pushl %eax                       /* magic number to ABL main   */

/*
 * Now we do a few things that are needed to execute the realmode code
 * in libsharedboot.a.  The realmode code needs to be relocated into
 * lower memory to address _rmtext.  The realmode code template is
 * part of the ABL image, from _rmtempl to _etext.
 */

    movl $_etext, %ecx               /* Calculate the amount of    */
    subl $_rmtempl, %ecx             /* code to copy into lower    */
    incl %ecx                        /* memory                     */
    movl $_rmtempl, %esi             /* Set the source index       */
    movl $_rmtext,  %edi             /* Set the destination index  */
    cld                              /* Clear the direction flag   */
    rep                              /* Copy the realmode code to  */
    movsb                            /* its destination in low mem */

    movl $1b, protstack              /* Record the PM stack loc.   */

    call  ABLmain                    /* Call ABLmain()             */

    pushl %eax                       /* Shut down the floppy motor */
    movw $0x3f2, %dx                 /* Keeping it on is annoying  */
    xorb %al, %al
    outb %al, %dx
    popl %eax

    cli                              /* Jump to CK with ints clear */

    movl (CK),%ebx                   /* Check that we got a        */
    orl %ebx,%ebx                    /* correct entry point.       */
    jnz 1f                           /* If so, jump to entry point */
2:  jmp 2b

1:                                   /* Jump to CK entry point     */
    .byte 0xea                       /* Opcode:  ljmp              */
CK: .long 0x00                       /*          entry point of CK */
    .word 0x08                       /*          CK code segment   */

/* We never reach this point */
