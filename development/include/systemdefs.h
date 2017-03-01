/*
 * Alliance Kernel-wide definitions
 *
 * This file defines all symbols, types and other stuff needed in all
 * SKs/AKs.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 06/1/99  scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifndef _SYSTEMDEFSH
#define _SYSTEMDEFSH

#include <typewrappers.h>  /* Type wrappers for Alliance OS */

/* Version for all Alliance components (BYTEs to avoid endianess issues) */

typedef struct {
    UBYTE Major;
    UBYTE Minor;
    UBYTE BugFix;
} Version; 

typedef UWORD32 Handle;  /* A generic handle to be used elsewhere */

#endif /*_SYSTEMDEFSH */
