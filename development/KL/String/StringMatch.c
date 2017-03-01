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
 * Copyright (C) 1994 Free Software Foundation, Inc.
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

/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de */

PUBLIC STRING *KLstringMatch(INPUT STRING *phaystack, INPUT STRING *pneedle)
/*
 * Find the first occurance of pneedle in phaystack
 *
 * INPUT:
 *     phaystack:  String to search
 *     pneedle:    Substring to search for
 *
 * RETURNS:
 *     KLstringMatch():  Pointer to beginning of the substring, or NIL
 */
{
    register CONST UBYTE *haystack, *needle;
    register UWORD16 b, c;

    haystack = (CONST UBYTE *)phaystack;

    if ((b= *(needle=(const UBYTE *)pneedle))) {
        haystack--;
        do
            if (!(c= *++haystack)) goto ret0;
        while (c!=b);

        if (!(c= *++needle)) goto foundneedle;
        ++needle;
        goto jin;

        for (;;) { 
            register UWORD16 a;
            register CONST UBYTE *rhaystack, *rneedle;

            do {
                if (!(a= *++haystack)) goto ret0;
                if (a==b) break;
                if (!(a= *++haystack)) goto ret0;
shloop:     } while (a!=b);

jin:        if (!(a= *++haystack)) goto ret0;

            if (a!=c) goto shloop;

            if (*(rhaystack=haystack--+1) == (a= *(rneedle=needle)))
                do {
                    if (!a) goto foundneedle;
                    if (*++rhaystack!=(a= *++needle)) break;
                    if (!a) goto foundneedle;
                } while (*++rhaystack == (a= *++needle));

            needle=rneedle;  /* took the register-poor aproach */

            if (!a) break;
        }
    }

foundneedle:
    return (STRING *)haystack;

ret0:
    return NIL;
}
