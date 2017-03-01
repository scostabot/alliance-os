/*
 * msg.h:  Message communication and buffer structures for linux
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

#ifndef __MSG_H
#define __MSG_H

#include <typewrappers.h>
#include <klibs.h>
#include <mutex.h>

#define SIGBUFFER  0x41427566
#define SIGCHANNEL 0x4143686e

typedef DATA DESCRIPTOR;

typedef struct {
    UDATA signature;
    UADDR size;
    UBYTE data[0];
} *MsgBuf;

typedef struct {
    UDATA signature;
    DESCRIPTOR shmem;
    DATA attached;
    Mutex bufMutex;
    KLfreeListDescriptor bufFree;
    UBYTE data[0];
} *MsgChannel;

MsgChannel KLopenMsgChannel(UADDR inisize);
VOID KLcloseMsgChannel(MsgChannel chan);
BOOLEAN KLmsgConnect(MsgChannel chan, pid_t thread);
MsgChannel KLmsgAccept(DATA shmid);
VOID KLmsgReject(DESCRIPTOR shmem);
VOID KLmsgDisconnect(MsgChannel chan);
MsgBuf KLallocMsgBuf(MsgChannel chan, UADDR size);
BOOLEAN KLfreeMsgBuf(MsgChannel chan, MsgBuf buffer);
VOID KLmsgSendSync(MsgChannel chan, MsgBuf buffer);
VOID KLmsgSendAsync(MsgChannel chan, MsgBuf buffer);

/***************************************************************/

#define SIGNLSHINVI 0x03   /* Shmem invite signal      */
#define SIGNLSHMRDY 0x04   /* Shmem message ready      */
#define SIGNLSHDEAD 0x05   /* Shmem connect broken     */
#define SIGNLSHRMAP 0x06   /* Shmem phys layout changed*/

VOID sendSignal(pid_t thread, DATA signal, DATA extra);
VOID waitSignal(VOID);
VOID initSignalEmu(VOID *handl);

/***************************************************************/

#define PAGESIZE 0x1000

#endif
