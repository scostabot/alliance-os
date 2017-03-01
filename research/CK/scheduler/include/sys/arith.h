/*
 * arith.h:  System-dependant arithmatic (shift with carry, etc.)
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 31/12/98  ramon       1.1    Added XCHG() macro
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

#ifndef __SYS_ARITH_H
#define __SYS_ARITH_H

#include <typewrappers.h>

/*
 * CLESS(x,y):  Signed comparison without overflow compensation
 *              This is a circular comparison (hence cless)
 *              Should compile to:  cmpl %b,%a; js label
 *                                            ... or something similar
 */

#define CLESS(x,y) ((((DATA)x)-((DATA)y))<0)
#define CLSE(x,y)  ((((DATA)x)-((DATA)y))<=0)


/*
 * XCHG(x,y):   Exchanges the contents of one memory location with a
 *              scalar, and returns the previous contents of the memory
 *              location.  Works on longs, shorts, as well as bytes.
 */

#define XCHG(x,y)                                                        \
    ({                                                                   \
        typeof(x) p;                                                     \
        asm ( "xchg %0, %1" : "=r" (p) : "m" (x), "0" (y) : "memory" );  \
        p;                                                               \
    })

#endif
