/*
 * demotasks.c:  The demo tasks code
 *
 * (C) 1998 Ramon van Handel, Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author       Rev    Notes
 * 07/11/98  ramon/jens   1.0    First full internal release
 * 16/11/98  ramon        1.1    Updated to use generic ckobjects.h
 * 26/11/98  jens         1.2    Added RT external event sync task (VBL)
 * 28/11/98  ramon        1.3    Added address space object
 * 29/11/98  ramon        1.4    Added init task
 * 19/12/98  ramon        1.5    Made calc for next exec time more accurate
 * 23/12/98  ramon        1.6    Added kernel object stuff
 * 31/12/98  ramon        1.7    Added resource allocation (just demo)
 * 01/01/99  ramon        1.8    Printing now works through trapping to kernel
 * 02/01/99  ramon        1.9    Added the two FPU test tasks
 * 14/01/99  haggai       1.10   Added reset task
 * 30/01/99  ramon        1.11   Added global interrupt allocation
 * 31/01/99  haggai/ramon 1.12   DemoKernel keyboard driver
 * 04/02/99  haggai       1.13   Added keyboard buffer and first DemoShell
 * 21/02/99  jens         1.14   Implemented VBL synchronized Keyboard driver
 * 19/03/99  jens         1.15   Removed preallocated page directory
 * 15/04/99  ramon        1.16   Separated demo system into demo/ tree
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

/**************************************************************/
/* NOTE:  This file contains platform-dependant code for i386 */
/**************************************************************/

/* This file represents 'user programs', and is not done */
/* in Alliance Coding Style                              */

#include <typewrappers.h>
#include <klibs.h>
#include "sys/sys.h"
#include "sys/colors.h"
#include "sys/ioport.h"
#include "sys/realtime.h"
#include "sys/8254.h"
#include "ckobjects.h"
#include "cksyscalls.h"
#include "demo.h"


/******************************************************************/
/* Application DemoKernel system call stubs                       */
/******************************************************************/

#define print_int(nr,pos)                                              \
    asm( "int $0x80" : : "a" (0x1), "b" ((long)nr), "c" ((long)pos) );

#define print_float(nr,pos)                                            \
    asm( "int $0x80" : : "a" (0x2), "b" ((unsigned long)nr),           \
         "c" ((unsigned long)((nr-(unsigned long)nr)*10000000000ULL)), \
         "d" ((long) pos) );

#define printf(str)                                                    \
    asm( "int $0x80" : : "a" (0x3), "b" ((unsigned long)str) );

#define getch()                                                        \
    ({                                                                 \
        volatile UDATA ret;                                            \
        while(TRUE) {                                                  \
            asm( "int $0x80" : "=a" (ret) : "a" (0x4) );               \
            if(ret!=0) break;                                          \
            quantumDone();                                             \
        }                                                              \
        ret;                                                           \
    })


/******************************************************************/
/* Timesharing tasks                                              */
/******************************************************************/

ThreadObject TSTASK1;
char  TSTACK1[4096];
char  TKSTACK1[4096];

void tstask1(void) {
	const char jiggle[] = "|/-\\";
	int count, count2 = 0;

	for (count=1;;count++) {
		if(count>0x20000 && !(count % 0x1000)) {
        		((short *)0x68000000)[80*18+10] = jiggle[count2++] | 0xf00;
			count2 %= 4;
		}

		print_int(count, 80*18);
	}
}

ThreadObject TSTASK2;
char  TSTACK2[4096];
char  TKSTACK2[4096];
void tstask2(void) {
	int count;
	for (count=1;;count++) {
		print_int(count, 80*19);
		CKthreadTunnel(TSTASK3.self,tt_Conservative);
	}
}

ThreadObject TSTASK3;
char  TSTACK3[4096];
char  TKSTACK3[4096];
void tstask3(void) {
	int count;
	for (count=1;;count++) {
		print_int(count, 80*20);
		CKthreadTunnel(TSTASK2.self,tt_Conservative);
	}
}

