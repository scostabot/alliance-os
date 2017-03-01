/*
 * ckobjects.h:  The loading and writeback structures of the CK objects
 *
 * (C) 1998, 1999 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 16/11/98  ramon       1.0    First release
 * 26/11/98  ramon       1.1    Added memory spaces, address spaces, etc.
 * 27/12/98  ramon       1.2    Added resource management structures
 * 30/12/98  ramon       1.3    Removed memory space objects
 * 31/12/98  ramon       1.4    Removed CKdescToVertex()-family macros
 * 15/01/99  ramon       1.5    Added signals
 * 20/01/99  ramon       1.6    Some more small changes for signal handling
 * 01/08/99  ramon       1.7    Big overhaul; new object scheme
 * 01/08/99  ramon       1.8    Created separate file with loading structs
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

#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <typewrappers.h>    /* Type wrappers                          */
#include "sys/taskstate.h"   /* Task state struct (hardware dependant) */
#include "sys/memory.h"      /* Virtual memory stuff                   */
#include "ckobjects.h"       /* The CK versions of these objects       */
#include "resource.h"        /* The resource loading structures        */

/***********************************************************************/

/*
 * Thread Objects:
 *
 * Each of these represents one thread in the system.
 * These are scheduler by the thread scheduler.
 */

typedef struct ThreadObject {
    /* Self pointer */
    DESCRIPTOR self;          /* Descriptor of this object              */

    /* Related Objects */
    DESCRIPTOR addressSpace;  /* Address space of thread                */
    DESCRIPTOR parentKernel;  /* The parent kernel of this thread       */

    /* Scheduler control variables */
    TIME quantum;             /* The thread's quantum value             */
    TIME defer;               /* The thread's defer value               */
    TIME time;                /* Time when this event should next occur */
    TIME realtime;            /* The realtime of start of this quantum  */

    /* Signal-related stuff */
    DATA interruptable;       /* Is 0 if signal can interrupt us        */

    /* Resource allocation data */
    Resource *trap;           /* Head of allocated global interrupts    */
    Resource *dmachan;        /* Head of allocated DMA channels         */
    Resource *ioregion;       /* Head of allocated I/O regions          */

    /* Hardware-related stuff */
    UserState userState;      /* The thread's processor user state      */
} __attribute__ ((packed)) ThreadObject;

/***********************************************************************/

/*
 * Address Space Objects:
 *
 * Each of these represents one virtual address space in the system.
 * These are mapped onto the parent kernel's physical memory.
 */

typedef struct AddrSpcObject {
    /* Self pointer */
    DESCRIPTOR self;          /* Descriptor of this object              */

    /* Related Objects */
    DESCRIPTOR parentKernel;  /* The parent kernel of this address spc  */
} __attribute__ ((packed)) AddrSpcObject;

/***********************************************************************/

/*
 * Shared Memory Objects:
 *
 * Each of these represents one shared memory space in the system.
 */

typedef struct SHMemObject {
    /* Self pointer */
    DESCRIPTOR self;          /* Descriptor of this object              */
} __attribute__ ((packed)) SHMemObject;

/***********************************************************************/

/*
 * Kernel Objects:
 *
 * A kernel object defines a set of resources (memory, signal handlers,
 * and others) that are shared by all of the threads belonging to the
 * kernel.
 */

typedef struct KernelObject {
    /* Self pointer */
    DESCRIPTOR self;          /* Descriptor of this object              */

    /* Memory stuff */
    DESCRIPTOR addrSpace;     /* The kernel address space               */

    /* Critical kernel threads */
    ADDR sigHandler;          /* Address of signal handler function     */

    /* Resource allocation data */
    Resource *physregion;     /* Head of allocated physical memory      */
} __attribute__ ((packed)) KernelObject;

#endif
