/*
 * objmgmt.c:  Main System Object Manager code
 *
 * (C) 1998, 1999 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author   Rev    Notes
 * 16/11/98  ramon    1.0    First prototype: generic caching + threads
 * 20/11/98  ramon    1.1    Added generic uncache + uncaching threads
 * 26/11/98  ramon    1.2    Added caching/uncaching address spaces
 * 11/12/98  jens     1.3    Added GateCode to CK(un)cacheObject
 * 16/12/98  jens     1.4    Added Platform dependant code include
 * 17/12/98  ramon    1.5    Changed critical section handling to mutexes
 * 21/12/98  ramon    1.6    Added kernel caching
 * 28/12/98  ramon    1.7    Fixed linked list updating while unloading
 * 31/12/98  ramon    1.8    Removed all those pesky CKdescToVertex()-macros
 * 15/04/99  jens     1.9    Added (un-)caching of SHMemObject
 * 01/08/99  ramon    1.10   Major changes;  now real caching / writeback
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
 * TODO:
 *
 * - Code documentation
 * - Add security handling (neglected for now)
 * - When paging is implemented, have object creation functions map the
 *   object data into the kernel aspace, and the other way around with
 *   unloading
 * - Code to handle circular kernel<->thread dependency
 * - When an object is unloaded because it is dependant on an object that
 *   was explicitly unloaded, then the parent kernel of that object needs
 *   to be signalled of that fact, unless the parent kernel itself ordered
 *   the unload or the kernel is scheduled for unload itself.
 * - CK resources need to be automatically freed on unload
 * - Signals need to be automatically freed on thread unload
 * - Eliminate memory leak:  if preProcessObject ok, but no mem, postProc
 *   object before returning ENOMEM
 * - Handle resource chain autoloading
 */

#include <typewrappers.h>
#include "sys/memory.h"
#include "sys/hwkernel.h"
#include "sys/sys.h"
#include "ckobjects.h"
#include "objects.h"
#include "ckerror.h"
#include "memory.h"
#include "sched.h"
#include "pri_heap.h"
#include "depgraph.h"
#include "mutex.h"

/*
 * The System Object Manager mutex
 * This makes sure that two threads don't start messing with the
 * ODG or the object linked lists at the same time, because that means
 * that something is going to be messed up
 */

LOCAL CKMutex sysObjManLock;


/*
 * These are the heads of the linked lists of peer objects.
 * Empty lists are NIL.
 */

PUBLIC VERTEX *kernelObjHead  = NIL;    /* Kernel objects        */
PUBLIC VERTEX *threadObjHead  = NIL;    /* Thread objects        */
PUBLIC VERTEX *addrSpcObjHead = NIL;    /* Address space objects */
PUBLIC VERTEX *shMemObjHead   = NIL;    /* Shared memory objects */


/**************************************************************************/
/* Macros/Inlines                                                         */
/**************************************************************************/

LOCAL inline VOID CKremoveDepSearch(VERTEX *object, VERTEX *child)
/*
 * Remove the object dependency with the specified child and parent
 * from the ODG
 *
 * INPUT:
 *     object:  Parent object of dependency to unload
 *     child:   Child  object of dependency to unload
 *
 * RETURNS:
 *     none
 */
{
    UDATA i;

    for(i=0; i<object->dependencies; i++)
        if(object->firstDependency[i].child == child) {
            CKremoveObjectDep(&object->firstDependency[i--]);
            break;
        }
}


/**************************************************************************/
/* Thead object loading / unloading                                       */
/**************************************************************************/

