/*
 * FreelistAlloc.c:  Functions to manage the memory
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 14/03/99  jens        1.0    Moved from CK to KLlibs
 * 15/03/99  jens        1.1    Fix little bug in Callbacks
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

LOCAL DATA KLgetFastLookupToUse(UADDR size)
{
    if(size>=(1<<13)) {
        if(size>=(1<<17)) {
            if(size>=(1<<19)) {
                return 7;
            } else {
                return 6;
            }
        } else {
            if(size>=(1<<15)) {
                return 5;
            } else {
                return 4;
            }
        }
    } else {
        if(size>=(1<<9)) {
            if(size>=(1<<11)) {
                return 3;
            } else {
                return 2;
            }
        } else {
            if(size>=(1<<7)) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}

LOCAL VOID inline KLfreeListSizeRemove(KLfreeListDescriptor *Desc,KLfreeListEntry *entry) {
    if(entry->nextsize!=NIL)
        entry->nextsize->prevsize=entry->prevsize;
    if(entry->prevsize!=NIL) {
        entry->prevsize->nextsize=entry->nextsize;
    } else {
        Desc->FirstFreeSizeEntry[entry->lookupsize]=entry->nextsize;
    }
}

LOCAL VOID inline KLfreeListSizeInsert(KLfreeListDescriptor *Desc,KLfreeListEntry *entry) {
    DATA TableNo=KLgetFastLookupToUse(entry->size);
    entry->lookupsize=TableNo;
    entry->prevsize=NIL;
    entry->nextsize=Desc->FirstFreeSizeEntry[TableNo];
    if(entry->nextsize!=NIL) entry->nextsize->prevsize=entry;
    Desc->FirstFreeSizeEntry[TableNo]=entry;
}

LOCAL VOID KLfreeListRemove(KLfreeListDescriptor *Desc,KLfreeListEntry *entry) {
    if(entry->prev!=NIL) entry->prev->next=entry->next; else Desc->FirstFreeEntry=entry->next;
    if(entry->next!=NIL) entry->next->prev=entry->prev;
    KLfreeListSizeRemove(Desc,entry);
}

LOCAL VOID KLfreeListInsert(KLfreeListDescriptor *Desc,KLfreeListEntry *preventry,KLfreeListEntry *entry,KLfreeListEntry *nextentry) {
    entry->prev=preventry;
    entry->next=nextentry;
    if(preventry!=NIL) preventry->next=entry; else Desc->FirstFreeEntry=entry;
    if(nextentry!=NIL) nextentry->prev=entry;
    KLfreeListSizeInsert(Desc,entry);
}

#define AdjustSize if(size<sizeof(KLfreeListEntry)) size=sizeof(KLfreeListEntry); else size=(size+3)&-4;

LOCAL VOID KLfreeListMemFreeChunk(KLfreeListDescriptor *Desc,KLfreeListEntry *preventry,KLfreeListEntry *entry,KLfreeListEntry *nextentry) {
    UADDR intdata,orgdata;
    UADDR size,orgsize;
    UADDR prevsize,nextsize;
    if(!Desc->DisallowFreeChunk) {
        if((Desc->PrepareFreeChunk!=NIL)&&(Desc->FreeChunk!=NIL)) {
            orgdata=intdata=(UADDR)entry;
            orgsize=size=entry->size;
            if(Desc->PrepareFreeChunk(&intdata,&size)) {
                prevsize=intdata-orgdata;
                nextsize=(intdata+size)-(orgdata+orgsize);
                if((prevsize!=0)&&(prevsize>sizeof(KLfreeListEntry))) {
                    if((nextsize==0)||(nextsize>sizeof(KLfreeListEntry))) {
                        Desc->FreeChunk(intdata,size);
                        if(prevsize!=0) {
                            entry=(KLfreeListEntry *)orgdata;
                            entry->size=prevsize;
                            KLfreeListInsert(Desc,preventry,entry,nextentry);
                            preventry=entry;
                        }
                        if(nextsize!=0) {
                            entry=(KLfreeListEntry *)(intdata+size);
                            entry->size=nextsize;
                            KLfreeListInsert(Desc,preventry,entry,nextentry);
                        }
                    }
                }
                return;
            }
        }
    }
    KLfreeListInsert(Desc,preventry,entry,nextentry);
}

PUBLIC VOID *KLfreeListMemAlloc(KLfreeListDescriptor *Desc,UADDR size)
{
    KLfreeListEntry *entry;
    KLfreeListEntry *newentry,*preventry,*nextentry,*bestentry=NIL;
    UADDR result,data,left,bestleft=(UADDR)-1;
    DATA TableNo;
    BOOLEAN firsttime=TRUE;

    /* the minimum size allocatable is the size of
     * the structure CKFreeListEntry.
     */

    AdjustSize
