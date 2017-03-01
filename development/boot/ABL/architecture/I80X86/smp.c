/*
 * smp.c:  ABL SMP startup code
 *
 * NOTE:  This file is a modified version of Erich Boleyn's SMP-IMPS
 *        code.  Original copyright message is reproduced below.
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 05/05/99  ramon       1.0    First release
 * 07/05/99  ramon       1.1    Non-default configs should work now
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
 *  <Insert copyright here : it must be BSD-like so anyone can use it>
 *
 *  Author:  Erich Boleyn  <erich@uruk.org>   http://www.uruk.org/~erich/
 *
 *  Source file implementing Intel MultiProcessor Specification (MPS)
 *  version 1.1 and 1.4 SMP hardware control for Intel Architecture CPUs,
 *  with hooks for running correctly on a standard PC without the hardware.
 *
 *  This file was created from information in the Intel MPS version 1.4
 *  document, order number 242016-004, which can be ordered from the
 *  Intel literature center.
 *
 *  General limitations of this code:
 *
 *   (1) : This code has never been tested on an MPS-compatible system with
 *           486 CPUs, but is expected to work.
 */

/*
 * XXX - TODO
 * 1. Document the whole business decently
 * 2. Always set the LAPIC and the IOAPIC to the same address so we can
 *    use a compile-time constant to access it
 * 3. Write in code to set up an SMP configuration structure to pass on
 *    to the CK
 * 4. Finish CPU bootup sequence / trampoline code
 * 5. Change variable names to reflect Alliance coding style
 * 6. Also parse extended entries
 * 7. Set up I/O APIC as well
 * 8. Probably a lot more...
 */


#include <typewrappers.h>
#include <klibs.h>
#include "cmos.h"
#include "smp.h"
#include "decl.h"


/* Defines that are here so as not to be in the global header file */

#define EBDA_SEG_ADDR        0x40e
#define BIOS_RESET_VECTOR    0x467
#define LAPIC_ADDR_DEFAULT   0xfee00000ul
#define IOAPIC_ADDR_DEFAULT  0xfec00000ul
#define CMOS_RESET_CODE      0x00f
#define	CMOS_RESET_JUMP      0x00a
#define CMOS_BASE_MEMORY     0x015
#define DEF_ENTRIES          23


/* The MP configuration table for default configurations */

LOCAL struct {
    imps_processor proc[2];
    imps_bus       bus[2];
    imps_ioapic    ioapic;
    imps_interrupt intin[16];
    imps_interrupt lintin[2];
} defconfig = {
    { { IMPS_BCT_PROCESSOR, 0, 0, 0, 0, 0},
      { IMPS_BCT_PROCESSOR, 1, 0, 0, 0, 0} },

    { { IMPS_BCT_BUS, 0, {'E', 'I', 'S', 'A', ' ', ' '} },
      { 255,          1, {'P', 'C', 'I', ' ', ' ', ' '} } },

    { IMPS_BCT_IOAPIC, 0, 0, IMPS_FLAG_ENABLED, IOAPIC_ADDR_DEFAULT },

    { { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_EXTINT, 0, 0, 0,  0xFF, 0},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 1,  0xFF, 1},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 0,  0xFF, 2},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 3,  0xFF, 3},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 4,  0xFF, 4},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 5,  0xFF, 5},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 6,  0xFF, 6},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 7,  0xFF, 7},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 8,  0xFF, 8},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 9,  0xFF, 9},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 10, 0xFF, 10},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 11, 0xFF, 11},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 12, 0xFF, 12},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 13, 0xFF, 13},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 14, 0xFF, 14},
      { IMPS_BCT_IO_INTERRUPT,    IMPS_INT_INT,    0, 0, 15, 0xFF, 15} },

    { { IMPS_BCT_LOCAL_INTERRUPT, IMPS_INT_EXTINT, 0, 0, 15, 0xFF, 0},
      { IMPS_BCT_LOCAL_INTERRUPT, IMPS_INT_NMI,    0, 0, 15, 0xFF, 1} }
};


/* Exported globals here */

PUBLIC DATA imps_enabled      = 0;
PUBLIC DATA imps_num_cpus     = 1;
PUBLIC DATA imps_all_discrete = 0;

PUBLIC UADDR imps_lapic_addr  = 0;
PUBLIC UBYTE imps_cpu_apic_map[IMPS_MAX_CPUS];
PUBLIC UBYTE imps_apic_cpu_map[IMPS_MAX_CPUS];


