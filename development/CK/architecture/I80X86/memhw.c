/*
 * memhw.c:  Functions to initialise the virtual memory hardware
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
 * 10/05/99  ramon       1.10   Fixed _end[] bug that messed up memory alloc
 * 23/07/99  ramon       1.11   Moved paging setup to ABL
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
#include "sys/sysdefs.h"
#include "sys/sys.h"
#include "sys/memory.h"
#include "memory.h"
#include "ckobjects.h"
#include "ckerror.h"
#include "sched.h"
#include "debug.h"


PUBLIC struct AddressSpace kernelAddrSpc;


PUBLIC VOID CKhwInitMem(VOID)
/*
 * Initialise the virtual memory system
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    kernelAddrSpc.size       = sysinf->numpagetab;
    kernelAddrSpc.pageDir    = sysinf->pagedir;
    kernelAddrSpc.pageTables = sysinf->pagedir + 1024;
}


PUBLIC LOCAL inline PTE *getPageAddress(AddressSpace addrSpc, ADDR logicalAddr)
{
    PTE *pageTable;
    UWORD32 pageTableEntry;
    UWORD32 pageDirectoryEntry = logicalAddr >> 22;

    pageTable=(PTE *)(addrSpc[pageDirectoryEntry].physAddr<<12);
    if(!pageTable) return NIL;
    pageTable=(PTE *)LINEAR(pageTable);
    pageTableEntry=(logicalAddr>>12)&0x3ff;

    return &pageTable[pageTableEntry];
}


PUBLIC ADDR
CKgetPageMapping(AddressSpace addrSpc, ADDR logicalPageAddr)
/*
 * Get physical address from logical address
 *
 * INPUT:
 *     logicalAddr - logical address to convert.
 *
 * RETURNS:
 *     physicalAddr - physical memory logical page is pointing at.
 */
{
    PTE *page=getPageAddress(addrSpc, logicalPageAddr);
    if (page==NIL) return 0; else return (ADDR)(page->physAddr<<12);
}

PUBLIC BOOLEAN
CKcopySharedPageMapping(AddressSpace sourceAddrSpc, ADDR sourceLogPageAddr,
                        AddressSpace destAddrSpc, ADDR destLogPageAddr,
                        ADDR length)
