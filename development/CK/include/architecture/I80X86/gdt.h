/*
 * gdt.h:   symbols and macros for building descriptors and descriptor tables
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
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

#ifndef __SYS_GDT_H
#define __SYS_GDT_H

#include <typewrappers.h>

/*
 * The descTable, stndDesc and gateDesc macros piece together a descriptor
 * table.
 *
 * Example usage:
 *
 *  descTable(GDT, 2) {
 *     stndDesc(0, 0, 0),
 *     stndDesc(0, 0xfffff, (D_CODE + D_READ + D_BIG + D_BIG_LIM))
 *  };
 *
 * gateDesc(offset, selector, control)   ;For gate descriptors
 * stndDesc(base, limit, control)	  ;For all other descriptors
 *
 *  base    is the full 32 bit base address of the segment
 *  limit   is one less than the segment length in 1 or 4K byte units
 *  control the sum of all the "D_" equates which apply (for call gates, you
 *          also add the "parameter dword count" to flags).
 *
 * desc_table(symbol, size)               ;Start a descriptor table
 *
 *  symbol  is the C symbol that is used for this descriptor table
 *  size    is the amount of descriptors defined in this table
 *
 * The descriptor table can be accessed in the code through the symbol [symbol],
 * which is defined as an array of DT_entry unions.  Every DT_entry union can
 * contain either a normal descriptor, gate descriptor, or some other value
 * which is 64 bits in size (this could be used to store custom info in the
 * dummy descriptor of the GDT, for instance.)
 *
 * The dummy value can be inserted like this:
 *
 *  descTable(GDT, 2) {
 *     {dummy: 0x1234567890123456},
 *     ...
 *  };
 */


/*
 *  Each descriptor should have exactly one of next 8 codes to define the
 *  type of descriptor
 */

#define D_LDT   0x200   /* LDT segment        */
#define D_TASK  0x500   /* Task gate          */
#define D_TSS   0x900   /* TSS                */
#define D_CALL  0x0C00  /* 386 call gate      */
#define D_INT   0x0E00  /* 386 interrupt gate */
#define D_TRAP  0x0F00  /* 386 trap gate      */
#define D_DATA  0x1000  /* Data segment       */
#define D_CODE  0x1800  /* Code segment       */


/*
 *  Descriptors may include the following as appropriate:
 */

#define D_DPL3         0x6000   /* DPL3 or mask for DPL                    */
#define D_DPL2         0x4000   /* DPL2 or mask for DPL                    */
#define D_DPL1         0x2000   /* DPL1 or mask for DPL                    */
#define D_PRESENT      0x8000   /* Present                                 */
#define D_NOT_PRESENT  0x8000   /* Not Present                             */
                                /* Note, the PRESENT bit is set by default */
                                /* Include NOT_PRESENT to turn it off      */
                                /* Do not specify D_PRESENT                */


/*
 *  Segment descriptors (not gates) may include:
 */

#define D_ACC      0x100  /* Accessed (Data or Code)          */

#define D_WRITE    0x200  /* Writable (Data segments only)    */
#define D_READ     0x200  /* Readable (Code segments only)    */
#define D_BUSY     0x200  /* Busy (TSS only)                  */

#define D_EXDOWN   0x400  /* Expand down (Data segments only) */
#define D_CONFORM  0x400  /* Conforming (Code segments only)  */

#define D_BIG      0x40	  /* Default to 32 bit mode           */
#define D_BIG_LIM  0x80	  /* Limit is in 4K units             */


/*
 *  Now we define the structures for descriptors and gates:
 */

struct x86Desc {
    UWORD16 limit_low;       /* limit 0..15    */
    UWORD16 base_low;        /* base  0..15    */
    UBYTE   base_med;        /* base  16..23   */
    UBYTE   access;          /* access byte    */
    UDATA   limit_high:4;    /* limit 16..19   */
    UDATA   granularity:4;   /* granularity    */
    UBYTE   base_high;       /* base 24..31    */
} __attribute__ ((packed));


struct x86Gate {
    UWORD16 offset_low;   /* offset 0..15    */
    UWORD16 selector;     /* selector        */	
    UWORD16 access;       /* access flags    */
    UWORD16 offset_high;  /* offset 16..31   */
} __attribute__ ((packed));


/*
 *  Descriptor tables are basically arrays of either standard or gate
 *  descriptors;  The exception is the first descriptor, which is a
 *  'dummy' and can be filled with anything without crashing the machine.
 *
 *  We define the union DT_entry as a general type of entry in a descriptor
 *  table;  It can contain a normal descriptor, a gate descriptor, or any
 *  64-bit number (for defining the dummy entries, if neccessary.)
 */

union DTEntry {
    struct  x86Desc desc;     /* Normal descriptor */
    struct  x86Gate gate;     /* Gate descriptor   */
    UWORD64 dummy;            /* Any other info    */
};


/*
 * Now we perform the macro magic of the day.
 */

#define descTable(name,length) union DTEntry name[length] = 

#define stndDesc(base, limit, control)                                  \
    {desc: {(limit & 0xffff), (base & 0xffff), ((base >> 16) & 0xff),   \
    ((control+D_PRESENT) >> 8), (limit >> 16), ((control & 0xff) >> 4), \
    (base >> 24)}}

#define gateDesc(offset, selector, control)                             \
    {gate: {(offset & 0xffff), selector, (control+D_PRESENT),           \
    (offset >> 16) }}

/* These are bound to look a bit cryptic, so I'll just show what the result
 * would look like:
 *
 * descTable(GDT, 3) {
 *    stndDesc(base, limit, control),
 *    stndDesc(base, limit, control),
 *    gateDesc(offset, selector, control)
 * };
 *
 * ... would produce something looking like: 
 *
 * union DTEntry GDT[3] = {
 *    {desc: {limit_low, base_low, base_med, access, limit_high, granularity, base_high}},
 *    {desc: {limit_low, base_low, base_med, access, limit_high, granularity, base_high}},
 *    {gate: {offset_low, selector, word_count, access, offset_high}}
 * }; 
 *
 * The values are conveted using a bit of cryptic, but straightforward magic.
 */

#endif
