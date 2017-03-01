/*
 * resource.c:  CK resource management
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 27/12/98  ramon       1.0    First release
 * 31/12/98  ramon       1.1    Protection using mutexes; system call access
 * 02/08/99  ramon       1.2    Major changes;  split user / kernel parts
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
#include "sys/arith.h"
#include "sys/memory.h"
#include "ckobjects.h"
#include "ckerror.h"
#include "ckresource.h"
#include "resource.h"
#include "memory.h"
#include "mutex.h"

/*
 * Resource allocation functions return an error code, or 0 on success.
 * On success, they return the resource descriptor in the the first ADDR
 * of the resource element in the resource loader structure.
 */

LOCAL CKMutex resourceAllocLock;

/**************************************************************************/
/* I/O Port allocation calls                                              */
/**************************************************************************/

#ifdef HAVEIOSPC

/*
 * What this is for:
 * When a kernel wants to reserve a region in the I/O address space of the
 * processor, it can register this region with the CK in this way.  If any
 * other kernel tries to allocate an overlapping region, an error will be
 * returned to it.  The CK doesn't actually enforce these permissions;
 * this is just for efficient communication between kernels.
 *
 * How it works:
 * The system is based on the Linux 2.0.35 system for I/O region
 * management.  All regions are linked by a linked list in numerical order
 * (ie, so that low ports are before high ones.)   CKIOfindGap(), the
 * workhorse function, finds the region after which we would need to
 * insert our region.  It returns NIL if the region overlaps an already
 * allocated region.  We allocate the CK resource, fill in the fields,
 * insert it into the linked list and live happily ever after.
 *
 * Original code by Copyright (C) 1995  Linus Torvalds, David Hinds
 */

LOCAL CKIOregionAlloc IOfirstAlloc = { 0, NIL, NIL, NIL, 0, 0, 0, NIL, NIL };

LOCAL CKMutex IOAllocLock;


LOCAL inline CKIOregionAlloc *CKIOfindGap(UADDR base, UADDR limit)
/*
 * Allocate a region of I/O address space
 *
 * INPUT:
 *     base:  base I/O port of region
 *     limit: limit of region (number of I/O ports to allocate)
 *
 * RETURNS:
 *     CKIOfindGap():  pointer to region after which to insert
 */
{
    CKIOregionAlloc *gap;   /* Should point to region we're looking for */

    if(!limit) return NIL;  /* We can't allocate nothing !              */

    for(gap=&IOfirstAlloc;;gap=gap->nextAlloc) {   /* Traverse allocations */
        if( (gap != &IOfirstAlloc) &&              /* Skip the head holder */
            (base < gap->base + gap->limit)) {     /* If the base IO addr  */
            gap = NIL;                             /* is inside this alloc */
            break;                                 /* fail the allocation  */
        }                                          /* If not failed yet &  */
        if( (gap->nextAlloc == NIL) ||             /* If we're at the end  */
            (gap->nextAlloc->base >= base+limit))  /* or next alloc does   */
            break;                                 /* not overlap ours     */
    }                                              /* We've found the gap  */

    return gap;   /* Return the address of the gap */
}

LOCAL DATA CKallocIORegion(Resource *resource, DESCRIPTOR owner, DATA flags)
/*
 * Allocate a region of I/O space for this kernel
 *
 * INPUT:
 *     resource:  Pointer to loader resource
 *     owner:     Object to allocate to
 *     flags:     Loading flags
 *
 * RETURNS:
 *     error code
 */
{
    CKIOregionAlloc *ckres; /* The CK resource structure           */
    CKIOregionAlloc *gap;   /* This is the region 'before' ours    */
    IOregionAlloc *res     = (IOregionAlloc *) resource;
    CKThreadObject *thread = (CKThreadObject *) owner;

    if(!CKvalidThread(owner))          /* Can only allocate to     */
        return ENOVALIDTHREAD;         /* thread objects           */

    CKenterCritical(&IOAllocLock);     /* Protect the linked list  */

    /* Find the region before ours */
    gap = CKIOfindGap((ADDR) res->base, (ADDR) res->limit);
    if(!gap) {
        CKexitCritical(&IOAllocLock);
        return ERESOURCEBUSY; /* It's already allocated */
    }

    /* Allocate CK resource structure */
    ckres = CKmemAlloc(sizeof(CKIOregionAlloc));
    if(!ckres) return ENOMEM;

    /* Now we'll fill in the new IO region allocation structure */
    ckres->base  = (ADDR) res->base;        /* The base IO port */
    ckres->limit = (ADDR) res->limit;       /* Amount of ports  */

    /* We set up the inter-kernel region list */
    ckres->nextAlloc  = gap->nextAlloc;  /* The region after ours       */
    gap->nextAlloc    = ckres;           /* We're next for gap          */
    ckres->prevAlloc  = gap;             /* The region before ours      */
    if(ckres->nextAlloc) 
        ckres->nextAlloc->prevAlloc = ckres; /* We're previous for next */

    CKexitCritical(&IOAllocLock);

    /* Set basic resource data */
    ckres->typeSignature = SIGIOREGION;
    ckres->owner         = owner;
    ckres->flags         = flags;

    CKenterCritical(&resourceAllocLock);

    /* Put resource into linked list */
    ckres->next                = NIL;
    ckres->prev                = thread->ioregion;
    if(thread->ioregion)
        thread->ioregion->next = (CKResource *) ckres;
    thread->ioregion           = (CKResource *) ckres;

    CKexitCritical(&resourceAllocLock);

    /* Put the resource descriptor in the resource loader */
    (ADDR) res->base = (ADDR) ckres;

    return 0;
}

