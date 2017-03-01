/*
 * kernel.c:  Harware-related parts of kernels (interrupt forwarding etc.)
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 21/12/98  ramon       1.0    First internal release
 * 23/12/98  ramon       1.1    Beginning interrupt forwarding
 * 26/12/98  ramon       1.2    Interrupt forwarding works
 * 09/01/99  ramon       1.3    Introduced noreturn optimisation
 * 15/01/99  ramon       1.4    Added signals (global forwarding)
 * 21/01/99  ramon       1.5    Major update to eliminate race conditions
 * 24/01/99  ramon       1.6    Fixed CK stack bug in signal handling
 * 29/01/99  ramon       1.7    Added global interrupt forwarding
 * 05/02/99  ramon       1.8    Fixed race condition in local fwd
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

#include <typewrappers.h>
#include "sys/sys.h"
#include "sys/sysdefs.h"
#include "sys/memory.h"
#include "sys/regs.h"
#include "sys/hwkernel.h"
#include "sys/realtime.h"
#include "sys/arith.h"
#include "sys/gdt.h"
#include "sys/8259.h"
#include "ckerror.h"
#include "ckobjects.h"
#include "cksignal.h"
#include "sched.h"
#include "memory.h"
#include "pri_heap.h"
#include "cksignal.h"
#include "ckresource.h"


/**************************************************************************/
/* General kernel functions                                               */
/**************************************************************************/

PUBLIC DATA CKpreProcessKernel(KernelObject *object, CKKernelObject *ckobject)
/*
 * Initialise a kernel
 *
 * INPUT:
 *     object:    Pointer to userspace kernel object
 *     ckobject:  Pointer to kernelspace kernel object
 *
 * RETURNS:
 *     CKpreProcessKernel():  Error code
 */
{
    /* Nothing (yet) */

    return 0;  /* Success */
}


PUBLIC VOID CKpostProcessKernel(KernelObject *object, CKKernelObject *ckobject)
/*
 * Clean up kernel
 *
 * INPUT:
 *     object:    Pointer to userspace kernel object
 *     ckobject:  Pointer to kernelspace kernel object
 *
 * RETURNS:
 *     none
 */
{
    /* Nothing (yet) */
}


/**************************************************************************/
/* Local interrupt forwarding                                             */
/**************************************************************************/

