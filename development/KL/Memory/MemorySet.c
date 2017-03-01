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
 * 30/10/98 scosta    1.0    First draft
 * 18/11/98 ramon     1.1    Updated to Alliance Coding Style
 * 19/12/99 scosta    1.2    Taken away typewrappers.h, changed arg, fixed
 *                           bug that writes much more bytes than right
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/* Copyright (C) 1991 Free Software Foundation, Inc.
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

PUBLIC VOID *KLmemorySet(OUTPUT VOID *dstpp, INPUT UBYTE c, INPUT ADDR len)
/*
 * Fills a memory region with a signle character
 *
 * INPUT:
 * dstpp:  Pointer to memory to be filled
 * c:      Character to fill with
 * len:    Amount of memory to fill
 *
 * OUTPUT:
 * *dstpp  memory filled
 *
 * RETURNS:
 * KLmemSet():  Pointer to memory region
 */
{
	ADDR dstp = (ADDR) dstpp;
	ADDR mylen=len;

	if (len >= 8) {
		ADDR xlen;
		DATA cccc;

		cccc = (UBYTE) c;
		cccc |= cccc << 8;
		cccc |= cccc << 16;
		if (ADDRSIZE > 4) cccc |= (cccc << 16) << 16;

		/*
		 * There are at least some bytes to set.
		 * No need to test for len == 0 in this alignment loop. 
		 */
		while ((dstp % ADDRSIZE) != 0) {
			((UBYTE *)(dstp++))[0] = (UBYTE) c;
			mylen--;
		}

		/* Write 8 `ADDR' per iteration until less than 8 `ADDR' remain. */
		xlen = mylen / (ADDRSIZE * 8);
		while (xlen > 0) {
			((ADDR *) dstp)[0] = cccc;
			((ADDR *) dstp)[1] = cccc;
			((ADDR *) dstp)[2] = cccc;
			((ADDR *) dstp)[3] = cccc;
			((ADDR *) dstp)[4] = cccc;
			((ADDR *) dstp)[5] = cccc;
			((ADDR *) dstp)[6] = cccc;
			((ADDR *) dstp)[7] = cccc;
			dstp += 8 * ADDRSIZE;
			xlen--;
		}
		mylen %= ADDRSIZE * 8;

		/* Write 1 `ADDR' per iteration until less than ADDRSIZE bytes remain */
		xlen = mylen / ADDRSIZE;
		while (xlen > 0) {
			((ADDR *) dstp)[0] = cccc;
			dstp += ADDRSIZE;
			xlen--;
		}
		mylen %= ADDRSIZE;
	}

	/* Write the last few bytes.  */
	while (mylen > 0) {
		((UBYTE *)(dstp++))[0] = (UBYTE) c;
		mylen--;
	}

	return dstpp;
}
