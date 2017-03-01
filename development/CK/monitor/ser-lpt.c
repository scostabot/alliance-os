/* Remote serial interface for local (hardwired) parallel ports, over
   a LapLink (PLIP) cable.  Copyright 1999, Free Software Foundation, Inc.

   Contributed by Ramon van Handel (vhandel@chem.vu.nl).

   This file is part of GDB.  

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/******************************************************************************/

/*
   This is the GDB serial interface for remote debugging over a
   parallel null-cable (also known as a LapLink cable).  Such
   a cable is often more practical in use than a serial nullmodem
   (not to mention faster for data transfer, but that is not the
   goal of this code).

   Currently, this code requests I/O port access from the host
   OS and writes directly to the ports.  This has a few drawbacks:

   - I don't know how this works on any other OS than linux
   - You probably need to be root in order to use this
   - It might confuse your driver system (i.e., you can't
     use the /dev/lp device after you've been debugging.)

   Eventually I'll probably need to replace this with a better
   system (such as implementing it on top of the linux parport
   user-level device which was introduced in the 2.3 series of
   the linux kernel), but for now this should work (I hope :)).

   The driver was modified from the ser-go32.c serial interface
   for DOS GO32.  Many thanks to the author of that code for
   writing an interface that was simple enough for me to be
   able to see what's important and what isn't ;).

   If you have any questions about this code, or have found a
   race condition that I've overlooked (or even nicer, if you
   fixed it already as well :)) do not hestitate to contact me
   by email at vhandel@chem.vu.nl.

   Below is some documentation on various aspects of the code.
*/

/*
   From linux/drivers/net/plip.c:
 
   The cable used is a de facto standard parallel null cable -- sold as
   a "LapLink" cable by various places.  You'll need a 12-conductor cable
   to make one yourself.  The wiring is:
 
       SLCTIN      17 - 17
       GROUND      25 - 25
       D0->ERROR   2 - 15          15 - 2
       D1->SLCT    3 - 13          13 - 3
       D2->PAPOUT  4 - 12          12 - 4
       D3->ACK     5 - 10          10 - 5
       D4->BUSY    6 - 11          11 - 6
 
   Do not connect the other pins.  They are
 
       D5,D6,D7 are 7,8,9
       STROBE is 1, FEED is 14, INIT is 16
       extra grounds are 18,19,20,21,22,23,24
*/

/*
   Ports 278, 378, 3BC:  printer data output  (readable)
 
         |7|6|5|4|3|2|1|0|
          | | | | | | | `---- data bit 0, hardware pin 2
          | | | | | | `----- data bit 1, hardware pin 3
          | | | | | `------ data bit 2, hardware pin 4
          | | | | `------- data bit 3, hardware pin 5
          | | | `-------- data bit 4, hardware pin 6
          | | `--------- data bit 5, hardware pin 7
          | `---------- data bit 6, hardware pin 8
          `----------- data bit 7, hardware pin 9
 
 
   Ports 279, 379, 3BD:  printer status register
 
         |7|6|5|4|3|2|1|0|
          | | | | | | | `---- 1 = time-out
          | | | | | `-+----- unused
          | | | | `-------- 1 = error, pin 15
          | | | `--------- 1 = on-line, pin 13
          | | `---------- 1 = out of paper, pin 12
          | `----------- 0 = Acknowledge, pin 10
          `------------ 0 = busy,  pin 11
 
   BEWARE !!!  The BUSY pin is hardware-inverted
 
 
   Ports 27A, 37A, 3BE:  printer control register
 
         |7|6|5|4|3|2|1|0|
          | | | | | | | `---- 1 = output data to printer,  (pin 1)
          | | | | | | `----- 1 = auto line feed,  (pin 14)
          | | | | | `------ 0 = initialize printer,  (pin 16)
          | | | | `------- 1 = printer reads output,  (pin 17)
          | | | `-------- 0 = IRQ disable,1=IRQ enable for ACK
          `-+-+--------- unused
*/

