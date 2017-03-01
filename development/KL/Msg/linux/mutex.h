/*
 * mutex.h:  Linux SHMEM-mutex functions
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 17/12/98  ramon       1.0    First internal release
 * 02/01/99  ramon       1.1    Avoiding processor lock in mutex code
 * 24/01/99  ramon       1.2    Killed awful mutex bug:  CKMutex volatile
 * 10/02/99  ramon       1.3    Made even more atomic, and faster
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

#ifndef __MUTEX_H
#define __MUTEX_H

#include <sched.h>
#include <typewrappers.h>

//typedef UDATA Mutex;


/*
 * XCHG(x,y):   Exchanges the contents of one memory location with a
 *              scalar, and returns the previous contents of the memory
 *              location.  Works on longs, shorts, as well as bytes.
 */

#define XCHG(x,y)                                                        \
    ({                                                                   \
        typeof(x) p;                                                     \
        asm ( "xchg %0, %1" : "=r" (p) : "m" (x), "0" (y) : "memory" );  \
        p;                                                               \
    })


LOCAL inline VOID enterCritical(Mutex *mutex)
/*
 * Enter a critical section protected by mutex
 *
 * INPUT:
 *     mutex:  Critical section mutex
 *
 * RETURNS:
 *     none
 */
{
    while(XCHG(*mutex,1)) sched_yield();  /* Poll for mutex             */
}


LOCAL inline VOID exitCritical(Mutex *mutex)
/*
 * Exit a critical section protected by mutex
 *
 * INPUT:
 *     mutex:  Critical section mutex
 *
 * RETURNS:
 *     none
 */
{
    *mutex = 0;   /* Exit from critical section */
}

#endif
