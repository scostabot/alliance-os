/*
 * regs.h:  The x86 registers, as pushed onto the stack by SAVEREGS
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 01/01/98  ramon       1.1    Using SAVEREGS (faster)
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

#ifndef __SYS_REGS_H
#define __SYS_REGS_H

#include <typewrappers.h>

struct regs {
    UWORD32 ebx, esi, edi, ebp, ecx, edx, eax;
} __attribute__ ((packed));

struct cregs {
    UWORD32 ecx, edx, eax;
} __attribute__ ((packed));

#endif
