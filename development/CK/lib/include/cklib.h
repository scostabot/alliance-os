/*
 * cklib.h:  The CK library
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 28/11/98  ramon       1.0    First release
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

#ifndef __LIB_CKLIB_H
#define __LIB_CKLIB_H

#include <typewrappers.h>
#include "ckobjects.h"

DATA cacheThread(VOID *entry, UBYTE *stack, TIME quantum, TIME defer, ThreadObject *eventblk, DESCRIPTOR addrspc, DESCRIPTOR kernel, UBYTE *kstack);

#endif