/*
 * Maps a range of one threads pages into another thread's.
 *
 * INPUT:
 *     sourceAddrSpc - Pointer to pageDirectory to copy from
 *     sourceLogPageAddr - where to copy from
 *     destAddrSpc - Pointer to pageDirectory to copy to
 *     destLogPageAddr - where to copy to
 *     length - amount of pages to copy
 *
 * RETURNS:
 *     FALSE - failure
 *     TRUE - success
 */
{
    DATA i;
    UWORD32 *pageTable,*pageTableS;
    UADDR pageTables;
    UWORD32 pte,pteS; /* page table entry */
    UWORD32 pde,pdeS; /* page directory entry */
    UWORD32 *usageCountTable;
    UADDR pdeStartS = (UADDR)sourceLogPageAddr;
    UADDR pdeStartD = (UADDR)destLogPageAddr,pdeEndD = (UADDR)(pdeStartD+length);
    DATA npn=0;
    for(pde=(pdeStartD>>22);pde<((pdeEndD+((1<<22)-1))>>22);pde++) {
        if((destAddrSpc[pde].flags&PTPRESENT)==0)  /* Is page table present?     */
            npn++;                             /* No, increment the number   */
    }                                          /* pages needed.              */
    if(npn!=0) {                               /* Are there any missing pdes?*/
        pageTables=(UADDR)CKmemAllocPages(npn);/* Yes, Allocate pagetables   */
        if(!pageTables)                        /* Was is ok?                 */
            return FALSE;                      /* No, return with error      */
        for(i=0;i<(npn*(PAGESIZE>>2));i++)     /* Clear the page tables      */
            ((ADDR *)pageTables)[i]=0;
        usageCountTable = (UWORD32 *)destAddrSpc+1024;/* Get usageCountTable     */
        usageCountTable[pde]+=npn;             /* Increment it with new pages*/
        pageTables=PHYS(pageTables)>>12;       /* We only need the page no.  */
        for(pde=(pdeStartD>>22);pde<((pdeEndD+((1<<22)-1))>>22);pde++) {
            if((destAddrSpc[pde].flags&PTPRESENT)==0) { /* Is the ptable present?*/
                destAddrSpc[pde].physAddr=pageTables;   /* No, set the pt address*/
                destAddrSpc[pde].avail=0;               /* and flags in the page */
                destAddrSpc[pde].flags=PTPRESENT | PTUSER | PTWRITE; /* dir.     */
                pageTables++;                  /* goto next page             */
            }
        }
    }

    pdeS=(pdeStartS>>22);
    pteS=(pdeStartS>>12)&0x3ff;
    pageTableS=(UWORD32 *)LINEAR(sourceAddrSpc[pdeS].physAddr<<12);
    for(pde=pdeStartD;pde<(pdeEndD&(-1<<22));pde=((pde>>22)+1)<<22) {
                                               /* step through page directory*/
        pageTable=(UWORD32 *)LINEAR(destAddrSpc[(pde>>22)].physAddr<<12); /* get pd entry */
        for(pte=((pde>>12)&0x3ff);pte<1024;pte++) { /* step through page tab */
            pageTable[pte]=pageTableS[pteS++];
            if(pteS==0x400) {
                pteS=0; /* reset page table entry source ptr */
                pdeS++; /* increment pagedir entry source ptr */
                pageTableS=(UWORD32 *)LINEAR(sourceAddrSpc[pdeS].physAddr<<12);
            }
        }
    }
    pageTable=(UWORD32 *)LINEAR(destAddrSpc[(pde>>22)].physAddr<<12); /* get pagedir entry */
    for(pte=((pde>>12)&0x3ff);pte<((pdeEndD>>12)&0x3ff);pte++) {
                                               /* step through page table    */
        pageTable[pte]=pageTableS[pteS++];
        if(pteS==0x400) {
            pteS=0; /* reset page table entry source ptr */
            pdeS++; /* increment pagedir entry source ptr */
            pageTableS=(UWORD32 *)LINEAR(sourceAddrSpc[pdeS].physAddr<<12);
        }
    }
    return TRUE;                               /* Success                    */
}

PUBLIC BOOLEAN
CKsetPageMapping(AddressSpace addrSpc, ADDR logicalPageAddr,
                 ADDR physicalPageAddr, ADDR length, UWORD32 Attr)
/*
 * Map physical page to logical page
 *
 * INPUT:
 *     addrSpc - Pointer to pageDirectory to Change
 *     logicalPageAddr - where to map physical address
 *     physicalPageAddr -
 *     length - length of mapping
 *     flags - special flags
 *
 * RETURNS:
 *     FALSE - failure
 *     TRUE - success
 */
{
    DATA i;
    PTE *pageTable;
    UADDR pageTables;
    UWORD32 pte; /* page table entry */
    UWORD32 pde; /* page directory entry */
    UADDR physPage;
    UADDR pdeStart = (UADDR)logicalPageAddr,pdeEnd = (UADDR)(pdeStart+length);
    UWORD32 *usageCountTable;
    DATA npn=0;

    /* find out how many pagetable that need to allocated in pagedirectory */

    for(pde=(pdeStart>>22);pde<((pdeEnd+((1<<22)-1))>>22);pde++) {
        if((addrSpc[pde].flags&PTPRESENT)==0)  /* Is page table present?     */
            npn++;                             /* No, increment the number   */
    }                                          /* pages needed.              */

    if(npn!=0) {                               /* Are there any missing pdes?*/
        pageTables=(UADDR)CKmemAllocPages(npn);/* Yes, Allocate pagetables   */
        if(!pageTables)                        /* Was is ok?                 */
            return FALSE;                      /* No, return with error      */
        for(i=0;i<(npn*(PAGESIZE>>2));i++)     /* Clear the page tables      */
            ((ADDR *)pageTables)[i]=0;
        usageCountTable = (UWORD32 *)addrSpc+1024;/* Get usageCountTable     */
        usageCountTable[pde]+=npn;             /* Increment it with new pages*/
        pageTables=PHYS(pageTables)>>12;       /* We only need the page no.  */
        for(pde=(pdeStart>>22);pde<((pdeEnd+((1<<22)-1))>>22);pde++) {
            if((addrSpc[pde].flags&PTPRESENT)==0) { /* Is the ptable present?*/
                addrSpc[pde].physAddr=pageTables;   /* No, set pt address    */
                addrSpc[pde].avail=0;               /* and flags in the page */
                addrSpc[pde].flags=PTPRESENT | PTUSER | PTWRITE; /* dir.     */
                pageTables++;                  /* goto next page             */
            }
        }
    }
    physPage=physicalPageAddr>>PAGEBITS;       /* Calculate phys page        */

    for(pde=pdeStart;pde<(pdeEnd&(-1<<22));pde=((pde>>22)+1)<<22) {
                                               /* step through page directory*/
        pageTable = (PTE *) LINEAR(addrSpc[(pde>>22)].physAddr<<12);
        for(pte=((pde>>12)&0x3ff);pte<1024;pte++) { /* step through page tab */
            pageTable[pte].physAddr=physPage;  /* set physPage,avail and     */
            pageTable[pte].avail=0;            /* flags.                     */
            pageTable[pte].flags=PTPRESENT | PTUSER | (Attr&0x1ff);
            physPage++;                        /* Increment physPage         */
        }
    }
    pageTable = (PTE *) LINEAR(addrSpc[(pde>>22)].physAddr<<12); /* get pdir entry */
    for(pte=((pde>>12)&0x3ff);pte<=((pdeEnd>>12)&0x3ff);pte++) {
                                               /* step through page table    */
        pageTable[pte].physAddr=physPage;      /* set physPage,avail and     */
        pageTable[pte].avail=0;                /* flags.                     */
        pageTable[pte].flags=PTPRESENT | PTUSER | (Attr&0x1ff);
        physPage++;                            /* Increment physPage         */
    }
    return TRUE;                               /* Success                    */
}

