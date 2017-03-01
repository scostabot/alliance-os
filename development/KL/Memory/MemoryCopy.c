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
 * 19/12/99 scosta    1.2    Taken away typewrappers.h
 * 12/12/00 ramon     1.3    Fixed bug in KLwordCopyFwd
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
 * memcpy -- copy memory to memory until the specified number of bytes
 * has been copied.  Overlap is NOT handled correctly.
 * Copyright (C) 1991 Free Software Foundation, Inc.
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


LOCAL inline VOID KLbyteCopyFwd(ADDR *dest, ADDR *src, ADDR size)
/*
 * Copy exactly size bytes from src to dest, without any assumptions about
 * alignment of the pointers.
 *
 * INPUT:
 *     dest:  Destination of copy
 *     src:   Source of copy
 *     size:  Amount of bytes to copy
 *
 * RETURNS:
 *     none
 */
{
	for(; size > 0; size--) {
		UBYTE copy = ((UBYTE *)((*src)++))[0];
		((UBYTE *)((*dest)++))[0] = copy;
	}
}


LOCAL inline ADDR KLwordCopyFwd(ADDR *dest, ADDR *src, ADDR size)
/*
 * Copy exactly size bytes from src to dest, where dest is word-aligned
 *
 * INPUT:
 *     dest:  Destination of copy
 *     src:   Source of copy
 *     size:  Amount of bytes to copy
 *
 * RETURNS:
 *     KLwordCopyFwd():  Amount of bytes left over
 */
{
	for(; size > 0; (size -= sizeof(UHALF))) {
		UHALF copy = (((UHALF *)(*src))++)[0];
		(((UHALF *)(*dest))++)[0] = copy;
	}
	return (-size);
}


PUBLIC VOID *KLmemoryCopy(OUTPUT VOID *dstpp, INPUT VOID *srcpp, INPUT ADDR len)
/*
 * Copy a chunk of memory from one place to the other
 *
 * INPUT:
 *     srcpp:  Source address for copy
 *     len:    Amount of bytes to copy
 *
 * OUTPUT:
 *     dstpp:  Destination address for copy
 *
 * RETURNS:
 *     KLmemoryCopy():  Pointer to destination
 */
{
	ADDR destPtr = (ADDR) dstpp;
	ADDR srcPtr  = (ADDR) srcpp;
	ADDR mylen   = len;          /* get rid of warning */

	/* Copy from the beginning to the end.  */

	/* If there not too few bytes to copy, use word copy.  */
	if (mylen >= 16) {
		/* Copy just a few bytes to make destPtr aligned.  */
		mylen -= (-destPtr) % ADDRSIZE;
		KLbyteCopyFwd(&destPtr, &srcPtr, ((-destPtr) % ADDRSIZE));

		/*
		 * Copy from srcPtr to destPtr taking advantage of the known
		 * alignment of destPtr.  Number of bytes remaining is put
		 * in the third argumnet, i.e. in len.  This number may
		 * vary from machine to machine.
	 	 */

		mylen = KLwordCopyFwd(&destPtr, &srcPtr, mylen);

		/* Fall out and copy the tail.  */
	}

	/* There are just a few bytes to copy.  Use byte memory operations */
	KLbyteCopyFwd(&destPtr, &srcPtr, mylen);

	return dstpp;
}
