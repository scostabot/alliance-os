/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1996   Erich Boleyn  <erich@uruk.org>
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

	.file	"asm.S"
	.code16

ENTRY(_start)
	/*
	 *  "start" should be loaded at 0x0:$STARTADDR.  This next
	 *  is to guarantee that.
	 */

	/* ljmp $0, $codestart */
	ljmp $0, $(codestart - _start + 0x8000)

	/*
	 *  Compatibility version number
	 *
	 *  These MUST be at byte offset 6 and 7 of the executable
	 *
	 *  DO NOT MOVE !!!
	 */
	. = _start + 0x6
	.byte	COMPAT_VERSION_MAJOR, COMPAT_VERSION_MINOR

	/*
	 *  This is a special data area 8 bytes from the beginning.
	 */

	. = _start + 0x8

VARIABLE(install_partition)
	.long	0x020000
VARIABLE(version_string)
	.string VERSION
VARIABLE(config_file)
#ifndef CONFIG_FILE_ASM
	.string "/boot/grub/menu.lst"
#else   /* CONFIG_FILE_ASM */
	CONFIG_FILE_ASM
#endif  /* CONFIG_FILE_ASM */

	/*
	 *  Leave some breathing room for the config file name.
	 */

	. = _start + 0x70

/* the code continues... */
codestart:
	cli		/* we're not safe here! */

	/* set up %ds, %ss, and %es */
	xorw	%ax, %ax
	movw	%ax, %ds
	movw	%ax, %ss
	movw	%ax, %es

	/* set up the real mode/BIOS stack */
	movl	$STACKOFF, %ebp
	movl	%ebp, %esp

	sti		/* we're safe again */

	/* save boot drive reference */
	addr32
	movb	%dl, boot_drive

	/* reset disk system (%ah = 0) */
	int	$0x13

	/* setup initial protected mode flags */
	movl $((PROTSTACKINIT-0x4)&0xffff),%edi
	movw $(((PROTSTACKINIT-0x4)&~0xffff)>>4),%ax
	movw %ax,%es
	addr32
	data32
	movl $0,%es:(%edi)

        /* load the GDT register */
        addr32
        data32
        lgdt    gdtdesc

	/* transition to protected mode */
	data32
	call real_to_prot
	.code32

	/*
	 *  Call the start of main body of C code, which does some
	 *  of it's own initialization before transferring to "cmain".
	 */
	call init_bios_info


/*
 *  This call is special...  it never returns...  in fact it should simply
 *  hang at this point!
 */

ENTRY(stop)
	call	prot_to_real

	/*
	 * This next part is sort of evil.  It takes advantage of the
	 * byte ordering on the x86 to work in either 16-bit or 32-bit
	 * mode, so think about it before changing it.
	 */

ENTRY(hard_stop)
	hlt
	jmp hard_stop


/*
 * chain_stage1(segment, offset, part_table_addr)
 *
 *  This starts another stage1 loader, at segment:offset.
 */

ENTRY(chain_stage1)
	/* no need to save anything, just use %esp */

	/* store %ESI, presuming %ES is 0 */
	movl	0xc(%esp), %esi

	/* store new offset */
	movl	0x8(%esp), %eax
	movl	%eax, offset

	/* store new segment */
	movw	0x4(%esp), %ax
	movw	%ax, segment

	/* set up to pass boot drive */
	movb	boot_drive, %dl

	call	prot_to_real
	.code16

	data32
	addr32
	ljmp	(offset)

	.code32


/*
 * chain_stage2(segment, offset)
 *
 *  This starts another stage2 loader, at segment:offset.  It presumes
 *  that the other one starts with this same "asm.S" file, and passes
 *  parameters by writing the embedded install variables.
 */

ENTRY(chain_stage2)
	/* no need to save anything, just use %esp */

	/* store new offset */
	movl	0x8(%esp), %eax
	movl	%eax, offset
	movl	%eax, %ebx

	/* store new segment */
	movw	0x4(%esp), %ax
	movw	%ax, segment
	shll	$4, %eax

	/* generate linear address */
	addl	%eax, %ebx

	/* store install info */
	movl	install_partition, %eax
	movl	%eax, install_partition-_start(%ebx)

	/* set up to pass boot drive */
	movb	boot_drive, %dl

	call	prot_to_real
	.code16

	data32
	addr32
	ljmp	(offset)
	.code32


/*
 * putchar(c)   : Puts character on the screen, interpreting '\n' as in the
 *                UNIX fashion.
 *
 * BIOS call "INT 10H Function 0Eh" to write character to console
 *      Call with       %ah = 0x0e
 *                      %al = character
 *                      %bh = page
 *                      %bl = foreground color ( graphics modes)
 */