PUBLIC VOID
CKmarkPageNotPresent(AddressSpace addrSpc, ADDR logicalPageAddr, ADDR length,
                     UWORD32 Data)
/*
 * Mark page as not present, and sets user defined data in it.
 * Paging will need this function.
 *
 * INPUT:
 *     addrSpc - Pointer to pageDirectory to Change
 *     logicalPageAddr - page to set data
 *     length - range of pages to unmark
 *     Data - user defined data
 *
 * RETURNS:
 *     none
 */
{
    UWORD32 pte; /* page table entry */
    UWORD32 pde; /* page directory entry */
    UWORD32 *page;
    UWORD32 *usageCountTable = (UWORD32 *)addrSpc+1024;
    UADDR pdeStart = (UADDR)logicalPageAddr,pdeEnd = (UADDR)(pdeStart+length);
    Data<<=1;

    for(pde=pdeStart;pde<(pdeEnd&(-1<<22));pde=((pde>>22)+1)<<22) {
                                               /* step through page directory*/
        page = (UWORD32 *) LINEAR(addrSpc[(pde>>22)].physAddr<<12);
        for(pte=((pde>>12)&0x3ff);pte<1024;pte++) { /* step through page tab */
            page[pte]=Data;                    /* mark page unused           */
            if(usageCountTable[pde>>22]!=0) {  /* Is it a system page?       */
                usageCountTable[pde>>22]--;    /* no, decrement usage count. */
                if(usageCountTable[pde>>22]==0) {  /* is it empty?           */
                    CKmemFreePages(page,1);    /* yes, free the page and     */
                    ((UWORD32 *)addrSpc)[pde>>22]=0; /* mark it not present   */
                }
            }
        }
    }
    page=(UWORD32 *) LINEAR(addrSpc[(pde>>22)].physAddr<<12);
    for(pte=((pde>>12)&0x3ff);pte<=((pdeEnd>>12)&0x3ff);pte++) {
                                               /* step through page table.   */
        page[pte]=Data;                        /* mark page unused.          */
        if(usageCountTable[pde>>22]!=0) {      /* Is it a system page?       */
            usageCountTable[pde>>22]--;        /* no, decrement usage count. */
            if(usageCountTable[pde>>22]==0) {  /* is it empty?               */
                CKmemFreePages(page,1);        /* yes, free the page and     */
                ((UWORD32 *)addrSpc)[pde>>22]=0; /* mark it not present.     */
            }
        }
    }
}

