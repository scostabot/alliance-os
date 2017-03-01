/*
 * sysdefs.h:  Systemwide defenitions, like segments and call gates
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 25/11/98  ramon       1.1    Updated for new GDT
 * 28/12/98  ramon       1.2    Added information for resource management
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

#ifndef __SYS_SYSDEFS_H
#define __SYS_SYSDEFS_H

/***********************************************************************/
/* Things in the GDT                                                   */
/***********************************************************************/

/* Selectors */

#define KCODE     0x8         /* Selector of the kernel's code segment */
#define KDATA     0x10        /* Selector of the kernel's data segment */
#define EKCODE    (0x18|1)    /* Selector of an EK's code segment      */
#define EKDATA    (0x20|1)    /* Selector of an EK's data segment      */
#define USERCODE  (0x28|3)    /* Selector of a user process's code seg */
#define USERDATA  (0x30|3)    /* Selector of a user process's data seg */
#define TSSSEL    0x38        /* Selector of the TSS                   */
#define CKCACHEOBJECTGATE       0x40   /* CK cache object system call  */
#define CKUNCACHEOBJECTGATE     0x48   /* CK uncache object syscall    */
#define CKSWITCHTHREADSTATEGATE 0x50   /* CK switch thread state       */
#define CKRESOURCEALLOCGATE     0x58   /* CK allocate resource syscall */
#define CKRESOURCEFREEGATE      0x60   /* CK free resource syscall     */

/* GDT entries */

#define KCODEDSC   1          /* GDT entry of the kernel code segment  */
#define KDATADSC   2          /* GDT entry of the kernel data segment  */
#define EKCODEDSC  3          /* GDT entry of the EK code segment      */
#define EKDATADSC  4          /* GDT entry of the EK data segment      */
#define UCODEDSC   5          /* GDT entry of the user code segment    */
#define UDATADSC   6          /* GDT entry of the user data segment    */
#define TSSDSC     7          /* GDT entry of the TSS                  */
#define CKCACHEOBJECTDSC        8      /* CK cache object system call  */
#define CKUNCACHEOBJECTDSC      9      /* CK uncache object syscall    */
#define CKSWITCHTHREADSTATEDSC 10      /* CK switch thread state       */
#define CKRESOURCEALLOCDSC     11      /* CK allocate resource syscall */
#define CKRESOURCEFREEDSC      12      /* CK free resource syscall     */
#define NOGDTENTRIES 13


/***********************************************************************/
/* The stack and flags                                                 */
/***********************************************************************/

#define KSTACKSIZE  4096      /* The kernel stack is 4k                */

/* CHANGEME:  IOPL=3, should be 1, 0x1202 */
#define STDFLAGS   0x3202     /* The standard flags; IOPL=3, ints on   */

/***********************************************************************/
/* Interrupts/Traps/Exceptions                                         */
/***********************************************************************/

#define INTERRUPTS (0x100)    /* The x86 series has 0x100 interrupts   */


/***********************************************************************/
/* DMA stuff                                                           */
/***********************************************************************/

#define HAVEDMA               /* x86 has DMA facilities                */
#define DMACHANNELS  8        /* 8 DMA channels, DMA4 is reserved      */


/***********************************************************************/
/* I/O address space stuff                                             */
/***********************************************************************/

#define HAVEIOSPC             /* x86 has a separate I/O address space  */
#define IOADDRSPC (64*1024)   /* The I/O address space is 64k          */

/***********************************************************************/
/* Memory stuff                                                        */
/***********************************************************************/

#define PAGEBLOCKSIZE 0x20000
#define NOPAGEMAPPINGS 0x4000 /* four pages with pagemappings */

/* The end */

#endif