ThreadObject TSTASK4;
char  TSTACK4[4096];
char  TKSTACK4[4096];
void tstask4(void) {
	int count;
	for (count=1;;count++)
		print_int(count, 80*21);
}


/******************************************************************/
/* Realtime tasks                                                 */
/******************************************************************/

ThreadObject RTTASK1;
char  RSTACK1[4096];
char  RKSTACK1[4096];
void rttask1(void) {
	int count;
	RTTASK1.time = RTTASK1.realtime;
	for (count=1;;count++)
        {
		print_int(count, 80*18+20);
                RTTASK1.time += 606200;
		quantumDone();
        }
}

ThreadObject RTTASK2;
char  RSTACK2[4096];
char  RKSTACK2[4096];
void rttask2(void) {
	int count;
        RTTASK2.time = RTTASK2.realtime;
	for (count=1;;count++)
        {
		print_int(count, 80*19+20);
                RTTASK2.time += 1212400;
		quantumDone();
        }
}


/******************************************************************/
/* Demo of RT task usage for screen updating in the GRSK          */
/******************************************************************/

/*
 * Notes:
 *
 * This code currently directly accesses the timer hardware; probably in
 * the future the CK should provide a function for retrieving the time
 * offset into the quantum for this kind of high-precision timing.
 *
 * The VBL miss rate depends highly on MARGIN.  Over a certain treshold
 * value of MARGIN the VBL miss rate is near-zero, while under it the VBL
 * miss rate is practically 100%.  The higher MARGIN, though, the more CPU
 * time the task will waste polling.  At system startup the optimal margin
 * should be measured using a simple loop (margin calibration.)
 *
 * For now, we use a hardcoded maximum interval value (0x5000) to check
 * for misses, but this would also be optimised using a loop in a real
 * version (interval calibration: 
 * MAXINTERV = [average interval over a few VBLs] + 2*MARGIN;
 * ... or something similar.)
 *
 * We invoke int $0x81 at every VBL so that the DemoKernel will flush its
 * keyboard buffer to the screen.  In the future this should of course be
 * implemented in a separate SK (GRSK), communicated with using
 * CORBA/ShMem.
 *
 * This technique could be used for any (external) polling event that is
 * triggered more or less periodically.
 */

#define MARGIN 0x30   /* This works on my 486DX/2-66 -- Ramon */

ThreadObject RTTASK3;
char  RSTACK3[4096];
char  RKSTACK3[4096];

static inline TIME SKgetRealtime(TIME rtime)
{
    TIME t_value;

    outportb(TMR_CTRL, TMR_LATCH+TMR_SC1);
    t_value  = inportb(TMR_CNT1);
    t_value |= (inportb(TMR_CNT1)<<8);
    t_value  = -(t_value + rtime) & 0xffff;

    return (rtime + t_value);
}

char *text =
    "This realtime task calculates its next execution time in "
    "synchronisation with the VBL of the graphics card.  This way, "
    "the text scrolls flicker-free without taking any polling time.  "
    "The GRSK could use this technique to update the screen while "
    "taking a minimum amount of CPU time for it.  Check out the source..."
    "                                     ";

void rttask3(void)
{
    char *scrpos = text;
    int a;
    TIME previousVBL, VBL;
    TIME interval = 1000;

    /* Poll for first VBL occurance */
    while((inportb(0x3da)&8)!=0);
    while((inportb(0x3da)&8)==0);
    VBL = SKgetRealtime(RTTASK3.realtime);
    while((inportb(0x3da)&8)!=0);

    for(;;) {
        /* Poll for VBL */
        previousVBL = VBL;
        while((inportb(0x3da)&8)==0);
        VBL = SKgetRealtime(RTTASK3.realtime);

        /* Let the DemoKernel flush the keyboard buffer to screen */
	asm("int $0x81");

        /* Do scroller */
        for(a=0; a<79; a++)
            ((UWORD16 *)(0x60000000))[a+80*23]=((UWORD16 *)(0x60000000))[a+80*23+1];
        if(scrpos[0]==0) scrpos = text;
        ((UBYTE *)(0x60000000))[160*23+79*2] = *scrpos++;
        ((UBYTE *)(0x60000000))[160*23+79*2+1] = 0xf;

        /* Calculate next execution time */
        interval = VBL - previousVBL;
        if(interval > 0x5000) {    /* Missed a VBL, don't miss again */
            interval = 0x0000;
        }
        RTTASK3.time = VBL + interval - MARGIN;

        /* Quantum done */
        quantumDone();
    }
}


