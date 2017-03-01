/*
 * ABL.ld:  ABL linker script
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 26/06/99  ramon       1.0    First release
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
 * The ABL needs a special linker script because it needs to execute
 * realmode code, which needs to be relocated into the lower memory
 * area.  We need to have the linker relocate all realmode addresses
 * correctly, while fooling the ELF loader into thinking that the code
 * to be executed in realmode is just part of the normal binary image
 * (ABL code will relocate it into lower memory.  We need to do this
 * this way because GRUB will only load sections above 1MB.)
 *
 * In order to help the linker, we mark the code it has to watch
 * out for by putting it in the .rmtext segment.  Now, all this code
 * and every reference to it will automatically point to the right
 * place in low memory.
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
 * ABL entry point symbol:  it's called _start().  Really.
 */

ENTRY(_start)


/*
 * Now we start telling ld about the segments in our file
 */

PHDRS
{
    text    PT_LOAD ;        /* Protected mode text segment        */
    rmtext  PT_LOAD ;        /* Low-memory relocated text segment  */
    data    PT_LOAD ;        /* Protected mode data segment        */
}


/*
 * Now we start telling ld about the sections in our file
 */

SECTIONS 
{
    /*
     * Dirty trick:  relocate all addresses to use 0x8000 as the
     * base address for this code, but map the segment in the file
     * to come right after .rodata so that GRUB doesn't complain.
     * Actually I'd have preferred it if GRUB would just load
     * any segment where I tell it to load it, but as it doesn't
     * we do it this way, and manually copy the .rmtext template
     * to the right place at ABL startup.
     */

    .rmtext 0x8000 : AT ( _rmtempl ) {
        _rmtext = .;
        *(.rmtext)
    } :rmtext


    /*
     * The entry point of the ABL is at 0x100000
     */

    . = 0x100000;


    /*
     * Mark this the start of the text section
     */

    _text = .;


    /*
     * Put all the .text / .rodata sections here
     */

    .text   : { *(.text)   } :text
    .rodata : { *(.rodata) } :text


    /*
     * The code to be executed in low memory starts here at _rmtempl,
     * and goes on until the end of the text, _etext.  It needs to
     * be copied to _rmtext by the ABL startup code.
     */

    _rmtempl = .;


    /*
     * Mark the end of the text segment, _etext
     */

    _etext = _rmtempl + SIZEOF ( .rmtext );


    /*
     * The data starts at the end of the text
     */

    . = _etext;


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
