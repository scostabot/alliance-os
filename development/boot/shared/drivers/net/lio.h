/*
 * ioport.h:  inline routines to manipulate i386 I/O ports
 *            Linux style --- 'cause I don't feel like changing all of
 *            this horrid linux driver code.  Perhaps some other time
 *            when I'm really bored I'll kill this file...    -- Ramon
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

#ifndef __GRUB_DRIVERS_NET_LIO_H
#define __GRUB_DRIVERS_NET_LIO_H


/* Normal port functions */

static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    asm volatile ("inb (%%dx),%%al":"=a" (ret):"d" (port));
    return ret;
}

static inline unsigned short inw(unsigned short port)
{
    unsigned short ret;
    asm volatile ("inw (%%dx),%%ax":"=a" (ret):"d" (port));
    return ret;
}

static inline unsigned long inl(unsigned short port)
{
    unsigned long ret;
    asm volatile ("inl (%%dx),%%eax":"=a" (ret):"d" (port));
    return ret;
}

#ifdef LINUX_OUT_MACROS
static inline void outb(unsigned char value, unsigned short port)
#else
static inline void outb(unsigned short port, unsigned char value)
#endif
{
    asm volatile ("outb %%al,(%%dx)"::"d" (port), "a" (value));
}

#ifdef LINUX_OUT_MACROS
static inline void outw(unsigned short value, unsigned short port)
#else
static inline void outw(unsigned short port, unsigned short value)
#endif
{
    asm volatile ("outw %%ax,%%dx"::"d" (port), "a" (value));
}

#ifdef LINUX_OUT_MACROS
static inline void outl(unsigned long value, unsigned short port)
#else
static inline void outl(unsigned short port, unsigned long value)
#endif
{
    asm volatile ("outl %%eax,%%dx"::"d" (port), "a" (value));
}



/* Delayed port functions */

#define DELAY  ";outb %%al,$0x80"

static inline unsigned char inb_p(unsigned short port)
{
    unsigned char ret;
    asm volatile ("inb (%%dx),%%al" DELAY :"=a" (ret):"d" (port));
    return ret;
}

static inline unsigned short inw_p(unsigned short port)
{
    unsigned short ret;
    asm volatile ("inw (%%dx),%%ax" DELAY :"=a" (ret):"d" (port));
    return ret;
}

static inline unsigned long inl_p(unsigned short port)
{
    unsigned long ret;
    asm volatile ("inl (%%dx),%%eax" DELAY :"=a" (ret):"d" (port));
    return ret;
}

#ifdef LINUX_OUT_MACROS
static inline void outb_p(unsigned char value, unsigned short port)
#else
static inline void outb_p(unsigned short port, unsigned char value)
#endif
{
    asm volatile ("outb %%al,(%%dx)" DELAY ::"d" (port), "a" (value));
}

#ifdef LINUX_OUT_MACROS
static inline void outw_p(unsigned short value, unsigned short port)
#else
static inline void outw_p(unsigned short port, unsigned short value)
#endif
{
    asm volatile ("outw %%ax,%%dx" DELAY ::"d" (port), "a" (value));
}

#ifdef LINUX_OUT_MACROS
static inline void outl_p(unsigned long value, unsigned short port)
#else
static inline void outl_p(unsigned short port, unsigned long value)
#endif
{
    asm volatile ("outl %%eax,%%dx" DELAY ::"d" (port), "a" (value));
}

#undef DELAY



/* String I/O functions */

static inline void insb(unsigned short port, void * addr, unsigned long count)
{
    asm volatile ("cld ; rep ; insb"
                  : "=D" (addr), "=c" (count)
                  : "d" (port),  "0" (addr), "1" (count));
}

static inline void insw(unsigned short port, void * addr, unsigned long count)
{
    asm volatile ("cld ; rep ; insw"
                  : "=D" (addr), "=c" (count)
                  : "d" (port),  "0" (addr), "1" (count));
}

static inline void insl(unsigned short port, void * addr, unsigned long count)
{
    asm volatile ("cld ; rep ; insl"
                  : "=D" (addr), "=c" (count)
                  : "d" (port),  "0" (addr), "1" (count));
}

static inline void outsb(unsigned short port, const void * addr, unsigned long count)
{
    asm volatile ("cld ; rep ; outsb"
                  : "=S" (addr), "=c" (count)
                  : "d" (port),  "0" (addr),"1" (count));
}

static inline void outsw(unsigned short port, const void * addr, unsigned long count)
{
    asm volatile ("cld ; rep ; outsw"
                  : "=S" (addr), "=c" (count)
                  : "d" (port),  "0" (addr),"1" (count));
}

static inline void outsl(unsigned short port, const void * addr, unsigned long count)
{
    asm volatile ("cld ; rep ; outsl"
                  : "=S" (addr), "=c" (count)
                  : "d" (port),  "0" (addr),"1" (count));
}


#endif
