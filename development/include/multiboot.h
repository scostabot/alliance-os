/*
 * multiboot.h:  Definitions for the Multiboot standard
 *
 * The Alliance Boot System is based on the multiboot standard.
 * This file contains the defentions needed to boot and run
 * multiboot-compatible kernels.
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 29/05/99  ramon       1.0    First release
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

#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

#include <typewrappers.h>

/*
 *  The structure type "mod_list" is used by the "multiboot_info" structure.
 */

struct mod_list {
    /* Memory used goes from bytes 'mod_start' to 'mod_end-1' */
    UADDR mod_start;
    UADDR mod_end;

    /* Module command line                                    */
    UADDR cmdline;

    /* padding to take it to 16 bytes (must be zero)          */
    UDATA pad;
};


/*
 *  INT-15, AX=E820 style "AddressRangeDescriptor"
 *  ...with a "size" parameter on the front which is the structure size - 4,
 *  pointing to the next one, up until the full buffer length of the memory
 *  map has been reached.
 */

struct AddrRangeDesc {
    UADDR size;
    UADDR BaseAddrLow;
    UADDR BaseAddrHigh;
    UADDR LengthLow;
    UADDR LengthHigh;
    UDATA Type;
};

/* Usable memory "Type", all others are reserved  */
#define MB_ARD_MEMORY  1


/*
 *  MultiBoot Info description
 *
 *  This is the struct passed to the boot image.  This is done by placing
 *  its address in the EAX register.
 */

struct multiboot_info {
    /* MultiBoot info version number */
    UDATA flags;

    /* Available memory from BIOS */
    UADDR mem_lower;
    UADDR mem_upper;

    /* "root" partition */
    UDATA boot_device;

    /* Kernel command line */
    UADDR cmdline;

    /* Boot-Module list */
    UDATA mods_count;
    UADDR mods_addr;

    union {
	struct {
	    /* (a.out) Kernel symbol table info */
            UADDR tabsize;
            UADDR strsize;
            UADDR addr;
            UDATA pad;
        } a;

        struct {
            /* (ELF) Kernel section header table */
            UDATA num;
            UADDR size;
            UADDR addr;
            UDATA shndx;
        } e;
    } syms;

    /* Memory Mapping buffer */
    UADDR mmap_length;
    UADDR mmap_addr;
};


/*
 *  Flags to be set in the 'flags' parameter above
 */

/* Is there basic lower/upper memory information? */
#define MB_INFO_MEMORY     0x1

/* Is there a boot device set?                    */
#define MB_INFO_BOOTDEV    0x2

/* Is the command-line defined?                   */
#define MB_INFO_CMDLINE    0x4

/* Are there modules to do something with?        */
#define MB_INFO_MODS       0x8

/* These next two are mutually exclusive */

/* Is there a symbol table loaded?                */
#define MB_INFO_AOUT_SYMS  0x10

/* Is there an ELF section header table?          */
#define MB_INFO_ELF_SHDR   0x20

/* Is there a full memory map?                    */
#define MB_INFO_MEM_MAP    0x40


/*
 *  The following value must be present in the EAX register.
 */

#define MULTIBOOT_VALID    0x2BADB002


/**********************************************************************/


/*
 *  MultiBoot Header description
 */

struct multiboot_header {
    /* Must be MULTIBOOT_MAGIC - see below                            */
    UDATA magic;

    /* Feature flags - see below                                      */
    UDATA flags;

    /* Checksum: The above fields plus this one must equal 0 mod 2^32 */
    UDATA checksum;

    /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set           */
    UDATA header_addr;
    UDATA load_addr;
    UDATA load_end_addr;
    UDATA bss_end_addr;
    UDATA entry_addr;
};


/*
 * The entire multiboot_header must be contained
 * within the first MULTIBOOT_SEARCH bytes of the kernel image.
 */

#define MULTIBOOT_SEARCH  8192
#define MULTIBOOT_MAGIC   0x1BADB002

#define MULTIBOOT_FOUND(addr, len)                                          \
    (!((addr) & 0x3) && ((len) >= 12) &&                                    \
     (*((DATA *)(addr)) == MULTIBOOT_MAGIC) &&                              \
     !(*((UDATA *)(addr)) + *((UDATA *)(addr+4)) + *((UDATA *)(addr+8))) && \
     (!(MULTIBOOT_AOUT_KLUDGE & *((DATA *)(addr+4))) || ((len) >= 32)))


/*
 * Features flags for 'flags'.
 */
 
/* If a set flag in MULTIBOOT_MUSTKNOW is unsupported we should fail  */
#define MULTIBOOT_MUSTKNOW     0x0000FFFF

/* currently unsupported flags...  this is a kind of version number   */
#define MULTIBOOT_UNSUPPORTED  0x0000FFFC

/* Align all boot modules on i386 page (4KB) boundaries               */
#define MULTIBOOT_PAGE_ALIGN   0x00000001

/* Must pass memory information to OS                                 */
#define MULTIBOOT_MEMORY_INFO  0x00000002

/* This flag indicates the use of the other fields in the header      */
#define MULTIBOOT_AOUT_KLUDGE  0x00010000


#endif  /* __MULTIBOOT_H */
