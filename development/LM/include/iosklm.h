/*
 * Definitions specifi to LM for IOSK
 *
 * Here are defined all typedefs and macros specific to IOSK LMs
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 24/09/99 scista    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifndef _IOSKLM
#define _IOSKLM

PUBLIC STRING *LMprobe(VOID);
PUBLIC BOOLEAN LMwrite(UBYTE *buffer, UWORD32 size, UWORD32 channel);

#endif /* _IOSKLM */
