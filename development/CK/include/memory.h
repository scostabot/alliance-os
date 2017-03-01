/*
 * memory.h:  CK memory managment defines
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 19/11/98  jens        1.0    First internal release
 * 27/12/98  jens        1.1    Added CKpagemapping(Alloc/Free)
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

#ifndef __MEMORY_H
#define __MEMORY_H

#include <typewrappers.h>

PUBLIC VOID CKinitMem(VOID);
PUBLIC VOID *CKmemAlloc(UADDR size);
PUBLIC VOID *CKmemAllocAlign(UADDR size, DATA align);
PUBLIC VOID *CKmemAllocPages(DATA nopages);
PUBLIC VOID *CKmemAllocRegion(VOID *data,UADDR size);
PUBLIC VOID CKmemFree(VOID *memory, UADDR size);
PUBLIC VOID CKmemFreePages(VOID *data,DATA nopages);

/*PUBLIC VOID *CKpageAlloc(DATA nopages);
PUBLIC VOID CKpageFree(DATA page);
PUBLIC VOID CKinitPage(VOID);

PUBLIC VOID CKpageMarkHardwareDependent(VOID);
PUBLIC VOID CKpageMarkMemUsed(DATA start,DATA end);

PUBLIC DATA CKpagemappingAlloc(VOID);
PUBLIC VOID CKpagemappingFree(DATA pagemapping);
*/
#endif


