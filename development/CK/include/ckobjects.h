/*
 * ckobjects.h:  The CK object types, object headers, &stuff
 *
 * This file contains defenitions (structures) for all the CK object
 * types, as well as object header structures and type definitions
 * concerning the CK objects.
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

#ifndef __CKOBJECTS_H
#define __CKOBJECTS_H

#include <typewrappers.h>    /* Type wrappers                          */
#include "sys/taskstate.h"   /* Task state struct (hardware dependant) */
#include "sys/memory.h"      /* Virtual memory stuff                   */

/***********************************************************************/

/*
 * The Object Header
 *
 * This is the structure that the CK uses to keep track of objects, and
 * their dependencies.  It is the first member of every CK object
 * structure, and is common to every object.
 * All the dependencies of each object are kept track of in the Object
 * Dependency Graph (see graph.c for more information on how object
 * headers are used.)  The types VERTEX and EDGE are used by the ODG code
 * They reperesent elements of the ODG.
 */

struct ObjectHeader;

typedef struct ObjectHeader VERTEX;
typedef struct { VERTEX *parent, *child; } EDGE;

typedef struct ObjectHeader {
    /* Object information */
    DATA   objectSignature;  /* Object Signature (det. type of object) */
    DATA   flags;            /* Miscellaneous flags                    */

    /* Object data */
    ADDR   objectData;       /* Pointer to the writeback object        */

    /* Security data */
    struct {
        UDATA level;         /* Security level                         */
        UDATA instance;      /* Security instance in security level    */
    } capability;

    /* Object list data */
    VERTEX *nextObject;      /* Pointer to next peer object            */
    VERTEX *prevObject;      /* Pointer to previous peer object        */

    /* Dependency data */
    UDATA  dependencies;     /* Number of dependant objects            */
    EDGE   *firstDependency; /* Pointer to first dependency in ODA     */
} __attribute__ ((packed)) ObjectHeader;

/* Object flags */

#define OFLOCKED    1    /* The object is locked into the CK */

/* The heads of the linked lists of objects */

extern VERTEX *kernelObjHead;
extern VERTEX *threadObjHead;
extern VERTEX *addrSpcObjHead;
extern VERTEX *shMemObjHead;

/*
 * Object Descriptors
 *
 * Object descriptors are used to specify any object loaded in the system 
 * in system calls etc. Object descriptors are defined as pointers to
 * their object headers in the CK address space.
 */

typedef struct ObjectHeader *DESCRIPTOR;

#define SIGTHREAD  0x416c4f54    /* Thread object         'AlOT'  */
#define SIGADDRSPC 0x416c4f41    /* Address space object  'AlOA'  */
#define SIGKERNEL  0x416c4f4b    /* Kernel object         'AlOK'  */
#define SIGSHMEM   0x416c4f53    /* Share memory object   'AlOS'  */

/*
 * Validation macros
 *
 * These macros are used to determine whether an object descriptor
 * points to a certain type of object.
 *
 * Usage:
 * if(!CKvalidThread(vertex)) CKprint("Divide by cucumber error !!\n");
 */

#define CKvalidThread(vertex)  (vertex->objectSignature == SIGTHREAD)
#define CKvalidAddrSpc(vertex) (vertex->objectSignature == SIGADDRSPC)
#define CKvalidKernel(vertex)  (vertex->objectSignature == SIGKERNEL)
#define CKvalidSHMem(vertex)   (vertex->objectSignature == SIGSHMEM)

/***********************************************************************/

/* Predeclarations;  these circular dependencies are *so* confusing !!! */
struct CKResource;
typedef struct CKResource CKResource;

/***********************************************************************/

/*
 * Thread Objects:
 *
 * Each of these represents one thread in the system.
 * These are scheduled by the thread scheduler.
 */

/*
 * The time values used by the scheduler are relative time values.  They
 * are represented by a signed long value (signed time values make it
 * possible to implement circle comparisons, which keep the scheduler
 * going.
 */

typedef DATA TIME;

/*
 * The signal structure contains information about a pending signal.
 * A signal consists of a signal number (declared in include/cksignals.h),
 * a data variable (extra) and the originator thread descriptor.  Each
 * signal also has a priority.  In alliance, in principle only the CK can
 * send signals.
 *
 * The highest bit of the signal number is used to store whether this
 * signal is currently being handled or not.  This is essential in order
 * to restart signals correctly if they are overridden by a high-priority
 * signal, as well as to determine whether a lower-priority signal can
 * be restarted.
 */

