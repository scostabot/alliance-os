/*
 * taskstate.h   structure for task state segments
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 01/01/99  ramon       1.1    TSS now contains i387 FPU state
 * 25/07/99  ramon       1.2    Removed reserved, now auto set 0 by CK
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

#ifndef __SYS_TASKSTATE_H
#define __SYS_TASKSTATE_H

struct i387reg {
    UWORD64 significand;
    UWORD16 exponent;
} __attribute__ ((packed));

struct i387context {
    UWORD32 cwd;                  /* The FPU control word register  */
    UWORD32 swd;                  /* The FPU status word register   */
    UWORD32 twd;                  /* The FPU tag word register      */
    UWORD32 fip;                  /* The FPU instruction pointer    */
    UWORD32 fcs;                  /* The FPU code selector          */
    UWORD32 foo;                  /* The operand offset (data ptr)  */
    UWORD32 fos;                  /* The operand selector (dataseg) */
    struct i387reg fpregs[8];     /* The eight 80-bit FP-regs       */
} __attribute__ ((packed));

struct i387 {
    struct i387context context;
    DATA used;
} __attribute__ ((packed));

struct TSS {
    UWORD32 back;                 /* Backlink                       */
    UWORD32 esp0;                 /* The CK stack pointer           */
    UWORD32 ss0;                  /* The CK stack selector          */
    UWORD32 esp1;                 /* The parent EK stack pointer    */
    UWORD32 ss1;                  /* The parent EK stack selector   */
    UWORD32 esp2;                 /* Unused                         */
    UWORD32 ss2;                  /* Unused                         */
    UWORD32 cr3;                  /* The page directory pointer     */
    UWORD32 eip;                  /* The instruction pointer        */
    UWORD32 eflags;               /* The flags                      */
    UWORD32 eax, ecx, edx, ebx;   /* The general purpose registers  */
    UWORD32 esp, ebp, esi, edi;   /* The special purpose registers  */
    UWORD32 es;                   /* The extra selector             */
    UWORD32 cs;                   /* The code selector              */
    UWORD32 ss;                   /* The application stack selector */
    UWORD32 ds;                   /* The data selector              */
    UWORD32 fs;                   /* And another extra selector     */
    UWORD32 gs;                   /* ... and another one            */
    UWORD32 ldt;                  /* The local descriptor table     */
    UWORD16 trap;                 /* The trap flag (for debugging)  */
    UWORD16 io;                   /* The I/O Map base address       */
    struct i387 i387;             /* The i387 FPU state             */
} __attribute__ ((packed));

typedef struct TSS TaskState;
typedef struct TSS UserState;

#endif