LOCAL DATA
CKloadThreadObject(ThreadObject *object, ADDR size, DATA flags)
/*
 * Loads a new thread object, modifies its dependencies, validates its data
 *
 * INPUT:
 *     object:  Pointer to object to be cached 
 *     size:    Object size (in bytes)         
 *     flags:   Flags                          
 *
 * RETURNS:
 *     object->self:         Descriptor of this object
 *     CKloadThreadObject(): 0 for success, or otherwise error code
 */
{
    DATA error;
    CKThreadObject *ckobject;

    /* Check for valid flags */
    if(flags & OFLOCKED) return ENOLOCKPERMS;  /* only the ABL can lock */

    /* Check whether the object is valid (as far as we can see) */
    if(!object->addressSpace || !CKvalidAddrSpc(object->addressSpace))
        return ENOVALIDADDRSPC;
    if(!(object->quantum && object->defer))
        return EINSCHEDPARMS;
    if(object->parentKernel && (object->parentKernel != ((VOID *) -1))
                            && !CKvalidKernel(object->parentKernel))
        return ENOVALIDKERNEL;

    /* Allocate memory for object header in CK address space */
    ckobject = CKmemAlloc(sizeof(CKThreadObject));
    if(!ckobject) return ENOMEM;

    /* Fill in the object structure */
    ckobject->head.objectSignature = (DATA) SIGTHREAD;
    ckobject->head.flags           = flags;
    ckobject->head.objectData      = (ADDR) object;
    ckobject->head.dependencies    = 0;
    ckobject->head.firstDependency = NIL;
    ckobject->head.nextObject      = NIL;
    ckobject->head.prevObject      = threadObjHead;
    if(threadObjHead) threadObjHead->nextObject = (VERTEX *) ckobject;
    threadObjHead = (VERTEX *) ckobject;

    if(object->parentKernel != ((VOID *) -1)) {
        ckobject->parentKernel = object->parentKernel;
    } else if(currtask) {
        ckobject->parentKernel = currtask->parentKernel;
    } else {
        ckobject->parentKernel = NIL;
    }

    ckobject->addressSpace = object->addressSpace;
    ckobject->trap         = NIL;
    ckobject->dmachan      = NIL;
    ckobject->ioregion     = NIL;
    ckobject->quantum      = object->quantum;
    ckobject->defer        = object->defer;

    ckobject->ts       = ts_Blocked;
    ckobject->signals  = NIL;

    if(ckobject->parentKernel)
        object->interruptable = 0;
    else
        object->interruptable = 1;

    /* This sets up the hardware-dependant part of the thread object */
    if((error = CKpreProcessTaskState(object, ckobject))) {
        CKmemFree(ckobject, sizeof(CKThreadObject));
        return error;
    }

    /* Load the object dependencies */
    CKaddObjectDep(ckobject->addressSpace, (VERTEX *) ckobject);
    if(ckobject->parentKernel) CKaddObjectDep(ckobject->parentKernel, (VERTEX *) ckobject);

    /* Modify the object contents */
    object->self = (DESCRIPTOR) ckobject;

    return 0;   /* Success */
}