retry:
    /* search the size lookup table for free entry big enough */
    for(TableNo=KLgetFastLookupToUse(size);TableNo<8;TableNo++) {
        entry=Desc->FirstFreeSizeEntry[TableNo];    /* get first size entry  */
        while(entry!=NIL) {                         /* are we done?          */
            if(size<=entry->size) {                 /* is entry big enough   */
                left=entry->size-size;              /* found a entry         */
                if(left!=0) {                       /* is it a perfect match */
                    if(left>=sizeof(KLfreeListEntry)) {
                                                    /* is there room for the */
                                                    /* freeentry header?     */
                        if(left<=bestleft) {        /* is this better than   */
                                                    /* last the try?         */
                            bestleft=left;          /* yes, store it for     */
                            bestentry=entry;        /* future use.           */
                        }
                    }
                } else {
                    result=(UADDR)entry;            /* it's a perfect match. */
                    KLfreeListRemove(Desc,entry);   /* remove entry from list*/
                    Desc->FreeMemory-=size;         /* decrease memory free. */
                    return (VOID *)result;          /* return result address.*/
                }
            }
            entry=entry->nextsize;                  /* get next entry.       */
        }
        if(bestentry!=NIL) {                        /* no entry was found.   */
            result=(UADDR)bestentry;                /* get result address.   */
            data=result+size;                       /* calc addr of new free */
                                                    /* list entry.           */
            preventry=bestentry->prev;              /* get old previous entry*/
            nextentry=bestentry->next;              /* get old next entry.   */
            KLfreeListRemove(Desc,bestentry);       /* remove old entry.     */
            newentry=(KLfreeListEntry *)data;       /* create new entry.     */
            newentry->size=bestleft;                /* set the size to       */
                                                    /* remaining bytes free. */
            KLfreeListInsert(Desc,preventry,newentry,nextentry);
                                                    /* insert entry into free*/
                                                    /* list.                 */
            Desc->FreeMemory-=size;                 /* decrease memory free. */
            return (VOID *)result;                  /* return result address.*/
        }
    }
    if(firsttime) {                                 /* is this a first time  */
                                                    /* out of memory failure?*/
        if(Desc->AllocChunk!=NULL) {                /* is AllocChunk avail?  */
            firsttime=FALSE;                        /* not first time anymore*/
            left=size;                              /* store size            */
            data=Desc->AllocChunk(&left);           /* call AllocChunk       */
            if(data!=0) {                           /* got memory?           */
                Desc->DisallowFreeChunk=FALSE;      /* make sure it does not */
                                                    /* free it immediately.  */
                KLfreeListMemFree(Desc,(VOID *)data,left);
                                                    /* add it to free list.  */
                Desc->DisallowFreeChunk=TRUE;       /* let the free release  */
                                                    /* big blocks again.     */
                goto retry;                         /* retry the alloc.      */
            }
        }
    }
    return NIL;                                     /* error : no memory was */
                                                    /* available             */
}

PUBLIC VOID *KLfreeListMemAllocAlign(KLfreeListDescriptor *Desc,UADDR size,DATA alignment)
{
    KLfreeListEntry *entry;
    KLfreeListEntry *newentry,*preventry,*nextentry,*bestentry=NIL;
    UADDR result=0,data,left,bestleft=(UADDR)-1;
    UADDR alignadd=(1<<alignment)-1,alignmask=-(1<<alignment);
    DATA TableNo;
    BOOLEAN firsttime=TRUE;

    /* the minimum size allocatable is the size of
     * the structure CKFreeListEntry.
     */

    AdjustSize
retry:
    for(TableNo=KLgetFastLookupToUse(size);TableNo<8;TableNo++) {
        entry=Desc->FirstFreeSizeEntry[TableNo];
        while(entry!=NIL) {
            if(size<=entry->size) {
                data=((UADDR)entry+alignadd)&alignmask;
                left=(data-(UADDR)entry);
                if(left!=0) {
                    if(left<sizeof(KLfreeListEntry)) {
                        data=((UADDR)entry+sizeof(KLfreeListEntry)+alignadd)&alignmask;
                        left=(data-(UADDR)entry);
                    }
                    if(entry->size<left) {
                        left=0;
                    } else {
                        left=entry->size-left;
                    }
                } else left=entry->size;
                if(left>=size) {
                    left-=size;
                    if(left!=0) {
                        if(left>=sizeof(KLfreeListEntry)) {
                            if(left<=bestleft) {
                                bestleft=left;
                                bestentry=entry;
                                result=data;
                            }
                        }
                    } else {
                        result=data;
                        if(data==(UADDR)entry) {
                            KLfreeListRemove(Desc,entry);
                        } else {
                            KLfreeListSizeRemove(Desc,entry);
                            entry->size=data-(UADDR)entry;
                            KLfreeListSizeInsert(Desc,entry);
                        }
                        Desc->FreeMemory-=size;
                        return (VOID *)result;
                    }
                }
            }
            entry=entry->nextsize;
        }
        if(bestentry!=NIL) {
            preventry=bestentry->prev;
            nextentry=bestentry->next;
            data=(UADDR)bestentry;
            KLfreeListRemove(Desc,bestentry);
            if(result!=data) {
                newentry=(KLfreeListEntry *)data;
                newentry->size=result-data;
                KLfreeListInsert(Desc,preventry,newentry,nextentry);
                preventry=newentry;
            }
            newentry=(KLfreeListEntry *)(result+size);
            newentry->size=bestleft;
            KLfreeListInsert(Desc,preventry,newentry,nextentry);
            Desc->FreeMemory-=size;
            return (VOID *)result;
        }
    }
    if(firsttime) {
        if(Desc->AllocChunk!=NULL) {
            firsttime=FALSE;
            left=size;
            data=Desc->AllocChunk(&left);
            if(data!=0) {
                Desc->DisallowFreeChunk=FALSE;
                KLfreeListMemFree(Desc,(VOID *)data,left);
                Desc->DisallowFreeChunk=TRUE;
                goto retry;
            }
        }
    }
    return NIL;
}

