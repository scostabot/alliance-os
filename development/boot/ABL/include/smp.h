/*
 * smp.h:  ABL SMP defenitions / structures
 *
 * NOTE:  This file is a modified version if Erich Boleyn's SMP-IMPS
 *        code.  Original copyright message is reproduced below.
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 05/05/99  ramon       1.0    First release
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

/*
 *  <Insert copyright here : it must be BSD-like so everyone can use it>
 *
 *  Author:  Erich Boleyn  <erich@uruk.org>   http://www.uruk.org/~erich/
 *
 *  Header file implementing Intel MultiProcessor Specification (MPS)
 *  version 1.1 and 1.4 SMP hardware control for Intel Architecture CPUs,
 *  with hooks for running correctly on a standard PC without the hardware.
 *
 *  This file was created from information in the Intel MPS version 1.4
 *  document, order number 242016-004, which can be ordered from the
 *  Intel literature center.
 */

#ifndef __SMP_IMPS_H
#define __SMP_IMPS_H


/* APIC Defines */

#define APIC_BROADCAST_ID		       	0xFF


/* Offsets to APIC registers from the base address */

#define IOAPIC_RW                     0x010
#define	IOAPIC_ID                     0x000
#define	APIC_DISCRETE_ID(x)           ((x) >> 24)
#define	IOAPIC_VER                    0x001
#define	APIC_VERSION(x)               ((x) & 0xff)
#define	IOAPIC_MAXREDIR(x)            (((x) >> 16) & 0xff)
#define	IOAPIC_REDIR                  0x010
#define LAPIC_ID                      0x020
#define	LAPIC_INTEGRATED_ID(x)        (((x) >> 24) & 0xf)
#define LAPIC_VER                     0x030
#define	LAPIC_MAX_INTEGRATED_LVT(x)   IOAPIC_MAXREDIR(x)
#define LAPIC_TPR                     0x080
#define LAPIC_APR                     0x090
#define LAPIC_PPR                     0x0a0
#define LAPIC_EOI                     0x0b0
#define LAPIC_LDR                     0x0d0
#define LAPIC_DFR                     0x0e0
#define LAPIC_SPIV                    0x0f0
#define	LAPIC_SPIV_ENABLE_APIC        0x100
#define LAPIC_ISR                     0x100
#define LAPIC_TMR                     0x180
#define LAPIC_IRR                     0x200
#define LAPIC_ESR                     0x280
#define LAPIC_ICR                     0x300
#define	LAPIC_DEST_MASK               0xffffff
#define LAPIC_LVTT                    0x320
#define LAPIC_LVTPC                   0x340
#define LAPIC_LVT0                    0x350
#define LAPIC_LVT1                    0x360
#define LAPIC_LVTE                    0x370
#define LAPIC_TICR                    0x380
#define LAPIC_TCCR                    0x390
#define LAPIC_TDCR                    0x3e0

#define IMPS_MAX_CPUS                 APIC_BROADCAST_ID


/*
 *  Defines representing limitations on values usable in different
 *  situations.  This mostly depends on whether the local apics are
 *  discrete or integrated.
 *
 *  NOTE:  It appears that the local APICs must either be all discrete
 *    or all integrated, or broadcasts won't work right.
 *  NOTE #2:  Given that, the maximum ID which can be sent to predictably
 *    is 14 for integrated APICs and 254 for discrete APICs.  So, this all
 *    implies that a maximum of 15 processors is supported with the
 *    integrated APIC, and a maximum of 255 processors with the discrete
 *    APIC.
 */

#define IMPS_LAPIC_ID(x)  ( imps_all_discrete ?                                \
                            APIC_DISCRETE_ID(x) : LAPIC_INTEGRATED_ID(x) )


/*
 *  This is the value that must be in the "sig" member of the MP
 *  Floating Pointer Structure.
 */

#define IMPS_FPS_SIGNATURE      ('_' | ('M'<<8) | ('P'<<16) | ('_'<<24))
#define IMPS_FPS_IMCRP_BIT      0x80
#define IMPS_FPS_DEFAULT_MAX    0x07


/*
 *  This is the value that must be in the "sig" member of the MP
 *  Configuration Table Header.
 */

#define IMPS_CTH_SIGNATURE      ('P' | ('C'<<8) | ('M'<<16) | ('P'<<24))


