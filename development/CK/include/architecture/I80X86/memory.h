/*
 * memory.h:  Defenition of page tables, directories, and the like
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 18/11/98  ramon       1.0    First release
 * 21/11/98  jens        1.1    Added missing PTE flags
 * 23/12/98  ramon       1.2    Added address conversion macros
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

#ifndef __SYS_MEMORY_H
#define __SYS_MEMORY_H

#include <typewrappers.h>

#define PAGEBITS 12
#define PAGESIZE (1<<PAGEBITS)
#define MAXMEMORY 0x100000000LL
#define MAXADDR (MAXMEMORY-1)

/* PTE flags: */
#define PTPRESENT  1   /* Present bit                                 */
#define PTWRITE    2   /* Write permission bit                        */
#define PTUSER     4   /* User/Supervisor bit (when included, user)   */
#define PTWRTTRU   8   /* Page-level write-through                    */
#define PTCACHEDIS 16  /* Page-level cache disable                    */
#define PTACCESSED 32  /* Accessed bit                                */
#define PTDIRTY    64  /* Dirty bit                                   */
                       /* Pentium and up                              */
#define PTBIGPAGE  128 /* Page size 2MB-4MB pages if set else 4kb     */
                       /* Pentium pro and up                          */
#define PTGLOBAL   256 /* Global page, keeps page from being flushed  */
		       /* from cache                                  */
#define PTNOPAGEOUT 512/* Don't let the swapper page this page out,   */
                       /* this is an alliance defined bit             */

typedef struct PageTableEntry {
    UWORD32 flags:10;  /* The or'ed flags                             */
    UWORD32 avail:2;   /* 2 bits available for programmer's use       */
    UWORD32 physAddr:20;/* This is an address in physical memory      */
                       /* Because pages are located on 4K boundaries, */ 
                       /* the low-order 12 bits are always zero. In a */
                       /* page directory, the page frame address is   */
                       /* the address of a page table. In a second-   */
                       /* level page table, the page frame address is */
                       /* the address of the page frame that contains */
                       /* the desired memory operand.                 */
} __attribute__ ((packed)) PTE;


/*
 * struct AddressSpace points to a page tables/directory structure.
 * This one is included in the actual aspace object data.
 */

struct AddressSpace {
    ADDR size;         /* amount of page tables */
    PTE  *pageDir;
    PTE  *pageTables;
};

typedef PTE * AddressSpace;


/*
 * TLB-related macros
 */

#define CKflushLocalTLB()  asm("movl %%cr3,%%eax; movl %%eax,%%cr3":::"%eax")


/*
 * LINEAR(x)  returns the linear address of physical address (x)
 * PHYS(x)    returns the physical address of kernel address (x)
 */

#define LINEAR(x) ((ADDR)(x)+0xc0000000)
#define PHYS(x)   ((ADDR)(x)-0xc0000000)

#endif
