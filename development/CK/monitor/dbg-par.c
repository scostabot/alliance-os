/*
 * dbg-par.c:  GDB parallel port stubs (used for remote debugging
 *             over a LapLink parallel cable)
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/08/99  ramon       1.0    First release
 * 09/08/99  ramon       1.1    Implemented async comms / fixed race conds
 * 15/08/99  ramon       1.2    Fixed fatal race condition: it works now !
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

/*
 *  From linux/drivers/net/plip.c:
 *
 *  The cable used is a de facto standard parallel null cable -- sold as
 *  a "LapLink" cable by various places.  You'll need a 12-conductor cable
 *  to make one yourself.  The wiring is:
 *
 *      SLCTIN      17 - 17
 *      GROUND      25 - 25
 *      D0->ERROR   2 - 15          15 - 2
 *      D1->SLCT    3 - 13          13 - 3
 *      D2->PAPOUT  4 - 12          12 - 4
 *      D3->ACK     5 - 10          10 - 5
 *      D4->BUSY    6 - 11          11 - 6
 *
 *  Do not connect the other pins.  They are
 *
 *      D5,D6,D7 are 7,8,9
 *      STROBE is 1, FEED is 14, INIT is 16
 *      extra grounds are 18,19,20,21,22,23,24
 */

/*
 *  Ports 278, 378, 3BC:  printer data output  (readable)
 *
 *        |7|6|5|4|3|2|1|0|
 *         | | | | | | | `---- data bit 0, hardware pin 2
 *         | | | | | | `----- data bit 1, hardware pin 3
 *         | | | | | `------ data bit 2, hardware pin 4
 *         | | | | `------- data bit 3, hardware pin 5
 *         | | | `-------- data bit 4, hardware pin 6
 *         | | `--------- data bit 5, hardware pin 7
 *         | `---------- data bit 6, hardware pin 8
 *         `----------- data bit 7, hardware pin 9
 *
 *
 *  Ports 279, 379, 3BD:  printer status register
 *
 *        |7|6|5|4|3|2|1|0|
 *         | | | | | | | `----- 1 = time-out
 *         | | | | | | `------ unused
 *         | | | | | `------- 0 = IRQ occured
 *         | | | | `-------- 1 = error, pin 15
 *         | | | `--------- 1 = on-line, pin 13
 *         | | `---------- 1 = out of paper, pin 12
 *         | `----------- 0 = Acknowledge, pin 10
 *         `------------ 0 = busy,  pin 11
 *
 *  BEWARE !!!  The BUSY pin is hardware-inverted
 *
 *
 *  Ports 27A, 37A, 3BE:  printer control register
 *
 *        |7|6|5|4|3|2|1|0|
 *         | | | | | | | `---- 1 = output data to printer,  (pin 1)
 *         | | | | | | `----- 1 = auto line feed,  (pin 14)
 *         | | | | | `------ 0 = initialize printer,  (pin 16)
 *         | | | | `------- 1 = printer reads output,  (pin 17)
 *         | | | `-------- 0 = IRQ disable,1=IRQ enable for ACK
 *         | | `--------- 1 = enable bidirectional port
 *         `-+---------- unused
 */

/*
 *  We use the following protocol to get a character stream from the
 *  parallel port:
 *
 *  We transfer one nibble at a time.  The nibble consists of
 *  output pins D1-D4, which correspond to input pins SLCT,PAPOUT,
 *  ACK, and BUSY respectively (the top nibble of the status
 *  register.)  D0 == ERROR is used for synchronisation.
 *  The protocol can be summarised as follows:
 *
 *      Send_Byte:
 *          WAIT FOR ERROR = 0
 *          D1..D4 := low nibble, D0 := 1
 *          WAIT FOR ERROR = 1
 *          D1..D4 := high nibble, D0 := 0
 *
 *      Recv_Byte:
 *          WAIT FOR ERROR = 1
 *          (S1..S4) ^ 0x8 = low nibble
 *          D0 := 1
 *          WAIT FOR ERROR = 0
 *          (S1..S4) ^ 0x8 = high nibble
 *          D0 := 0
 *
 *  The whole matter is slightly complicated because GDB requires
 *  support for asynchronous transfers.  We implement a circular
 *  FIFO buffer and try to find an elegant way to circumvent all
 *  those horrible race conditions.  For more information, see
 *  the documentation in the code.
 */