PUBLIC VOID *KLfreeListMemAllocRegion(KLfreeListDescriptor *Desc,UADDR data,UADDR size)
{
    KLfreeListEntry *entry,*preventry,*nextentry,*newentry;
    DATA TableNo;
    UADDR left,result;

    AdjustSize
    for(TableNo=KLgetFastLookupToUse(size);TableNo<8;TableNo++) {
        entry=Desc->FirstFreeSizeEntry[TableNo];
        while(entry!=NIL) {
            if(data>=(UADDR)entry) {
                left=data-(UADDR)entry;
                if(left!=0) {
                    if(left<sizeof(KLfreeListEntry)) {
                        return NIL; /* Not enough room */
                    }
                    if(entry->size<left) {
                        left=0;
                    } else {
                        left=entry->size-left;
                    }
                } else left=entry->size;
                if(left>=size) {
                    left-=size;
                    result=data;
                    if(left==0) {
                        if(data==(UADDR)entry) {
                            KLfreeListRemove(Desc,entry);
                        } else {
                            KLfreeListSizeRemove(Desc,entry);
                            entry->size=data-(UADDR)entry;
                            KLfreeListSizeInsert(Desc,entry);
                        }
                        Desc->FreeMemory-=size;
                        return (VOID *)result;
                    } else {
                        preventry=entry->prev;
                        nextentry=entry->next;
                        data=(UADDR)entry;
                        KLfreeListRemove(Desc,entry);
                        if(result!=data) {
                            newentry=(KLfreeListEntry *)data;
                            newentry->size=result-data;
                            KLfreeListInsert(Desc,preventry,newentry,nextentry);
                            preventry=newentry;
                        }
                        newentry=(KLfreeListEntry *)(result+size);
                        newentry->size=left;
                        KLfreeListInsert(Desc,preventry,newentry,nextentry);
                        Desc->FreeMemory-=size;
                        return (VOID *)result;
                    }
                }
            } else break;
            entry=entry->nextsize;
        }
    }
    return NIL;
}

PUBLIC VOID KLfreeListMemFree(KLfreeListDescriptor *Desc,VOID *data,UADDR size) {
    UADDR intdata=(UADDR)data;
    KLfreeListEntry *nextentry,*preventry=NIL;
    KLfreeListEntry *entry;

    AdjustSize
    nextentry=Desc->FirstFreeEntry;
    Desc->FreeMemory+=size;
    if(nextentry!=NIL) {
        while(nextentry->next!=NIL) {
            if(intdata<(UADDR)nextentry) break;
            nextentry=nextentry->next;
        }
        if(intdata<(UADDR)nextentry) {
            preventry=nextentry->prev;
        } else {
            preventry=nextentry;
            nextentry=NIL;
        }
        if(preventry!=NIL) {
            if((UADDR)preventry+preventry->size==intdata) {
                entry=preventry;
                intdata=(UADDR)entry;
                size+=entry->size;
                preventry=preventry->prev;
                KLfreeListRemove(Desc,entry);
            }
        }

        if(nextentry!=NIL) {
            if(intdata+size==(UADDR)nextentry) {
                size+=nextentry->size;
                entry=nextentry;
                nextentry=nextentry->next;
                KLfreeListRemove(Desc,entry);
                entry=(KLfreeListEntry *)intdata;
                entry->size=size;
                KLfreeListMemFreeChunk(Desc,preventry,entry,nextentry);
                return;
            }
        }
    }
    entry=(KLfreeListEntry *)intdata;
    entry->size=size;
    KLfreeListMemFreeChunk(Desc,preventry,entry,nextentry);
}
#undef AdjustSize
