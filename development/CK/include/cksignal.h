/*
 * cksignal.h:  Signal codes passed on to signal handlers by the CK
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 23/12/98  ramon       1.0    First release
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

#ifndef __CKSIGNAL_H
#define __CKSIGNAL_H

#include <typewrappers.h>
#include "ckobjects.h"

/* Signal defenitions */
#define SIGNLINT    0x01   /* Interrupt occured signal */
#define SIGNLWRTBK  0x02   /* Object writeback signal  */
#define SIGNLSHINVI 0x03   /* Shmem invite signal      */
#define SIGNLSHMRDY 0x04   /* Shmem message ready      */
#define SIGNLSHDEAD 0x05   /* Shmem connect broken     */
#define SIGNLSHRMAP 0x06   /* Shmem phys layout changed*/
#define SIGNLSHNOTIFY 0x07 /* Shmem notify             */

/* Signal functions declaration */
DATA CKsendSignal(EVENT *thread, UDATA signal, UDATA extra, UDATA priority);

#endif
