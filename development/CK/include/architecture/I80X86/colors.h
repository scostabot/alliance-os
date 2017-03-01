/*
 * colors.h:  Console color defenitions (VGA)
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
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

#ifndef __SYS_COLORS_H
#define __SYS_COLORS_H

#define BLACK                   0x00
#define BLUE                    0x01
#define CYAN                    0x02
#define GREEN                   0x03
#define RED                     0x04
#define MAGENTA                 0x05
#define BROWN                   0x06
#define LIGHTGRAY               0x07
#define DARKGRAY                0x08
#define LIGHTBLUE               0x09
#define LIGHTGREEN              0x0A
#define LIGHTCYAN               0x0B
#define DARKERGRAY              0x0C
#define LIGHTERGRAY             0x0D
#define YELLOW                  0x0E
#define WHITE                   0x0F

#define T_FG_BLUE                  1
#define T_FG_GREEN                 2
#define T_FG_RED                   4
#define T_BOLD                     8
#define T_BG_BLUE                 16
#define T_BG_GREEN                32
#define T_BG_RED                  64
#define T_BLINK                  128

#endif