/*
 * CKforwardLocalInterrupt() is the main interrupt forwarding routine.
 * It sets up a stack frame for the kernel in question, which looks like
 * this:
 *
 *  0x3c(%esp)   %ss (pad) \
 *  0x38(%esp)   %esp       |
 *  0x34(%esp)   %eflags     >  Copy from own stack
 *  0x30(%esp)   %cs (pad)  |
 *  0x2c(%esp)   %eip      /
 *  0x28(%esp)   %ds (pad)
 *  0x24(%esp)   %eax
 *  0x20(%esp)   %edx
 *  0x1c(%esp)   %ecx
 *  0x18(%esp)   %ebp
 *  0x14(%esp)   %edi
 *  0x10(%esp)   %esi
 *  0x0c(%esp)   %ebx
 *  0x08(%esp)   calling thread descriptor
 *  0x04(%esp)   interrupt number
 *  0x00(%esp)   SIGNLINT
 *
 * Afterwards, it switches to the EK signal handler code using a feigned
 * iret.  We do this by pusing the correct stack and entry addresses on
 * the stack, and doing iret.  Don't worry about stack cleanup, all
 * information about a kernel stack is lost on interrupt return anyway.
 *
 *************************************************************************
 *
 * Benchmark:
 *
 * The following benchmark data was measured on an Intel 486 DX/2-66MHz.
 * For each case, 10 samples were taken.  Version 1.2 of kernel.c and
 * version 1.3 of exceptions.S were used for the benchmark.
 *
 * The interrupt forwarding benchmark attempts to measure the overhead
 * connected with interrupt forwarding to local kernels.  The times 
 * measured were times between the invocation of the interrupt, until
 * the entry of the C part of the interrupt handler (ie, they *do* include
 * the stub time) in the case (1) of a monolithic kernel (entry into
 * CKforwardLocalInterrupt()) and (2) of CK local interrupt forwarding
 * (entry into the AK signal handler using a minimal stub.)  Also a
 * baseline was measured, in order to account for the code that obtains 
 * the timing data, which is normally absent (I/O operations are quite
 * slow.) The following results were obtained (values in microticks):
 *
 *    Value           | Baseline       | Monolithic sys. | Local Int. Fwd.
 *  ------------------+----------------+-----------------+----------------
 *    Mean time       | 9.50 (7.97 us) | 29.6 (24.8 us)  | 56.0 (47.0 us)
 *    Standard dev.   | 0.50 (0.42 us) | 0.49 (0.41 us)  | 0.00 (0.00 us)
 *
 * Where one microtick was assumed to be equivalent to 839 ns (this is
 * approximately the case, and sufficient for this purpose.)
 * Correcting the timing values with the baseline, we get:
 *
 *    Adjusted value  | Monolithic system   | Local Interrupt Forwarding
 *  ------------------+---------------------+-----------------------------
 *    Mean time       | 20.1 (16.9 us)      | 46.5 (39.0 us)
 *    Standard dev.   | 0.49 (0.41 us)      | 0.00 (0.00 us)
 *
 * Thus, interrupt forwarding intoduces approximately 22.1 us overhead
 * into the interrupt handling process (returning from an interrupt
 * doesn't introduce any overhead in forwarded interrupts, because the
 * CK interrupt forwarder sets up the stack of the AK-level signal handler
 * in the same way the processor would do it if it trapped directly to the
 * AK.  Thus, the normal processor return-from-interrupt procedure is
 * used, which will take the same amount of time whether it returns from a
 * monolithic or from an application kernel.)
 *
 * Conclusion:
 * 22.1 us of overhead is negligible compared to the time spent on
 * processing the interrupt.  Furthermore, it is not more time than many
 * an other routine CPU task.  Therefore, we can probably safely say that
 * interrupt forwarding causes no significant delay in CK functionality
 * compared to a monolithic kernel, like UNIX.
 */

PUBLIC VOID CKforwardLocalInterrupt() __attribute__((noreturn));

PUBLIC VOID CKforwardLocalInterrupt(UWORD32 ds, struct regs regs, UWORD32 intr,
    UWORD32 eip, UWORD32 cs, UWORD32 eflags, UWORD32 *sp, UWORD32 ss)
