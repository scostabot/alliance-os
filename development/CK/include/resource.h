/*
 * resource.h:  Defenitions for resource management
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 27/12/98  ramon       1.0    First release
 * 01/08/99  ramon       1.0    Now split into user/kernel part
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

#ifndef __RESOURCE_H
#define __RESOURCE_H

#include <typewrappers.h>
#include "ckobjects.h"
#include "ckresource.h"

/* struct Resource is used for allocating resources */
typedef struct Resource {
    DATA typeSignature;    /* Resource type                          */
    DATA resource[0];      /* The resource data itself               */
} __attribute__ ((packed)) Resource;


/* These are the expanded resources */

typedef struct IOregionAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                  */

    /* Resource data */
    UADDR  base;           /* Base I/O adress space address (port)     */
    UADDR  limit;          /* Limit of allocation (ports to allocate)  */
} __attribute__ ((packed)) IOregionAlloc;

typedef struct DMAchannelAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                  */

    /* Resource data */
    UADDR  channel;        /* DMA channel that was allocated           */
} __attribute__ ((packed)) DMAchannelAlloc;

typedef struct TrapAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                  */

    /* Resource data */
    UADDR  trap;           /* Interrupt that was allocated             */
    EVENT *sigThread;      /* The signal handler thead                 */
    DATA   priority;       /* Priority of the interrupt                */
} __attribute__ ((packed)) TrapAlloc;

typedef struct PhysRegionAlloc {
    /* Resource header */
    DATA typeSignature;    /* Resource type signature                  */

    /* Resource data */
    UADDR start;           /* Base address of this physical memory reg */
    UADDR length;          /* Length of this physregion, in bytes      */
} __attribute__ ((packed)) PhysRegionAlloc;

#endif