#if 0
LOCAL DATA ABLbootCPU(imps_processor *proc)
/*
 * Primary function for booting individual CPUs.
 *
 * INPUT:
 *     proc:   Processor to boot
 *
 * RETURNS:
 *     0 on error, otherwise success
 */
{
    UDATA apicid = proc->apic_id;
    UDATA bootaddr, send_status, accept_status, cfg;
    UADDR bios_reset_vector = BIOS_RESET_VECTOR;
    extern UBYTE ABLtrampolineMP[];

    /* copy the trampoline code to low memory */
    bootaddr = 256*1024;
    KLmemoryCopy((UBYTE *)bootaddr, ABLtrampolineMP, 32);

    /* set BIOS reset vector */
    CMOS_WRITE_BYTE(CMOS_RESET_CODE, CMOS_RESET_JUMP);
    *((volatile UDATA *) bios_reset_vector) = bootaddr << 12;

    /* clear the error register */
    if(proc->apic_ver & 0x10) {
        IMPS_APIC_WRITE(LAPIC_ESR, 0);
        accept_status = IMPS_APIC_READ(LAPIC_ESR);
    }

#if 0
    /* assert INIT IPI */
    cfg = IMPS_APIC_READ(LAPIC_ICR+1);
    cfg &= LAPIC_DEST_MASK;
    IMPS_APIC_WRITE(LAPIC_ICR+1, cfg);
    cfg = IMPS_APIC_READ(LAPIC_ACR);
    cfg &= ;

    /* Finish adding startup sequence */
#endif

    /* clean up BIOS reset vector */
    CMOS_WRITE_BYTE(CMOS_RESET_CODE, 0);
    *((volatile UDATA *) bios_reset_vector) = 0;
 
    /* Generic CPU startup sequence ends here */
    return 1;
}
#endif


LOCAL VOID ABLparseMP(UADDR start, DATA count)
/*
 * This function parses the MP Configuration Table
 *
 * INPUT:
 *     start:   Start of MP configuration table
 *     length:  Length of MP configuration table
 *
 * RETURNS:
 *     none
 */
{
    /* Loop through the MP configuration table */
    while(count-- > 0) {

        /* Find out the type of entry */
        switch(*((UBYTE *)start)) {
            case IMPS_BCT_PROCESSOR:     /* Another processor found     */
            {
                imps_processor *proc = (imps_processor *) start;

                /* Skip this processor if it isn't enabled */
                if(!(proc->flags & IMPS_FLAG_ENABLED)) {
                    start += 12;
                    break;
                }

                /* Check whether it has a discrete or internal local APIC */
                if(proc->apic_ver > 0xf)
                    imps_all_discrete = 0;

                /* Skip it if it's the boot processor (it's accounted for) */
                if(proc->flags & IMPS_CPUFLAG_BOOT) {
                    start += 12;
                    break;
                }

                /* Map the CPU's local APIC in the CPU<->APIC table */
                imps_cpu_apic_map[imps_num_cpus] = proc->apic_id;
                imps_apic_cpu_map[proc->apic_id] = imps_num_cpus;

                /* Boot the CPU and count it as an installed processor */
                /* if (ABLbootCPU(proc) */ imps_num_cpus++;

                start += 12;
                break;
            }

            case IMPS_BCT_BUS:           /* Another bus found           */
            {
//              imps_bus *bus = (imps_bus *) start;

                break;
            }

            case IMPS_BCT_IOAPIC:        /* Another I/O APIC found      */
            {
                imps_ioapic *ioapic = (imps_ioapic *) start;

                /* Skip this I/O APIC if it isn't enabled */
                if(!(ioapic->flags & IMPS_FLAG_ENABLED))
                    break;

                break;
            }

            case IMPS_BCT_IO_INTERRUPT:  /* Another I/O interrupt found */
            {
//              imps_interrupt *intr = (imps_interrupt *) start;

                break;
            }

            case IMPS_BCT_LOCAL_INTERRUPT:   /* Another local int found */
            {
//              imps_interrupt *intr = (imps_interrupt *) start;

                break;
            }

            default:
                print("smp:  Warning:  Unknown MP Configuration Table entry type %d encountered\n", *((UBYTE *)start));
        }

        start += 8;
    }
}


LOCAL DATA ABLchecksumMP(UADDR start, ADDR length)
/*
 * MPS checksum function: calculate checksum on a memory range
 *
 * INPUT:
 *     start:   Start of memory to checksum
 *     length:  Amount of memory to checksum
 *
 * RETURNS:
 *     The checksum for the specified memory area
 */
{
    UDATA sum = 0;

    while(length-- > 0) {
        sum += *((UBYTE *) (start++));
    }

    return (sum & 0xff);
}