#define SFBUSY (1 << ((sizeof(UDATA) * 8) - 1))

typedef struct Signal {
    UDATA signal;               /* The signal number                   */
    UDATA extra;                /* The signal data                     */
    UDATA priority;             /* The signal priority                 */
    DESCRIPTOR sender;          /* Sender thread of this signal        */
    struct Signal *next;        /* Pointer to the next pending signal  */
} __attribute__ ((packed)) SIGNAL;

/* Now we get the thread object itself */

typedef enum {    /* ThreadState contains state of thread. Can be: */
    ts_Running,   /* Running in timesharing mode                   */
    ts_RealTime,  /* Running in realtime mode                      */
    ts_Idle,      /* Running only when the system is idle          */
    ts_Blocked    /* Blocked (not currently running)               */
} ThreadState;

/* Tunnel types */

typedef enum {
    tt_Conservative,
    tt_Swapping,
    tt_Unblocking
} TunnelType;
/*
 * The type EVENT is one scheduling event (ie, one scheduling quantum,
 * including all information that belongs.)  This information is stored in
 * the thread object structure.  EVENT is used by the thread scheduler
 * (see sched.c, pri_heap.c.)
 */

struct CKThreadObject;
typedef struct CKThreadObject EVENT;

typedef struct CKThreadObject {
    /* Object Header */
    struct ObjectHeader head; /* The object header                      */

    /* Related Objects */
    DESCRIPTOR addressSpace;  /* Address space of thread                */
    DESCRIPTOR parentKernel;  /* The parent kernel of this thread       */

    /* Scheduler control variables */
    TIME quantum;             /* The thread's quantum value             */
    TIME defer;               /* The thread's defer value               */
    TIME time;                /* Time when this event should next occur */

    /* Scheduler scratch space */
    EVENT **heap;             /* Backpointer to event in the heap       */
    ThreadState ts;           /* Thread state                           */

    /* Signal-related stuff */
    SIGNAL *signals;          /* Head of linked list of pending signals */

    /* Resource allocation data */
    CKResource *trap;         /* Head of allocated global interrupts    */
    CKResource *dmachan;      /* Head of allocated DMA channels         */
    CKResource *ioregion;     /* Head of allocated I/O regions          */

    /* Hardware-related stuff */
    TaskState taskState;      /* The thread's processor state           */
} __attribute__ ((packed)) CKThreadObject;

/***********************************************************************/

/*
 * Address Space Objects:
 *
 * Each of these represents one virtual address space in the system.
 * These are mapped onto the parent kernel's physical memory.
 */

typedef struct CKAddrSpcObject {
    /* Object Header */
    struct ObjectHeader head; /* The object header                      */

    /* Related Objects */
    DESCRIPTOR parentKernel;  /* The parent kernel of this address spc  */

    /* The address space data */
    AddressSpace addrSpc;     /* This contains page tables etc.         */
} __attribute__ ((packed)) CKAddrSpcObject;

/***********************************************************************/

/*
 * Shared Memory Objects:
 *
 * Each of these represents one shared memory space in the system.
 */

#include "shmem.h"

typedef struct CKSHMemObject {
    /* Object Header */
    struct ObjectHeader head; /* The object header                      */

    /* The SHMEMOBJ data */
    SHMEMOBJ *shMem;                   /* This contains shMem etc.       */
} __attribute__ ((packed)) CKSHMemObject;

/***********************************************************************/

/*
 * Kernel Objects:
 *
 * A kernel object defines a set of resources (memory, signal handlers,
 * and others) that are shared by all of the threads belonging to the
 * kernel.
 */

typedef struct CKKernelObject {
    /* Object Header */
    struct ObjectHeader head; /* The object header                      */

    /* Memory stuff */
    DESCRIPTOR addrSpace;     /* The kernel address space               */

    /* Critical kernel threads */
    ADDR sigHandler;          /* Address of signal handler function     */

    /* Resource allocation data */
    CKResource *physregion;   /* Head of allocated physical memory      */
} __attribute__ ((packed)) CKKernelObject;

/***********************************************************************/

#include "ckresource.h"
#include "resource.h"
#include "objects.h"

#endif
