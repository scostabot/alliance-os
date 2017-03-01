/*
 * Alliance Kernel Library
 * 
 * The library defines some common facilities, i.e., basic standard
 * library functions usable from any kernel module (CK/AK/SK).
 * The functions defined in this library are like C standard
 * library counterparts, only much fewer.
 * Whenever possible, we have mantained the function definition 
 * exactly the same as C counterpart.
 * In this case, we have copied from GNU glibc the specific C function,
 * changed only the header and some typedefs, so original GNU file 
 * structure is mantained.
 * 
 * HISTORY
 * Date     Author    Rev    Notes
 * 22/10/98 scosta    1.0    First draft
 * 18/11/98 ramon     1.1    Updated to Alliance Coding Style
 * 02/05/99 scosta    1.2    Taken away typewrappers.h
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/*
 * Copyright (C) 1991 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU C Library; see the file COPYING.LIB.  If
 * not, write to the Free Software Foundation, Inc., 675 Mass Ave,
 * Cambridge, MA 02139, USA. 
 */

#include <klibs.h>

PUBLIC STRING *KLstringCopy(OUTPUT STRING *dest, INPUT STRING *src)
/*
 * Copy a string
 *
 * INPUT:
 *     src:   String to copy
 * OUTPUT:
 *     dest:  Destination to copy to
 *
 * RETURNS:
 *     KLstringCopy():  Pointer to destination string
 */
{
	BYTE c;
	BYTE *s = (BYTE *) src;
	CONST UWORD32 off = dest - src - 1;

	do {
		c = *s++;
		s[off] = c;
	} while (c != '\0');

	return dest;
}