ENTRY(putchar)
	push	%ebp
	push	%eax
	push	%ebx

	movb	0x10(%esp), %bl

	/* if not '\n', just print the character */
	cmpb	$0xa, %bl
	jne	pc_notnewline

	/* if newline, print CR as well */
	pushl	$0xd
	call	putchar
	popl	%eax

pc_notnewline:
	call	prot_to_real
	.code16

	movb	%bl, %al
	movb	$0xe, %ah
	movw	$1, %bx
	int	$0x10

	data32
	call	real_to_prot
	.code32

	pop	%ebx
	pop	%eax
	pop	%ebp
	ret


/*
 *
 * get_memsize(i) :  return the memory size in KB. i == 0 for conventional
 *		memory, i == 1 for extended memory
 *	BIOS call "INT 12H" to get conventional memory size
 *	BIOS call "INT 15H, AH=88H" to get extended memory size
 *		Both have the return value in AX.
 *
 */

ENTRY(get_memsize)
	push	%ebp
	push	%ebx

	mov	0xc(%esp), %ebx

	call	prot_to_real		/* enter real mode */
	.code16

	cmpb	$0x1, %bl
	data32
	je	xext
	
	int	$0x12
	data32
	jmp	xdone

xext:
	movb	$0x88, %ah
	int	$0x15

xdone:
	movw	%ax, %bx

	data32
	call	real_to_prot
	.code32

	movw	%bx, %ax
	pop	%ebx
	pop	%ebp
	ret


/*
 *
 * get_eisamemsize() :  return packed EISA memory map, lower 16 bits is
 *		memory between 1M and 16M in 1K parts, upper 16 bits is
 *		memory above 16M in 64K parts.  If error, return -1.
 *	BIOS call "INT 15H, AH=E801H" to get EISA memory map,
 *		AX = memory between 1M and 16M in 1K parts.
 *		BX = memory above 16M in 64K parts.
 *
 */

ENTRY(get_eisamemsize)
	push	%ebp
	push	%ebx
	push	%ecx
	push	%edx

	call	prot_to_real		/* enter real mode */
	.code16

	movw	$0xe801, %ax
	int	$0x15

	shll	$16, %ebx
	movw	%ax, %bx

	data32
	call	real_to_prot
	.code32

	movl	$0xFFFFFFFF, %eax
	cmpb	$0x86, %bh
	je	xnoteisa

	movl	%ebx, %eax

xnoteisa:
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret


/*
 *
 * get_mem_map(addr, cont) :  address and old continuation value (zero to
 *		start), for the Query System Address Map BIOS call.
 *
 *  Sets the first 4-byte int value of "addr" to the size returned by
 *  the call.  If the call fails, sets it to zero.
 *
 *	Returns:  new (non-zero) continuation value, 0 if done.
 *
 * NOTE: Currently hard-coded for a maximum buffer length of 1024.
 */

ENTRY(get_mem_map)
	push	%ebp
	push	%ebx
	push	%ecx
	push	%edx
	push	%edi
	push	%esi

	/* place address (+4) in ES:DI */
	movl	0x1c(%esp), %eax
	addl	$4, %eax
	movl	%eax, %edi
	andl	$0xf, %edi
	shrl	$4, %eax
	movl	%eax, %esi

	/* set continuation value */
	movl	0x20(%esp), %ebx

	/* set default maximum buffer size */
	movl	$0x14, %ecx

	/* set EDX to 'SMAP' */
	movl	$0x534d4150, %edx

	call	prot_to_real		/* enter real mode */
	.code16

	movw	%si, %es
	movw	$0xe820, %ax
	int	$0x15

	data32
	jc	xnosmap

	cmpl	$0x534d4150, %eax
	data32
	jne	xnosmap

	cmpl	$0x14, %ecx
	data32
	jl	xnosmap

	cmpl	$0x400, %ecx
	data32
	jg	xnosmap

	data32
	jmp	xsmap

xnosmap:
	movl	$0, %ecx

xsmap:
	data32
	call	real_to_prot
	.code32

	/* write length of buffer (zero if error) into "addr" */
	movl	0x1c(%esp), %eax
	movl	%ecx, (%eax)

	/* set return value to continuation */
	movl	%ebx, %eax

	pop	%esi
	pop	%edi
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret

/*
 * gateA20(int linear)
 *
 * Gate address-line 20 for high memory.
 *
 * This routine is probably overconservative in what it does, but so what?
 *
 * It also eats any keystrokes in the keyboard buffer.  :-(
 */

