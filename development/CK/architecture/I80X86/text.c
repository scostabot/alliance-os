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
 * 03/02/99  haggai      1.3    Added speaker functions
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
#include "sys/memory.h"
#include "sched.h"
#include "mutex.h"


LOCAL DATA  TEXTATTR;
LOCAL DATA  WINDOW = 25;
LOCAL UBYTE *VIDMEM = (UBYTE *) LINEAR(0xb8000);

LOCAL CKMutex textLock;


LOCAL STRING buffer[1024];
LOCAL STRING dbuffer[1024];


PUBLIC DATA CKprint(CONST STRING *formatString, ...)
/*
 * Output formatted text to the VGA console
 *
 * INPUT:
 *     formatString:  Format string
 *
 * RETURNS:
 *     CKprint():  Length of printed string
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

            case '\a':                  /* Bell found                   */
                CKbeep();
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


PUBLIC VOID CKclrScr(VOID)
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
        VIDMEM[i*2] = 0x20;
        VIDMEM[i*2+1] = TEXTATTR;
    }

    outportb(0x3d4, 0x0f);              /* Set the cursor to the        */
    outportb(0x3d5, 0);                 /* upper-left corner of the     */
    outportw(0x3d4, 0x0e);              /* screen                       */
    outportb(0x3d5, 0);
}


VOID CKsetTextAttr(DATA attr)
/*
 * Set the text attribute for CKprint()
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


PUBLIC VOID CKinitCons(VOID)
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
    WINDOW   = 16;
    CKclrScr();
}


PUBLIC DATA CKdebugPrint(CONST STRING *formatString, ...)
/*
 * Output formatted text to the VGA console
 * Special version without mutexes for the debugger
 * The text output should never hang the debugger, because then the
 * system would just hang.  Thus, we provide a special print function
 * for it.
 *
 * INPUT:
 *     formatString:  Format string
 *
 * RETURNS:
 *     CKdebugPrint():  Length of printed string
 */
{
    UDATA currChar, videoMemOffs, i;
    va_list arguments;
    DATA returnValue;
    STRING *string = dbuffer;

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

            case '\a':                  /* Bell found                   */
                CKbeep();
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

    videoMemOffs >>= 1;                 /* Set the new cursor position  */
    outportb(0x3d4, 0x0f);
    outportb(0x3d5, (videoMemOffs) & 0x0ff);
    outportw(0x3d4, 0x0e);
    outportb(0x3d5, videoMemOffs >> 8);

    return returnValue;
}


PUBLIC VOID CKspeakerOn(UDATA freq)
/*
 * Turns on the speaker playing the specified frequency.
 *
 * INPUT:
 *     freq:  The frequency of the sound to play, in Hz.
 *
 * RETURNS:
 *     none
 */
{
    UWORD16 cnt = 1192755 / freq;

    doutportb(TMR_CNT2, cnt & 0xff);    /* Send low clock byte   */
    doutportb(TMR_CNT2, cnt >> 8);      /* Send high clock byte  */
    doutportb(0x61, 0x03);              /* Turn on the speaker   */
}


PUBLIC VOID CKspeakerOff(VOID)
/*
 * Turns off the speaker.
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    doutportb(0x61, 0x00);  /* Turn off the speaker */
}


PUBLIC VOID CKbeep(VOID)
/*
 * Makes a short beep
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKspeakerOn(300);       /* Turn on speaker with 300Hz tone */
    CKdelay(500000);        /* Wait about a second */
    CKspeakerOff();         /* Turn the speaker off */
}

