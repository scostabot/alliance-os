/*
 * realtime.h:  Support macros for realtime tasks
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
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

#ifndef __SYS_REALTIME_H
#define __SYS_REALTIME_H

#include <typewrappers.h>

LOCAL inline VOID quantumDone(VOID)
/*
 * Finish the current quantum
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    asm("int $0x68");   /* 0x68 == M_VEC */
}

#endif
