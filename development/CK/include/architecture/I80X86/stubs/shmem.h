/*
 * shmem.h: shared memory call stubs
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 02/05/99  jens        1.0    First full internal release
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

#ifndef __SYS_STUBS_SHMEM_H
#define __SYS_STUBS_SHMEM_H

#include <typewrappers.h>
#include "ckobjects.h"

PUBLIC DATA CKshMemSetRangeKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,DESCRIPTOR object,ADDR logAddr,ADDR size)
{
	return CKshMemSetRangeKernelCode(object,logAddr,size);
}

PUBLIC DATA CKshMemInviteKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,DESCRIPTOR object,EVENT *thread)
{
	return CKshMemInviteKernelCode(object,thread);
}

PUBLIC DATA CKshMemRemapKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,DESCRIPTOR object,ADDR logAddr,ADDR size)
{
	return CKshMemRemapKernelCode(object,logAddr,size);
}

PUBLIC DATA CKshMemAttachKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,DESCRIPTOR object,ADDR logAddr,ADDR size)
{
	return CKshMemAttachKernelCode(object,logAddr,size);
}

PUBLIC DATA CKshMemDetachKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,DESCRIPTOR object)
{
	return CKshMemDetachKernelCode(object);
}

PUBLIC DATA CKshMemNotifyKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,DESCRIPTOR object)
{
    return CKshMemNotifyKernelCode(object);
}

#endif
