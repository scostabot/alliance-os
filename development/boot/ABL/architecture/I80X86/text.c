/*
 * text.c:  Console text output routines (VGA)
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 15/11/98  ramon       1.1    Complete revamp; uses cursor; fancy CKprint
 * 26/12/98  ramon       1.2    Mutex protection and screen window sizing
 * 09/01/99  ramon       1.3    Adapted for ABL usage
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
#include <stdarg.h>
#include <klibs.h>
#include "sys/colors.h"
#include "sys/ioport.h"

LOCAL DATA TEXTATTR;
LOCAL DATA WINDOW = 25;

PUBLIC DATA print(CONST STRING *formatString, ...)
/*
 * Output formatted text to the VGA console
 *
 * INPUT:
 *     formatString:  Format string
 *
 * RETURNS:
 *     print():  Length of printed string
 */
{
    UDATA currChar, videoMemOffs, i;
    va_list arguments;
    DATA returnValue;
    STRING buffer[1024];
    STRING *string = buffer;

    outportb(0x3d4, 0x0e);              /* Get cursor Y position        */
    videoMemOffs = inportb(0x3d5);
    videoMemOffs <<= 8;
    outportb(0x3d4, 0x0f);              /* And add cursor X position    */
    videoMemOffs += inportb(0x3d5);
    videoMemOffs <<= 1;

    /* Convert format string to printing string */
    va_start(arguments, formatString);
    returnValue = KLformatToString(string, formatString, arguments);
    va_end(arguments);

    while((currChar = *(string++)))     /* Loop through the string      */
    {
        switch(currChar)                /* Is it a special character ?  */
        {
            case '\n':                  /* Newline found                */
                videoMemOffs = (videoMemOffs/160)*160 + 160;
                break;

            case '\r':                  /* Carriage return found        */
                videoMemOffs = (videoMemOffs/160)*160;
                break;

            case '\b':
                ((UBYTE *)(0xb8000))[--videoMemOffs] = TEXTATTR;
                ((UBYTE *)(0xb8000))[--videoMemOffs] = ' ';
                break;

            default:                    /* Normal character             */
                ((UBYTE *)(0xb8000))[videoMemOffs++] = currChar;
                ((UBYTE *)(0xb8000))[videoMemOffs++] = TEXTATTR;
                break;
        }

        if(videoMemOffs >= 160*WINDOW)  /* Are we off-screen ?          */
        {
            for(i = 0; i < 160*(WINDOW-1); i++) /* Scroll the screen up */
            {
                ((UBYTE *)(0xb8000))[i] = ((UBYTE *)(0xb8000))[i+160];
            }
            for(i = 0; i < 80; i++)     /* Empty the bottom row         */
            {
                ((UBYTE *)(0xb8000))[(160*(WINDOW-1))+(i*2)]   = 0x20;
                ((UBYTE *)(0xb8000))[(160*(WINDOW-1))+(i*2)+1] = TEXTATTR;
            }
            videoMemOffs -= 160;        /* We're on the bottom row      */
        }
    }

    videoMemOffs >>= 1;                 /* Set the new cursor position  */
    outportb(0x3d4, 0x0f);
    outportb(0x3d5, videoMemOffs & 0x0ff);
    outportw(0x3d4, 0x0e);
    outportb(0x3d5, videoMemOffs >> 8);

    return returnValue;
}


PUBLIC DATA sprint(STRING *buffer, CONST STRING *formatString, ...)
/*
 * Output formatted text to buffer
 *
 * INPUT:
 *     formatString:  Format string
 *     buffer:        Buffer to output to
 *
 * RETURNS:
 *     sprint():  Length of printed string
 */
{
    va_list arguments;
    DATA returnValue;

    /* Convert format string to printing string */
    va_start(arguments, formatString);
    returnValue = KLformatToString(buffer, formatString, arguments);
    va_end(arguments);

    return returnValue;
}


PUBLIC VOID ABLclrScr(VOID)
/*
 * Clear the VGA console
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    UWORD32 i;

    for(i = 0; i < 80*25; i++)          /* Fill the screen with         */
    {                                   /* background color             */
        ((UBYTE *)(0xb8000))[i*2] = 0x20;
        ((UBYTE *)(0xb8000))[i*2+1] = TEXTATTR;
    }

    outportb(0x3d4, 0x0f);              /* Set the cursor to the        */
    outportb(0x3d5, 0);                 /* upper-left corner of the     */
    outportw(0x3d4, 0x0e);              /* screen                       */
    outportb(0x3d5, 0);
}


VOID ABLsetTextAttr(DATA attr)
/*
 * Set the text attribute for print()
 *
 * INPUT:
 *     attr:  the text attribute
 *
 * RETURNS:
 *     none
 */
{
    TEXTATTR = attr;
}


PUBLIC VOID ABLinitCons(VOID)
/*
 * Initialise the VGA console
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    TEXTATTR = (T_FG_BLUE | T_BOLD);
    ABLclrScr();
}