/*
 *  These are the "type" values for Base MP Configuration Table entries.
 */

#define	IMPS_FLAG_ENABLED           1
#define IMPS_BCT_PROCESSOR          0
#define	IMPS_CPUFLAG_BOOT           2
#define IMPS_BCT_BUS                1
#define IMPS_BCT_IOAPIC             2
#define IMPS_BCT_IO_INTERRUPT       3
#define IMPS_BCT_LOCAL_INTERRUPT    4
#define	IMPS_INT_INT                0
#define	IMPS_INT_NMI                1
#define	IMPS_INT_SMI                2
#define	IMPS_INT_EXTINT             3


/*
 *  Typedefs and data item definitions done here.
 */

typedef struct imps_fps imps_fps;       /* MP floating pointer structure */
typedef struct imps_cth imps_cth;       /* MP configuration table header */
typedef struct imps_processor imps_processor;
typedef struct imps_bus imps_bus;
typedef struct imps_ioapic imps_ioapic;
typedef struct imps_interrupt imps_interrupt;


/*
 *  MP Floating Pointer Structure (fps)
 *
 *  Look at page 4-3 of the MP spec for the starting definitions of
 *  this structure.
 */

struct imps_fps
{
    UDATA sig;
    imps_cth *cth_ptr;
    UBYTE length;
    UBYTE spec_rev;
    UBYTE checksum;
    UBYTE feature_info[5];
};


/*
 *  MP Configuration Table Header  (cth)
 *
 *  Look at page 4-5 of the MP spec for the starting definitions of
 *  this structure.
 */

struct imps_cth
{
    UDATA   sig;
    UWORD16 base_length;
    UBYTE   spec_rev;
    UBYTE   checksum;
    BYTE    oem_id[8];
    BYTE    prod_id[12];
    UDATA   oem_table_ptr;
    UWORD16 oem_table_size;
    UWORD16 entry_count;
    UDATA   lapic_addr;
    UWORD16 extended_length;
    UBYTE   extended_checksum;
    BYTE    reserved[1];
};


/*
 *  Base MP Configuration Table Types.  They are sorted according to
 *  type (i.e. all of type 0 come first, etc.).  Look on page 4-6 for
 *  the start of the descriptions.
 */

struct imps_processor
{
    UBYTE type;            /* must be 0      */
    UBYTE apic_id;
    UBYTE apic_ver;
    UBYTE flags;
    UDATA signature;
    UDATA features;
    BYTE  reserved[8];
};

struct imps_bus
{
    UBYTE type;            /* must be 1      */
    UBYTE id;
    BYTE  bus_type[6];
};

struct imps_ioapic
{
    UBYTE type;            /* must be 2      */
    UBYTE id;
    UBYTE ver;
    UBYTE flags;
    UADDR addr;
};

struct imps_interrupt
{
    UBYTE   type;          /* must be 3 or 4 */
    UBYTE   int_type;
    UWORD16 flags;
    UBYTE   source_bus_id;
    UBYTE   source_bus_irq;
    UBYTE   dest_apic_id;
    UBYTE   dest_apic_intin;
};


/*
 *  "imps_all_discrete" is non-zero if all of the CPU apics are
 *  discrete.  This is useful to determine if more than 15 CPUs can
 *  be supported.
 */

extern DATA imps_all_discrete;


/*
 *  "imps_enabled" is non-zero if the probe sequence found IMPS
 *  information and was successful.
 */

extern DATA imps_enabled;


/*
 *  This contains the local apic hardware address.
 */

extern UADDR imps_lapic_addr;


/*
 *  This represents the number of CPUs found.
 */

extern DATA imps_num_cpus;


/*
 *  These map from virtual cpu numbers to APIC id's and back.
 */
extern UBYTE imps_cpu_apic_map[IMPS_MAX_CPUS];
extern UBYTE imps_apic_cpu_map[IMPS_MAX_CPUS];


/*
 *  Defines that use variables
 */

#define IMPS_APIC_READ(x)  (*((volatile UDATA *) (imps_lapic_addr+(x))))
#define IMPS_APIC_WRITE(x, y) \
                     (*((volatile UDATA *) (imps_lapic_addr+(x))) = (y))


#endif

