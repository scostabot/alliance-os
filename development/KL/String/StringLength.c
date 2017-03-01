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
 * 26/10/98 scosta    1.0    First draft
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

#include <klibs.h>

/*
 * Copyright (C) 1991, 1993 Free Software Foundation, Inc.
 * Written by Torbjorn Granlund (tege@sics.se),
 * with help from Dan Sahlin (dan@sics.se);
 * commentary by Jim Blandy (jimb@ai.mit.edu).
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


PUBLIC ADDR KLstringLength(INPUT STRING *str)
/*
 * Return the length of the null-terminated string str. Scan for
 * the null terminator quickly by testing four bytes at a time.
 *
 * INPUT:
 *     str:  String to get length of
 *
 * RETURNS:
 *     KLstringLength():  Length of string str
 */
{
	CONST BYTE *charPtr;
	CONST UDATA *longwordPtr;
	UDATA longword, magicbits, himagic, lomagic;

	/*
	 * Handle the first few characters by reading one character at a time.
	 * Do this until charPtr is aligned on a longword boundary. 
	 */
	for (charPtr = str; ((UDATA) charPtr &
	     (sizeof (longword) - 1)) != 0; ++charPtr) {
		if (*charPtr == '\0')
			return charPtr - str;
	}

	/*
	 * All these elucidatory comments refer to 4-byte longwords,
	 * but the theory applies equally well to 8-byte longwords.
	 */
	longwordPtr = (UDATA *) charPtr;

	/*
	 * Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
	 * the "holes."  Note that there is a hole just to the left of
	 * each byte, with an extra at the end:
	 *
	 * bits:  01111110 11111110 11111110 11111111
	 * bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD 
	 *
	 * The 1-bits make sure that carries propagate to the next 0-bit.
	 * The 0-bits provide holes for carries to fall into.
	 */
	magicbits = 0x7efefeffL;
	himagic = 0x80808080L;
	lomagic = 0x01010101L;

	if (sizeof (longword) > 4) {
		/* 64-bit version of the magic.  */
		magicbits = ((0x7efefefeL << 16) << 16) | 0xfefefeffL;
		himagic = ((himagic << 16) << 16) | himagic;
		lomagic = ((lomagic << 16) << 16) | lomagic;
	}
#if 0
	if (sizeof (longword) > 8) abort();
#endif
	/*
	 * Instead of the traditional loop which tests each character,
	 * we will test a longword at a time.  The tricky part is testing
	 * if *any of the four* bytes in the longword in question are zero.
	 */
	for (;;) {
		/*
		 * We tentatively exit the loop if adding MAGIC_BITS to
		 * LONGWORD fails to change any of the hole bits of LONGWORD.
		 *
		 * 1) Is this safe?  Will it catch all the zero bytes?
		 * Suppose there is a byte with all zeros.  Any carry bits
		 * propagating from its left will fall into the hole at its
		 * least significant bit and stop.  Since there will be no
		 * carry from its most significant bit, the LSB of the
		 * byte to the left will be unchanged, and the zero will be
		 * detected.
		 *
		 * 2) Is this worthwhile?  Will it ignore everything except
		 * zero bytes?  Suppose every byte of LONGWORD has a bit set
		 * somewhere.  There will be a carry into bit 8.  If bit 8
		 * is set, this will carry into bit 16.  If bit 8 is clear,
		 * one of bits 9-15 must be set, so there will be a carry
		 * into bit 16.  Similarly, there will be a carry into bit
		 * 24.  If one of bits 24-30 is set, there will be a carry
		 * into bit 31, so all of the hole bits will be changed.
		 *
		 * The one misfire occurs when bits 24-30 are clear and bit
		 * 31 is set; in this case, the hole at bit 31 is not
		 * changed.  If we had access to the processor carry flag,
		 * we could close this loophole by putting the fourth hole
		 * at bit 32!
		 *
		 * So it ignores everything except 128's, when they're aligned
		 * properly.
		 */

		longword = *longwordPtr++;

		if (((longword - lomagic) & himagic) != 0)
		{
			/*
			 * Which of the bytes was the zero?  If none of them were, it
			 * was a misfire; continue the search.
			 */

			CONST BYTE *cp = (CONST BYTE *) (longwordPtr - 1);

			if (cp[0] == 0) return cp - str;
			if (cp[1] == 0) return cp - str + 1;
			if (cp[2] == 0) return cp - str + 2;
			if (cp[3] == 0) return cp - str + 3;
			if (sizeof (longword) > 4)
			{
				if (cp[4] == 0) return cp - str + 4;
				if (cp[5] == 0) return cp - str + 5;
				if (cp[6] == 0) return cp - str + 6;
				if (cp[7] == 0) return cp - str + 7;
			}
		}
	}
}
