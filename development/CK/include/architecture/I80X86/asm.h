/*
 * asm.h:  defenitions for use in assembly files
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 01/01/99  ramon       1.0    First release
 * 02/01/99  ramon       1.1    Added SAVECREGS/RESTORECREGS
 * 20/01/99  ramon       1.2    Added currtask offsets
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

#ifndef __SYS_ASM_H
#define __SYS_ASM_H

/*
 * SAVEREGS and RESTOREREGS are used to push all the registers on the
 * stack
 */

#define SAVEREGS     \
    pushl %eax;      \
    pushl %edx;      \
    pushl %ecx;      \
    pushl %ebp;      \
    pushl %edi;      \
    pushl %esi;      \
    pushl %ebx;

#define RESTOREREGS  \
    popl %ebx;       \
    popl %esi;       \
    popl %edi;       \
    popl %ebp;       \
    popl %ecx;       \
    popl %edx;       \
    popl %eax;


/*
 * SAVECREGS and RESTORECREGS save and restore only those registers
 * that are used by C routines:  %eax, %ecx, %edx
 */

#define SAVECREGS    \
    pushl %eax;      \
    pushl %edx;      \
    pushl %ecx;

#define RESTORECREGS \
    popl %ecx;       \
    popl %edx;       \
    popl %eax;


/*
 * SAVEXREGS is used to store the registers that aren't stored by
 * SAVECREGS:  %ebp, %edi, %esi, and %ebx
 */

#define SAVEXREGS    \
    pushl %ebp;      \
    pushl %edi;      \
    pushl %esi;      \
    pushl %ebx;


/*
 * These are offsets into the thread object structure, used for signal
 * recognition and restarting
 */

#define CURRTASK currtask(,1)    /* Usage:  movl CURRTASK,%ebp     */
#define SIGNALS  0x40            /* Signals head                   */

#define USERTASK(ctreg) 0x8(ctreg) /* eg: movl USERTASK(%ebp),%ebp */
#define INTRPTBL 0x1c            /* Offset of interruptable entry  */


/*
 * SIGPEND is the offset into the signal structure of the signal number
 */

#define SIGPEND  0x00           /* Pending signal                      */
#define SFBUSY   0x80000000     /* Signal busy mask (from ckobjects.h) */

#endif
