/*
 * main.c:  Main kernel startup code
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 26/11/98  ramon       1.2    Init GRSK demotask / New HW init scheme
 * 28/11/98  ramon       1.3    Caching objects / New stack sizes
 * 17/12/98  ramon       1.4    Create address space for usermode threads
 * 18/03/99  jens        1.5    removed CKinitPage / user mappings
 * 23/07/99  ramon       1.6    Modified to work for real ABL boot
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
#include "sys/sys.h"
#include "sys/memory.h"
#include "sys/hwkernel.h"
#include "sys/realtime.h"
#include "ckobjects.h"
#include "ckkernelcalls.h"
#include "sched.h"
#include "memory.h"
#include "cklib.h"

PUBLIC VOID CKmain(VOID)
/*
 * The hardware-independant part of the kernel startup
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    bootmodule *nextmod = (bootmodule *) LINEAR(sysinf->modules);

    CKprint("Initialising CK...\n");

    CKinitMem();                    /* Initialise CK memory micromgmt   */
    CKinitSched();                  /* Initialise timer and scheduler   */

    CKlock();                       /* Lock interrupts until we're done */

    CKprint("Initialising all modules loaded by ABL...\n");

    /* Loop through the loaded modules and load them into the CK */
    while(nextmod > (bootmodule *) LINEAR(0)) {
        DESCRIPTOR addrspcdesc;
        AddrSpcObject *addrspc;
        ThreadObject  *thread;
        DATA error;

        CKprint("Loading module: phbase=0x%x, entry=0x%x, size=0x%x\n",nextmod->physstart,nextmod->entry,nextmod->size);

        /* Allocate an address space object for the module */
        addrspc = (AddrSpcObject *) CKmemAllocPages(1);

        /* Cache the address space object */
        addrspc->parentKernel = NIL;
        if((error = CKcacheObjectKernelCode((ADDR)addrspc,SIGADDRSPC,sizeof(AddrSpcObject),0))) {
            CKprint("Error in caching address space: %d\n",error);
            goto next;
        }

        /* Map the module into its address space */
        addrspcdesc = addrspc->self;
        if((error = CKloadMappingKernelCode(addrspcdesc,nextmod->physstart,nextmod->entry,nextmod->size,PTWRITE)))
            CKprint("Error in loading mapping: %d\n",error);

        /* Map the address space and thread object page into the addrspc */
        if((error = CKloadMappingKernelCode(addrspcdesc,PHYS(addrspc),nextmod->entry-PAGESIZE,PAGESIZE-1,PTWRITE)))
            CKprint("Error in loading mapping: %d\n",error);

        /* Allocate a thread object for the module */
        thread = (ThreadObject *) (addrspc + 1);

        /* Cache the thread object */
        if((error = CKcacheUserThread(nextmod->entry,0,thread,addrspcdesc)))
            CKprint("Error in caching thread: %d\n",error);

next:
        nextmod = (bootmodule *) LINEAR(nextmod->next);
    }

    CKunlock();                     /* We're go !!!                     */
    quantumDone();                  /* Kickstart all initial tasks      */

    CKidle();                       /* The idle loop                    */
}
