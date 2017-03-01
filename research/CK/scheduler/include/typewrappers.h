/*
 * typewrappers.h:  Definition of standard Alliance type wrappers
 *
 * This file create new metatypes and modifiers that must be used for
 * developing all code inside Alliance.
 * Using this code enables better portability to other HW environments
 * and enhance readability.
 *
 * (C) 1998 Stefano Costa, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author    Rev   Notes
 * 04/08/98  scosta    1.0   Creation
 * 06/09/98  ramon/dja 1.1   Added 64-bit wrappers
 * 09/09/98  scosta    1.2   Added NIL
 * 23/09/98  scosta    1.3   Added INPUT, MEMORYALIGN
 * 22/10/98  scosta    1.4   Added CONST
 * 07/11/98  ramon     1.5   Changed BOOLEAN/TRUE/FALSE, added DATA/ADDR
 * 15/11/98  ramon     1.6   Added HALF, UHALF
 * 09/12/98  djarb     1.7   Fixed boolean conflict with GLIB
 * 06/01/99  scosta    1.8   Taken away unused MEMORYALIGN
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

#ifndef __STDWRAPPERS_H
#define __STDWRAPPERS_H

#if I80X86 || PENTIUM

/* Kernel type wrappers */

typedef char BYTE;              /* The smallest addressable unit for CPU */
typedef unsigned char UBYTE;    /* Unsigned version of above */

typedef short WORD16;           /* A signed 16-bit word */
typedef unsigned short UWORD16; /* Unsigned version of above */

typedef int WORD32;             /* A signed 32-bit word */
typedef unsigned int UWORD32;   /* Unsigned version of above */

typedef long long WORD64;       /* A signed 64-bit word */
typedef unsigned long long UWORD64; /* Unsigned version of above */

typedef long ADDR;              /* Size of the machine's address bus */
typedef unsigned long UADDR;    /* Unsigned version of above */

typedef long DATA;              /* Size of the machine's data bus */
typedef unsigned long UDATA;    /* Unsigned version of above */

typedef short HALF;             /* Half of the machine's data bus */
typedef unsigned short UHALF;   /* Unsigned version of above */

/* User processes type wrappers */

typedef short INT16;            /* A signed 16-bit integer word */
typedef unsigned short UINT16;  /* Unsigned version of above */

typedef long INT32;             /* A signed 32-bit integer word */
typedef unsigned long UINT32;   /* Unsigned version of above */

typedef long long INT64;        /* A signed 64-bit integer word */
typedef unsigned long long UINT64; /* Unsigned version of above */

typedef float FLOAT32;          /* A 32-bit floating point number */
typedef double FLOAT64;         /* A 64-bit floating point number */

#endif /* I80X86 || PENTIUM */

/* Common type wrappers */

#define ADDRSIZE sizeof(ADDR)
#define DATASIZE sizeof(DATA)

/* Because GLIB uses these symbols */
#ifndef FALSE

typedef enum {
    FALSE,
    TRUE
} BOOLEAN;

#else

#define BOOLEAN int

#endif

typedef char STRING;            /* Alphanumeric string type */

/* Function definitions wrappers */

#define VOID void
#define INPUT const

#define NIL ((VOID *) 0)          /* The null element for pointers */

/* Type modifiers */

#define CONST const             /* Mainly for aesthetic reasons */
#define LOCAL static            /* Defines that following variable is local */
#define PUBLIC                  /* Defines that symbol is global */

#endif /* STDWRAPPERS */