/*
 * Perform interrupt forwarding to the parent kernel of the current thread
 *
 * INPUT:
 *     intr:   Interrupt number (padded to dword)
 *     ds:     User data segment (padded to dword)
 *     regs:   User register contents
 *     eip:    %eip of invoking user code
 *     cs:     Code segment of invoking user code
 *     eflags: Flags of invoking user code
 *     esp:    Stack pointer of invoking user code
 *     ss:     Stack segment of invoking user code
 *
 * RETURNS:
 *     never returns
 */
{
    /*
     * Though usually EGCS register allocation is great and generally
     * speeds up programs immensely, here it is a nuisance.  The way it's
     * coded now is very delicate;  I had to experiment with this for a
     * while trying out various obscure forms of syntax until EGCS produced
     * the assembly code we're looking for (and does not keep kernelsp
     * cached in a register, thus inducing race conditions with CKsignal()).
     * I hope new optimisations in future EGCS versions won't kill this
     * again.
     * Beware when changing this code; make sure you know what you're
     * doing.  We want to have the new kernelsp written back to the stack.
     * If we don't do this, race conditions with CKsignal() will make us
     * be sorry for it sooner or later.
     */

    /* Get the kernel stack pointer and convert it to linear address */
    register UWORD32 *kernelsp = (UWORD32 *) currtask->taskState.esp1;

    /* Save the current stack for later */
    register UWORD32 esp = (UWORD32) sp;

    /* Calculate the bottom of the stack frame */
    kernelsp -= 16;

    /* Avoid race condition with signal forwarding */
    (&sp)[0] = kernelsp;

    /* Now we set up our stack frame */
    kernelsp[15] = XCHG(ss,EKDATA); /* These are the stack items that */
    kernelsp[14] = esp;          /* Will be needed for safe return    */
    kernelsp[13] = eflags;       /* using iret. They are copies off   */
    kernelsp[12] = (UWORD16) cs; /* our own CK stack, where they were */
    kernelsp[11] = eip;          /* pushed by our friendly processor  */

    kernelsp[10] = (UWORD16) ds; /* This is the user data segment     */

    kernelsp[9]  = regs.eax;     /* These are the registers as they   */
    kernelsp[8]  = regs.edx;     /* were on interrupt, in SAVEREGS    */
    kernelsp[7]  = regs.ecx;     /* fashion (see include/sys/asm.h)   */
    kernelsp[6]  = regs.ebp;
    kernelsp[5]  = regs.edi;
    kernelsp[4]  = regs.esi;     /* ... perhaps push directly ?       */
    kernelsp[3]  = regs.ebx;     /* ie not copy, saves time -- Ramon  */

    kernelsp[2]  = (UWORD32) currtask;  /* Calling thread descriptor  */
    kernelsp[1]  = intr;         /* This is the interrupt number      */
    kernelsp[0]  = SIGNLINT;     /* 'Interrupt occured' signal        */

    /* Now we're ready to take off for kernelland... */
    asm volatile (
        "pushl %0  \n"   /* Push the EK stack segment                 */
        "pushl %1  \n"   /* Push the EK stack pointer                 */
        "pushl %2  \n"   /* Push the flags                            */
        "pushl %3  \n"   /* Push the EK code segment                  */
        "pushl %4  \n"   /* Push the EK signal handler entry point    */
        "iret      \n"   /* Forward interrupt to EK signal handler    */
        :
        : "p" (EKDATA), "g" (kernelsp), "p" (STDFLAGS),
          "p" (EKCODE), "g" (((struct CKKernelObject *) currtask->parentKernel)->sigHandler)
    );

    for(;;);  /* Kill an EGCS warning about our 'noreturn' optimisation */
}


/**************************************************************************/
/* Signal forwarding                                                      */
/**************************************************************************/

/*
 * CKsignal() works in a very similar way to CKforwardLocalInterrupt().
 * CKsignal() is called from the scheduler or from the 'finish signal'
 * interrupt to forward any pending signals to the signal handler of
 * the current thread's parent kernel.  Like CKforwardLocalInterrupt()
 * it sets up a stack frame for the kernel.  Unlike
 * CKforwardLocalInterrupt(), though, it is designed to take into account
 * stacked interrupts (CKforwardLocalInterrupt() only forwards software
 * interrupts, which are not stacked.  CKsignal() needs to take into
 * account that it is either invoked while a software interrupt is being
 * processed, or while another signal is being processed in case of
 * higher-priority signals.)  Also, unlike with software interrupts,
 * signals cannot directly return to the user code, but need to return
 * contol to the CK after finishing signal processing, so the CK can
 * forward any other pending signals.  The same interface is used for the
 * EK signal handler using a trick with the stack, which is documented
 * below.
 *
 * The stack frame for the kernel signal handler looks like this:
 *
 *  0x54(%esp)   %ss (pad) \
 *  0x50(%esp)   %esp       |
 *  0x4c(%esp)   %eflags     >  Copy from own stack
 *  0x48(%esp)   %cs (pad)  |
 *  0x44(%esp)   %eip      /
 *  0x40(%esp)   %ds (pad) of user code
 *  0x3c(%esp)   EOS code:  1: int $0x6a; jmp 1b;
 *  0x38(%esp)   old CK stack pointer
 *  0x34(%esp)   %eflags   of EK
 *  0x30(%esp)   %cs (pad) of EK
 *  0x2c(%esp)   0x50(%esp) [%eip of end of signal (EOS) code]
 *  0x28(%esp)   %ds (pad) of EK
 *  0x24(%esp)   %eax
 *  0x20(%esp)   %edx
 *  0x1c(%esp)   %ecx
 *  0x18(%esp)   %ebp
 *  0x14(%esp)   %edi
 *  0x10(%esp)   %esi
 *  0x0c(%esp)   %ebx
 *  0x08(%esp)   sender thread descriptor
 *  0x04(%esp)   extra (parameter) data
 *  0x00(%esp)   signal number
 *
 * The EK signal handler restores the registers like with a forwarded
 * interrupt, and then does iret.  In this case, though, we haven't pushed
 * the return address of user code on the stack.  In stead, at the
 * beginning of our stack frame, we've sneaked a few dwords of code that
 * invokes the CK EOS-call to handle any other pending signals, and then
 * does iret again.  This time, we pass to where we were before this
 * signal was invoked, which may be a previous signal being handled, or
 * user code.
 */

