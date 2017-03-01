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
 * 02/05/99  jens        1.3    Added shMem callgate definitions
 * 23/07/99  ramon       1.4    Added sysinfo/bootinfo structure
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

#include "sys/memory.h"

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
#define CKSETTHREADSTATEGATE    0x50   /* CK set thread state          */
#define CKTHREADTUNNELGATE      0x58   /* CK thread tunnel syscall     */
#define CKRESOURCEALLOCGATE     0x60   /* CK allocate resource syscall */
#define CKRESOURCEFREEGATE      0x68   /* CK free resource syscall     */
#define CKLOADMAPPINGGATE       0x70   /* CK load pagemapping syscall  */
#define CKUNLOADMAPPINGGATE     0x78   /* CK unload pagemapping syscall*/
#define CKSHMEMSETRANGEGATE     0x80   /* CK shMem addPhysical syscall */
#define CKSHMEMINVITEGATE       0x88   /* CK shared mem invite syscall */
#define CKSHMEMREMAPGATE        0x90   /* CK shared mem remap syscall  */
#define CKSHMEMATTACHGATE       0x98   /* CK shared mem attach syscall */
#define CKSHMEMDETACHGATE       0xa0   /* CK shared mem detach syscall */
#define CKSHMEMNOTIFYGATE       0xa8   /* CK shared mem detach syscall */

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
#define CKSETTHREADSTATEDSC    10      /* CK set thread state syscall  */
#define CKTHREADTUNNELDSC      11      /* CK thread tunnel syscall     */
#define CKRESOURCEALLOCDSC     12      /* CK allocate resource syscall */
#define CKRESOURCEFREEDSC      13      /* CK free resource syscall     */
#define CKLOADMAPPINGDSC       14      /* CK load pagemapping syscall  */
#define CKUNLOADMAPPINGDSC     15      /* CK unload pagemapping syscall*/
#define CKSHMEMSETRANGEDSC     16      /* CK shMem addPhysical syscall */
#define CKSHMEMINVITEDSC       17      /* CK shared mem invite syscall */
#define CKSHMEMREMAPDSC        18      /* CK shared mem remap syscall  */
#define CKSHMEMATTACHDSC       19      /* CK shared mem attach syscall */
#define CKSHMEMDETACHDSC       20      /* CK shared mem detach syscall */
#define CKSHMEMNOTIFYDSC       21      /* CK shared mem detach syscall */
#define NOGDTENTRIES 22


/***********************************************************************/
/* The stack and flags                                                 */
/***********************************************************************/

#define KSTACKSIZE  8192      /* The kernel stack is 8k                */

/* CHANGEME:  IOPL=3, should be 0, 0x202 */
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

#define PAGEBLOCKBITS 17
#define PAGEBLOCKSIZE (1<<PAGEBLOCKBITS)
#define NOPAGEMAPPINGS 0x4000 /* four pages with pagemappings */


/***********************************************************************/
/* SysInfo/Boot stuff (passed by ABL in %eax)                          */
/***********************************************************************/

typedef struct __bootmodule {
    UADDR physstart;           /* Base physical address of module      */
    UADDR size;                /* Total memory size of module          */
    UADDR entry;               /* Logical entry point of module        */
    struct __bootmodule *next; /* Link to next module in list          */
} __attribute__ ((packed)) bootmodule;

typedef struct {
    /* Memory information */
    UADDR lowMemTop;    /* Highest allocated address in low memory     */
    UADDR totalMem;     /* Total amount of physical memory in system   */

    /* CK page directory and page tables */
    PTE *pagedir;       /* Base address of the page directory/tables   */
    DATA numpagetab;    /* Amount of pagetables (excluding pagedir !)  */

    /* Processor information */
    /* Put a linked list here of processors, with info on each */

    /* Loaded modules information */
    bootmodule *modules;
} __attribute__ ((packed)) bootinfo;

typedef bootinfo *sysinfo;


/* The end */

#endif