LOCAL DATA
CKunloadThreadObject(CKThreadObject *object)
/*
 * Unloads the specified thread object
 *
 * INPUT:
 *     object:  Pointer to object to be uncached 
 *
 * RETURNS:
 *     CKunloadThreadObject(): 0 for success, or otherwise error code
 */
{
    ThreadObject *thread = (ThreadObject *) object->head.objectData;

    if(currtask == object) {
        /*
         * Whoops... this one's tricky, the thread is trying to
         * shut itself down !!  We need to be careful not to get into
         * any race conditions with the scheduler, or terrible things
         * will happen to us.
         *
         * Actually, we need to save_flags()/cli()/restore_flags()
         * in the scheduler functions but I haven't implemented that
         * yet.  We need to revamp those anyway, so for now kludge
         * a bit by simply getting the scheduler lock.
         *
         * XXX --- Fix this:  kernel stack would be freed while we're
         *         still using it !!!
         *
         * XXX --- Another problem, if this is called in a cascade of
         *         dependency unloads, it'll be terminated before its
         *         time
         *
         * XXX --- This process is full of errors and bugs and big
         *         problems.  We need to think of a good way to free
         *         objects, also address spaces and kernels, that are
         *         currently in use.
         *         How UNIXoids do this: when a thread exits (we could
         *         extend this to objects, if we decide to use a similar
         *         system), it isn't directly unloaded but it becomes
         *         a zombie.  Then, the owner of the thread needs to
         *         to waitpid()... when it does, then the rest of the
         *         object is freed by the owner.
         */

        extern CKMutex schedLock;

        CKenterCritical(&schedLock);
    }

    /* Block the thread */
    CKblockThread(object);

    /* remove dependencies */
    CKremoveDepSearch(object->addressSpace, (VERTEX *) object);
    if(object->parentKernel) CKremoveDepSearch(object->parentKernel, (VERTEX *) object);

    /* remove from linked list */
    if(object->head.prevObject) object->head.prevObject->nextObject = object->head.nextObject;
    if(object->head.nextObject) object->head.nextObject->prevObject = object->head.prevObject;
    else threadObjHead = object->head.prevObject;

    /* writeback */
    thread->self = NIL;

    /* postprocess the taskstate (deallocate kernel stack etc.) */
    CKpostProcessTaskState(thread, object);

    /* free the object header memory */
    CKmemFree(object,sizeof(CKThreadObject));

    if(currtask == object) {
        /*
         * Okay, we're done, so we release the scheduler lock.
         * But we also need to make sure that we don't give
         * up the quantum before we've released all of the system
         * object manager locks, because we'll never be restarted
         * by now, so future sysobjman calls would lock up the
         * system eventually.
         *
         * Another problem is that the scheduler will try to access
         * the previous thread structure (in currtask), but, whoopsie,
         * we've just freed it !!!  Solution:  we set currtask to
         * the zombietask structure in sched.c, which should take
         * care of fooling the scheduler.
         *
         *
         * So:  (1) set currtask to fool the scheduler,
         *      (2) release all locks, and (3) surrender quantum
         */

        extern EVENT   zombietask;
        extern CKMutex schedLock;

        currtask = &zombietask;

        CKexitCritical(&sysObjManLock);
        CKexitCritical(&schedLock);

        quantumDone();
    }

    return 0;   /* Success */
}


/**************************************************************************/
/* Address Space object loading / unloading                               */
/**************************************************************************/

LOCAL DATA
CKloadAddrSpcObject(AddrSpcObject *object, ADDR size, DATA flags)
/*
 * Loads a new address space object, modifies its dependencies, validates
 * its data
 *
 * INPUT:
 *     object:  Pointer to object to be cached 
 *     size:    Object size (in bytes)         
 *     flags:   Flags                          
 *
 * RETURNS:
 *     object->self:          Descriptor of this object
 *     CKloadAddrSpcObject(): 0 for success, or otherwise error code
 */
{
    DATA error;
    CKAddrSpcObject *ckobject;

    /* Check for valid flags */
    if(flags & OFLOCKED) return ENOLOCKPERMS;  /* only the ABL can lock */

    /* Check whether the object is valid (as far as we can see) */
    if(object->parentKernel && (object->parentKernel != ((VOID *) -1))
                            && !CKvalidKernel(object->parentKernel))
        return ENOVALIDKERNEL;

    /* Allocate memory for object header in CK address space */
    ckobject = CKmemAlloc(sizeof(CKAddrSpcObject));
    if(!ckobject) return ENOMEM;

    /* Fill in the object structure */
    ckobject->head.objectSignature = (DATA) SIGADDRSPC;
    ckobject->head.flags           = flags;
    ckobject->head.objectData      = (ADDR) object;
    ckobject->head.dependencies    = 0;
    ckobject->head.firstDependency = NIL;
    ckobject->head.nextObject      = NIL;
    ckobject->head.prevObject      = addrSpcObjHead;
    if(addrSpcObjHead) addrSpcObjHead->nextObject = (VERTEX *) ckobject;
    addrSpcObjHead = (VERTEX *) ckobject;

    if(object->parentKernel != ((VOID *) -1)) {
        ckobject->parentKernel = object->parentKernel;
    } else if(currtask) {
        ckobject->parentKernel = currtask->parentKernel;
    } else {
        ckobject->parentKernel = NIL;
    }

    /* This sets up the hardware-dependant part of the address space */ 
    if((error = CKpreProcessAddrSpc(ckobject))) {
        CKmemFree(ckobject, sizeof(CKAddrSpcObject));
        return error;
    }

    /* Load the object dependencies */
    if(ckobject->parentKernel) CKaddObjectDep(ckobject->parentKernel, (VERTEX *) ckobject);

    /* Modify the object contents */
    object->self = (DESCRIPTOR) ckobject;

    return 0;   /* Success */
}