LOCAL DATA CKfreeIORegion(CKIOregionAlloc *res)
/*
 * Free a region of I/O space
 *
 * INPUT:
 *     res:  Resource to free
 *
 * RETURNS:
 *     error code
 */
{
    CKenterCritical(&IOAllocLock);

    /* Remove from the inter-kernel region list */
    res->prevAlloc->nextAlloc = res->nextAlloc;
    if(res->nextAlloc) res->nextAlloc->prevAlloc = res->prevAlloc;

    CKexitCritical(&IOAllocLock);

    CKenterCritical(&resourceAllocLock);

    /* Remove resource from linked list */
    if(res->prev) res->prev->next = res->next;
    if(res->next)
        res->next->prev = res->prev;
    else
        ((CKThreadObject *) res->owner)->ioregion = res->prev;

    CKexitCritical(&resourceAllocLock);

    /* Free CK resource structure */
    CKmemFree(res,sizeof(CKIOregionAlloc));

    return 0;
}

#endif

/**************************************************************************/
/* DMA channel allocation calls                                           */
/**************************************************************************/

#ifdef HAVEDMA

LOCAL DATA DMAallocationMap[DMACHANNELS];

LOCAL DATA CKallocDMA(Resource *resource, DESCRIPTOR owner, DATA flags)
/*
 * Allocate a DMA channel
 *
 * INPUT:
 *     resource:  Pointer to loader resource
 *     owner:     Object to allocate to
 *     flags:     Loading flags
 *
 * RETURNS:
 *     error code
 */
{
    CKDMAchannelAlloc *ckres;
    DMAchannelAlloc   *res = (DMAchannelAlloc *) resource;
    CKThreadObject *thread = (CKThreadObject *) owner;

    if(!CKvalidThread(owner))          /* Can only allocate to     */
        return ENOVALIDTHREAD;         /* thread objects           */

    /*
     * We use XCHG() from sys/arith.h to allocate the DMA channel because
     * it is an atomic operation, and spares us using another mutex
     */

    if(res->channel >= DMACHANNELS) return EINRESOURCE;
    if(XCHG(DMAallocationMap[res->channel],1)) return ERESOURCEBUSY;

    /* Allocate CK resource structure */
    ckres = CKmemAlloc(sizeof(CKDMAchannelAlloc));
    if(!ckres) {
        DMAallocationMap[res->channel] = 0;
        return ENOMEM;
    }

    /* Set basic resource data */
    ckres->typeSignature = SIGDMA;
    ckres->owner         = owner;

    CKenterCritical(&resourceAllocLock);

    /* Put resource into linked list */
    ckres->next               = NIL;
    ckres->prev               = thread->dmachan;
    if(thread->dmachan)
        thread->dmachan->next = (CKResource *) ckres;
    thread->dmachan           = (CKResource *) ckres;

    CKexitCritical(&resourceAllocLock);

    /* Fill in the resource data itself */
    ckres->flags   = flags;
    ckres->channel = (UADDR) res->channel;

    /* Put the resource descriptor in the resource loader */
    (ADDR) res->channel = (ADDR) ckres;

    return 0;
}

