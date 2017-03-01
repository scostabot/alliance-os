/*
 * shmem.c:  Functions to manage the CK's shared memory
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 10/04/99  jens        1.0    First full internal release
 * 25/05/99  jens        1.1    Now uses DESCRIPTOR's
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
#include <klibs.h>
#include "sys/sys.h"
#include "mutex.h"
#include "memory.h"
#include "shmem.h"
#include "ckerror.h"
#include "cksignal.h"
#include "sched.h"

//LOCAL CKMutex CKShMemLock;

SHTHREADLIST *fetchThread(SHTHREADLIST *stl,EVENT *thread,SHTHREADLIST **prev) {
    *prev=NIL;
    while(stl!=NIL) {
        if(stl->thread==thread) break;
        *prev=stl;
        stl=stl->next;
    }
    return stl;
}

/*SHPHYSMEMLIST *fetchPhys(SHMEMOBJ *smo,ADDR physAddr,SHPHYSMEMLIST **prev) {
    SHPHYSMEMLIST *spl=smo->firstPhys;
    *prev=NIL;
    while(spl!=NIL) {
        if(spl->physAddr==physAddr) break;
        *prev=spl;
        spl=spl->next;
    }
    return spl;
}*/

LOCAL DATA inline CKshMemInviteKernelCode(DESCRIPTOR object,EVENT *thread) {

    SHMEMOBJ *smo;
    struct CKSHMemObject *shobj;
    SHTHREADLIST *stl;

    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    shobj=(struct CKSHMemObject *)object;
    smo=shobj->shMem;

    if(smo->dead) return ENOVALIDSHMEM;                   /* object is dead? */
    stl=(SHTHREADLIST *)CKmemAlloc(sizeof(SHTHREADLIST)); /* alloc space     */
    if(stl==NIL) return ENOMEM;                           /* not enough      */
    stl->thread=thread;                                   /* set thread      */
    stl->next=NIL;                                        /* clr next ptr    */

    CKenterCritical(&smo->mutex);                         /* enter critical  */
    if(smo->firstInvited==NIL)				              /* add to invite-  */
        smo->firstInvited=stl;				              /* list            */
    if(smo->lastInvited!=NIL)
        smo->lastInvited->next=stl;
    smo->lastInvited=stl;
    CKexitCritical(&smo->mutex);                          /* exit critical   */

    CKsendSignal(thread, SIGNLSHINVI, (UDATA)object, SHMEMSIGNALPRIORITY);
    return 0;					                          /* success         */
}

LOCAL DATA inline CKshMemSetRangeKernelCode(DESCRIPTOR object,ADDR logicalAddr,ADDR size) {
    SHMEMOBJ *smo;
    struct CKSHMemObject *shobj;
    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    shobj=(struct CKSHMemObject *)object;
    smo=shobj->shMem;

    if(currtask!=smo->owner) return ENOSECURITY;
    if(smo->dead) return ENOVALIDSHMEM;                   /* object is dead? */
    CKenterCritical(&smo->mutex);                         /* enter critical  */
    smo->logicalAddr=logicalAddr;
    smo->size=size;
    CKexitCritical(&smo->mutex);                          /* exit critical  */
    return 0;
}

PUBLIC DATA CKshMemDestroy(DESCRIPTOR object)
{
    SHMEMOBJ *smo;
    struct CKSHMemObject *shobj;
    EVENT *thrd;
    SHTHREADLIST *stl,*ctl;

    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    shobj=(struct CKSHMemObject *)object;
    smo=shobj->shMem;

    if(currtask!=smo->owner) return ENOSECURITY;
    if(smo->dead) return ENOVALIDSHMEM;                   /* object is dead?*/

    CKenterCritical(&smo->mutex);                         /* enter critical */

    /* mark object as dead */
    smo->dead=TRUE;

    /* clear invite list, signal the loss of a shmemobj, free used memory */
    stl=smo->firstInvited;
    /* no one is invited anyone */
    smo->firstInvited=NULL;
    smo->lastInvited=NULL;
    while(stl!=NIL) {
        ctl=stl; stl=stl->next;
        thrd=ctl->thread;
        CKmemFree(ctl,sizeof(SHTHREADLIST));
        CKexitCritical(&smo->mutex);                      /* exit critical  */
        CKsendSignal(thrd, SIGNLSHDEAD, (UDATA)object, SHMEMSIGNALPRIORITY);
        CKenterCritical(&smo->mutex);                     /* enter critical */
    }
    CKexitCritical(&smo->mutex);                          /* exit critical  */

    /* detach attached threads and signal the loss of a shmemobj */
    stl=smo->firstThread;
    while(stl!=NIL) {
        CKsendSignal(stl->thread, SIGNLSHDEAD, (UDATA)object, SHMEMSIGNALPRIORITY);
        stl=stl->next;
    }

    /* wait for all threads to detach */

    return 0; /* success */
}

