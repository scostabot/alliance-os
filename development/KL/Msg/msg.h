/*
 * msg.h:  Message communication and buffer structures
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
#include "ckobjects.h"

#define SIGBUFFER  0x41427566
#define SIGCHANNEL 0x4143686e

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

#endif