LOCAL DATA CKfreeDMA(CKDMAchannelAlloc *res)
/*
 * Free a DMA channel
 *
 * INPUT:
 *     res:  Resource to free
 *
 * RETURNS:
 *     error code
 */
{
    DMAallocationMap[res->channel] = 0;

    CKenterCritical(&resourceAllocLock);

    /* Remove resource from linked list */
    if(res->prev) res->prev->next = res->next;
    if(res->next)
        res->next->prev = res->prev;
    else
        ((CKThreadObject *)res->owner)->dmachan = res->prev;

    CKexitCritical(&resourceAllocLock);

    /* Free CK resource structure */
    CKmemFree(res,sizeof(CKDMAchannelAlloc));

    return 0;
}

#endif

/**************************************************************************/
/* Interrupt allocation calls                                             */
/**************************************************************************/

PUBLIC CKTrapAlloc *trapHeads[INTERRUPTS];

LOCAL DATA CKallocTrap(Resource *resource, DESCRIPTOR owner, DATA flags)
/*
 * Allocate a global interrupt
 *
 * INPUT:
 *     resource:  Pointer to loader resource
 *     owner:     Object to allocate to
 *     flags:     Loading flags
 *
 * RETURNS:
 *     error code
 */
{
    CKTrapAlloc *ckres;
    TrapAlloc   *res       = (TrapAlloc *) resource;
    CKThreadObject *thread = (CKThreadObject *) owner;
    DATA error;

    if(!CKvalidThread(owner))          /* Can only allocate to     */
        return ENOVALIDTHREAD;         /* thread objects           */

    /* Allocate CK resource structure */
    ckres = CKmemAlloc(sizeof(CKTrapAlloc));
    if(!ckres) return ENOMEM;

    CKenterCritical(&resourceAllocLock);

    /* Have plaform-dependant part redirect trap to this kernel */
    if((error=CKpreProcessTrapAlloc(res->trap, ckres, flags))) {
        CKexitCritical(&resourceAllocLock);
        CKmemFree(ckres,sizeof(TrapAlloc));
        return error;
    }

    /* Put resource into linked list */
    ckres->next            = NIL;
    ckres->prev            = thread->trap;
    if(thread->trap)
        thread->trap->next = (CKResource *) ckres;
    thread->trap           = (CKResource *) ckres;

    /* Allocate trap */
    ckres->ntp = trapHeads[res->trap];
    trapHeads[res->trap] = ckres;

    CKexitCritical(&resourceAllocLock);

    /* Set basic resource data */
    ckres->typeSignature = SIGTRAP;
    ckres->owner         = owner;

    /* Fill in the resource data itself */
    ckres->flags                 = flags;
    ckres->trap                  = (ADDR)    res->trap;
    ckres->sigThread             = (EVENT *) res->sigThread;
    ckres->priority              = (DATA)    res->priority;

    /* Put the resource descriptor in the resource loader */
    (ADDR) res->trap = (ADDR) ckres;

    return 0;
}

LOCAL DATA CKfreeTrap(CKTrapAlloc *res)
/*
 * Free a global interrupt
 *
 * INPUT:
 *     res:  Resource to free
 *
 * RETURNS:
 *     error code
 */
{
    CKpostProcessTrapAlloc(res);

    CKenterCritical(&resourceAllocLock);

    /* Remove resource from linked list */
    if(res->prev) res->prev->next = res->next;
    if(res->next)
        res->next->prev = res->prev;
    else
        ((CKThreadObject *)res->owner)->trap = res->prev;

    if(trapHeads[res->trap] == res)
        trapHeads[res->trap] = res->ntp;
    else {
        CKTrapAlloc *pos;

        for (
            pos = trapHeads[res->trap]->ntp;
            pos->ntp == res;
            pos = pos->ntp
        );

        pos->ntp = res->ntp;
    }

    CKexitCritical(&resourceAllocLock);

    /* Free CK resource structure */
    CKmemFree(res,sizeof(CKTrapAlloc));

    return 0;
}


/**************************************************************************/
/* Physical memory region allocation calls                                */
/**************************************************************************/

