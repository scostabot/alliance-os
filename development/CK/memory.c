/*
 * memory.c:  Functions to manage the CK's private memory
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 18/11/98  jens        1.0    First full internal release
 * 19/11/98  ramon       1.1    Coding Style update && CKinitMem()
 * 20/11/98  jens        1.2    bugfixes, added CKmemUsed and
 *                              CKmemAvail variables
 * 29/11/98  jens        1.3    CKpage functions implemented.
 * 30/11/98  jens        1.4	fixed misc errors.
 * 01/12/98  jens        1.5    moved paging function to paging.c
 * 04/12/98  jens        1.6    Changed Algorithm to free list 
 *                              allocation, incredible fast.
 * 25/12/98  jens        1.7    uses mutexes
 * 31/01/99  jens        1.8    Fix to use new CKpageAlloc scheme
 * 21/02/99  jens        1.9    Fixed CKmemAddPages
 * 10/03/99  jens        1.10   Total rewrite
 * 14/03/99  jens        1.11   Uses standard Freelist Alloc from klibs
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
 * Benchmark (version 1.6):
 * Just to give an indication of the performance of the memory code,
 * CKmemAlloc/CKmemFree does 250000 random alloc's/free's a second on
 * a P166 8megs
 */

#include <typewrappers.h>
#include <klibs.h>
#include "sys/sys.h"
#include "memory.h"
#include "mutex.h"

LOCAL CKMutex CKFreeListLock;
LOCAL KLfreeListDescriptor CKFreeListDescriptor={NIL};

PUBLIC VOID *CKmemAlloc(UADDR size)
/*
 * Allocate private CK memory, no alignment is performed.
 *
 * INPUT:
 *     size = amount of space needed
 *
 * RETURNS:
 *     NIL - error
 *     pointer to memory block
 */
{
    VOID *result;
    CKenterCritical(&CKFreeListLock);
    result=KLfreeListMemAlloc(&CKFreeListDescriptor,size);
    CKexitCritical(&CKFreeListLock);
    return result;
}

PUBLIC VOID *CKmemAllocAlign(UADDR size,DATA alignment)
/*
 * Allocate private CK memory aligned
 *
 * INPUT:
 *     size = amount of space needed
 *     align = power of 2 size to align to.
 * RETURNS:
 *     NIL - error
 *     pointer to memory block
 */
{
    VOID *result;
    CKenterCritical(&CKFreeListLock);
    result=KLfreeListMemAllocAlign(&CKFreeListDescriptor,size,alignment);
    CKexitCritical(&CKFreeListLock);
    return result;
}

PUBLIC VOID *CKmemAllocPages(DATA nopages)
/*
 * Allocate a number of private CK memory pages, memory is aligned
 * to the machine dependent PAGESIZE
 *
 * INPUT:
 *     nopages = number of pages to allocate
 *
 * RETURNS:
 *     NIL - error
 *     address of buffer allocated
 */
{
    VOID *result;
    CKenterCritical(&CKFreeListLock);
    result=KLfreeListMemAllocAlign(&CKFreeListDescriptor,nopages<<PAGEBITS,PAGEBITS);
    CKexitCritical(&CKFreeListLock);
    return result;
}

PUBLIC VOID *CKmemAllocRegion(VOID *data,UADDR size)
/*
 * Allocate a region of private CK memory.
 *
 *
 * INPUT:
 *     data - pointer to memory block
 *     nopages - size of memory block
 *
 * RETURNS:
 *     NIL - error
 *     else address of buffer allocated
 */
{
    VOID *result;
    CKenterCritical(&CKFreeListLock);
    result=KLfreeListMemAllocRegion(&CKFreeListDescriptor,(UADDR)data,size);
    CKexitCritical(&CKFreeListLock);
    return result;
}

VOID CKmemFree(VOID *data,UADDR size)
/*
 * Free private CK memory
 *
 * INPUT:
 *     memory, pointer to memory to free
 *
 * RETURNS:
 *     none
 */
{
    CKenterCritical(&CKFreeListLock);
    KLfreeListMemFree(&CKFreeListDescriptor,data,size);
    CKexitCritical(&CKFreeListLock);
}

PUBLIC VOID CKmemFreePages(VOID *data,DATA nopages)
/*
 * Free a number of private CK pages
 *
 * INPUT:
 *     memory  - pointer to pages to free
 *     nopages - number of pages to free
 *
 * RETURNS:
 *     none
 */
{
    CKenterCritical(&CKFreeListLock);
    CKmemFree(data,nopages<<PAGEBITS);
    CKexitCritical(&CKFreeListLock);
}

PUBLIC VOID CKinitMem()
/*
 * Initialises the CK memory micromanagment
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    DATA i;

    /* Setup Freelist descriptor */

    CKFreeListDescriptor.FirstFreeEntry=NIL;
    for(i=0;i<8;i++) CKFreeListDescriptor.FirstFreeSizeEntry[i]=NIL;

    /* Set amount of memory to 0 */

    CKFreeListDescriptor.FreeMemory=0;

    /* No callbacks are needed in the CK */

    CKFreeListDescriptor.AllocChunk=NIL;
    CKFreeListDescriptor.PrepareFreeChunk=NIL;
    CKFreeListDescriptor.FreeChunk=NIL;

    /* Setup memory and mark hardware dependent memory used */

    CKpageMarkHardwareDependent();
}
