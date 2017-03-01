/*
 * ckobjects.h:  The CK object types, object headers, &stuff
 *
 * NOTE:  modified 8/3/99 for the scheduler simulation [Ramon]
 *
 * This file contains defenitions (structures) for all the CK object
 * types, as well as object header structures and type defenitions
 * concerning the CK objects.
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
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

/*
 * The Object Header
 *
 * This is the structure that the CK uses internally to keep track of
 * objects, and their dependencies.  Every object loaded into the CK is
 * represented by one object header.  All the dependencies of each object
 * are kept track of in the Object Dependency Graph (see graph.c for more 
 * information on how object headers are used.)
 */

struct ObjectHeader;    /* Predeclaration */
struct CKResource;      /* Predeclaration */

/*
 * These are the CK resource type enum and signatures.  They are used
 * to describe CK resources
 */

typedef enum {
    RESPHYSREGION,
    RESTRAP,
    RESDMA,
    RESIOREGION,
    TOTALRES
} ResourceType;

#define SIGPHYSREGION  0x416c5250    /*  'AlRP'  */
#define SIGTRAP        0x416c5254    /*  'AlRT'  */
#define SIGDMA         0x416c5244    /*  'AlRD'  */
#define SIGIOREGION    0x416c5249    /*  'AlRI'  */

/*
 * The types VERTEX and EDGE are used by the ODG code (graph.c.)  They
 * reperesent elements of the ODG.
 */

typedef struct ObjectHeader VERTEX;
typedef struct {VERTEX *parent, *child;} EDGE;

/*
 * The type CKResource is used by the CK resource management code
 * (resource.c)
 */

typedef struct CKResource CKResource;

/* Now we get the objectHeader structure itself */

struct ObjectHeader {
    /* Object information */
    DATA   objectSignature;  /* Object Signature (det. type of object) */
    DATA   flags;            /* Miscellaneous flags                    */

    /* Object data */
    ADDR   objectSize;       /* Object Size in bytes                   */
    ADDR   objectData;       /* Pointer to the object itself           */
    VERTEX *parentKernel;    /* Pointer to the parent kernel           */

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

    /* Resource allocation data */
    CKResource *resources[TOTALRES];  /* Resource allocation lists     */
};

/* Object flags */

#define OFLOCKED    1    /* The object is locked into the CK */
#define OFTHKERNEL  2    /* The object is a kernel thread    */

/* The heads of the linked lists of objects */

extern VERTEX *kernelObjHead;
extern VERTEX *threadObjHead;
extern VERTEX *addrSpcObjHead;

/*
 * Resource allocation structure
 *
 * This is used to allocate resources like physical memory, DMA channels,
 * etc. to specific kernels
 */

struct CKResource {
    DATA typeSignature;    /* Resource type signature                */
    ADDR size;             /* Size of this resource structure        */
    VERTEX *ownerKernel;   /* Kernel that allocated this resource    */

    CKResource *next;      /* Next resource of kernel with same type */
    CKResource *prev;      /* Previous resource of same type         */

    DATA flags;            /* Flags for this resource                */
    DATA resource[0];      /* The resource data itself               */
}; 

typedef struct CKResource * RESOURCE;


/*
 * Object Descriptors
 *
 * Object descriptors are used to specify any object loaded in the system 
 * in system calls etc. Object descriptors are defined as pointers to
 * their object headers in the CK address space.
 */

typedef struct ObjectHeader *DESCRIPTOR;


/*
 * Object signatures
 *
 * These determine what type of object a descriptor points to
 */

#define SIGTHREAD  0x416c4f54    /* Thread object         'AlOT'  */
#define SIGADDRSPC 0x416c4f41    /* Address space object  'AlOA'  */
#define SIGKERNEL  0x416c4f4b    /* Kernel object         'AlOK'  */

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


/*
 * Now we get to the actual object types:
 *
 * Thread Objects, Kernel Objects, and Address Space Objects.
 */

/*
 * Thread Objects:
 *
 * Each of these represents one thread in the system.
 * These are scheduler by the thread scheduler.
 */

struct ThreadObject;    /* Predeclaration */

/*
 * The type EVENT is one scheduling event (ie, one scheduling quantum,
 * including all information that belongs.)  This information is stored in
 * the thread object structure.  EVENT is used by the thread scheduler
 * (see sched.c, pri_heap.c.)
 */

typedef struct ThreadObject EVENT;  /* one scheduling event */

/*
 * The time values used by the scheduler are relative time values.  They
 * are represented by a signed long value (signed time values make it
 * possible to implement circle comparisons, which keep the scheduler
 * going.
 */

typedef DATA TIME;

/*
 * The signal structure contains information about a pending signal.
 * A signal consists of a signal number (declared in include/cksignals.h,)
 * a data variable (extra) and the originator thread descriptor.  Each
 * signal also has a priority.  In alliance, in principle only the CK can
 * send signals.  The most-often used signal is the global interrupt
 * signal.
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
} SIGNAL;

/* Now we get the thread object itself */

typedef enum {    /* ThreadState contains state of thread. Can be: */
    ts_Running,   /* Running in timesharing mode                   */
    ts_RealTime,  /* Running in realtime mode                      */
    ts_Idle,      /* Running only when the system is idle          */
    ts_Blocked    /* Blocked (not currently running)               */
} ThreadState;

struct ThreadObject {
    /* Self pointer */
    DESCRIPTOR self;          /* Descriptor of this object              */

    /* Memory-related stuff */
    DESCRIPTOR addressSpace;  /* Address space of thread                */
    ADDR inBuffer;            /* Incoming IOC buffer                    */
    ADDR outBuffer;           /* Outgoing IOC buffer                    */

    /* Scheduler control variables */
    TIME quantum;             /* The thread's quantum value  (see the   */
    TIME defer;               /* The thread's defer value     CK specs) */
    TIME time;                /* Time when this event should next occur */

    /* Signal-related stuff */
    SIGNAL *signals;          /* Head of linked list of pending signals */
    DATA interruptable;       /* Is 0 if signal can interrupt us        */

    /* Scheduler scratch space */
    EVENT **heap;             /* Backpointer to event in the heap       */
    ThreadState ts;           /* Thread state                           */
    TIME realtime;            /* If RT task, current realtime value     */

    /* Hardware-related stuff */
    VOID *taskState;          /* The thread's processor state           */
};


/*
 * Address Space Objects:
 *
 * Each of these represents one virtual address space in the system.
 * These are mapped onto the parent kernel's physical memory.
 */

struct AddrSpcObject {
    /* Self pointer */
    DESCRIPTOR self;                  /* Descriptor of this object      */
};


/*
 * Kernel Objects:
 *
 * These provide services for their applications, as well as being
 * CORBA servers
 */

struct KernelObject {
    /* Self pointer */
    DESCRIPTOR self;          /* Descriptor of this object              */

    /* Memory-related stuff */
    DESCRIPTOR addrSpace;     /* The kernel address space               */
    ADDR inBuffer;            /* Incoming IOC buffer                    */
    ADDR outBuffer;           /* Outgoing IOC buffer                    */

    /* Critical kernel threads */
    DESCRIPTOR ORBThread;     /* Server ORB thread                      */
    DESCRIPTOR sigThread;     /* Signal/Interrupt handler thread        */
    ADDR sigHandler;          /* Address of signal handler function     */
};

/* Rest:  TODO */

#endif
