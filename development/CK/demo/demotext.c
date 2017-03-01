/*
 * text.c:  Console text output routines (VGA)
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 15/11/98  ramon       1.1    Complete revamp; uses cursor; fancy print
 * 26/12/98  ramon       1.2    Mutex protection and screen window sizing
 * 03/02/99  haggai      1.3    Added speaker functions
 * 23/07/99  ramon       1.4    Modified for use by demo system
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
#include "sys/8254.h"
#include "sys/colors.h"
#include "sys/sys.h"
#include "sys/ioport.h"
#include "sys/hwkernel.h"
#include "sched.h"
#include "mutex.h"


LOCAL DATA  TEXTATTR = (T_FG_BLUE | T_BOLD);
LOCAL DATA  WINDOW   = 16;
LOCAL UBYTE *VIDMEM  = (UBYTE *) 0x60000000;

LOCAL CKMutex textLock;
LOCAL STRING buffer[1024];


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
    STRING *string = buffer;

    outportb(0x3d4, 0x0e);              /* Get cursor Y position        */
    videoMemOffs = inportb(0x3d5);
    videoMemOffs <<= 8;
    outportb(0x3d4, 0x0f);              /* And add cursor X position    */
    videoMemOffs += inportb(0x3d5);
    videoMemOffs <<= 1;

    CKenterCritical(&textLock);         /* Protect the text buffer      */

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

            case '\t':                  /* Tab found                    */
                videoMemOffs = (videoMemOffs/8)*8 + 8;
                break;

            case '\b':                  /* Backspace found              */
                videoMemOffs -= 2;
                VIDMEM[videoMemOffs]   = ' ';
                VIDMEM[videoMemOffs+1] = TEXTATTR;
                break;

            default:                    /* Normal character             */
                VIDMEM[videoMemOffs++] = currChar;
                VIDMEM[videoMemOffs++] = TEXTATTR;
                break;
        }

        if(videoMemOffs >= 160*WINDOW)  /* Are we off-screen ?          */
        {
            for(i = 0; i < 160*(WINDOW-1); i++) /* Scroll the screen up */
            {
                VIDMEM[i] = VIDMEM[i+160];
            }
            for(i = 0; i < 80; i++)     /* Empty the bottom row         */
            {
                VIDMEM[(160*(WINDOW-1))+(i*2)]   = 0x20;
                VIDMEM[(160*(WINDOW-1))+(i*2)+1] = TEXTATTR;
            }
            videoMemOffs -= 160;        /* We're on the bottom row      */
        }
    }

    CKexitCritical(&textLock);          /* Protect the text buffer      */

    videoMemOffs >>= 1;                 /* Set the new cursor position  */
    outportb(0x3d4, 0x0f);
    outportb(0x3d5, videoMemOffs & 0x0ff);
    outportw(0x3d4, 0x0e);
    outportb(0x3d5, videoMemOffs >> 8);

    return returnValue;
}


VOID setTextAttr(DATA attr)
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
