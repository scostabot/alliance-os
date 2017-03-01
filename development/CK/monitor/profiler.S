/*
 * profiler.S:  Assembly routines for the GNU profiler support code
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 03/02/99  ramon       1.0    First release
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

.text

/*
 * Copyright (c) 1998 University of Utah and the Flux Group.
 * All rights reserved.
 * 
 * This file is part of the Flux OSKit.  The OSKit is free software, also known
 * as "open source;" you can redistribute it and/or modify it under the terms
 * of the GNU General Public License (GPL), version 2, as published by the Free
 * Software Foundation (FSF).  To explore alternate licensing terms, contact
 * the University of Utah at csl-dist@cs.utah.edu or +1-801-585-3271.
 * 
 * The OSKit is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GPL for more details.  You should have
 * received a copy of the GPL along with the OSKit; see the file COPYING.  If
 * not, write to the FSF, 59 Temple Place #330, Boston, MA 02111-1307, USA.
 */

/*
 * Copyright (c) 1996 Bruce D. Evans.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      $\Id: prof_machdep.c,v 1.2 1996/04/08 16:41:06 wollman Exp $
 */


#include "sys/asm.h"


/*
 * mcount() is called on every procedure entry.  Its job is to call the
 * C language _mcount(), which maintains the profiler call graph.
 *
 * There are two versions of this routine:  mcount and __mcount.  The
 * __mcount version would be called directly after procedure entry;
 * mcount would be called if the caller stack frame has been set up
 * already.
 *
 * We have to get extract parameters from the stack:  frompc (in %edx) and
 * selfpc (in %eax).  See profiler.c for more information.
 */

.globl mcount
.type mcount,@function
.align 16,0x90
mcount:                   /* mcount, stack frame version              */
    cmpl $0, _gmonparam   /* Check whether we're ready for profiling  */
    jne 2f                /* If not, just return                      */

    SAVECREGS             /* Save the user registers                  */
    movl 4(%ebp), %edx    /* Fetch the caller return address          */
    jmp 1f                /* Skip direct version                      */

.globl __mcount
.type __mcount,@function
.align 16,0x90
__mcount:                 /* __mcount, direct version                 */
    cmpl $0, _gmonparam   /* Check whether we're ready for profiling  */
    jne 2f                /* If not, just return                      */

    SAVECREGS             /* Save the user registers                  */
    movl 16(%esp), %edx   /* Fetch the caller return address          */

1:
    movl 12(%esp), %eax   /* Fetch the caller's address               */

    pushf                 /* Save the flags                           */
    pushl %eax            /* Save the caller address                  */
    pushl %edx            /* Save the caller return address           */

    call _mcount          /* Update call graph                        */

    addl $0x8,%esp        /* Remove the parameters                    */
    popf                  /* Restore the flags                        */
    RESTORECREGS          /* Restore the user registers               */

2:
    ret                   /* Return to caller                         */



/*
 * __profil() is the stub that invokes the profiler histogram data
 * collection code in profiler.c, CKprofilSampler().
 */

.globl __profil
.type __profil,@function
.align 16,0x90
__profil:                 /* Histogram collection stub entry point    */
    SAVECREGS             /* Save the user registers                  */
    pushw %ds             /* Save the user data segment               */
    pushw %ss             /* Switch to kernel data segement           */
    popw %ds              /* (%ss is always valid, ss0 in TSS)        */
    cld                   /* GCC likes this                           */
    call CKprofilSampler  /* Update profiling information (histogram) */
    popw %ds              /* Restore user data segment                */
    movb $0x20,%al        /* Send EOI to slave PIC                    */
    outb %al,$0xa0
    jmp 1f                /* Short delay to give the PIC time to      */
1:  jmp 1f                /* process the EOI                          */
1:  outb %al,$0x20        /* Send EOI to master PIC                   */
    RESTORECREGS          /* Restore the user registers               */
    iret                  /* Return from interrupt                    */