LOCAL DATA
CKunloadAddrSpcObject(CKAddrSpcObject *object)
/*
 * Unloads the specified address space object
 *
 * INPUT:
 *     object:  Pointer to object to be uncached 
 *
 * RETURNS:
 *     CKunloadAddrSpcObject(): 0 for success, or otherwise error code
 */
{
    AddrSpcObject *aspace = (AddrSpcObject *) object->head.objectData;

    /* remove dependencies */
    if(object->parentKernel) CKremoveDepSearch(object->parentKernel, (VERTEX *) object);

    /* remove from linked list */
    if(object->head.prevObject) object->head.prevObject->nextObject = object->head.nextObject;
    if(object->head.nextObject) object->head.nextObject->prevObject = object->head.prevObject;
    else addrSpcObjHead = object->head.prevObject;

    /* writeback */
    aspace->self = NIL;

    /* postprocess the address space */
    CKpostProcessAddrSpc(object);

    /* free the object header memory */
    CKmemFree(object,sizeof(CKAddrSpcObject));

    return 0;   /* Success */
}


/**************************************************************************/
/* Shared memory object loading / unloading                               */
/**************************************************************************/

LOCAL DATA
CKloadSHMemObject(SHMemObject *object, ADDR size, DATA flags)
/*
 * Loads a new shared memory object, modifies its dependencies, validates
 * its data
 *
 * INPUT:
 *     object:  Pointer to object to be cached
 *     size:    Object size (in bytes)
 *     flags:   Flags
 *
 * RETURNS:
 *     object->self:          Descriptor of this object
 *     CKloadSHMemObject(): 0 for success, or otherwise error code
 */
{
    DATA i;
    DATA *ptr;
    CKSHMemObject *ckobject;

    /* Check for valid flags */
    if(flags & OFLOCKED) return ENOLOCKPERMS;  /* only the ABL can lock */

    /* Allocate memory for object header in CK address space */
    ckobject = CKmemAlloc(sizeof(CKSHMemObject));
    if(!ckobject) return ENOMEM;

    /* Fill in the object structure */
    ckobject->head.objectSignature = (DATA) SIGSHMEM;
    ckobject->head.flags           = flags;
    ckobject->head.objectData      = (ADDR) object;
    ckobject->head.dependencies    = 0;
    ckobject->head.firstDependency = NIL;
    ckobject->head.nextObject      = NIL;
    ckobject->head.prevObject      = shMemObjHead;
    if(shMemObjHead) shMemObjHead->nextObject = (VERTEX *) ckobject;
    shMemObjHead = (VERTEX *) ckobject;

    /* Set up shared memory region */
    ckobject->shMem = (SHMEMOBJ *) CKmemAlloc(sizeof(SHMEMOBJ));
    if(!ckobject->shMem) {
        CKmemFree(ckobject, sizeof(CKSHMemObject));
        return ENOMEM;
    }
    ptr = (DATA *) ckobject->shMem;
    for(i=0; i<(sizeof(SHMEMOBJ)/sizeof(DATA)); i++) ptr[i]=0;
    ckobject->shMem->owner = currtask;

    /* Load the object dependencies */
    CKaddObjectDep((VERTEX *) ckobject->shMem->owner, (VERTEX *) ckobject);

    /* Modify the object contents */
    object->self = (DESCRIPTOR) ckobject;

    return 0;   /* Success */
}

