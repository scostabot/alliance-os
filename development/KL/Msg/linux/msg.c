/*
 * msg.c:  Message communication and buffer functions
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 16/04/99  ramon       1.0    First full internal release
 * 08/05/99  ramon       1.1    Adapted for use on linux
 * 14/06/99  jens        1.2    Fixed very small bug
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
 * This file is an adapted version of KL/Msg/msg.c, which is used
 * to emulate the message buffering system on linux.  This file uses
 * the signal emulator in signal.c, but combines the message buffering
 * calls with shared memory calls (a separate alliance shared memory
 * emulator would be complicated and there's no point in making one
 * anyway.)
 */

/* Linux includes */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* Alliance includes */
#include <typewrappers.h>
#include <klibs.h>
#include <mutex.h>
#include "msg.h"

/* Important defines */
#define SHMEMKEY 0xdeaf
#define SHMEMADR (BYTE *)0x200000

/* Global variables */
DATA  shmid, attached;
pid_t atthread = 0;  /* The thread we're communicating with */


/***************************************************************************/


MsgChannel KLopenMsgChannel(UADDR inisize)
{
    DATA i;
    MsgChannel shmem;
    UADDR size = (inisize / PAGESIZE) + ( (inisize%PAGESIZE) ? 1 : 0 );

    if(!size)
        size = 1;

    /* Set up shmem area */
    shmid = shmget(SHMEMKEY, size*PAGESIZE, 0777 | IPC_CREAT /* | IPC_EXCL */);
    if(shmid == -1) {
        if(errno == EEXIST)
            printf("msgemu: segment already exists!\n");

        printf("msgemu: error getting shared memory segment, errno=%d\n", errno);
        exit(255);
    }

    /* We attach the area at a fixed address so signaling isn't complicated */
    if((shmem = (MsgChannel) shmat(shmid, SHMEMADR, 0)) == (VOID *)-1) {
        printf("msgemu:  attach to shared memory area failed: errno=%d\n", errno);
        exit(255);
    }
    attached = 1;

    /* Clear the shmem area */
    memset(shmem, 0, size*PAGESIZE);

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

    /*
     * Ramon here was the problem, you did a mem free which creates a 
     * structure in the data area and then cleared it. 
     */

    for(i=0;i<((size*PAGESIZE)-(UDATA)(shmem->data-(UBYTE *)shmem));i++) shmem->data[i] = 0;
    KLfreeListMemFree(&shmem->bufFree, shmem->data, (size*PAGESIZE)-(UDATA)(shmem->data-(UBYTE *)shmem));

    /*
     * Return the virtual address of the allocated shared message channel
     */
    return shmem;
}


VOID KLcloseMsgChannel(MsgChannel chan)
{
    struct shmid_ds ds;

    if(atthread)
        sendSignal(atthread, SIGNLSHDEAD, (DATA)chan);

    if(shmdt((char *) chan)) {
        printf("msgemu:  detaching from shared memory area failed\n");
        exit(255);
    }

    if(shmctl(shmid, IPC_RMID, &ds)) {
        printf("msgemu:  destroying of shared memory area failed\n");
        exit(255);
    }
}


/***************************************************************************/

BOOLEAN KLmsgConnect(MsgChannel chan, pid_t thread)
{
    /* Mark who we're communicating with */
    atthread = thread;

    /* Invite a thread to join the shmem channel */
    sendSignal(thread, SIGNLSHINVI, shmid);

    /* Block until reply */
    waitSignal();

    /* Check whether the thread attached */
    if(attached == chan->attached)
        return FALSE;
    else
        attached = chan->attached;

    return TRUE;
}


MsgChannel KLmsgAccept(DATA shmid)
{
    MsgChannel shmem;

    /* Accept invitation to join message channel */
    if((shmem = (MsgChannel) shmat(shmid, SHMEMADR, 0)) == (VOID *)-1) {
        printf("msgemu:  attach to shared memory area failed: errno=%d\n", errno);
        exit(255);
    }

    /* Mark us attached */
    shmem->attached++;

    /* Send a signal back to notify that we're here      */
    sendSignal(atthread, SIGNLSHMRDY, (DATA)shmem);

    return shmem;
}


VOID KLmsgReject(DESCRIPTOR shmem)
{
    /* Send a signal back to notify that we reject the signal */
    sendSignal(atthread, SIGNLSHMRDY, 0);
}


VOID KLmsgDisconnect(MsgChannel chan)
{
    /* Mark us gone, and signal other thread that we are */
    chan->attached--;
    if(atthread)
        sendSignal(atthread, SIGNLSHMRDY, (DATA)chan);

    /* Disconnect self from the shmem channel    */
    if(shmdt((char *) chan)) {
        printf("msgemu:  detaching from shared memory area failed\n");
        exit(255);
    }
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
    sendSignal(atthread, SIGNLSHMRDY, (DATA)buffer);
    waitSignal();
}


VOID KLmsgSendAsync(MsgChannel chan, MsgBuf buffer)
{
    /* Signal the other connected threads and return */
    sendSignal(atthread, SIGNLSHMRDY, (DATA)buffer);
}