ENTRY(gateA20)
	pushl	%eax

	call    gloop1

	movb	$KC_CMD_WOUT, %al
	outb	$K_CMD

gloopint1:
	inb	$K_STATUS
	andb	$K_IBUF_FUL, %al
	jnz	gloopint1

	movb	$KB_OUTPUT_MASK, %al
	cmpb	$0, 0x8(%esp)
	jz	gdoit

	orb	$KB_A20_ENABLE, %al
gdoit:
	outb	$K_RDWR

	call	gloop1

	popl	%eax
	ret

gloop1:
	inb	$K_STATUS
	andb	$K_IBUF_FUL, %al
	jnz	gloop1

gloop2:
	inb	$K_STATUS
	andb	$K_OBUF_FUL, %al
	jz	gloop2ret
	inb	$K_RDWR
	jmp	gloop2

gloop2ret:
	ret


/*
 * linux_boot()
 *
 * Does some funky things (including on the stack!), then jumps to the
 * entry point of the Linux setup code.
 */

ENTRY(linux_boot)
	/* don't worry about saving anything, we're committed at this point */
	cld	/* forward copying */

	/* copy kernel */
	movl	$LINUX_SETUP, %eax
	movl	LINUX_KERNEL_LEN_OFFSET(%eax), %ecx
	shll	$2, %ecx
	movl	$LINUX_STAGING_AREA, %esi
	movl	$LINUX_KERNEL, %edi

	rep
	movsl

ENTRY(big_linux_boot)
	/* XXX new stack pointer in safe area for calling functions */
	movl	$0x4000, %esp
	call	stop_floppy

	/* final setup for linux boot */

	movw	$LINUX_SETUP_SEG, %ax
	movw	%ax, segment

	xorl	%eax, %eax
	movl	%eax, offset

	call	prot_to_real
	.code16

	/* final setup for linux boot */
	movw	$LINUX_SETUP_STACK, %sp

	movw	$LINUX_INIT_SEG, %ax
	movw	%ax, %ss

	/* jump to start */
	data32
	addr32
	ljmp	(offset)

	.code32


/*
 * multi_boot(int start, int mbi)
 *
 *  This starts a kernel in the manner expected of the multiboot standard.
 */

ENTRY(multi_boot)
	/* no need to save anything */
	call	stop_floppy

	movl	$0x2BADB002, %eax
	movl	0x8(%esp), %ebx

	/* boot kernel here (absolute address call) */
	call	*0x4(%esp)

	/* error */
	call	stop


/*
 * cls()
 * BIOS call "INT 10H Function 0Fh" to get current video mode
 *	Call with	%ah = 0x0f
 *      Returns         %al = (video mode)
 *                      %bh = (page number)
 * BIOS call "INT 10H Function 00h" to set the video mode (clears screen)
 *	Call with	%ah = 0x00
 *                      %al = (video mode)
 */


ENTRY(cls)
	push	%ebp
	push	%eax
	push	%ebx                    /* save EBX */

	call	prot_to_real
	.code16

	movb	$0xf, %ah
	int	$0x10			/* Get Current Video mode */
        xorb	%ah, %ah
        int	$0x10                   /* Set Video mode (clears screen) */

	data32
	call	real_to_prot
	.code32

	pop	%ebx
	pop	%eax
	pop	%ebp
	ret


/*
 * getxy()
 * BIOS call "INT 10H Function 03h" to get cursor position
 *	Call with	%ah = 0x03
 *			%bh = page
 *      Returns         %ch = starting scan line
 *                      %cl = ending scan line
 *                      %dh = row (0 is top)
 *                      %dl = column (0 is left)
 */


ENTRY(getxy)
	push	%ebp
	push	%ebx                    /* save EBX */
	push	%ecx                    /* save ECX */
	push	%edx

	call	prot_to_real
	.code16

        xorb	%bh, %bh                /* set page to 0 */
	movb	$0x3, %ah
	int	$0x10			/* get cursor position */

	data32
	call	real_to_prot
	.code32

	movb	%dl, %ah
	movb	%dh, %al

	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%ebp
	ret


/*
 * gotoxy(x,y)
 * BIOS call "INT 10H Function 02h" to set cursor position
 *	Call with	%ah = 0x02
 *			%bh = page
 *                      %dh = row (0 is top)
 *                      %dl = column (0 is left)
 */


