/*
 * sched.h:  Defenitions for the scheduler
 *
 * NOTE:  modified 8/3/99 for the scheduler simulation [Ramon]
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
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

#ifndef __SCHED_H
#define __SCHED_H

#include <typewrappers.h>
#include "ckobjects.h"

#define MAXEVENTS 1024      /* Maximum scheduler events at one time */

extern EVENT *currtask;
extern TIME realtime;

VOID CKinitSched(VOID);

VOID CKblockThread(EVENT *thread);
VOID CKunblockThread(EVENT *thread);
VOID CKidleThread(EVENT *thread);

#endif
