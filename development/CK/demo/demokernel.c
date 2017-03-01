/*
 * demokernel.c:  The demo mini-kernel signal handler & drivers
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

#define __DEMOKERNEL_C

/**************************************************************/
/* NOTE:  This file contains platform-dependant code for i386 */
/**************************************************************/

/* This file represents 'user programs', and is not done */
/* in Alliance Coding Style                              */

#include <typewrappers.h>
#include "sys/sys.h"
#include "sys/ioport.h"
#include "sys/regs.h"
#include "cksignal.h"
#include "demo.h"
#include "cksyscalls.h"


/******************************************************************/
/* Helper functions                                               */
/******************************************************************/

static inline void ak_print(int nr,int pos)
{
   unsigned char o,i;
   for(i=0;i<8;i++)
   {
      o=nr&0xf;
      if(o>9) o+=7;
      o+='0';
      ((short *)(0x60000000))[(7-i)+pos]=(o|(((0<<4)|0xf)<<8));
      nr>>=4;
   }
}

static inline void ak_print_dec_float(unsigned long nr, unsigned long flt, int pos)
{
   unsigned char temp[21];
   unsigned int i = 20, j;

   do {
      temp[i--] = flt % 10 + '0';
      flt /= 10;
   } while(i>10);

   temp[i--] = '.';

   do {
      temp[i--] = nr % 10 + '0';
      nr /= 10;
   } while(nr);

   for(j=++i;i<21;i++)
      ((short *)(0x60000000))[i-j+pos] = (temp[i]|(((0<<4)|0xf)<<8));
}


/******************************************************************/
/* DemoKernel keyboard driver                                     */
/******************************************************************/

#define KEYBUFFERSIZE 32 /* Use x^2 numbers, use of & faster than if */
#define KEYREPEATWAIT 15
#define KEYREPEATRATE 1

UDATA keybTop = 0, keybBottom = 0;
UDATA keybCurrent = 0;
UDATA keybRepeatTime=0;
UDATA keyboardBuffer[KEYBUFFERSIZE];

UDATA AKkeybTop = 0, AKkeybBottom = 0;
UDATA AKkeybBuffer[KEYBUFFERSIZE];

UBYTE lastkey;

#define reboot() asm (  "1:  in $0x64,%%al      \n"    \
                        "    testb $0x2,%%al    \n"    \
                        "    jnz 1b             \n"    \
                        "    movb $0xfe,%%al    \n"    \
                        "    outb %%al,$0x64    \n"    \
			::: "al")

VOID AKkeybDriver(VOID)
{
    UBYTE c,status;
    c = inportb(0x60);

    /* handshake with keyboard */
    status = inportb(0x61);
    outportb(0x61,status|0x80);
    outportb(0x61,status);

    if(AKkeybTop!=((AKkeybBottom-1)&(KEYBUFFERSIZE-1))) {
	AKkeybBuffer[AKkeybTop++]=c;
	AKkeybTop&=KEYBUFFERSIZE-1;
    }
}

BOOLEAN AKkeybInsertKey(UBYTE c)
{
    UDATA key;
    static BOOLEAN ctrl = FALSE, alt = FALSE, shift = FALSE;
    switch (c) {
        case 29:        /* Control down */
            ctrl = TRUE;
            return TRUE;
        case 157:       /* Control up */
            ctrl = FALSE;
            return TRUE;
        case 56:        /* Alt down */
            alt = TRUE;
            return TRUE;
        case 184:       /* Alt up */
            alt = FALSE;
            return TRUE;
        case 0x2a:      /* Shift down */
        case 0x36:      /* Shift down */
        case 0xaa:      /* Shift up */
        case 0xb6:      /* Shift up */
            shift ^= TRUE;
            return TRUE;
        case 83:        /* delete down */
            if (alt && ctrl)
                for (;;)
		    reboot();
            /* fallthrough */
        default:
    }

    key = (shift == TRUE ? scan2ascii[1][c] : scan2ascii[0][c]);

    if(!key) return TRUE;

    if(key & SPECIALMASK) {
        switch(key) {
            case CAPS:
                shift ^= TRUE;
                return TRUE;
        }
    } else {
        if (keybTop == ((keybBottom -1)&(KEYBUFFERSIZE-1))) return FALSE;
        keyboardBuffer[keybTop] = key;
	keybTop++;
	keybTop&=KEYBUFFERSIZE-1;
    }
    return TRUE;
}