LOCAL DATA
CKunloadSHMemObject(CKSHMemObject *object)
/*
 * Unloads the specified shared memory object
 *
 * INPUT:
 *     object:  Pointer to object to be uncached
 *
 * RETURNS:
 *     CKunloadSHMemObject(): 0 for success, or otherwise error code
 */
{
    SHMemObject *shmo = (SHMemObject *) object->head.objectData;

    /* remove dependencies */
    CKremoveDepSearch((VERTEX *) object->shMem->owner, (VERTEX *) object);

    /* remove from linked list */
    if(object->head.prevObject) object->head.prevObject->nextObject = object->head.nextObject;
    if(object->head.nextObject) object->head.nextObject->prevObject = object->head.prevObject;
    else shMemObjHead = object->head.prevObject;

    /* writeback */
    shmo->self = NIL;

    /* postprocess the shared memory object */
    /* detach all other threads, etc..      */
    CKshMemDestroy((DESCRIPTOR)object);

    /* free the shared memory object */
    CKmemFree(object->shMem, sizeof(SHMEMOBJ));

    /* free the object header memory */
    CKmemFree(object,sizeof(CKSHMemObject));

    return 0;   /* Success */
}


/**************************************************************************/
/* Kernel object loading / unloading                                      */
/**************************************************************************/

LOCAL DATA
CKloadKernelObject(KernelObject *object, ADDR size, DATA flags)
/*
 * Loads a new kernel object, modifies its dependencies, validates
 * its data
 *
 * INPUT:
 *     object:  Pointer to object to be cached 
 *     size:    Object size (in bytes)         
 *     flags:   Flags                          
 *
 * RETURNS:
 *     object->self:          Descriptor of this object
 *     CKloadKernelObject():  0 for success, or otherwise error code
 */
{
    DATA error;
    CKKernelObject *ckobject;

    /* Check for valid flags */
    if(flags & OFLOCKED) return ENOLOCKPERMS;  /* only the ABL can lock */

    /* Check whether the object is valid (as far as we can see) */
    if(!object->addrSpace || !CKvalidAddrSpc(object->addrSpace))
        return ENOVALIDADDRSPC;
    if(!object->sigHandler)
        return ENOSIGHANDL;

    /* Allocate memory for object header in CK address space */
    ckobject = CKmemAlloc(sizeof(CKKernelObject));
    if(!ckobject) return ENOMEM;

    /* Fill in the object structure */
    ckobject->head.objectSignature = (DATA) SIGKERNEL;
    ckobject->head.flags           = flags;
    ckobject->head.objectData      = (ADDR) object;
    ckobject->head.dependencies    = 0;
    ckobject->head.firstDependency = NIL;
    ckobject->head.nextObject      = NIL;
    ckobject->head.prevObject      = kernelObjHead;
    if(kernelObjHead) kernelObjHead->nextObject = (VERTEX *) ckobject;
    kernelObjHead = (VERTEX *) ckobject;

    ckobject->physregion = NIL;
    ckobject->addrSpace  = object->addrSpace;
    ckobject->sigHandler = object->sigHandler;

    /* This checks the hardware-dependant part of the kernel object */ 
    if((error = CKpreProcessKernel(object, ckobject))) {
        CKmemFree(ckobject, sizeof(CKKernelObject));
        return error;
    }

    /* Load the object dependencies */

    /* Modify the object contents */
    object->self = (DESCRIPTOR) ckobject;

    return 0;   /* Success */
}

LOCAL DATA
CKunloadKernelObject(CKKernelObject *object)
/*
 * Unloads the specified kernel object
 *
 * INPUT:
 *     object:  Pointer to object to be uncached 
 *
 * RETURNS:
 *     CKunloadKernelObject(): 0 for success, or otherwise error code
 */
{
    KernelObject *kernel = (KernelObject *) object->head.objectData;

    /* remove dependencies */

    /* remove from linked list */
    if(object->head.prevObject) object->head.prevObject->nextObject = object->head.nextObject;
    if(object->head.nextObject) object->head.nextObject->prevObject = object->head.prevObject;
    else kernelObjHead = object->head.prevObject;

    /* writeback */
    kernel->self = NIL;

    /* postprocess the address space */
    CKpostProcessKernel(kernel, object);

    /* free the object header memory */
    CKmemFree(object,sizeof(CKKernelObject));

    return 0;   /* Success */
}


/**************************************************************************/
/* Generic unload call                                                    */
/**************************************************************************/

