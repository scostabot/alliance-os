/*
 * mutex.h:  Mutex functions for the CK
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

#include <typewrappers.h>
#include "sys/arith.h"
#include "sys/realtime.h"


typedef UDATA CKMutex;


LOCAL inline VOID CKenterCritical(CKMutex *mutex)
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
    while(XCHG(*mutex,1)) quantumDone();  /* Poll for mutex             */
}


LOCAL inline VOID CKexitCritical(CKMutex *mutex)
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
