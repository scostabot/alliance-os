/*
 * resource.h:  Defenitions for resource management
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 27/12/98  ramon       1.0    First release
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

#ifndef __CKRESOURCE_H
#define __CKRESOURCE_H

#include <typewrappers.h>
#include "ckobjects.h"
#include "objects.h"

/*
 * Resource allocation structure
 *
 * This is used to allocate resources like physical memory, DMA channels,
 * etc. to specific objects
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

struct CKResource {
    DATA typeSignature;    /* Resource type signature                */
    DESCRIPTOR owner;      /* Object that allocated this resource    */

    CKResource *next;      /* Next resource of kernel with same type */
    CKResource *prev;      /* Previous resource of same type         */

    DATA flags;            /* Flags for this resource                */
    DATA resource[0];      /* The resource data itself               */
} __attribute__ ((packed));

typedef struct CKResource *RESOURCE;


/* These are the expanded CK resources */

typedef struct CKIOregionAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                */
    DESCRIPTOR owner;      /* Object that allocated this resource    */
    CKResource *next;      /* Next resource of kernel with same type */
    CKResource *prev;      /* Previous resource of same type         */
    DATA flags;            /* Flags for this resource                */

    /* Resource data */
    UADDR  base;           /* Base I/O adress space address (port)     */
    UADDR  limit;          /* Limit of allocation (ports to allocate)  */
    struct CKIOregionAlloc *prevAlloc;  /* Numerically prvious region  */
    struct CKIOregionAlloc *nextAlloc;  /* Numerically next region     */
} __attribute__ ((packed)) CKIOregionAlloc;

typedef struct CKDMAchannelAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                */
    DESCRIPTOR owner;      /* Object that allocated this resource    */
    CKResource *next;      /* Next resource of kernel with same type */
    CKResource *prev;      /* Previous resource of same type         */
    DATA flags;            /* Flags for this resource                */

    /* Resource data */
    UADDR  channel;        /* DMA channel that was allocated           */
} __attribute__ ((packed)) CKDMAchannelAlloc;

typedef struct CKTrapAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                */
    DESCRIPTOR owner;      /* Object that allocated this resource    */
    CKResource *next;      /* Next resource of kernel with same type */
    CKResource *prev;      /* Previous resource of same type         */
    DATA flags;            /* Flags for this resource                */

    /* Resource data */
    UADDR  trap;           /* Interrupt that was allocated             */
    EVENT *sigThread;      /* The signal handler thead                 */
    DATA   priority;       /* Priority of the interrupt                */
    struct CKTrapAlloc *ntp; /* Next trap allocation for this trap     */
    ADDR   stub;           /* The stub generated for this trap         */
    ADDR   stubsize;       /* The size of the stub generated           */
    ADDR   retrostub;      /* Stub to activate when this is unloaded   */
} __attribute__ ((packed)) CKTrapAlloc;

typedef struct CKPhysRegionAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                */
    DESCRIPTOR owner;      /* Object that allocated this resource    */
    CKResource *next;      /* Next resource of kernel with same type */
    CKResource *prev;      /* Previous resource of same type         */
    DATA flags;            /* Flags for this resource                */

    /* Resource data */
    UADDR start;           /* Base address of this physical memory reg */
    UADDR length;          /* Length of this physregion, in bytes      */
} __attribute__ ((packed)) CKPhysRegionAlloc;

#endif