ENTRY(gotoxy)
	push	%ebp
	push	%eax
	push	%ebx                    /* save EBX */
	push	%edx

	movb	0x14(%esp), %dl          /* %dl = x */
	movb	0x18(%esp), %dh          /* %dh = y */

	call	prot_to_real
	.code16

        xorb	%bh, %bh                /* set page to 0 */
	movb	$0x2, %ah
	int	$0x10			/* set cursor position */

	data32
	call	real_to_prot
	.code32

	pop	%edx
	pop	%ebx
	pop	%eax
	pop	%ebp
	ret


/*
 * set_attrib(attr) :  Sets the character attributes for character at
 *		current cursor position.
 *
 *  Bitfields for character's display attribute:
 *  Bit(s)	Description
 *   7		foreground blink
 *   6-4	background color
 *   3		foreground bright
 *   2-0	foreground color
 *
 *  Values for character color:
 *		Normal		Bright
 *   000b	black		dark gray
 *   001b	blue		light blue
 *   010b	green		light green
 *   011b	cyan		light cyan
 *   100b	red		light red
 *   101b	magenta		light magenta
 *   110b	brown		yellow
 *   111b	light gray	white
 *
 * BIOS call "INT 10H Function 08h" to read character and attribute data
 *	Call with	%ah = 0x08
 *			%bh = page
 *	Returns		%ah = character attribute
 *			%al = character value
 * BIOS call "INT 10H Function 09h" to write character and attribute data
 *	Call with	%ah = 0x09
 *			%al = character value
 *			%bh = page
 *			%bl = character attribute
 *			%cx = count to display (???, possible side-effects!!)
 */

ENTRY(set_attrib)
	push	%ebp
	push	%eax
	push	%ebx
	push	%ecx

	movl	0x14(%esp), %ecx
	xorl	%ebx, %ebx

	call	prot_to_real
	.code16

	movb	$0x8, %ah
	int	$0x10
	movb	$0x9, %ah
	movb	%cl, %bl
	movw	$1, %cx
	int	$0x10

	data32
	call	real_to_prot
	.code32

	pop	%ecx
	pop	%ebx
	pop	%eax
	pop	%ebp
	ret


/*
 * getrtsecs()
 *	if a seconds value can be read, read it and return it (BCD),
 *      otherwise return 0xFF
 * BIOS call "INT 1AH Function 02H" to check whether a character is pending
 *	Call with	%ah = 0x2
 *	Return:
 *		If RT Clock can give correct values
 *			%ch = hour (BCD)
 *			%cl = minutes (BCD)
 *                      %dh = seconds (BCD)
 *                      %dl = daylight savings time (00h std, 01h daylight)
 *			Carry flag = clear
 *		else
 *			Carry flag = set
 *                         (this indicates that the clock is updating, or
 *                          that it isn't running)
 */
ENTRY(getrtsecs)
	push	%ebp
	push	%ecx
	push	%edx

	call	prot_to_real		/* enter real mode */
	.code16

	movb	$0x2, %ah
	int	$0x1a

	data32
	jnc	gottime
	movb	$0xff, %dh

gottime:
	data32
	call	real_to_prot
	.code32

	movb	%dh, %al

	pop	%edx
	pop	%ecx
	pop	%ebp
	ret


/*
 *  This is the area for all of the special variables.
 */

	.p2align	2	/* force 4-byte alignment */

VARIABLE(boot_drive)
	.long	0

	/* an address can only be long-jumped to if it is in memory, this
	   is used by multiple routines */
offset:
	.long	0x8000
segment:
	.word	0

/*
 * This is the Global Descriptor Table
 *
 *  An entry, a "Segment Descriptor", looks like this:
 *
 * 31          24         19   16                 7           0
 * ------------------------------------------------------------
 * |             | |B| |A|       | |   |1|0|E|W|A|            |
 * | BASE 31..24 |G|/|0|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |
 * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
 * ------------------------------------------------------------
 * |                             |                            |
 * |        BASE 15..0           |       LIMIT 15..0          |
 * |                             |                            |
 * ------------------------------------------------------------
 *
 *  Note the ordering of the data items is reversed from the above
 *  description.
 */

	.p2align	2	/* force 4-byte alignment */
gdt:
	.word	0, 0
	.byte	0, 0, 0, 0

	/* code segment */
	.word	0xFFFF, 0
	.byte	0, 0x9A, 0xCF, 0

	/* data segment */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0xCF, 0

	/* 16 bit real mode CS */
	.word	0xFFFF, 0
	.byte	0, 0x9E, 0, 0

	/* 16 bit real mode DS */
	.word	0xFFFF, 0
	.byte	0, 0x92, 0, 0


/* this is the GDT descriptor */
gdtdesc:
	.word	0x27			/* limit */
	.long	gdt			/* addr */


