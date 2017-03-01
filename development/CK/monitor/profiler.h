/*
 * profiler.h:  Data structures for the GNU profiler support code
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

/*-
 * Copyright (c) 1982, 1986, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)gmon.h	8.2 (Berkeley) 1/4/94
 */

#ifndef	__SYS_GMON_H
#define	__SYS_GMON_H

#include <typewrappers.h>


/*
 * histogram counters are unsigned shorts (according to the kernel).
 */

#define	HISTCOUNTER  UHALF


/*
 * fraction of text space to allocate for histogram counters here, 1/2
 */

#define	HISTFRACTION  2


/*
 * Fraction of text space to allocate for from hash buckets.
 * The value of HASHFRACTION is based on the minimum number of bytes
 * of separation between two subroutine call points in the object code.
 * Given MIN_SUBR_SEPARATION bytes of separation the value of
 * HASHFRACTION is calculated as:
 *
 *	HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
 *
 * For example, on the VAX, the shortest two call sequence is:
 *
 *	calls	$0,(r0)
 *	calls	$0,(r0)
 *
 * which is separated by only three bytes, thus HASHFRACTION is
 * calculated as:
 *
 *	HASHFRACTION = 3 / (2 * 2 - 1) = 1
 *
 * Note that the division above rounds down, thus if MIN_SUBR_FRACTION
 * is less than three, this algorithm will not work!
 *
 * In practice, however, call instructions are rarely at a minimal
 * distance.  Hence, we will define HASHFRACTION to be 2 across all
 * architectures.  This saves a reasonable amount of space for
 * profiling data structures without (in practice) sacrificing
 * any granularity.
 */

#define	HASHFRACTION  2


/*
 * percent of text space to allocate for tostructs with a minimum.
 */

#define ARCDENSITY   2
#define MINARCS      50
#define MAXARCS      ((1 << (8 * sizeof(HISTCOUNTER))) - 2)

struct tostruct {
    UADDR selfpc;
    UDATA count;
    UHALF link;
    UHALF pad;
};


/*
 * a raw arc, with pointers to the calling site and
 * the called site and a count.
 */

struct rawarc {
    UADDR raw_frompc;
    UADDR raw_selfpc;
    DATA  raw_count;
};


/*
 * general rounding functions.
 */

#define ROUNDDOWN(x,y)	(((x)/(y))*(y))
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))


/*
 * The profiling data structures are housed in this structure.
 */

struct gmonparam {
    DATA             state;
    UHALF            *kcount;
    UDATA            kcountsize;
    UHALF            *froms;
    UADDR            fromssize;
    struct tostruct  *tos;
    UADDR            tossize;
    DATA             tolimit;
    UADDR            lowpc;
    UADDR            highpc;
    UADDR            textsize;
    UADDR            hashfraction;
    ADDR             log_hashfraction;
};

extern struct gmonparam _gmonparam;


/*
 * Possible states of profiling.
 */

#define	GMON_PROF_ON	0
#define	GMON_PROF_BUSY	1
#define	GMON_PROF_ERROR	2
#define	GMON_PROF_OFF	3


/*
 * Now we specify the format of the gmon output file.
 *
 * A gmon.out file consists of a header (defined by gmonhdr) followed by
 * a sequence of records.  Each record starts with a one-byte tag
 * identifying the type of records, followed by records specific data.
 */

#define	GMON_MAGIC	"gmon"	/* magic cookie   */
#define GMON_VERSION	1	/* version number */


/*
 * Raw header as it appears on file (without padding).  This header
 * always comes first in gmon.out and is then followed by a series
 * records defined below.
 */

struct gmonhdr {
    BYTE cookie[4];
    BYTE version[4];
    BYTE spare[3 * 4];
};


/* types of records in this file: */

typedef enum {
    GMON_TAG_TIME_HIST = 0,
    GMON_TAG_CG_ARC    = 1,
    GMON_TAG_BB_COUNT  = 2
} GMON_Record_Tag;

struct gmon_hist_hdr {
    BYTE low_pc[sizeof(BYTE *)];   /* base pc address of sample buffer */
    BYTE high_pc[sizeof(BYTE *)];  /* max pc address of sampled buffer */
    BYTE hist_size[4];             /* size of sample buffer            */
    BYTE prof_rate[4];             /* profiling clock rate             */
    BYTE dimen[15];                /* phys. dim., usually "seconds"    */
    BYTE dimen_abbrev;             /* usually 's' for "seconds"        */
};

struct gmon_cg_arc_record {
    BYTE from_pc[sizeof(BYTE *)];  /* address within caller's body     */
    BYTE self_pc[sizeof(BYTE *)];  /* address within callee's body     */
    BYTE count[4];                 /* number of arc traversals         */
};


/* Predeclarations */

VOID CKprofilerStartup(UADDR lowpc, UADDR highpc);
VOID CKprofilerCleanup(VOID);
VOID CKprofilerControl(DATA mode);

#endif