PUBLIC VOID CKsignal(struct regs regs, UWORD32 ds,
    UWORD32 eip, UWORD32 cs, UWORD32 eflags, UWORD32 esp, UWORD32 ss)
/*
 * Perform signal forwarding to the parent kernel of the current thread
 *
 * INPUT:
 *     regs:   User register contents
 *     ds:     User data segment (padded to dword)
 *     eip:    %eip of invoking user code
 *     cs:     Code segment of invoking user code
 *     eflags: Flags of invoking user code
 *     esp:    Stack pointer of invoking user code
 *     ss:     Stack segment of invoking user code
 *
 * RETURNS:
 *     never returns
 */
{
    register UWORD32 *kernelsp;

    /* Lock us from interruptions, so our stack frame isn't messed up */
    ((ThreadObject *)currtask->head.objectData)->interruptable++;

    /* Find the location for our stack frame */
    if((UWORD16) cs == EKCODE) {
        /*
         * We appear to be in the EK already, so we can just take the
         * EK stack pointer and start our frame there.
         */

        kernelsp = (UWORD32 *) esp;

        /* Now we set up our stack frame */
        kernelsp -= 22;

        kernelsp[21] = (UWORD16) ss; /* This is the stack pos. that     */
        kernelsp[20] = esp;          /* Will be needed for safe return  */
        kernelsp[19] = eflags;       /* These are copies off our CK     */
        kernelsp[18] = (UWORD16) cs; /* stack where they were pushed    */
        kernelsp[17] = eip;          /* by our friendly processor       */
        kernelsp[16] = ds;           /* Data segment to return to       */

        kernelsp[15] = 0xfceb6acd;   /* EOS code: 1: int $0x6a; jmp 1b; */
    } else if((UWORD16) cs == KCODE) {
        /*
         * Oops... we appear to have preempted our task just while
         * it was in the CK.  But apparently the interruptable flag
         * wasn't set, or we wouldn't get here in the first place.
         * We can preempt the CK, but we do need to figure out the
         * correct position for the signal frame on the EK stack.
         * We rely on the fact that both ways of getting into the CK,
         * using interrupts and call gates, push ss:esp at the beginning
         * of the CK stack.  Moreover, any sequential frames on the CK
         * stack can only point to previous positions in the CK, so if
         * we can get the position of the signal frame on the EK stack
         * it'll be in those first two dwords at the top of the CK stack.
         *
         * The stack frame to return to CK code looks slightly differently
         * than the normal frame, as described above.  Contrary to with
         * code in a different ring, we leave the CK return code on the CK
         * stack and increment esp0 in the TSS so it doesn't get corrupted.
         * On the EK stack we dump KDATA (more as a magic number for EOS
         * than anything else:  we are assured that this'll work as it is
         * in place of %ds, which will never be KDATA unless we're coming
         * from the kernel anyway) and the return location on the CK stack,
         * as well as the original esp0.  Using a bit of common magic EOS
         * can restore things correctly later on.
         *
         * Beware of a small hack:  we do not use &ds as stack return pos.,
         * but (&ds - 2) so that it looks to EOS just like the EOS code
         * and original esp0 were there (which are actually on the EK
         * stack, not the CK stack.)
         */

        if(*((UWORD16 *) currtask->taskState.esp0 - 2) == EKDATA)
            kernelsp = (UWORD32 *) *((UWORD32 *) currtask->taskState.esp0 - 2);
        else
            kernelsp = (UWORD32 *) currtask->taskState.esp1;

        /* Now we set up our stack frame */
        kernelsp -= 18;

        kernelsp[17] = (UADDR) &ds - 8; /* Location of CK stack return  */
        kernelsp[16] = KDATA;           /* Recognise return to kernel   */
        kernelsp[15] = 0xfceb6acd;      /* EOS: 1: int $0x6a; jmp 1b;   */

        kernelsp[14] = currtask->taskState.esp0; /* Save old CK stack   */
        currtask->taskState.esp0 = (UADDR) &ds;  /* Init new CK stack   */
    } else {
        /*
         * We're coming from user code.  We use the EK stack top pointer
         * from the TSS for our frame.
         */

        kernelsp = (UWORD32 *) currtask->taskState.esp1;

        /* Now we set up our stack frame */
        kernelsp -= 22;

        kernelsp[21] = (UWORD16) ss; /* This is the stack pos. that     */
        kernelsp[20] = esp;          /* Will be needed for safe return  */
        kernelsp[19] = eflags;       /* These are copies off our CK     */
        kernelsp[18] = (UWORD16) cs; /* stack where they were pushed    */
        kernelsp[17] = eip;          /* by our friendly processor       */
        kernelsp[16] = ds;           /* Data segment to return to       */

        kernelsp[15] = 0xfceb6acd;   /* EOS code: 1: int $0x6a; jmp 1b; */
    }

    /* This part of the stack frame is the same in all situations     */
    kernelsp[13] = STDFLAGS;                /* Flags for EOS code     */
    kernelsp[12] = EKCODE;                  /* Data sel. for EOS code */
    kernelsp[11] = (UWORD32) &kernelsp[15]; /* Location of EOS code   */
    kernelsp[10] = EKDATA;                  /* Location of EOS data   */

    kernelsp[9]  = regs.eax;     /* These are the registers as they   */
    kernelsp[8]  = regs.edx;     /* were on interrupt, in SAVEREGS    */
    kernelsp[7]  = regs.ecx;     /* fashion (see include/sys/asm.h)   */
    kernelsp[6]  = regs.ebp;
    kernelsp[5]  = regs.edi;
    kernelsp[4]  = regs.esi;     /* ... perhaps push directly ?       */
    kernelsp[3]  = regs.ebx;     /* ie not copy, saves time -- Ramon  */

    kernelsp[2]  = (UWORD32) currtask->signals->sender; /* Sender     */
    kernelsp[1]  = currtask->signals->extra;            /* Extra data */
    kernelsp[0]  = currtask->signals->signal & ~SFBUSY; /* Signal no. */

    /* Ok, our frame is ready */
    CKlock();
    ((ThreadObject *)currtask->head.objectData)->interruptable--;

    /* Now we're ready to take off for kernelland... */
    asm volatile (
        "pushl %0  \n"   /* Push the EK stack segment                 */
        "pushl %1  \n"   /* Push the EK stack pointer                 */
        "pushl %2  \n"   /* Push the flags                            */
        "pushl %3  \n"   /* Push the EK code segment                  */
        "pushl %4  \n"   /* Push the EK signal handler entry point    */
        "iret      \n"   /* Forward signal to EK signal handler       */
        :
        : "p" (EKDATA), "g" (kernelsp), "p" (STDFLAGS),
          "p" (EKCODE), "g" (((struct CKKernelObject *) currtask->parentKernel)->sigHandler)
    );
}