/******************************************************************/
/* FPU test tasks                                                 */
/******************************************************************/

/*
 * In order to test the FPU, I had to think of a problem that requires
 * the FPU in order to work. In the end, I decided to make two tasks,
 * one that calculates pi and one 2*pi in as many decimals as fit in a
 * long double.
 *
 * There are many ways to calculate pi, as pi is a constant that
 * constantly pops up in mathematics in some of the most unpredictable
 * places.  Usually, pi is expressed as an infinite sum.  Some of these
 * sums converge to pi faster than others do.  As I don't have a sum
 * table handy, I used the method that first came to mind.
 *
 * Probably one of the easiest way to find a sum for pi is to develop a
 * fourier series for some simple function.  After a bit of trial and
 * error, I came up with the function:
 *
 *       /   1  ^  x in (0,pi)
 *      |  
 * y = <     0  ^  x = 0
 *      |
 *       \  -1  ^  x in (-pi,0)
 *
 * with period 2*pi.  This is an odd function, so developing the fourier
 * sine-series for this function is straightforward and relatively simple:
 *        
 * F(y) = (2 * SUM ( (((-1)^(k-1) + 1) * sin(kx)) / k, k = 1 -> inf )) / pi;
 *
 * We pick a convenient value for x, say pi/2.  y(pi/2) = 1 thus F(y) = 1.
 * sin(k*pi/2) disappears when k is even, so substituting k = 2n-1 and
 * sin(k*pi/2) = (-1)^(n-1) for all n we get:
 *
 * 1    = (2 * SUM ( (2 * (-1)^(n-1)) / (2n-1), n = 1 -> inf )) / pi;
 *
 * which can be rewritten to:
 *
 * pi   = SUM ( (4 * (-1)^(n-1) / (2n-1)), n = 1 -> inf );
 *
 * We use this sum to calculate pi in our FPU demotasks.
 *
 * Apparently, this sum converges only very slowly to pi.  This is
 * probably a good thing, otherwise we wouldn't even notice the computer
 * is calculating something :).  The sum alternates (it contains a (-1)^q
 * term) so things are bound to look a bit weird on the screen.  Because
 * of that, I only print half of the calculated terms to the screen;  that
 * way, it seems like the digits only decrement, which is more pleasant to
 * the eye.
 */

ThreadObject FPTASK1;
char  FSTACK1[4096];
char  FKSTACK1[4096];
void fptask1(void) {
        long double pi   =  0;
        long double sign = -1;
        unsigned long long  k;

        for(k=1;;k++) {
                sign *= -1;
                pi += (8*sign)/(2*k-1);
                if(k % 2) print_float(pi, 80*18+40);
        }
}

ThreadObject FPTASK2;
char  FSTACK2[4096];
char  FKSTACK2[4096];
void fptask2(void) {
        long double pi   =  0;
        long double sign = -1;
        unsigned long long  k;

        for(k=1;;k++) {
                sign *= -1;
                pi += (4*sign)/(2*k-1);
                if(k % 2) print_float(pi, 80*19+40);
        }
}


/**************************************************************************/
/* Alliance DemoShell                                                     */
/**************************************************************************/

ThreadObject DSTASK;
char DSSTACK[4096];
char DSKSTACK[4096];

