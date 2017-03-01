/*
 * shmem.h:  Functions to manage the CK's shared memory
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 10/04/99  jens        1.0    First full internal release
 * 06/10/99  jens        1.1    Cleanup
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

#ifndef SHMEM_H
#define SHMEM_H

#define SHMEMSIGNALPRIORITY 5

#include "mutex.h"

typedef struct _SHThreadList {
    struct _SHThreadList *next;
    EVENT *thread;
    ADDR logicalAddr;
    ADDR size;
} SHTHREADLIST;

typedef struct {
    CKMutex mutex;
    EVENT *owner;
    BOOLEAN dead;

    ADDR logicalAddr,size;
    SHTHREADLIST  *firstInvited, *lastInvited;
    SHTHREADLIST  *firstThread,  *lastThread;
} SHMEMOBJ;

PUBLIC DATA CKshMemDestroy(DESCRIPTOR object);

#endif