/*
 * CKendOfSignal() finishes off handling a signal, kicks it out of the
 * pending signal queue, and checks for any other pending signals.
 * If available, it invokes the next pending signal;  otherwise, it
 * returns to user code.
 *
 * We make the decision here what is to be done next.  The actual handling
 * is done by our magic stub (exceptions.S) according to our return value.
 */

PUBLIC ADDR CKendOfSignal(struct cregs regs, UWORD32 ds,
    UWORD32 eip, UWORD32 cs, UWORD32 eflags, UWORD32 *esp, UWORD32 ss)
/*
 * Finish signal being handled and (re-)start the next pending one
 *
 * INPUT:
 *     regs:   User register contents
 *     ds:     User data segment (padded to dword)
 *     eip:    %eip of invoking user code
 *     cs:     Code segment of invoking user code
 *     eflags: Flags of invoking user code
 *     esp:    Stack pointer of invoking user code
 *     ss:     Stack segment of invoking user code
 *
 * RETURNS:
 *     CKendOfSignal():  0 if we need to start a new signal frame,
 *                       otherwise address of previous stack frame
 */
{
    SIGNAL *tempSig;

    /* Remove the finished signal from the queue */
    if(currtask->signals && currtask->signals->signal & SFBUSY) {
        tempSig = currtask->signals;
        currtask->signals = currtask->signals->next;
        CKmemFree(tempSig,sizeof(SIGNAL));
    }

    /* Okay, time for the next signal */
    if(!currtask->signals || currtask->signals->signal & SFBUSY) {
        /* Move to the prvious location on the stack frame */
        if(esp[2] == KDATA) {
            currtask->taskState.esp0 = esp[0];
            esp = (UWORD32 *) esp[3];
        }
        (++esp)[0] = regs.eax;
        return (ADDR) esp;
    }

    /*
     * Okay, once we get here we know we need to start a new signal stack
     * frame.  We return 0 and let our magic stub do the work :)
     */

    currtask->signals->signal |= SFBUSY;
    return 0;
}