/*
   We use the following protocol to get a character stream from the
   parallel port:
 
   We transfer one nibble at a time.  The nibble consists of
   output pins D1-D4, which correspond to input pins SLCT,PAPOUT,
   ACK, and BUSY respectively (the top nibble of the status
   register.)  D0 == ERROR is used for synchronisation.
   The protocol can be summarised as follows:
 
       Send_Byte:
           WAIT FOR ERROR = 0
           D1..D4 := low nibble, D0 := 1
           WAIT FOR ERROR = 1
           D1..D4 := high nibble, D0 := 0
 
       Recv_Byte:
           WAIT FOR ERROR = 1
           (S1..S4) ^ 0x8 = low nibble
           D0 := 1
           WAIT FOR ERROR = 0
           (S1..S4) ^ 0x8 = high nibble
           D0 := 0

   The whole matter is slightly complicated because GDB requires
   support for asynchronous transfers.  We implement a circular
   FIFO buffer and try to find an elegant way to circumvent all
   those horrible race conditions.  For more information, see
   the documentation in the code.
*/

/******************************************************************************/

#include <sys/perm.h>
#include <asm/io.h>
#include <unistd.h>
#include <signal.h>
#include "defs.h"
#include "gdbcmd.h"
#include "serial.h"

/******************************************************************************/

/* Scary macros go here */

static unsigned char tmp;             /* Holds last written data byte   */

#define SETD0() ({                    /* D0 := 1                        */     \
    outb_p((tmp|0x01),port->data);    /* Set D0 (bit 0)                 */     \
})

#define RSETD0() ({                   /* D0 := 0                        */     \
    outb_p((tmp&~0x01),port->data);   /* Reset D0 (bit 0)               */     \
})

#define GETNIBBLE() ({                /* Get a nibble from remote end   */     \
    unsigned char __tmp;              /* Temporary holder of status reg */     \
    __tmp = inb(port->stat);          /* Get the status register        */     \
    __tmp >>= 4;                      /* Isolate the data bits          */     \
    __tmp;                            /* Return them to the function    */     \
})

#define HANDSHAKE() ({                /* Handshake with the remote end  */     \
    if(!port->connected) {                                                     \
        outb(0x1e, port->data);                                                \
        while(((inb(port->stat)>>3)^0x10)!=0x1e);                              \
        outb_p(0x00, port->data);                                              \
        port->connected = 1;                                                   \
    }                                                                          \
})


/******************************************************************************/

#define BUFSIZE  512			/* FIFO size			*/
#define DELAY    5000			/* Race avoidance delay         */

static struct par_ttystate
{
  int		data;			/* Data port			*/
  int		stat;			/* Status port			*/
  int		ctrl;			/* Control port			*/
  int		refcnt;			/* Reference count		*/
  int		connected;		/* Handshake flag		*/
  unsigned char	parbuf[BUFSIZE];	/* Incoming character buffer	*/
  unsigned char *wrtptr;		/* Write pointer into buffer	*/
  unsigned char *rdptr;			/* Read pointer into the buffer	*/
} ports[3] = {
  { 0x3bc, 0x3bd, 0x3be }, 
  { 0x378, 0x379, 0x37a }, 
  { 0x278, 0x279, 0x27a }
};

/******************************************************************************/

static int  par_open PARAMS ((serial_t scb, const char *name));
static void par_raw PARAMS ((serial_t scb));
static int  par_readchar PARAMS ((serial_t scb, int timeout));
static int  par_setbaudrate PARAMS ((serial_t scb, int rate));
static int  par_write PARAMS ((serial_t scb, const char *str, int len));
static void par_close PARAMS ((serial_t scb));
static serial_ttystate par_get_tty_state PARAMS ((serial_t scb));
static int  par_set_tty_state PARAMS ((serial_t scb, serial_ttystate state));

/******************************************************************************/