/************************************************************************/
/* Settings                                                             */
/************************************************************************/

/* What parallel port to use for communications */
#define parport  0


/************************************************************************/
/* Definitions / Includes / Declarations                                */
/************************************************************************/

/* Includes */
#include <typewrappers.h>
#include "sys/sys.h"
#include "sys/gdt.h"
#include "sys/ioport.h"

/* Declarations */
extern void set_debug_traps(void);
extern void breakpoint(void);

/* Definitions */
#define BUFSIZE  512    /* FIFO size    */
#define DELAY    5000   /* Race delay   */

/* Parallel port locations */
#if   (parport == 0)
#define parirq  0x007   /* IRQ          */
#define pardata 0x3bc   /* Data port    */
#define parstat 0x3bd   /* Status port  */
#define parctrl 0x3be   /* Control port */
#elif (parport == 1)
#define parirq  0x007   /* IRQ          */
#define pardata 0x378   /* Data port    */
#define parstat 0x379   /* Status port  */
#define parctrl 0x37a   /* Control port */
#elif (parport == 2)
#define parirq  0x007   /* IRQ          */
#define pardata 0x278   /* Data port    */
#define parstat 0x279   /* Status port  */
#define parctrl 0x27a   /* Control port */
#else
#error "DEBUG:  A parallel port needs to be specified for remote debugging
        to work.  Alternatively, compile in the serial stubs"
#endif


/************************************************************************/
/* Macros                                                               */
/************************************************************************/

static unsigned char tmp;             /* Holds last written data byte   */
int gdb_connected = 0;                /* Is 0 if we're not connected    */

#define SETD0() ({                    /* D0 := 1                        */     \
    doutportb(pardata,(tmp|0x01));    /* Set D0 (bit 0)                 */     \
})

#define RSETD0() ({                   /* D0 := 0                        */     \
    doutportb(pardata,(tmp&~0x01));   /* Reset D0 (bit 0)               */     \
})

#define GETNIBBLE() ({                /* Get a nibble from remote end   */     \
    unsigned char __tmp;              /* Temporary holder of status reg */     \
    __tmp = inportb(parstat);         /* Get the status register        */     \
    __tmp >>= 4;                      /* Isolate the data bits          */     \
    __tmp;                            /* Return them to the function    */     \
})

#define HANDSHAKE() ({                /* Handshake with the remote end  */     \
    if(!gdb_connected) {                                                       \
        CKdebugPrint("gdbstub: Entering debugger... please initiate remote GDB now.\n");\
        outportb(pardata, 0x1e);                                               \
        while(((inportb(parstat)>>3)^0x10)!=0x1e);                             \
        doutportb(pardata, 0x00);                                              \
        CKdebugPrint("gdbstub: Debugger attached\n");                          \
        gdb_connected = 1;                                                     \
    }                                                                          \
})


/************************************************************************/
/* Communication functions                                              */
/************************************************************************/

void CKinitDebugger(void)
{
    /* Reset the parallel port */     /*        ____                    */
    doutportb(parctrl, 0x8);          /* Assert INIT bit in control     */
    doutportb(parctrl, 0xc);          /* Deassert it:  now initialized  */

    /* Reset all data pins */
    doutportb(pardata, 0x0);          /* Set all the data pins to 0     */

    /* Bootstrap GDB */
    set_debug_traps();                /* Initialise GDB exception handl */
#ifdef DEBUG
    breakpoint();                     /* Start debugging                */
#endif
}

