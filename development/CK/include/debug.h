/*
 * debug.h:  CK Debugging functions
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 15/05/99  jens        1.0    First full internal release
 * 17/05/99  ramon       1.1    Fixed assert to C-standard
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

#ifndef __DEBUG_H
#define __DEBUG_H

#include "sys/hwkernel.h"

#define ASSERT(cond)  if((!cond)) {                                   \
    CKprint("Assertion failed in %s at line %d",__FILE__,__LINE__);   \
    CKlock();  for(;;);                                               \
}

#endif