LOCAL DATA inline CKshMemAttachKernelCode(DESCRIPTOR object,ADDR logAddr,ADDR size)
{
    SHMEMOBJ *smo;
    struct CKSHMemObject *shobj;
    SHTHREADLIST *stl,*ptl;
//    SHPHYSMEMLIST *spl;
    VERTEX *objHead,*ownerObjHead;
    struct CKAddrSpcObject *addrspc,*owneraddrspc;
//    ADDR cursz=0,allsz=0;
    DATA sz;

    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    shobj=(struct CKSHMemObject *)object;
    smo=shobj->shMem;

    if(currtask==smo->owner) return ENOSECURITY;          /* currtask is owner */
    if(smo->dead) return ENOVALIDSHMEM;                   /* object is dead?*/

    /* fetch addressSpace object */

    objHead=(VERTEX *)currtask->addressSpace;
    if(objHead==NIL) return ENOVALIDADDRSPC;
    if(!CKvalidAddrSpc(objHead)) return ENOVALIDADDRSPC;

    ownerObjHead=(VERTEX *)smo->owner->addressSpace;
    if(ownerObjHead==NIL) return ENOVALIDADDRSPC;
    if(!CKvalidAddrSpc(ownerObjHead)) return ENOVALIDADDRSPC;

    addrspc=(struct CKAddrSpcObject *)objHead;
    owneraddrspc=(struct CKAddrSpcObject *)ownerObjHead;

    CKenterCritical(&smo->mutex);                       /* enter critical  */
    stl=fetchThread(smo->firstInvited,currtask,&ptl);
    if(stl==NIL) {
        CKexitCritical(&smo->mutex);                    /* exit critical  */
        return ENOSECURITY;
    }
    sz=smo->size;
    if(size<sz) sz=size;
    if(!CKcopySharedPageMapping(owneraddrspc->addrSpc, smo->logicalAddr,
                                addrspc->addrSpc, logAddr, sz)) {
        CKexitCritical(&smo->mutex);
        return ENOSECURITY;
    }

    /* detach invited thread */
    if(stl==smo->lastInvited) smo->lastInvited=ptl;
    if(ptl==NIL) smo->firstInvited=stl->next; else ptl->next=stl->next;

    /* fill out information */

    stl->next=NIL;          /* clr next ptr    */
    stl->logicalAddr=logAddr;
    stl->size=sz;

    /* attach thread to threadlist */

    if(smo->firstThread==NIL) smo->firstThread=stl;
    if(smo->lastThread!=NIL) smo->lastThread->next=stl;
    smo->lastThread=stl;
    CKexitCritical(&smo->mutex);                        /* exit critical  */
    return 0; /* success */
}

LOCAL DATA inline CKshMemDetachKernelCode(DESCRIPTOR object) {
    SHMEMOBJ *smo;
    struct CKSHMemObject *shobj;
    VERTEX *objHead=(VERTEX *)currtask->addressSpace;
    SHTHREADLIST *stl,*ptl;
    struct CKAddrSpcObject *addrspc;

    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    shobj=(struct CKSHMemObject *)object;
    smo=shobj->shMem;

    if(smo->dead) return ENOVALIDSHMEM;                 /* object is dead?       */
    if(objHead==NIL) return ENOVALIDADDRSPC;
    if(!CKvalidAddrSpc(objHead)) return ENOVALIDADDRSPC;
    addrspc=(struct CKAddrSpcObject *)objHead;

    CKenterCritical(&smo->mutex);                       /* enter critical        */
    stl=fetchThread(smo->firstThread,currtask,&ptl);    /* is curThread in list? */
    if(stl==NIL) {
        CKexitCritical(&smo->mutex);                    /* enter critical        */
        return ENOSECURITY;                             /* No, return error code */
    }

    /* fetch addressSpace object */

    CKmarkPageNotPresent(addrspc->addrSpc,stl->logicalAddr,stl->size,0);

    /* detach from thread list */

    if(stl==smo->lastThread) smo->lastThread=ptl;
    if(ptl==NIL) smo->firstThread=stl->next; else ptl->next=stl->next;

    if(smo->dead) {
        /* SHMemObj is dead, just release the memory */
        CKmemFree(stl,sizeof(SHTHREADLIST));
    } else {
        /* attach to invite list */
        stl->next=NIL;
        if(smo->firstInvited==NIL) smo->firstInvited=stl;
        if(smo->lastInvited!=NIL) smo->lastInvited->next=stl;
        smo->lastInvited=stl;
    }
    CKexitCritical(&smo->mutex);                        /* enter critical        */
    return 0; /* success */
}

LOCAL DATA inline CKshMemRemapKernelCode(DESCRIPTOR object,ADDR logAddr,ADDR size) {
    DATA err;

    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    err=CKshMemDetachKernelCode(object);                      /* detach memory area    */
    if(err!=0) return err;
    return CKshMemAttachKernelCode(object,logAddr,size);  /* attach new memory area*/
}

LOCAL DATA inline CKshMemNotifyKernelCode(DESCRIPTOR object) {
    SHMEMOBJ *smo;
    struct CKSHMemObject *shobj;
    EVENT *thrd;
    SHTHREADLIST *stl,*ctl;

    if(!CKvalidSHMem(object)) return ENOVALIDSHMEM;
    shobj=(struct CKSHMemObject *)object;
    smo=shobj->shMem;
    if(currtask!=smo->owner) return ENOSECURITY;
    if(smo->dead) return ENOVALIDSHMEM;                   /* object is dead?*/

    /* clear invite list, signal the loss of a shmemobj, free used memory */
    stl=smo->firstInvited;
    while(stl!=NIL) {
        ctl=stl; stl=stl->next;
        thrd=ctl->thread;
        CKsendSignal(thrd, SIGNLSHNOTIFY, (UDATA)object, SHMEMSIGNALPRIORITY);
    }
    return 0; /* success */
}

/**************************************************************************/
/* System Shared Memory System Calls (platform dependant)                 */
/* these functions is the same as above "Code" is just change to "Stub",  */
/* and some platform dependant stuff is done.                             */
/* ---------------------------------------------------------------------- */
/* Include Need to be here, CKshMem calls are inline expanded in it       */
/* can't be linked, won't produce the same result, faster code.           */
/**************************************************************************/

#include "sys/stubs/shmem.h"