VOID AKkeybDoRepeat(VOID)
{
    UBYTE c;
    while(AKkeybTop!=AKkeybBottom) {
	c=AKkeybBuffer[AKkeybBottom++];
	AKkeybBottom&=KEYBUFFERSIZE-1;
	if(c==lastkey) continue; /* filter out hardware repeat keys */
	lastkey=c;
	keybRepeatTime=KEYREPEATWAIT;
	if(AKkeybInsertKey(lastkey)) return;
    }
    if((lastkey&0x80)==0) {	/* generate software repeat keys */
	if(keybRepeatTime<=0) {
	    AKkeybInsertKey(lastkey);
	    keybRepeatTime=KEYREPEATRATE;
	}
	keybRepeatTime--;
    }
}


/******************************************************************/
/* Application kernel system call handler                         */
/******************************************************************/

static inline long AKsystemCall(long function, long parm1, long parm2, long parm3, struct regs *regs)
{
    switch(function) {
        case 0x1:
            ak_print(parm1, parm2);
            break;
        case 0x2:
            ak_print_dec_float(parm1, parm2, parm3);
            break;
        case 0x3:
            print((STRING*) parm1);
            break;
        case 0x4: {
            UDATA key;
            if(keybTop==keybBottom) {
                regs->eax=0;
                break;
            }
            key = keyboardBuffer[keybBottom];
            keybBottom++;
            keybBottom&=KEYBUFFERSIZE-1;
            regs->eax = key;
            break;
        }
        default:
            return 0xdeadbeef;
    }

    return 0;
}


/******************************************************************/
/* Application kernel signal handler stub                         */
/******************************************************************/

asm (
    ".globl AKsigHandlStub     \n"   /* The EK signal handler stub */
    "AKsigHandlStub:           \n"
    "    pushw %ss             \n"   /* Switch to EK data segment  */
    "    popw %ds              \n"   /* %ss is always valid (ss1)  */
    "    cld                   \n"   /* GCC likes this             */
    "    call AKsigHandler     \n"   /* Handle the signal          */
    "    addl $0xc,%esp        \n"   /* Remove the parameters      */
    "    popl %ebx             \n"   /* Restore the user registers */
    "    popl %esi             \n"
    "    popl %edi             \n"
    "    popl %ebp             \n"
    "    popl %ecx             \n"
    "    popl %edx             \n"
    "    popl %eax             \n"
    "    popw %ds              \n"   /* Restore the  user data seg */
    "    addl $0x2,%esp        \n"   /* ... and remove padding     */
    "    iret                  \n"   /* Return from interrupt      */
);


/******************************************************************/
/* Application kernel signal handler                              */
/******************************************************************/

long signalCount = 0;

VOID AKsigHandler(long signal, long extra, long caller, struct regs regs)
{
    switch(signal) {
        case SIGNLINT:
            switch(extra) {
                case 0x81:
                    AKkeybDoRepeat();
                    break;
                case 0x80:
                    if(AKsystemCall(regs.eax, regs.ebx, regs.ecx, regs.edx, &regs)) {
                        signalCount++;
                        print("DemoKernel: [%d] Received signal 0x%x from thread 0x%x\n", signalCount, signal, caller);
                        print("DemoKernel: [%d] Interrupt signal for int $0x%x (unidentified system call 0x%x)\n", signalCount, extra, regs.eax);
                    }
                    break;
                case 0x69:
                    AKkeybDriver();
                    break;
                default:
                    signalCount++;
                    print("DemoKernel: [%d] Received signal 0x%x from thread 0x%x\n", signalCount, signal, caller);
                    print("DemoKernel: [%d] Interrupt signal for int $0x%x\n", signalCount, extra);
                    break;
            }
            break;
        case SIGNLSHINVI:
            print("DemoKernel: [%d] Received SHinvite Signal from thread 0x%x\n", signalCount, caller);
            print("trying to attach to shmemobj %x...",extra);
            CKshMemAttach((DESCRIPTOR)extra,0x68000000,0x2000);
            print("ok\n");
            break;
        case SIGNLSHRMAP:
            print("DemoKernel: [%d] Received SHremap Signal from thread 0x%x\n", signalCount, caller);
            break;
        case SIGNLSHDEAD:
            print("DemoKernel: [%d] Received SHdead Signal from thread 0x%x\n", signalCount, caller);
            print("trying to detach from shmemobj %x...",extra);
            CKshMemDetach((DESCRIPTOR)extra);
            print("ok\n");
            break;
        case SIGNLSHNOTIFY:
            print("DemoKernel: [%d] Received SHnotify Signal from thread 0x%x\n", signalCount, caller);
            break;
        default:
            signalCount++;
            print("DemoKernel: [%d] Received signal 0x%x from thread 0x%x\n", signalCount, signal, caller);
            print("DemoKernel: [%d] Unidentified signal with value 0x%x. Ignoring...\n",extra);
            break;
    }
}

#undef __DEMOKERNEL_C
