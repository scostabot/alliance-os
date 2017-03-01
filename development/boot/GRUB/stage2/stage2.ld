/*
 * stage2.ld:  GRUB stage 2 linker script
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 25/08/99  ramon       1.0    First release
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

/*
 * GRUB stage2 needs a special linker script because it needs to execute
 * realmode code.  This wasn't a problem earlier when GRUB stage2 was
 * still small.  However, with netboot support GRUB stage2 now overflows
 * the 64k segment boundary, which the realmode code can't handle.
 * In order to solve this we need to make sure that all of the code
 * that must be executed in realmode comes *first* in the binary, so
 * that it all fits in the segment.  The PM code couldn't care less
 * about RM segment boundaries.
 *
 * In order to help the linker, we mark the code it has to watch
 * out for by putting it in the .rmtext segment.  We link stuff
 * in this segment in first.  The .rmtext doubles up as a realmode
 * marker for the ABL (see boot/ABL/scripts/ABL.ld).
 *
 * For more info, see ld's texinfo documentation.
 */

/*
 * Output format:  32 bit i386 iABI-ELF
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")


/*
 * Architecture:  i386 and compatibles
 */

OUTPUT_ARCH(i386)


/*
 * stage2 entry point symbol:  it's called _start().  Really.
 */

ENTRY(_start)


/*
 * Now we start telling ld about the segments in our file
 */

PHDRS
{
    text    PT_LOAD ;        /* Protected mode text segment        */
    data    PT_LOAD ;        /* Protected mode data segment        */
}


/*
 * Now we start telling ld about the sections in our file
 */

SECTIONS 
{
    /*
     * The entry point of stage2 is at 0x8000
     */

    . = 0x8000;

    /*
     * Mark this the start of the text section
     */

    _text = .;

    /*
     * Put all the text sections here
     */

    .rmtext : { *(.rmtext) } :text
    .text   : { *(.text)   } :text
    .rodata : { *(.rodata) } :text

    /*
     * Mark the end of the text segment, _etext
     */

    _etext = .;

    /*
     * Put all the .data sections here
     */

    .data : { *(.data) } :data

    /*
     * Mark the end of the text segment, _edata
     */

    _edata = .;

    /*
     * Put all the .bss sections here
     */

    .bss : { *(.bss) } :data

    /*
     * Mark the end of the ELF file, _end
     */

    _end = .;
}
