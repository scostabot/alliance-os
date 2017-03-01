/*
 * ckkernelcalls.h:  The CK system calls, kernel code part
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 18/12/98  ramon       1.0    First release
 * 18/03/99  jens        1.1    Add PageMapping calls
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

#ifndef __CKKERNELCALLS_H
#define __CKKERNELCALLS_H

#include <typewrappers.h>
#include "sys/sysdefs.h"
#include "ckobjects.h"
#include "ckresource.h"

/* System Object Manager calls */
PUBLIC DATA CKcacheObjectKernelCode(ADDR object, DATA signature, ADDR size, DATA flags);
PUBLIC DATA CKunCacheObjectKernelCode(DESCRIPTOR object);

/* Thread Scheduler calls */
PUBLIC DATA CKswitchThreadStateKernelCode(DESCRIPTOR object);

/* Resource management calls */
PUBLIC DATA CKresourceAllocKernelCode(Resource *resource, DESCRIPTOR kernel, DATA flags);
PUBLIC DATA CKresourceFreeKernelCode(RESOURCE resource);

/* Pagemapping calls */

PUBLIC DATA CKloadMappingKernelCode(DESCRIPTOR object,ADDR physaddr,ADDR logicaladdr,ADDR length,UWORD32 flags);
PUBLIC DATA CKunloadMappingKernelCode(DESCRIPTOR object,ADDR logicaladdr,ADDR length);

#endif
