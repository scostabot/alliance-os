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
 * 27/10/98 scosta    1.0    First draft
 * 18/11/98 ramon     1.1    Updated to Alliance Coding Style
 * 18/12/99 scosta    1.2    Taken away typewrappers.h, fixed memory
 *                           access bug
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>

/*
 * Copyright (C) 1991, 1993 Free Software Foundation, Inc.
 * Contributed by Torbjorn Granlund (tege@sics.se).
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

PUBLIC DATA KLmemoryCompare(INPUT VOID *a, INPUT VOID *b, INPUT ADDR len)
/*
 * Compare two memory areas
 *
 * INPUT:
 * a:   First memory areas
 * b:   Memory area to compare a to
 * len:	maximum length in bytes to check
 *
 * RETURNS:
 * KLmemoryCompare() < 0:  a is less than b
 * KLmemoryCompare() = 0:  a is equal to b
 * KLmemoryCompare() > 0:  a is greater than b
 */
{
	BYTE *srcp1 = (BYTE *) a;
	BYTE *srcp2 = (BYTE *) b;
	DATA a0, b0;
	ADDR i=0;

	do {
		a0 = (DATA) *srcp1++;
		b0 = (DATA) *srcp2++;
		if(++i==len) {
			break;
		}
	} while (a0 == b0);

	return (a0 - b0);
}
