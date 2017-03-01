/*
 * dbg-ser.c:  GDB serial port stubs (used for remote debugging
 *             over a null-modem serial cable)
 *
 * NOTE:  This file wasn't written by me, I found it somewhere on the
 *        net.  IIRC, it comes from those nice guys at ACM SIGOPS
 *        (http://www.acm.uiuc.edu/sigops/).  I have edited it for
 *        alliance in a way that *should* work, but I haven't had
 *        the chance to test it, so I don't know whether it actually
 *        does work.  Try it if you need it.                    -- Ramon
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 06/08/99  ramon       1.0    First release
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

/************************************************************************/
/* Settings                                                             */
/************************************************************************/

/* What parallel port to use for communications */
#define combase com1


/************************************************************************/
/* Definitions                                                          */
/************************************************************************/

/* Serial port locations */
#define com1 0x3f8 
#define com2 0x2f8


/************************************************************************/
/* Communication functions                                              */
/************************************************************************/

void CKinitDebugger(void)
{
    /* Initialise the serial port:  9600 bps, 8-N-1 */
    outportb(combase+3, inportb(combase + 3) | 0x80);
    outportb(combase, 12);
    outportb(combase+1, 0);
    outportb(combase+3, inportb(combase + 3) & 0x7f);

    /* Bootstrap GDB */
    set_debug_traps();                /* Initialise GDB exception handl */
    breakpoint();                     /* Start debugging                */
}

int getDebugChar(void)
{
    while (!(inportb(combase + 5) & 0x01));
    return inportb(combase);
}

void putDebugChar(int ch)
{
    while (!(inportb(combase + 5) & 0x20));
    outportb(combase, (char) ch);
}


/************************************************************************/
/* Other GDB support functions                                          */
/************************************************************************/

void exceptionHandler(int exc, void *addr)
{
    CKsetVector(addr, (unsigned char)exc, (D_INT + D_PRESENT + D_DPL3));
}


/* The end */