PUBLIC BOOLEAN 
CKchangePageAttr(AddressSpace addrSpc, ADDR logicalPageAddr, UWORD32 Attr)
/*
 * Map physical page to logical page
 *
 * INPUT:
 *     logicalPageAddr - where to map physical address
 *     Attr - new page attributes
 *
 * RETURNS:
 *     TRUE - ok
 *     FALSE - page attributes could'nt be change
 */
{
    PTE *page=(PTE *)getPageAddress(addrSpc, logicalPageAddr);
    if (page==NIL) return FALSE;
    if ((page->flags&PTPRESENT)==0) return FALSE;

    page->flags=(page->flags&1)|(Attr&0x1ff);
    return TRUE;
}


PUBLIC DATA CKpreProcessAddrSpc(CKAddrSpcObject *object)
/*
 * Check the user page tables for correct mapping to memory space object,
 * and map the CK and owner kernel virtual address space regions.
 *
 * INPUT:
 *     object:  Pointer to address space object to preprocess
 *
 * RETURNS:
 *     CKpreProcessAddrSpc():  Error code
 */
{
    PTE *pageTableEntry, *kernelPTE;
    UDATA i;

    /* check to see if object is valid */
    if(!object) return ENOVALIDADDRSPC;

    /* allocate a page for the page directory */
    if(!(object->addrSpc=CKmemAllocPages(2))) return ENOMEM;

    /* clear the pagetable */
    for(i=0;i<(PAGESIZE>>1);i++) { ((ADDR *)object->addrSpc)[i]=0; }

    /* Setup CK mapping in page dir */
    for(i=0;i<256;i++) {
        pageTableEntry           = object->addrSpc+i+768;
        kernelPTE                = ((PTE *)LINEAR(kernelAddrSpc.pageDir))+i+768;
        pageTableEntry->flags    = kernelPTE->flags;
        pageTableEntry->avail    = kernelPTE->avail;
        pageTableEntry->physAddr = kernelPTE->physAddr;
    }

    return 0;
}


PUBLIC VOID CKpostProcessAddrSpc(CKAddrSpcObject *object)
/*
 * Cleanup for address space
 *
 * INPUT:
 *     object:  Pointer to address space object to postprocess
 *
 * RETURNS:
 *     none
 */
{
    UDATA i;
    PTE *pageDir;
    UWORD32 *usageCountTable;

    if(!object) return; // ENOVALIDADDRSPC;
    if(!object->addrSpc) return; // ENOMEM;
    pageDir=object->addrSpc;
    usageCountTable=(UWORD32 *)pageDir+1024;

    for(i=0;i<1024;i++) {
        if(usageCountTable[i]!=0) {
            CKmemFreePages((VOID *) LINEAR(pageDir[i].physAddr<<12),1);
        }
    }
    CKmemFreePages(pageDir,2);
}


extern UADDR _start[];  /* start of kernel */
extern UADDR _end[];    /* end of kernel   */

PUBLIC VOID CKpageMarkHardwareDependent(VOID)
/*
 * Setup hardware dependent pages as used, device memory etc.
 * is used by CKinitPage()
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    bootmodule *nextmod = (bootmodule *) LINEAR(sysinf->modules);

    ASSERT(_start);
    ASSERT(_end);
    ASSERT(sysinf->lowMemTop);
    ASSERT(sysinf->totalMem);

    /* Add free memory to free list, skip the kernel and page tables etc. */
    CKmemFree((VOID *)LINEAR(sysinf->lowMemTop),(ADDR)_start-LINEAR(sysinf->lowMemTop));
    CKmemFree((VOID *)_end,LINEAR(sysinf->totalMem)-(ADDR)_end);

    /* Reserve bios area 0xa0000-0x100000 */
    CKmemAllocRegion((VOID *)LINEAR(0xa0000),0x60000);

    /* Reserve the memory for the ABL-loaded modules */
    while(nextmod > (bootmodule *) LINEAR(0)) {
        /* Align at page boundaries, for security !!!! */
        UADDR loc  = LINEAR(nextmod->physstart) & ~0xfff;
        UADDR size = ((nextmod->size & 0xfff) ? ((nextmod->size & ~0xfff) + 0x1000) : nextmod->size);

        CKmemAllocRegion((VOID *)loc,size);
        nextmod = (bootmodule *) LINEAR(nextmod->next);
    }
}