LOCAL DATA CKallocPhysRegion(Resource *resource, DESCRIPTOR owner, DATA flags)
/*
 * Allocate a physical memory region
 *
 * INPUT:
 *     resource:  Pointer to loader resource
 *     owner:     Object to allocate to
 *     flags:     Loading flags
 *
 * RETURNS:
 *     error code
 */
{
    CKPhysRegionAlloc *ckres;
    PhysRegionAlloc   *res = (PhysRegionAlloc *) resource;
    CKKernelObject *kernel = (CKKernelObject *) owner;

    if(!CKvalidKernel(owner))          /* Can only allocate to     */
        return ENOVALIDTHREAD;         /* kernel objects           */

    /* Allocate CK resource structure */
    ckres = CKmemAlloc(sizeof(CKPhysRegionAlloc));
    if(!ckres) return ENOMEM;

    /* Fill in the resource data itself */
    if(!(ckres->start = PHYS(CKmemAllocAlign(PAGEBLOCKSIZE,PAGEBLOCKBITS)))) {
        CKmemFree(ckres,sizeof(CKPhysRegionAlloc));
        return ENOMEM;
    }

    ckres->length = 128;

    CKenterCritical(&resourceAllocLock);

    /* Put resource into linked list */
    ckres->next                  = NIL;
    ckres->prev                  = kernel->physregion;
    if(kernel->physregion)
        kernel->physregion->next = (CKResource *) ckres;
    kernel->physregion           = (CKResource *) ckres;

    CKexitCritical(&resourceAllocLock);

    /* Set basic resource data */
    ckres->typeSignature = SIGPHYSREGION;
    ckres->owner         = owner;

    /* Put the resource descriptor in the resource loader */
    (ADDR) res->start  = (ADDR) ckres;
    (ADDR) res->length = (ADDR) ckres->start;

    return 0;
}

LOCAL DATA CKfreePhysRegion(CKPhysRegionAlloc *res)
/*
 * Free a global interrupt
 *
 * INPUT:
 *     res:  Resource to free
 *
 * RETURNS:
 *     error code
 */
{
    CKmemFree((VOID *)res->start,PAGEBLOCKSIZE);

    CKenterCritical(&resourceAllocLock);

    /* Remove resource from linked list */
    if(res->prev) res->prev->next = res->next;
    if(res->next)
        res->next->prev = res->prev;
    else
        ((CKKernelObject *)res->owner)->physregion = res->prev;

    CKexitCritical(&resourceAllocLock);

    /* Free CK resource structure */
    CKmemFree(res,sizeof(CKPhysRegionAlloc));

    return 0;
}


/**************************************************************************/
/* Resource allocation system calls                                       */
/**************************************************************************/

PUBLIC inline DATA CKresourceAllocKernelCode(Resource *resource, DESCRIPTOR owner, DATA flags)
/*
 * Allocate a CK resource
 * NOTE:  This is a system call
 *
 * INPUT:
 *     resource:  Pointer to loader resource
 *     owner:     Object to allocate to
 *     flags:     Loading flags
 *
 * RETURNS:
 *     error code
 */
{
    DATA error;

    switch(resource->typeSignature) {
        case SIGPHYSREGION:
            error = CKallocPhysRegion(resource, owner, flags);
            break;
        case SIGTRAP:
            error = CKallocTrap(resource, owner, flags);
            break;
#ifdef HAVEDMA
        case SIGDMA:
            error = CKallocDMA(resource, owner, flags);
            break;
#endif
#ifdef HAVEIOSPC
        case SIGIOREGION:
            error = CKallocIORegion(resource, owner, flags);
            break;
#endif
        default:
            error = 0;  /* For now */
            break;
    }

    return error;
}


PUBLIC inline DATA CKresourceFreeKernelCode(RESOURCE resource)
/*
 * Free a CK resource
 * NOTE:  This is a system call
 *
 * INPUT:
 *     res:  Resource to free
 *
 * RETURNS:
 *     error code
 */
{
    DATA error;

    switch(resource->typeSignature) {
        case SIGPHYSREGION:
            error = CKfreePhysRegion((CKPhysRegionAlloc *) resource);
            break;
        case SIGTRAP:
            error = CKfreeTrap((CKTrapAlloc *) resource);
            break;
#ifdef HAVEDMA
        case SIGDMA:
            error = CKfreeDMA((CKDMAchannelAlloc *) resource);
            break;
#endif
#ifdef HAVEIOSPC
        case SIGIOREGION:
            error = CKfreeIORegion((CKIOregionAlloc *) resource);
            break;
#endif
        default:
            error = 0;
            break;
    }

    return error;
}


/**************************************************************************/
/* CK Resource Management System Calls (platform dependant)               */
/* these functions is the same as above "Code" is just change to "Stub",  */
/* and some platform dependant stuff is done.                             */
/* ---------------------------------------------------------------------- */
/* Include Need to be here, CK(un)CacheObject is inline expanded in it    */
/* can't be linked, won't produce the same result, faster code.           */
/**************************************************************************/

#include "sys/stubs/resource.h"


