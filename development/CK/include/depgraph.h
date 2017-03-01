/*
 * depgraph.h:  Defenitions for ODG
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 16/11/98  ramon       1.0    First release
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

#ifndef __DEPGRAPH_H
#define __DEPGRAPH_H

#include <typewrappers.h>
#include "ckobjects.h"

#define MAXDEPS 4096           /* Maximum amount of dependencies */

PUBLIC VOID CKaddObjectDep(VERTEX *parent, VERTEX *child);
PUBLIC VOID CKremoveObjectDep(EDGE *dep);

#endif
