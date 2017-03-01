/*
 * gdt.c:  GDT and the stacks
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 26/04/99  ramon       1.0    First internal release
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
#include "sys/gdt.h"           /* Descriptor definitions                */

PUBLIC descTable(GDT, 5) {     /* The Global Descriptor Table           */
    {dummy: 0},
    stndDesc(0x00000000,0xfffff,(D_CODE+D_READ+D_BIG+D_BIG_LIM)),
    stndDesc(0x00000000,0xfffff,(D_DATA+D_WRITE+D_BIG+D_BIG_LIM)),
    stndDesc(0x00000000,0xffbff,(D_CODE+D_READ+D_BIG_LIM)),
    stndDesc(0x00000000,0xfffff,(D_DATA+D_WRITE+D_BIG_LIM)),
};

PUBLIC struct                  /* Loading structure for the GDT         */
{ 
    UWORD16 limit       __attribute__ ((packed)); 
    union DTEntry *idt  __attribute__ ((packed)); 
} loadgdt = { (5 * sizeof(union DTEntry) - 1), GDT };