static int par_open (serial_t scb, const char *name)
{
  struct par_ttystate *port;
  int fd, i;

  /* Get port number and location */
  fd = name[3] - '1';
  if (fd < 0 || fd > 2)
    {
      errno = ENOENT;
      return -1;
    }
  port = &ports[fd];

  /* Increase the reference count */
  if (port->refcnt++ > 0)
    {
      /* Device already opened another user.  Just point at it. */
      scb->fd = fd;
      return 0;
    }

  /* Get port permissions */
  if(ioperm(port->data, 0x3, 0xff) < 0)
    {
      fprintf_unfiltered(gdb_stderr, "ioperm failed: cannot get port permissions; are you running as root ?\n");
      errno = ENODEV;
      return -1;
    }

  /* Get delay port permissions */
  if(ioperm(0x80, 0x1, 0xff) < 0)
    {
      fprintf_unfiltered(gdb_stderr, "ioperm failed: cannot get delay port\n");
      errno = ENODEV;
      return -1;
    }

  /* record port */
  scb->fd = fd;
  port->wrtptr = port->rdptr = port->parbuf;

  /* Reset the parallel port */
  outb_p(0x8, port->ctrl);
  outb_p(0xc, port->ctrl);

  /* Reset all data pins */
  outb_p(0x0, port->data);

  /* Force handshake on next access */
  port->connected = 0;

  return 0;
}

static void par_close (serial_t scb)
{
  struct par_ttystate *port;

  if (!scb)
    return;

  /* Decrease the reference count */
  port = &ports[scb->fd];
  if(!(--(port->refcnt)))
    {
      /* Reset all data pins */
      outb_p(0x0, port->data);
#if 0
      /* Force handshake on next access */
      port->connected = 0;
#endif
    }

  return;
}

static int par_noop (serial_t scb)
{
  /* We don't need any flushing */
  return 0;
}

static void par_raw (serial_t scb)
{
  /* Always in raw mode */
  return;
}

/* Read a character with user-specified timeout.  TIMEOUT is number of
   seconds to wait, or -1 to wait forever.  Use timeout of 0 to effect a
   poll.  Returns char if successful.  Returns -2 if timeout expired, EOF
   if line dropped dead, or -3 for any other error (see errno in that
   case). */

volatile int alrm = 0;

static void par_timeout(int i)
{
  alrm = 1;
  return;
}

static int par_getc (struct par_ttystate *port, int timeout)
{
  unsigned char low, high;

  if(timeout == 0)
    return -2;

  /* Get signal on timeout */
  alrm = 0;
  if(timeout > 0)
    {
      signal(SIGALRM, par_timeout);
      alarm(timeout);
    }

  /* Get the byte from the port */
  HANDSHAKE();                      /* Handshake with remote end      */
  while(!(inb(port->stat)&0x8)) {   /* WAIT FOR ERROR = 1             */
    if(alrm) {                      /* Whoops, timeout...             */
      alrm = 0;
      return -2;
    }
  }
  low = GETNIBBLE();                /* S1..S4 = low nibble            */
  SETD0();                          /* D0 := 1                        */
  while(inb(port->stat)&0x8) {      /* WAIT FOR ERROR = 0             */
    if(alrm) {                      /* Whoops, timeout...             */
      alrm = 0;
      return -2;
    }
  }
  high = GETNIBBLE();               /* S1..S4 = high nibble           */

  alarm(0);                         /* Cancel the timeout             */

  return (low + (high<<4)) ^ 0x88;  /* Return the full byte           */
}

static int par_readchar (serial_t scb, int timeout)
{
  struct par_ttystate *port = &ports[scb->fd];
  int c;

  if (port->wrtptr == port->rdptr)
    {
      RSETD0();
      c = par_getc(port, timeout);
      RSETD0();
      return c;
    }
  else
    {
      c = (int) *port->rdptr++;
      if((port->rdptr-port->parbuf)==BUFSIZE)
        port->rdptr = port->parbuf;
      return c;
    }
}

static serial_ttystate par_get_tty_state (serial_t scb)
{
  struct par_ttystate *port = &ports[scb->fd];
  struct par_ttystate *state;

  state = (struct par_ttystate *) xmalloc (sizeof *state);
  *state = *port;
  return (serial_ttystate) state;
}