LOCAL DATA CKunloadObject(VERTEX *object)
/*
 * Unloads the specified object from the CK   
 *
 * INPUT:
 *     object:        Descriptor of object to uncache
 *
 * RETURNS:
 *     CKunloadObject(): 0 for success, or otherwise error code
 */
{
    DATA i, error;

    if(object->flags & OFLOCKED) return ELOCKED;

    if(object->dependencies) {   /* Unload dependants */
        for(i=0;i<object->dependencies;i++) {
            if((error = CKunloadObject(object->firstDependency[i--].child)))
                return error;
        }
    }

    switch(object->objectSignature) {
        case SIGTHREAD:  /* Thread */
            return CKunloadThreadObject((CKThreadObject *)object);
            break;

        case SIGADDRSPC:
            return CKunloadAddrSpcObject((CKAddrSpcObject *)object);
            break;

        case SIGKERNEL:
            return CKunloadKernelObject((CKKernelObject *)object);
            break;

        case SIGSHMEM:
            return CKunloadSHMemObject((CKSHMemObject *)object);
            break;

        default:
            return EUNDEFOBJ;
    }
}

/**************************************************************************/
/* System Object Manager System Calls (platform independant)              */
/**************************************************************************/

PUBLIC inline DATA CKcacheObjectKernelCode(ADDR object, DATA signature, ADDR size, DATA flags)
/*
 * Caches the specified object into the CK   
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object:        Pointer to object to be cached
 *     signature:     Object type (i.e. 'AlOA')
 *     size:          Object size (in bytes)
 *     flags:         Flags
 *
 * RETURNS:
 *     object->self:    Descriptor of this object
 *     CKcacheObject(): 0 for success, or otherwise error code
 */
{
    DATA error;

    CKenterCritical(&sysObjManLock);

    switch(signature) {
        case SIGTHREAD:  /* Thread */
            error = CKloadThreadObject((ThreadObject *) object, size, flags);
            break;

        case SIGADDRSPC:
            error = CKloadAddrSpcObject((AddrSpcObject *) object, size, flags);
            break;

        case SIGKERNEL:
            error = CKloadKernelObject((KernelObject *) object, size, flags);
            break;

        case SIGSHMEM:  /* Shared Memory Object */
            error = CKloadSHMemObject((SHMemObject *) object, size, flags);
            break;

        default:
            error = EUNDEFOBJ;
    }

    CKexitCritical(&sysObjManLock);

    return error;
}

PUBLIC DATA inline CKunCacheObjectKernelCode(DESCRIPTOR object)
/*
 * Uncaches the specified object from the CK   
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object:        Descriptor of object to uncache
 *
 * RETURNS:
 *     CKunCacheObject(): 0 for success, or otherwise error code
 */
{
    DATA   i, error = 0;

    if(object->flags & OFLOCKED) return ELOCKED;

    CKenterCritical(&sysObjManLock);

    if(object->dependencies) {   /* Unload dependants */
        for(i=0;i<object->dependencies;i++) {
            error = CKunloadObject(object->firstDependency[i--].child);
        }
    }

    if(!error) {
        switch(object->objectSignature) {
            case SIGTHREAD:  /* Thread */
                error = CKunloadThreadObject((CKThreadObject *)object);
                break;

            case SIGADDRSPC:
                error = CKunloadAddrSpcObject((CKAddrSpcObject *)object);
                break;

            case SIGKERNEL:
                error = CKunloadKernelObject((CKKernelObject *)object);
                break;

            case SIGSHMEM:
                error = CKunloadSHMemObject((CKSHMemObject *)object);
                break;

            default:
                error = EUNDEFOBJ;
        }
    }

    CKexitCritical(&sysObjManLock);

    return error;
}

/**************************************************************************/
/* System Object Manager System Calls (platform dependant)                */
/* these functions is the same as above "Code" is just change to "Stub",  */
/* and some platform dependant stuff is done.                             */
/* ---------------------------------------------------------------------- */
/* Include Need to be here, CK(un)CacheObject is inline expanded in it    */
/* can't be linked, won't produce the same result, faster code.           */
/**************************************************************************/

#include "sys/stubs/objmgmt.h"