/**************************************************************************/
/* Global interrupt forwarding                                            */
/**************************************************************************/

extern union DTEntry IDT[];

extern UBYTE __fwd_global_template[];
extern UADDR __fwd_global_template_end;
extern UBYTE __fwd_global_irq_templ_bh[];
extern UADDR __fwd_globl_irq_templ_bhend;
extern UBYTE __fwd_global_irq_templ_th[];
extern UADDR __fwd_globl_irq_templ_thend;

extern UBYTE schedLock, doingRealtime;

extern CKTrapAlloc *trapHeads[];

extern VOID __spurious_int(VOID);

#define STUBLEN ((UADDR) &__fwd_global_template_end - (UADDR)__fwd_global_template)
#define MIRQLEN ((UADDR) &__fwd_globl_irq_templ_bhend - (UADDR)__fwd_global_irq_templ_bh)
#define SIRQLEN ((UADDR) &__fwd_globl_irq_templ_thend - (UADDR)__fwd_global_irq_templ_th)


PUBLIC DATA CKpreProcessTrapAlloc(ADDR trap, CKTrapAlloc *ckres, DATA flags)
/*
 * Allocate an interrupt for global forwarding
 *
 * INPUT:
 *     trap:       The trap to allocate
 *     ckres:      CK resource structure to fill in
 *     flags:      Flags for allocation
 *
 * RETURNS:
 *     0 on success, otherwise error
 */
{
    /* Check for trap validity */
    if(trap > 0xff || trap < 0x20 || trap == M_VEC || trap == M_VEC+2)
        return EINRESOURCE;

    /* Generate global interrupt forwarding stub */
    if(trapHeads[trap]) {
        /* Stub already exists, find and use existing stub */
        ckres->stub      = trapHeads[trap]->stub;
        ckres->stubsize  = trapHeads[trap]->stubsize;
        ckres->retrostub = trapHeads[trap]->retrostub;
    } else if(trap > M_VEC && trap < (S_VEC+8)) {
        /* Generate new global IRQ stub from stub template */
        UDATA  stubbyte;
        UBYTE *template = (trap >= S_VEC ? __fwd_global_irq_templ_th : __fwd_global_irq_templ_bh);
        ckres->stubsize=(trap >= S_VEC ? SIRQLEN : MIRQLEN);
        ckres->stub = (ADDR) CKmemAllocAlign(ckres->stubsize,4);

        for(stubbyte = 0; stubbyte < (trap >= S_VEC ? SIRQLEN : MIRQLEN); stubbyte++) {
            ((UBYTE *) ckres->stub)[stubbyte] = template[stubbyte];
            if(template[stubbyte] == 0xbb)
                ((UBYTE *) ckres->stub)[stubbyte] = (UBYTE) trap;
        }
        CKsetVector((VOID *) ckres->stub, trap, (D_PRESENT+D_INT+D_DPL3));
        CKenableIRQ(trap - M_VEC);
    } else {
        /* Generate new global stub from stub template */
        UDATA stubbyte;
        ckres->stubsize = STUBLEN;
        ckres->stub = (ADDR) CKmemAllocAlign(STUBLEN,4);
        for(stubbyte = 0; stubbyte < STUBLEN; stubbyte++) {
            ((UBYTE *) ckres->stub)[stubbyte] = __fwd_global_template[stubbyte];
            if(__fwd_global_template[stubbyte] == 0xbb)
                ((UBYTE *) ckres->stub)[stubbyte] = (UBYTE) trap;
        }
        ckres->retrostub = ((UWORD32) IDT[trap].gate.offset_low & ((UWORD32) IDT[trap].gate.offset_high << 16));
        CKsetVector((VOID *) ckres->stub, trap, (D_PRESENT+D_TRAP+D_DPL3));
    }

    return 0;   /* Success */
}