static int par_set_tty_state (serial_t scb, serial_ttystate ttystate)
{
  /* Nothing to set... don't you go mess with my I/O port values ! */
  return 0;
}

static int par_noflush_set_tty_state (serial_t scb,
               serial_ttystate new_ttystate, serial_ttystate old_ttystate)
{
  /* Nothing to set... don't you go mess with my I/O port values ! */
  return 0;
}

static int par_flush_input (serial_t scb)
{
  struct par_ttystate *port = &ports[scb->fd];

  port->rdptr = port->wrtptr;

  return 0;
}

static void par_print_tty_state (serial_t scb, serial_ttystate ttystate)
{
  /* Nothing to print */
  return;
}

static int par_setbaudrate (serial_t scb, int rate)
{
  /* Baudrate ?  wuzzdat ? */
  return 0;
}

static int par_setstopbits (serial_t scb, int num)
{
  /* Stop bits ?  Uh ? */
  return 0;
}

static int par_write (serial_t scb, const char *str, int len)
{
  struct par_ttystate *port = &ports[scb->fd];
  int i;

  while (len > 0) 
    {
      char c = *str;

      HANDSHAKE();                  /* Handshake with remote end */

      /*
          Tricky: declare data in tmp, but don't tell remote about it yet.
          This is to avoid a race condition with the remote end.
          The problem:  getChar() leaves D0, which is ERROR on the remote
          end, on.  Now, say the remote end just finished sending a
          character and we've got it though this function while waiting
          for the remote end to shut up and let us say something.  Now,
          if there is a sufficient speed difference between the two
          machines, a race condition can appear:  that the remote end,
          executing getChar(), will get the low nibble before we had
          the chance to put it on the port (after all, if ERROR is
          asserted that means data is ready.)  In order to circumvent
          this problem we put the low nibble in tmp beforehand without
          D0 asserted -- this way, the race condition cannot occur,
          because when getChar() sets D0 it'll automatically set our
          data with it.
      */
      tmp = ((c&0xf)<<1);                /* Preset nibble to write in tmp  */

      /*
          Why this is an if() --
          The asynchrounous protocol is set up in such a way, that if
          both ends try to putDebugChar() at the same time, they'll
          get to send characters in turn.  This avoids several sticky
          race conditions that would occur otherwise.
      */
      if(inb(port->stat)&0x8) {                   /* WAIT FOR ERROR = 0    */
        /* Put incoming char in buffer */
        *(port->wrtptr++) = (unsigned char) par_getc(port,-1);

        /* Handle circular FIFO buffer */
        if((port->wrtptr-port->parbuf)==BUFSIZE)
            port->wrtptr = port->parbuf;
      }

      /*
         Now we're ready to transmit our nibbles to the remote end.
      */
      outb_p(((c&0xf)<<1)+1,port->data); /* D1..D4 := low nibble, D0 := 1  */
      while(!(inb(port->stat)&0x8));     /* WAIT FOR ERROR = 1             */
      outb_p((c&0xf0)>>3,port->data);    /* D1..D4 := high nibble, D0 := 0 */

      /*
         Delay to avoid race
       */
      for(i=0;i<DELAY;i++);

      str++;
      len--;
    }

  return 0;
}

static int par_sendbreak (serial_t scb)
{
  /* Who needs breaks ? */
  return 0;
}

/******************************************************************************/

static struct serial_ops par_ops =
{
  "parallel",
  0,
  par_open,
  par_close,
  par_readchar,
  par_write,
  par_noop,
  par_flush_input,
  par_sendbreak,
  par_raw,
  par_get_tty_state,
  par_set_tty_state,
  par_print_tty_state,
  par_noflush_set_tty_state,
  par_setbaudrate,
  par_setstopbits,
};

void
_initialize_ser_par ()
{
  serial_add_interface (&par_ops);
}

/* The end */

