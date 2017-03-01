/*
 * paging.c:  Page mapping calls
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 01/12/98  jens        1.0	first release
 * 07/12/98  jens        1.1    now platform independant
 * 25/12/98  jens        1.2    uses mutexes
 * 27/12/98  jens        1.3    CKpagemapping(Alloc/Free) implemented
 * 31/01/99  jens        1.4    Complete rewrite buddy algorithm removed
 *                              Now uses 128k page blocks
 * 17/02/99  jens        1.5    Implemented Continous bitmap alloc
 * 14/03/99  jens        1.6    Uses standard bitmap alloc from klibs
 * 15/03/99  jens        1.7    Now uses freelist alloc for page alloc
 * 17/03/99  jens        1.8    Implemented CK(un)loadMapping and
 *                              cleaned up the code
 * 07/05/99  ramon       1.9    Added flushing of TLB
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
#include "ckerror.h"
#include "ckobjects.h"
#include "sched.h"

PUBLIC DATA inline CKloadMappingKernelCode(DESCRIPTOR object,ADDR physaddr,ADDR logicaladdr,ADDR length,UWORD32 flags)
/*
 * Loads a pagemapping into a address space
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object:        Descriptor of address space object to load mapping into
 *     physaddr:      Physical address of memory
 *     logicaladdr:   Logical address of memory (how the thread see's it)
 *     flags:         Any PT flags see sys/memory.h
 *
 * RETURNS:
 *     CKloadMapping(): 0 for success, or otherwise error code
 */
{
    struct CKAddrSpcObject *obj;
    DATA error = 0;

    switch(object->objectSignature) {
        case SIGADDRSPC:

            /* here a check should be done to see if the thread owns the memory */

            obj=(struct CKAddrSpcObject *)object;
            if(!CKsetPageMapping(obj->addrSpc, logicaladdr, physaddr, length, flags)) {
                error = ENOMEM;
            }
            break;
        default:
            error = ENOVALIDADDRSPC;
    }

    if(object == currtask->addressSpace)
        CKflushLocalTLB();  /* Make this smarter later on --
                             * CKsetPageMapping() can use invlpg on 486+ */
    return error;
}

PUBLIC DATA inline CKunloadMappingKernelCode(DESCRIPTOR object,ADDR logicaladdr,ADDR length)
/*
 * Unloads a pagemapping from a address space
 * NOTE:  This is a system call
 *
 * INPUT:
 *     object:        Descriptor of address space object to unload mapping from
 *     logicaladdr:   Logical address of memory (how the thread see's it)
 *
 * RETURNS:
 *     CKunloadMapping(): 0 for success, or otherwise error code
 */
{
    struct CKAddrSpcObject *obj;
    DATA error = 0;

    switch(object->objectSignature) {
        case SIGADDRSPC:
            obj=(struct CKAddrSpcObject *)object;
            CKmarkPageNotPresent(obj->addrSpc, logicaladdr, 0, length);
            break;
        default:
            error = ENOVALIDADDRSPC;
    }

    if(object == currtask->addressSpace)
        CKflushLocalTLB();  /* Make this smarter later on --
                             * CKmarkPageNotPresent() can use invlpg on 486+ */
    return error;
}

/**************************************************************************/
/* System Object Manager System Calls (platform dependant)                */
/* these functions is the same as above "Code" is just change to "Stub",  */
/* and some platform dependant stuff is done.                             */
/* ---------------------------------------------------------------------- */
/* Include Need to be here, CK(un)loadMapping is inline expanded in it    */
/* can't be linked, won't produce the same result, faster code.           */
/**************************************************************************/

#include "sys/stubs/paging.h"
