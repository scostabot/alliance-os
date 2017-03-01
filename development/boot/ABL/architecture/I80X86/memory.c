/*
 * memory.c:  Functions to initialise the virtual memory hardware
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 20/11/98  jens        1.1    CKhwInitMem
 * 25/11/98  ramon       1.2    Updated for new segmentation scheme
 * 25/11/98  jens        1.3    Added PTE manipulation functions
 * 26/11/98  ramon       1.4    Updated for even newer segmentation scheme
 * 27/11/98  ramon       1.5    Added address space pre/postprocessing
 * 07/12/98  jens        1.6    Added CKpageMarkHardwareDependent
 * 31/01/99  jens        1.7    Fix to use new CKpageAlloc scheme
 * 21/03/99  jens        1.8    CKpre/postProcessAddrSpc done.
 * 15/04/99  jens        1.9    Added CKpre/postProcessSHMem
 * 01/05/99  ramon       1.10   Modified for ABL / Removed CK baggage
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

#include <typewrappers.h>
#include "sys/memory.h"
#include "decl.h"

/*
 * XXX -- TODO:  make sure the page tables don't exceed low memory size
 *               and leave memory for DMA transfers.  This puts a limit on
 *               the amount of kernel memory we map...  but that's not all
 *               that important.
 */


PUBLIC VOID ABLinitMem(VOID)
/*
 * Preallocate the page tables and set up configuration data
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    /* Start the page tables right after the first page */
    PTE *pageDir     = (PTE *) 0x1000;
    PTE *pageTables  = pageDir + 1024;
    UADDR mappedPhys = ((totalMem < (16<<20)) ? totalMem : (16<<20));

    /* Mark the pagetables as used memory */
    lowMemTop = ((UADDR) pageTables) + ((mappedPhys + (20<<20)) / 1024);

    binf = (bootinfo *) lowMemTop;

    binf->numpagetab = (lowMemTop - (UADDR)pageTables) / 0x1000;
    binf->pagedir    = pageDir;
    binf->totalMem   = totalMem;

    lowMemTop += sizeof(bootinfo);
}


PUBLIC VOID ABLsetupMem(VOID)
/*
 * Setup the virtual memory hardware (paging etc.)
 * Set up memory parameters in boot configuration structure
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    /* Start the page tables right after the first page */
    PTE *pageDir     = (PTE *) 0x1000;
    PTE *pageTables  = pageDir + 1024;
    UADDR mappedPhys = ((totalMem < (16<<20)) ? totalMem : (16<<20));

    /* Other variables */
    PTE *pageTableEntry;
    DATA i;

    /* Notify user... we're here !!! */
    print("ABL:  Initialising paging...\n");

    /* Clear the page directory */
    for(i = 0; i < 1024; i++) {
        pageTableEntry           = pageDir + i;
        pageTableEntry->flags    = 0;
        pageTableEntry->avail    = 0;
        pageTableEntry->physAddr = 0;
    }
    
    /* Setup page directory for CK (0xc0000000) and identity map at 0x0 */
    for(i = 0; i < (mappedPhys / (4 * 1024 * 1024)); i++) {
        pageTableEntry           = pageDir + i;
        pageTableEntry->flags    = PTPRESENT | PTWRITE;
        pageTableEntry->avail    = 0;
        pageTableEntry->physAddr = (((UADDR) pageTables) >> 12) + i;
    }

    for(i = 0; i < (mappedPhys / (4 * 1024 * 1024)); i++) {
        pageTableEntry           = pageDir + i + (0xc0000000 / (4 * 1024 * 1024));
        pageTableEntry->flags    = PTPRESENT | PTWRITE;
        pageTableEntry->avail    = 0;
        pageTableEntry->physAddr = (((UADDR) pageTables) >> 12) + i;
    }

    /* Set up I/O mapped device memory at 0xfec00000 */
    for(i = 0; i < 5; i++) {
        pageTableEntry           = pageDir + i + (0xfec00000 / (4 * 1024 * 1024));
        pageTableEntry->flags    = PTPRESENT | PTWRITE;
        pageTableEntry->avail    = 0;
        pageTableEntry->physAddr = (((UADDR) pageTables) >> 12) + i + (mappedPhys / (4 * 1024 * 1024)); 
    }

    /* Setup physical memory page tables */
    for(i = 0; i < (mappedPhys / (4 * 1024)); i++) {
        /*
         * Get page entry, we'll just fill it like this, cause we know
         * the pageTables are in a row.
         */
        pageTableEntry           = pageTables + i;
        pageTableEntry->flags    = PTPRESENT | PTWRITE;
        pageTableEntry->avail    = 0;
        pageTableEntry->physAddr = i;
    }

    /* Setup I/O region page tables */
    for(i = 0; i < (5 * 1024); i++) {
        pageTableEntry           = pageTables + i + (mappedPhys / (4 * 1024));
        pageTableEntry->flags    = PTPRESENT | PTWRITE;
        pageTableEntry->avail    = 0;
        pageTableEntry->physAddr = i + (0xfec00000 / (4 * 1024));
    }

    /* Load %cr3 with kernelPageTables */
    asm("movl %%eax,%%cr3" : : "a" (((UADDR) pageDir) & 0xfffff000));
    
    /* Enable paging */
    asm volatile (
        "movl %%cr0,%%eax       \n"   /* Get current MSW      */
        "orl $0x80000000,%%eax  \n"   /* Set PG bit           */
        "movl %%eax,%%cr0       \n"   /* Load MSW into %cr0   */
        "movl %%cr3,%%eax       \n"   /* Flush TLB            */
        "movl %%eax,%%cr3       \n"
        "ljmp %0,$1f            \n"   /* Flush prefetch queue */
        "1:                     \n"
        :
        : "p" (0x08)
        : "%eax"
    );
}