unsigned char parbuf[BUFSIZE];        /* Incoming character buffer      */
unsigned char *wrtptr = parbuf;       /* Write pointer into the buffer  */
unsigned char *rdptr  = parbuf;       /* Read pointer into the buffer   */

unsigned char getChar(void)
{
    unsigned char low, high;

    HANDSHAKE();                      /* Handshake with remote end      */
    while(!(inportb(parstat)&0x8));   /* WAIT FOR ERROR = 1             */
    low = GETNIBBLE();                /* S1..S4 = low nibble            */
    SETD0();                          /* D0 := 1                        */
    while(inportb(parstat)&0x8);      /* WAIT FOR ERROR = 0             */
    high = GETNIBBLE();               /* S1..S4 = high nibble           */

    return (low + (high<<4)) ^ 0x88;  /* Return the full byte           */
}

int getDebugChar(void)
{
    unsigned char c;

    if(wrtptr == rdptr) {             /* Have we got characters pending */
        RSETD0();                     /* in the buffer ?  Nope, mark us */
        c = getChar();                /* ready to get a char, get one   */
        RSETD0();                     /* from remote, mark us ready for */
        return c;                     /* next byte, and return char     */
    } else {
        c = *rdptr++;                 /* Yep.  Get it from the buffer   */
        if((rdptr-parbuf)==BUFSIZE)   /* Handle circular FIFO buffer    */
            rdptr = parbuf;           /* Overflow, wrap around to start */
        return c;                     /* Return character from buffer   */
    }
}

void putDebugChar(int ch)
{
    int i;

    HANDSHAKE();                      /* Handshake with remote end      */

    /*
     *  Tricky: declare data in tmp, but don't tell remote about it yet.
     *  This is to avoid a race condition with the remote end.
     *  The problem:  getChar() leaves D0, which is ERROR on the remote
     *  end, on.  Now, say the remote end just finished sending a
     *  character and we've got it though this function while waiting
     *  for the remote end to shut up and let us say something.  Now,
     *  if there is a sufficient speed difference between the two
     *  machines, a race condition can appear:  that the remote end,
     *  executing getChar(), will get the low nibble before we had
     *  the chance to put it on the port (after all, if ERROR is
     *  asserted that means data is ready.)  In order to circumvent
     *  this problem we put the low nibble in tmp beforehand without
     *  D0 asserted -- this way, the race condition cannot occur,
     *  because when getChar() sets D0 it'll automatically set our
     *  data with it.
     */
    tmp = ((ch&0xf)<<1);              /* Preset nibble to write in tmp  */

    /*
     *  Why this is an if() --
     *  The asynchrounous protocol is set up in such a way, that if
     *  both ends try to putDebugChar() at the same time, they'll
     *  get to send characters in turn.  This avoids several sticky
     *  race conditions that would occur otherwise.
     */
    if(inportb(parstat)&0x8) {        /* WAIT FOR ERROR = 0             */
        *wrtptr++ = getChar();        /* Put incoming char in buffer    */
        if((wrtptr-parbuf)==BUFSIZE)  /* Handle circular FIFO buffer    */
            wrtptr = parbuf;          /* Reached end, wrap around to    */
    }

    /*
     * Now we're ready to transmit our nibbles to the remote end.
     */
    doutportb(pardata,((ch&0xf)<<1)+1); /* D1..D4 := low nibble,D0 := 1 */
    while(!(inportb(parstat)&0x8));   /* WAIT FOR ERROR = 1             */
    doutportb(pardata,(ch&0xf0)>>3);  /* D1..D4 := high nibble, D0 := 0 */

    for(i=0;i<DELAY;i++);             /* Delay to avoid race condition  */
}


/************************************************************************/
/* Other GDB support functions                                          */
/************************************************************************/

void exceptionHandler(int exc, void *addr)
{
    CKsetVector(addr, (unsigned char)exc, (D_INT + D_PRESENT + D_DPL3));
}

/* The end */
