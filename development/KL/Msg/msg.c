/*
 * msg.c:  Message communication and buffer functions
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 16/04/99  ramon       1.0    First full internal release
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
#include <sys/memory.h>
#include <klibs.h>
#include <mutex.h>
#include "msg.h"


/***************************************************************************/


MsgChannel KLopenMsgChannel(UADDR inisize)
{
    DATA i;
    MsgChannel shmem;
    UADDR size = (inisize / PAGESIZE) + ( (inisize%PAGESIZE) ? 1 : 0 );

    if(!size)
        size = 1;

    /* Set up shmem area here */

    /*
     * Set up message channel and free list
     */
    exitCritical(&shmem->bufMutex);
    shmem->signature = SIGCHANNEL;
    shmem->attached  = 1;
    shmem->bufFree.FirstFreeEntry = NIL;
    for(i=0;i<8;i++) shmem->bufFree.FirstFreeSizeEntry[i] = NIL;
    shmem->bufFree.FreeMemory = 0;

    /*
     * We can't use the normal callbacks because they have to work in
     * both address spaces, and these don't
     */
    shmem->bufFree.AllocChunk = NIL;
    shmem->bufFree.PrepareFreeChunk = NIL;
    shmem->bufFree.FreeChunk = NIL;

    /*
     * Mark the rest of the shmem area free for buffer allocation and
     * clear it for security reasons
     */
    for(i=0;i<((size*PAGESIZE)-(UDATA)(shmem->data-(UBYTE *)shmem));i++) shmem->data[i] = 0;
    KLfreeListMemFree(&shmem->bufFree,shmem->data,(size*PAGESIZE)-(UDATA)(shmem->data-(UBYTE *)shmem));

    /*
     * Return the virtual address of the allocated shared message channel
     */
    return shmem;
}


VOID KLcloseMsgChannel(MsgChannel chan)
{
    /* Detach from the shmem area and stuff */
}


/***************************************************************************/


BOOLEAN KLmsgConnect(MsgChannel chan, DESCRIPTOR thread)
{
    /* Invite a thread to join the shmem channel */
    /* Block until reply                         */
}


BOOLEAN KLmsgConnectTimeout(MsgChannel chan, DESCRIPTOR thread, TIME timeout)
{
    /* Invite a thread to join the shmem channel */
    /* Block or timeout until reply              */
}


MsgChannel KLmsgAccept(DESCRIPTOR shmem)
{
    /* Accept invitation to join message channel */
}


VOID KLmsgReject(DESCRIPTOR shmem)
{
    /* Reject invitation to join message channel */
}


VOID KLmsgDisconnect(MsgChannel chan)
{
    /* Disconnect self from the shmem channel    */
}


/***************************************************************************/


MsgBuf KLallocMsgBuf(MsgChannel chan, UADDR size)
{
    MsgBuf newbuf;

    /*
     * Allocate the buffer of the specified size in the shared memory
     * area.  Grow the area if it's not big enough.  The free list is
     * protected using the area mutex
     */
    enterCritical(&chan->bufMutex);
    newbuf = KLfreeListMemAlloc(&chan->bufFree, size+sizeof(MsgBuf));
    exitCritical(&chan->bufMutex);
    if(!newbuf) {
        /* Allocate more shared memory */
        return NIL;
    }

    /*
     * Set the buffer signature and size
     */
    newbuf->signature = SIGBUFFER;
    newbuf->size = size;

    /*
     * Return the virtual address of the allocated buffer 
     */
    return newbuf;
}


BOOLEAN KLfreeMsgBuf(MsgChannel chan, MsgBuf buffer)
{
    /*
     * Check for buffer validity:  did we previously allocate this
     * buffer ?  Or are we trying to allocate free space outside the
     * shmem, or the buffer was corrupted ?  If so, just error out.
     */
    if(buffer->signature != SIGBUFFER) return FALSE;

    /*
     * Free the buffer in the freelist
     */
    enterCritical(&chan->bufMutex);
    KLfreeListMemFree(&chan->bufFree, buffer, buffer->size);
    exitCritical(&chan->bufMutex);

    /*
     * Always successful, really, once we get here
     */
    return TRUE;
}


/***************************************************************************/


VOID KLmsgSendSync(MsgChannel chan, MsgBuf buffer)
{
    /* Signal the other connected threads and block for one reply */
}


BOOLEAN KLmsgSendSyncTimeout(MsgChannel chan, MsgBuf buffer, TIME timeout)
{
    /*
     * Signal the other connected threads and block for one reply
     * or timout
     */
}


VOID KLmsgSendAsync(MsgChannel chan, MsgBuf buffer)
{
    /* Signal the other connected threads and return */
}