LOCAL DATA ABLcheckMP(imps_fps *fps)
/*
 * This function checks the MP tables for fatal errors
 *
 * INPUT:
 *     fps:  Pointer to the floating pointer table
 *
 * RETURNS:
 *     0 on success, otherwise error
 */
{
    DATA sum;
    imps_cth *local_cth = (imps_cth *) fps->cth_ptr;

    /* If this is a default configuration, check whether it's valid */
    if(fps->feature_info[0] > IMPS_FPS_DEFAULT_MAX) {
        print("smp:  Invalid MP System Configuration type %d\n", fps->feature_info[0]);
        return 1;
    }

    /* Check the MP configuration table */
    if(fps->cth_ptr) {
        /* Calculate the MP configuration table checksum */
        sum = ABLchecksumMP((UADDR)local_cth, local_cth->base_length);

        /* Check for a valid signature and checksum */
        if(local_cth->sig != IMPS_CTH_SIGNATURE || sum) {
            print("smp:  Bad MP Config Table sig 0x%x and/or checksum 0x%x\n", (UADDR) fps->cth_ptr, sum);
            return 1;
        }

        /* Check that the version information corrseponds between the tables */
        if(local_cth->spec_rev != fps->spec_rev) {
            print("smp:  Bad MP Config Table sub-revision # %d\n", local_cth->spec_rev);
            return 1;
        }

        /* Check the extended entries as well if they're available */
        if(local_cth->extended_length) {
            /* Calculate the extended entries checksum */
            sum = (ABLchecksumMP(((UADDR)local_cth) +
                   local_cth->base_length, local_cth->extended_length) +
                   local_cth->extended_checksum) & 0xff;

            /* Check if the checksum is valid */
            if(sum) {
                print("smp:  Bad Extended MP Config Table checksum 0x%x\n", sum);
                return 1;
            }
        }
    } else if(!fps->feature_info[0]) {
        /*
         * There's no MP config table, so this must be one of the default
         * configurations (see chapter 5 of the spec.  Check that this
         * is a valid default configuration.
         */
        print("smp:  Missing configuration information\n");
        return 1;
    }

    return 0;    /* Success */
}


LOCAL VOID ABLreadMP(imps_fps *fps)
/*
 * This function reads and parses the floating pointer table.
 *
 * INPUT:
 *     fps:  Pointer to the floating pointer table
 *
 * RETURNS:
 *     none
 */
{
    DATA  apicid;
    UADDR cth_start, cth_count;
    imps_cth *local_cth = (imps_cth *) fps->cth_ptr;
    STRING *str;

    print("smp:  Intel MultiProcessor Spec 1.%d BIOS support detected\n", fps->spec_rev);

    /* Check that all tables are sane and correct */
    if(ABLcheckMP(fps)) {
        print("smp:  The MP configuration table is corrupt.  Disabling MPS support\n");
        return;
    }

    /* Get the APIC address and interrupt configuration */
    if(fps->feature_info[1] & IMPS_FPS_IMCRP_BIT) {
        str = "IMCR and PIC";
    } else {
        str = "Virtual Wire";
    }

    /* Find out the address of the local APIC */
    if(fps->cth_ptr) {
        imps_lapic_addr = local_cth->lapic_addr;
    } else {
        imps_lapic_addr = LAPIC_ADDR_DEFAULT;
    }

    print("smp:  APIC config: \"%s mode\"    Local APIC address: 0x%x\n", str, imps_lapic_addr);

    /* Enable the local APIC for the current CPU (the BSP) */
    apicid = IMPS_APIC_READ(LAPIC_SPIV);
    IMPS_APIC_WRITE(LAPIC_SPIV, apicid|LAPIC_SPIV_ENABLE_APIC);

    /* Check whether it's a discrete APIC on an internal one (Pentium+) */
    imps_all_discrete = !(IMPS_APIC_READ(LAPIC_VER) & 0xf0);

    /* Get our APIC ID and map it in the CPU<->APIC table */
    apicid = IMPS_LAPIC_ID(IMPS_APIC_READ(LAPIC_ID));
    imps_cpu_apic_map[0] = apicid;
    imps_apic_cpu_map[apicid] = 0;

    /* Find the MP Configuration Table Header */
    if(fps->cth_ptr) {
        /* Print some interesting data to the screen */
        STRING str1[16], str2[16];
        KLmemoryCopy(str1, local_cth->oem_id, 8);
        str1[8] = 0;
        KLmemoryCopy(str2, local_cth->prod_id, 12);
        str2[12] = 0;
        print("smp:  OEM id: %s  Product id: %s\n", str1, str2);

        /* Set configuration table header location to the found CTH */
        cth_start = ((UADDR) local_cth) + sizeof(imps_cth);
        cth_count = local_cth->entry_count;
    } else {
        /*
         * Okay, this computer doesn't have a configuration table.
         * That means it must be one of the standard configurations as
         * described in chapter 5 of the spec.  We set up some information
         * and then parse our own default configuration table in stead.
         */

        /* Get and set up the I/O APIC's ID and version in the defconfig */
        *((volatile UDATA *) IOAPIC_ADDR_DEFAULT) =  IOAPIC_ID;
        defconfig.ioapic.id = APIC_DISCRETE_ID(*((volatile UDATA *)
                                        (IOAPIC_ADDR_DEFAULT+IOAPIC_RW)));
        *((volatile UDATA *) IOAPIC_ADDR_DEFAULT) =  IOAPIC_VER;
        defconfig.ioapic.ver = APIC_VERSION(*((volatile UDATA *)
                                        (IOAPIC_ADDR_DEFAULT+IOAPIC_RW)));
        defconfig.proc[apicid].flags = IMPS_FLAG_ENABLED|IMPS_CPUFLAG_BOOT;
        defconfig.proc[!apicid].flags = IMPS_FLAG_ENABLED;

        /* Change the bus information in defconfig if neccessary */
        if(fps->feature_info[0] == 1 || fps->feature_info[0] == 5) {
            KLmemoryCopy(defconfig.bus[0].bus_type, "ISA   ", 6);
        }
        if(fps->feature_info[0] == 4 || fps->feature_info[0] == 7) {
            KLmemoryCopy(defconfig.bus[0].bus_type, "MCA   ", 6);
        }

        /* Check whether we have integrated local APICs and a PCI bus */
        if(fps->feature_info[0] > 4) {
            defconfig.proc[0].apic_ver = 0x10;
            defconfig.proc[1].apic_ver = 0x10;
            defconfig.bus[1].type = IMPS_BCT_BUS;
        }

        /* Check for IRQ0 and DMA chaining to the I/O APIC */
        if(fps->feature_info[0] == 2) {
            defconfig.intin[2].type = 255;
            defconfig.intin[13].type = 255;
        }

        /* Check whether the I/O APIC inputs are inverted */
        if(fps->feature_info[0] == 7) {
            defconfig.intin[0].type = 255;
        }

        /* Set configuration table header location to the default config CTH */
        cth_start = (UADDR) &defconfig;
        cth_count = DEF_ENTRIES;
    }

    /* Parse the MP configuration table */
    ABLparseMP(cth_start, cth_count);

    /* Enable multiprocessing */
    imps_enabled = 1;
}


