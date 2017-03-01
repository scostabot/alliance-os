/*
 * idle.c:  The idle loop
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 20/01/99  ramon       1.1    Made useful idle task
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

#include <typewrappers.h>

PUBLIC UWORD64 idleCount = 0;

PUBLIC VOID CKidle(VOID)
/*
 * The idle loop
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    for(;;) {
        idleCount++;   /* Count idle time (for system load calculation) */
        asm("hlt");    /* Halt the processor until the next interrupt   */
    }
}
