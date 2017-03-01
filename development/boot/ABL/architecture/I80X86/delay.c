/*
 * delay.c:  Delay calibration, the Linux way
 *
 * NOTE:  Based on the method used in linux 2.0.30 init/main.c
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 19/12/98  ramon       1.0    First release
 * 25/07/99  ramon       1.1    Inlining fixes; timer fixes
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
 */

#include <typewrappers.h>                /* Include files               */
#include "sys/ioport.h"
#include "sys/8259.h"
#include "sys/8254.h"
#include "sys/gdt.h"
#include "decl.h"

extern VOID ABLdelayISR(VOID);           /* Timer ISR for calibration   */

/**************************************************************************/

/*
 * Goal:
 * The goal is to create a reasonably accurate version of the delay()
 * function, which creates a delay in multiples of one microsecond.
 *
 * Method:
 * A simple loop would delay the computer wonderfully.  Ie, take something
 * like:
 *
 *         for(i=0;i<BIGNUMBER;i++);  // Delay loop
 *
 * That kills time wonderfully.  All we need to do is to find out what
 * BIGNUMBER we need to use to delay one microsecond.  Finding the correct
 * BIGNUMBER for your machine is called delay loop calibration.
 *
 * We can calibrate the delay loop using the PIT.  In the initialisation
 * phase of the OS, when the PIT is not yet hooked to the scheduler, it
 * can be freely used to do this.  When the delay loop has been calibrated
 * against the PIT, the PIT is free to be used for other purposes.
 *
 * There are many ways to calibrate a delay loop with the PIT.  This is
 * one way (used in Linux.)  More documentation in the code.
 */

/**************************************************************************/

/*
 * The timer ISR:
 * In order to calibrate the delay loop, we program the PIT channel 0 with
 * a fixed timer rate of MILLISEC milliseconds.  The only thing the ISR
 * does is increment a timer, ticks, on every interrupt (that is, every
 * 10ms in this case.)  The main program will use this counter in order to
 * calibrate the delay loop.
 */

LOCAL volatile UDATA ticks = 0;

VOID delayCalibInt(VOID)      /* The timer ISR                  */
{
    ticks++;                  /* Increment the ticks counter    */
}

/**************************************************************************/

/*
 * Calibrate the delay loop:
 * This is the most important part of the whole business.  I'll explain
 * the general strategy here, which is supplemented by comments in the
 * code.
 *
 * Stage 1:
 * First, we are going to make a rough approximation of delayCount. We
 * are going to try to determine between which two powers of 2 delayCount
 * needs to be.  In order to do that, we first wait until the next timer
 * tick has started (we know it has when the ticks variable has just been
 * incremented.)  Then we start a delay with delayCount of 1.  When the
 * delay has finished, we look whether a timer tick has passed during the
 * delay (nah !).  If not, we shift delayCount to the right once and try
 * again.  We continue until after the delay, one timer tick has passed.
 * We then know that our value for delayCount is too big, so the 'real'
 * delayCount is between (delayCount) and (delayCount>>1).
 *
 * Stage 2:
 * Afterwards, we start refining our value.  We shift our delayCount to
 * the right once to obtain out bottom-value for delayCount (ie we then
 * know that the 'real' delayCount is bigger than the actual
 * delayCount.)  delayCount is a power of 2, so only one bit is set.
 * What we do is, we start setting bits in delayCount and see whether
 * delayCount is bigger than a tick afterwards. An example helps
 * clarifying matters:
 *
 * Imagine we found in stage 1 that delayCount is between 00100000b and
 * 01000000b.  (In reality it would be much bigger, but for the example
 * this will do.)  Before we start our fine-calibration, we set
 * delayCount to 00100000b.
 * Now we turn on the bit after the set bit, ie:  00110000b
 * We do the delay like we did previously, and see whether ticks has
 * changed during it.  If it has, we know that the value is too big, and
 * turn the bit back off.  If it hasn't, we leave it on.  In either case,
 * we proceed with the next bit:
 *
 *     00100000b   <--- start value
 *     00110000b   <--- tick hasn't passed
 *     00111000b   <--- tick has passed, turn it off: 00110000b
 *     00110100b   <--- tick has passed, turn it off: 00110000b
 *     00110010b   <--- tick hasn't passed
 *     00110011b   <--- tick hasn't passed
 *
 * And there is our delayCount value.
 *
 * In order to save time (calibrating can be a time-consuming matter, and
 * tuning it extremely finely is often useless because at a certain level
 * you get additional delay by calling the delay() function, and returning
 * from it, etc.) we only calculate delayCount up to a certain precision
 * (ie, we only calculate PRECISION extra bits after stage 1.)  A
 * PRECISION value of 8 gives good results (according to the linux
 * sources, it gives an imprecision of approximately 1%.)
 */

