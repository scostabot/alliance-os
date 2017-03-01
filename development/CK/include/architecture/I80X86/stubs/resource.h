/*
 * resource.h:  Resource allocation system call stubs
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 31/12/98  ramon       1.0    First release
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

#ifndef __SYS_STUBS_RESOURCE_H
#define __SYS_STUBS_RESOURCE_H

#include <typewrappers.h>
#include "ckobjects.h"

PUBLIC DATA CKresourceAllocKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,Resource *resource,DESCRIPTOR kernel,DATA flags)
{
    return CKresourceAllocKernelCode(resource, kernel, flags);
}

PUBLIC DATA CKresourceFreeKernelStub(DATA edi,DATA ebp,DATA e,DATA f,DATA ds,DATA a,DATA b,DATA c,DATA d,RESOURCE resource)
{
    return CKresourceFreeKernelCode(resource);
}

#endif