#define CLSIZE 256

#define reboot() asm (  "1:  in $0x64,%%al      \n"    \
                        "    testb $0x2,%%al    \n"    \
                        "    jnz 1b             \n"    \
                        "    movb $0xfe,%%al    \n"    \
                        "    outb %%al,$0x64    \n"    \
                        ::: "al")

/* execute command from command line */

VOID docommand(STRING *commandLine)
{
    STRING* firstWord = commandLine; /* First word on the command line is the command */
    UDATA i = 0;                     /* index for the ending of the first word */

    /* trim spaces on the left */
    while (*firstWord == ' ' && firstWord < commandLine + CLSIZE)
        firstWord++;

    /* find ending of the first word */
    while ((firstWord[i] != 0 && firstWord[i] != ' ') && (firstWord + i < commandLine + CLSIZE))
        i++;
        firstWord[i] = 0;

        /* Which command is this ? */
        if (KLstringCompare(firstWord,"help") == 0)
            print("Available Commands:\n"
                   "about	- shows info about this kernel\n"
                   "help	- shows this screen\n"
                   "reboot	- reboots the computer\n" );
        else if (KLstringCompare(firstWord,"reboot") == 0) {
            print("Rebooting...\n");
            for (;;)
                reboot();
        }
        else if (KLstringCompare(firstWord,"about") == 0) {
            /* Separate prints so print() won't go crazy */
            print("This is the demo system running on top of the Alliance CK.\n");
            print("The CK is currently being developed.  In order to test CK\n");
            print("functionality, we have coded some tasks to run on top of it.\n");
            print("When the CK is finished, these things will be removed.\n\n");
            print("The screen is split into two parts:  In the top part, text\n");
            print("and kernel messages will appear.  At the bottom, some demotasks\n");
            print("are running.  All tasks run on top of the DemoKernel, a tiny EK\n");
            print("that supports a few system calls and has a simplistic keyboard\n");
            print("driver.\n\n");
            print("The Alliance Cache Kernel is (c) 1998,1999 the Alliance Operating System team\n");
            print("All code is distributed under the terms of the GNU General Public License\n");
            print("See the file LICENSE for more details.\n");
        }
        else if (KLstringCompare(firstWord,"") == 0)
            print("\b");
        else
            print("'%s': Command Unknown.\n", firstWord);
}

/* DemoShell thread */

void demoshell(void) {
    UDATA key;
    STRING commandLine [CLSIZE+1];
    UDATA curChar;
    BOOLEAN gotbeep=FALSE;             /* kill annoying multi beep            */

    setTextAttr(T_FG_RED + T_BOLD);

    print("\nAlliance Demo Shell Version 1.0\n");
    print("Type 'help' to see a list of commands.");

    while (TRUE) {                     /* loop forever                        */
        curChar = 0;
        print("\n# ");                 /* print the prompt                    */
        key = getch();                 /* get the first char                  */
        while (key != '\r') {          /* read the line until we get an enter */
            switch (key) {
                case 8:                /* found a backspace                   */
                    if (curChar > 0) { /* are there characters on the line?   */
                        curChar--;
                        print("\b");
			gotbeep=FALSE;
                    } else {
			if(!gotbeep) {
			    gotbeep=TRUE;
#if 0
                    	    print("\a");         /* beep                      */
#endif
			}
                    }
		    break;
                default:	                 /* normal characer           */
                    if (curChar >= CLSIZE) {     /* no room in buffer         */
			if(!gotbeep) {
			    gotbeep=TRUE;
#if 0
                    	    print("\a");         /* beep                      */
#endif
			}
                        break;
                    }
		    gotbeep=FALSE;
                    print("%c",key);
                    commandLine[curChar] = key;
                    curChar++;
                    break;
            }
            key = getch();
        }
        print("\n");
        commandLine[curChar] = 0;
        docommand(commandLine);    /* execute the command */
    }
}
