/*
 * main.c:  Main ABL code
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 09/01/99  ramon       1.0    First release
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
#include <multiboot.h>
#include "sys/sysdefs.h"
#include "sys/memory.h"
#include "smp.h"
#include "decl.h"


/*
 * The ABL doesn't have real memory micromanagement.  We use a memory
 * layout as described in the docs.  We use the following variables to
 * keep track of which memory is being used.  These are also passed on
 * to the CK.  All sizes in bytes.
 *
 * Warning: lowMemTop/extMemTop separation, skip first page etc. are
 * x86 specific - should probably move these out of here.
 */

extern UADDR _end[];                /* End of ABL in memory        */

PUBLIC UADDR lowMemTop = PAGESIZE;  /* Skip first page (BIOS info) */
PUBLIC UADDR extMemTop;             /* Skip ABL code/data          */
PUBLIC UADDR totalMem;              /* Total memory                */

PUBLIC bootinfo *binf;              /* The boot information struct */


/*
 * The main ABL code
 */

PUBLIC bootinfo *ABLmain(UDATA mbmagic, struct multiboot_info *mbinfo)
/*
 * The ABL startup code
 *
 * INPUT:
 *     mbmagic:  The multiboot magic number in %eax
 *     mbinfo:   The multiboot info structure
 *
 * RETURNS:
 *     Pointer to the boot information structure
 */
{
    ABLinitCons();

    print("ABL:  Starting Alliance...\n");

    if(mbmagic != MULTIBOOT_VALID || !(mbinfo->flags & MB_INFO_MEMORY) || !(mbinfo->flags & MB_INFO_CMDLINE) || mbinfo->cmdline == 0) {
        print("ABL:  Error:  Alliance only works with a multiboot-compatible bootloader.\n");
        print("ABL:  Please reboot, and check that your bootloader works, that it supports\n");
        print("ABL:  passing the memory size and that the command line is set to the ABL\n");
        print("ABL:  configuration script.\n");
        return NIL;
    }

    /* Pass multiboot info to the shared boot library */
    mbinf = mbinfo;

    /* Do memory setup */
    print("ABL:  Lower memory: %dk\n", mbinfo->mem_lower);
    print("ABL:  Upper memory: %dk\n", mbinfo->mem_upper);
    extMemTop = (UADDR) _end;
    totalMem  = (mbinfo->mem_upper + 1024) * 1024;
    ABLinitMem();

    /* Load the configuration script and execute it */
    print("ABL:  Loading ABL configuration script\n");
    {
        STRING *loc = (STRING *)mbinf->cmdline - 1;
        while((++loc)[0] == ' ');  /* Strip the ABL filename off the */
        while((++loc)[0] != ' ');  /* kernel commandline.  This part */
        while((++loc)[0] == ' ');  /* of multiboot is a bit weird... */
        mbinf->cmdline = (UADDR) loc;
    }
    if(!open((STRING *)mbinfo->cmdline)) {
        print_error();
        print("ABL:  Could not open configuration script\n");
        return NIL;
    }
    filepos = 0;
    filemax = read((UADDR)_end, -1);
    extMemTop = (UADDR) _end + filemax + 1;
    print("ABL:  Executing ABL configuration script\n");
    if(!ABLdoScript((STRING *)_end, (filemax + 1))) {
        print("ABL:  Script execution failed, halting...\n");
        return NIL;
    }

    /* Setup interrupts and paging */
    ABLinitIntr();
    ABLsetupMem();

    print("ABL:  Probing for multiple processors\n");
    ABLinitMP(mbinfo->mem_lower*1024);
    print("ABL:  %d processors installed in system\n", imps_num_cpus);

    print("ABL:  Calibrating ABL delay loop... ");
    print("%d loops\n", ABLcalDelayLoop());

    /* Set up the rest of the boot info structure */
    binf->lowMemTop  = lowMemTop;

    /* Jump to the CK entry point... and enjoy ! */
    print("ABL:  Booting system...\n");

    return binf;
}