#define PRECISION 8                 /* Calibration precision          */

/*
 * Basically, delayCount is the number you have to count to in order to
 * kill MILLISEC ms.  delay() takes the amount of microseconds to delay as
 * parameter.
 *
 * The goal of this code is to determine what delayCount is for your
 * machine.
 *
 * Make sure that delayCount is initialised to a non-zero power of two.
 * We'll need that later.
 */

PUBLIC UDATA delayCount = (1 << 12);

PUBLIC UDATA ABLcalDelayLoop(VOID)
{
    UDATA prevtick;                 /* Temporary variable             */
    UDATA i;                        /* Counter variable               */
    UDATA calibBit;                 /* Bit to calibrate (see below)   */
    UDATA shifts = -1;              /* Bits calibrated (coarse)       */

    /* Initialise timer interrupt with MILLISECOND ms interval        */
    outportb(TMR_CTRL, (TMR_SC0 + TMR_both + TMR_MD2));
    outportb(TMR_CNT0, (1193180/FREQ)&0xff);
    outportb(TMR_CNT0, ((1193180/FREQ)>>8)&0xff);
    ABLsetVector(ABLdelayISR, M_VEC, (D_INT + D_PRESENT));
    ABLenableIRQ(0);

    /* Stage 1:  Coarse calibration                                   */
retry:
    do {
        delayCount <<= 1;           /* Next delay count to try        */
        shifts++;                   /* Count this shift               */

        prevtick=ticks;             /* Wait for the start of the next */
        while(prevtick==ticks);     /* timer tick                     */

        prevtick=ticks;             /* Start measurement now          */
        __delay(delayCount);        /* Do the delay                   */
    } while(prevtick == ticks);     /* Until delay is just too big    */

    if(!shifts) {                   /* If we've passed a period right */
        delayCount = 1;             /* away, this must be a REAL slow */
        goto retry;                 /* computer - do it the slow way. */
    }

    delayCount >>= 1;               /* Get bottom value for delay     */

    /* Stage 2:  Fine calibration                                     */

    calibBit = delayCount;          /* Which bit are we going to test */

    for(i=0;i<PRECISION;i++) {
        calibBit >>= 1;             /* Next bit to calibrate          */
        if(!calibBit) break;        /* If we have done all bits, stop */

        delayCount |= calibBit;     /* Set the bit in delayCount      */

        prevtick=ticks;             /* Wait for the start of the next */
        while(prevtick==ticks);     /* timer tick                     */

        prevtick=ticks;             /* Start measurement now          */
        __delay(delayCount);        /* Do the delay                   */

        if(prevtick != ticks)       /* If a tick has passed, turn the */
            delayCount &= ~calibBit;       /* calibrated bit back off */
    }

    /* We're finished:  Do the finishing touches                      */

    ABLdisableIRQ(0);               /* Our wonderful PIT can stop now */

    return delayCount;
}

/**************************************************************************/

/*
 * The delay function:
 *
 * When I was testing the delay code, I noticed that the calibration we use
 * is very delicate.  Originally, I had inlined the delay for() loops into
 * the delay() and calibrateDelayLoop() functions, but this doesn't work.
 * Due to different register allocation or something similar the compiler
 * might have generated different code in the two situations (I didn't
 * check this.)  Or alignment differences might have caused the problems.
 * Anyway, to solve the problem, we always use the __delay() function for
 * the delay for() loop, because it always is the same code with the same
 * alignment. __delay() is called from delay() as well as from
 * calibrateDelayLoop().  By using __delay() we can fine-tune our
 * calibration without it losing its finesse afterwards.
 * Make sure this is the last C function in the file, or GCC will inline
 * it implicitly !!!
 */

PUBLIC VOID __delay(UDATA loops)
{
    UDATA c;
    for(c=0;c<loops;c++);
}

/**************************************************************************/

/*
 * The hardware part of the timer ISR:
 * This code is invoked when the PIT finishes its period.
 */

asm (
   ".globl ABLdelayISR     \n"
   "ABLdelayISR:           \n"
   "   pusha               \n"   /* Save all registers             */
   "   pushw %ds           \n"   /* Save user code data segment    */
   "   pushw %ss           \n"   /* Get kernel data segment        */
   "   popw %ds            \n"   /* %ss always valid (ss0 in TSS)  */
   "                       \n"
   "   call delayCalibInt  \n"   /* Call the actual ISR code       */
   "                       \n"
   "   movb $0x20,%al      \n"   /* Send EOI to the 8259           */
   "   outb %al,$0x20      \n"
   "   popw %ds            \n"   /* Restore user code data segment */
   "   popa                \n"   /* Restore registers              */
   "   iret                \n"   /* Return from interrupt          */
);

/**************************************************************************/

/* The end */
