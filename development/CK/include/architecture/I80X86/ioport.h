/*
 * ioport.h:  inline routines to manipulate i386 I/O ports
 *
 * Copyright (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 18/01/99  ramon       1.1    Added delayed port functions
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
 */

#ifndef __SYS_IOPORT_H
#define __SYS_IOPORT_H

#include <typewrappers.h>

/* Normal port functions */

LOCAL inline UBYTE inportb(UWORD16 port)
/*
 * Input a byte from an I/O port
 *
 * INPUT:
 *     port:  Port to get the byte from
 *
 * RETURNS:
 *     inportb():  Value read at the port
 */
{
    UBYTE ret;

    asm volatile ("inb (%%dx),%%al":"=a" (ret):"d" (port));
    return ret;
}


LOCAL inline UWORD16 inportw(UWORD16 port)
/*
 * Input a short from an I/O port
 *
 * INPUT:
 *     port:  Port to get the short from
 *
 * RETURNS:
 *     inportw():  Value read at the port
 */
{
    UWORD16 ret;

    asm volatile ("inw (%%dx),%%ax":"=a" (ret):"d" (port));
    return ret;
}


LOCAL inline VOID outportb(UWORD16 port, UBYTE value)
/*
 * Output a byte to an I/O port
 *
 * INPUT:
 *     port:  Port to output the byte to
 *     value: Value to output to the port
 *
 * RETURNS:
 *     none
 */
{
    asm volatile ("outb %%al,(%%dx)"::"d" (port), "a" (value));
}


LOCAL inline VOID outportw(UWORD16 port, UWORD16 value)
/*
 * Output a word to an I/O port
 *
 * INPUT:
 *     port:  Port to output the byte to
 *     value: Value to output to the port
 *
 * RETURNS:
 *     none
 */
{
    asm volatile ("outw %%ax,%%dx"::"d" (port), "a" (value));
}


/* Delayed port functions */

LOCAL inline UBYTE dinportb(UWORD16 port)
/*
 * Input a byte from an I/O port and delay a few us
 *
 * INPUT:
 *     port:  Port to get the byte from
 *
 * RETURNS:
 *     inportb():  Value read at the port
 */
{
    UBYTE ret;

    asm volatile ("inb (%%dx),%%al;outb %%al,$0x80":"=a" (ret):"d" (port));
    return ret;
}


LOCAL inline UWORD16 dinportw(UWORD16 port)
/*
 * Input a short from an I/O port and delay a few us
 *
 * INPUT:
 *     port:  Port to get the short from
 *
 * RETURNS:
 *     inportw():  Value read at the port
 */
{
    UWORD16 ret;

    asm volatile ("inw (%%dx),%%ax;outb %%al,$0x80":"=a" (ret):"d" (port));
    return ret;
}


LOCAL inline VOID doutportb(UWORD16 port, UBYTE value)
/*
 * Output a byte to an I/O port and delay a few us
 *
 * INPUT:
 *     port:  Port to output the byte to
 *     value: Value to output to the port
 *
 * RETURNS:
 *     none
 */
{
    asm volatile ("outb %%al,(%%dx);outb %%al,$0x80"::"d" (port), "a" (value));
}


LOCAL inline VOID doutportw(UWORD16 port, UWORD16 value)
/*
 * Output a word to an I/O port and delay a few us
 *
 * INPUT:
 *     port:  Port to output the byte to
 *     value: Value to output to the port
 *
 * RETURNS:
 *     none
 */
{
    asm volatile ("outw %%ax,%%dx;outb %%al,$0x80"::"d" (port), "a" (value));
}

#endif