LOCAL DATA ABLscanMP(UADDR start, ADDR length)
/*
 *  Given a region to check, this actually looks for the "MP Floating
 *  Pointer Structure".  The return value indicates if the correct
 *  signature and checksum for a floating pointer structure of the
 *  appropriate spec revision was found.  If so, then do not search
 *  further.
 *
 *  NOTE:  The memory scan will always be in the bottom 1 MB.
 *
 *  This function presumes that "start" will always be aligned to a 16-bit
 *  boundary.
 *
 * INPUT:
 *     start:   Start of memory to scan
 *     length:  Amount of memory to scan
 *
 * RETURNS:
 *     0 if floating pointer table not found, otherwise found and parsed
 */
{
    while (length > 0) {
        imps_fps *fps = (imps_fps *) start;

        if(fps->sig == IMPS_FPS_SIGNATURE && fps->length == 1 &&
          (fps->spec_rev == 1 || fps->spec_rev == 4) &&
          !ABLchecksumMP(start, 16)) {
            ABLreadMP(fps);
            return 1;
        }

        length -= 16;
        start += 16;
    }

    return 0;
}


PUBLIC DATA ABLinitMP(UADDR mem_lower)
/*
 *  This is the primary function for probing for MPS compatible hardware
 *  and BIOS information.  The probe looks for the "MP Floating Pointer
 *  Structure" at locations listed at the top of page 4-2 of the spec.
 *
 *  We assume that the memory mapping is linear and that the memory in
 *  which the MP config tables are is not messed up.
 *
 * INPUT:
 *     mem_lower:   The amount of lower (conventional) memory in the system
 *
 * RETURNS:
 *     ABLinitMP(): The amount of processors installed in the system
 */
{
    /* Determine possible address of the EBDA (extended BIOS data area) */
    UADDR ebda_addr = *((UWORD16 *) EBDA_SEG_ADDR) << 4;

    /* Sanity check:  if we have weird low memory layout, this can't be SMP */
    if(mem_lower < 512*1024 || mem_lower > 640*1024) {
        return 1;
    }

    /* If the EBDA doesn't fit into low memory, scan page 0 in stead */
    if(ebda_addr > mem_lower - 1024 || ebda_addr +
       *((UBYTE *) ebda_addr) * 1024 > mem_lower) {
        ebda_addr = 1;
    }

    /* Scan the memory for the MP Floating Pointer table and parse it */
    if(((ebda_addr &&
         ABLscanMP(ebda_addr, 1024))  || ABLscanMP(mem_lower - 1024, 1024) ||
         ABLscanMP(0xF0000, 0x10000)) && imps_enabled) {
        return imps_num_cpus;
    }

    /* If no BIOS info on MPS hardware is found, then return failure */
    return 1;
}
