/*
 * hwkernel.h:  Support macros for kernel (PL0) code
 *
 * NOTE:  modified 8/3/99 for the scheduler simulation [Ramon]
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

#ifndef __SYS_HWKERNEL_H
#define __SYS_HWKERNEL_H

#include <typewrappers.h>

LOCAL inline VOID CKlock(VOID)
/*
 * Enter a critical section (disable interrupts)
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
}

LOCAL inline VOID CKunlock(VOID)
/*
 * Exit a critical section (enable interrupts)
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
}

LOCAL inline VOID flushRegs(VOID)
/*
 * Flush EGCS register-cached variables
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    asm( "" : : : "memory" );
}

#endif