PUBLIC VOID CKpostProcessTrapAlloc(CKTrapAlloc *res)
/*
 * Deallocate an interrupt for global forwarding
 *
 * INPUT:
 *     res: CK resource structure to postprocess
 *
 * RETURNS:
 *     none
 */
{
    /* If we are the last allocator we discard the stub */
    if((trapHeads[res->trap] == res) && !res->ntp) {
        if(res->trap > M_VEC && res->trap < (S_VEC+8)) {
            /* Disable the IRQ */
            CKdisableIRQ(res->trap - M_VEC);
            CKsetVector(__spurious_int, res->trap, (D_PRESENT+D_TRAP+D_DPL3));
        } else {
            /* Restore local forwarding */
            CKsetVector((VOID *) res->retrostub, res->trap, (D_PRESENT+D_TRAP+D_DPL3));
        }

        /* Discard the stub */
        CKmemFree((VOID *) res->stub,res->stubsize);
    }
}


PUBLIC VOID CKforwardGlobalInterrupt(UWORD32 ds, UWORD32 intr)
/*
 * Perform interrupt forwarding for globally allocated interrupts
 *
 * INPUT:
 *     intr:   Interrupt number (padded to dword)
 *     ds:     User data segment (padded to dword)
 *
 * RETURNS:
 *     none
 */
{
    register CKTrapAlloc *allocator;

    /* Dispatch signals to all allocators of this interrupt */
    for (
        allocator  = trapHeads[intr];
        allocator != NIL;
        allocator  = allocator->ntp
    )
    {
        CKlock();
        CKsendSignal(allocator->sigThread, SIGNLINT, intr, allocator->priority);
        CKprioritise(allocator->sigThread);
        CKunlock();
    }

    /* Relinquish control to signal handlers */
    if(!doingRealtime && !schedLock)
        quantumDone();
}
