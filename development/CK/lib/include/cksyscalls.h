/*
 * cksyscalls.h:  The CK system calls
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 27/11/98  ramon       1.0    First release
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

#ifndef __CKSYSCALLS_H
#define __CKSYSCALLS_H

#include <typewrappers.h>
#include "sys/sysdefs.h"
#include "ckobjects.h"

/* System Object Manager calls */
PUBLIC DATA CKcacheObject(ADDR object, DATA signature, ADDR size, DATA flags);
PUBLIC DATA CKunCacheObject(DESCRIPTOR object);

/* Thread Scheduler calls */
PUBLIC DATA CKsetThreadState(DESCRIPTOR object,ThreadState state);
PUBLIC DATA CKthreadTunnel(DESCRIPTOR object,TunnelType type);

/* Resource management calls */
PUBLIC DATA CKresourceAlloc(VOID *resource, DESCRIPTOR kernel, DATA flags);
PUBLIC DATA CKresourceFree(RESOURCE resource);
 
/* Pagemapping calls */

PUBLIC DATA CKloadMapping(DESCRIPTOR object,ADDR physaddr,ADDR logicaladdr,ADDR length,UWORD32 flags);
PUBLIC DATA CKunloadMapping(DESCRIPTOR object,ADDR logicaladdr,ADDR length);

/* Shared memory calls */

PUBLIC DATA CKshMemSetRange(DESCRIPTOR object,ADDR logAddr,ADDR size);
PUBLIC DATA CKshMemInvite(DESCRIPTOR object,EVENT *thread);
PUBLIC DATA CKshMemRemap(DESCRIPTOR object,ADDR logAddr,ADDR size);
PUBLIC DATA CKshMemAttach(DESCRIPTOR object,ADDR logAddr,ADDR size);
PUBLIC DATA CKshMemDetach(DESCRIPTOR object);
PUBLIC DATA CKshMemNotify(DESCRIPTOR object);

#endif
