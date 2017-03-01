/*
 * ckerror.h:  Error codes returned by CK system calls
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 19/11/98  ramon       1.0    First release
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

#ifndef __CKERROR_H
#define __CKERROR_H

#define ENOVALIDADDRSPC -1    /* Address Space Descriptor not valid */
#define ENOVALIDKERNEL  -2    /* Kernel Object Descriptor not valid */
#define EINSCHEDPARMS   -3    /* Quantum or Defer is invalid        */
#define ENOMEM          -4    /* Out of physical memory             */
#define EUNDEFOBJ       -5    /* Undefined object type              */
#define ELOCKED         -6    /* The specified object is locked     */
#define ENOLOCKPERMS    -7    /* Caller has no locking permissions  */
#define ENOSECURITY     -8    /* Caller doesn't have permissions    */
#define ENOVIRTMEM      -9    /* Virtual memory error               */
#define ESEGMENTATION  -10    /* Segmentation error / Wrong perms   */
#define ENOPHYSMEM     -11    /* Physical Memory Space invalid      */
#define ENOVALIDTHREAD -12    /* Operation on thread obj not corrct */
#define ENOSIGHANDL    -13    /* Not a valid signal handler setup   */
#define EINKRNLTHREAD  -14    /* Krnl thrd has parent krnl on load  */
#define EINRESOURCE    -15    /* Error in resource loading struct   */
#define ERESOURCEBUSY  -16    /* Resource is allocated already      */
#define ENOVALIDSHMEM  -17    /* SHMem Object Descriptor not valid  */
#define EINVALIDPARAMS -18    /* Parameters are invalid             */

#endif
