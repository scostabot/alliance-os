/*
 * gdb-stub.c:  GDB remote debugging stubs
 *
 * This file was mostly taken from the Flux OSKit source distribution.
 * The version that comes with the GDB source distribution (i386-stub.c)
 * is simply too buggy.  However, I added in some stuff from the original
 * i386-stub.c (mostly the interrupt stub code), which I have edited
 * freely.  I also changed some of the code in order to make it work
 * with the Alliance CK environment. In particular, I replaced all the
 * stdio.h and string.h dependancies with Alliance counterparts from the
 * KL.  Communication stubs can be found in dbg-par.c and dbg-ser.c.
 *                                                            -- Ramon
 *
 * TODO:
 *   - Go though the protocol and add things that are alliance-specific
 *     Such as thread support with 'H' packets.
 *   - Document all of this.
 *   - Support missing parts of the protocol
 *   - Hook the serial interface IRQ, in order to support BRK
 *   - Handle rehandshaking correctly (is this possible ?)
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 06/08/99  ramon       1.0    First release
 * 16/08/99  ramon       1.1    Fixed several fatal bugs
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
 * Copyright (c) 1994-1996 Sleepless Software
 * Copyright (c) 1997-1998 University of Utah and the Flux Group.
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
 *  Remote serial-line source-level debugging for the Flux OS Toolkit.
 *  Loosely based on i386-stub.c from GDB 4.14
 */

#include <typewrappers.h>
#include <klibs.h>
#include "sys/sys.h"


/***************************************************************************/
/* Defines                                                                 */
/***************************************************************************/

/*
 *  BUFMAX defines the maximum number of characters in inbound/outbound
 *  buffers.  At least NUMREGBYTES*2 are needed for register packets.
 */
#define BUFMAX 400

/*
 * Use this macro to insert a breakpoint manually anywhere in your code.
 */
#define gdb_breakpoint() asm volatile("int $3" : : : "memory");


/***************************************************************************/
/* Declarations                                                            */
/***************************************************************************/

/*
 *  These functions implement how we send and receive characters
 *  over the serial line.
 */
extern void putDebugChar(int);
extern int  getDebugChar(void);

/*
 *  This function is used to hook and exception in the IDT
 */
extern void exceptionHandler(int, void *);

/*
 *  True if we have an active connection to a remote debugger.
 *  This is used to avoid sending 'O' (console output) messages
 *  before the connection has been established or after it is closed,
 *  which tends to hang us and/or confuse the debugger.
 */
static int connected;

/*
 *  True if we output debugging information for this file.
 */
int gdb_serial_debug;

static const char hexchars[]="0123456789abcdef";


/***************************************************************************/
/* Structures                                                              */
/***************************************************************************/

/* This structure represents the x86 register state frame as GDB wants it. */
struct gdb_state {
        unsigned        eax;
        unsigned        ecx;
        unsigned        edx;
        unsigned        ebx;
        unsigned        esp;
        unsigned        ebp;
        unsigned        esi;
        unsigned        edi;
        unsigned        eip;
        unsigned        eflags;
        unsigned        cs;
        unsigned        ss;
        unsigned        ds;
        unsigned        es;
        unsigned        fs;
        unsigned        gs;
};

struct gdb_params
{
        /*
         * An ID number for the current thread, -1 if no thread callbacks.
         */
        int thread_id;

        /*
         * The signal number describing why the program stopped.
         * On GDB_CONT return, signal to take when continuing or zero
         * to continue execution normally.
         */
        char signo;

        /*
         * Register state of the current thread.
         * The caller must provide this buffer.
         */
        struct gdb_state *regs;

        /*
         * Set by `gdb_serial_converse' when values in `regs' have been
         * changed.  The caller (or callback) then needs to commit these
         * changes to the thread.
         */
        int regs_changed;
};

/*
 * Return values from `gdb_serial_converse' (first one is never returned).
 * These indicate to the caller what the user has asked GDB to do.
 */
enum gdb_action {
        GDB_MORE,               /* internal - keeping reading gdb cmds */
        GDB_CONT,               /* continue with signo */
        GDB_RUN,                /* rerun the program from the beginning */
        GDB_KILL,               /* kill the program */
        GDB_DETACH              /* continue; debugger is going away */
};

/***************************************************************************/
/* Helper functions                                                        */
/***************************************************************************/

static int hex(char ch)
{
    if ((ch >= '0') && (ch <= '9')) return (ch-'0');
    if ((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
    if ((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
    return (-1);
}

static int hex2bin(char *in, unsigned char *out, int len)
{
    if (gdb_serial_debug)
        CKdebugPrint("gdbstub: hex2bin %d chars: %.*s\n", len, len, in);

    while (len--) {
        int ch = (hex(in[0]) << 4) | hex(in[1]);
        if (ch < 0)
            return -1;
        *out++ = ch;
        in += 2;
    }

    return 0;
}

static int hex2int(char *str, char **ptr)
{
    int hexValue, intValue = 0;

    *ptr = str;

    while (**ptr) {
        hexValue = hex(**ptr);
        if (hexValue >=0)
            intValue = (intValue << 4) | hexValue;
        else
            break;

        (*ptr)++;
    }

    return intValue;
}

/*
 * Copy between kernel address space and a GDB buffer,
 * detecting and recovering from any invalid accesses that occur.
 * Since we are merely copying in the same address space (the kernel),
 * both the gdb_copyin and gdb_copyout routines can be the same.
 *
 * arg0:        source address
 * arg1:        destination address
 * arg2:        byte count
 */
extern int gdb_copyin(long src, void *dest, long size);
extern int gdb_copyout(void *src, long dest, long size);

asm (
    ".text                                \n"
    ".globl gdb_copyin                    \n"
    ".globl gdb_copyout                   \n"
    ".align 4                             \n"
    "gdb_copyin:                          \n"
    "gdb_copyout:                         \n"
    "    pushl %esi                       \n"
    "    pushl %edi                       \n" /* save registers          */
    "                                     \n"
    "    movl 8+4(%esp),%esi              \n" /* get user start address  */
    "    movl 8+8(%esp),%edi              \n" /* get kernel dest address */
    "    movl 8+12(%esp),%edx             \n" /* get count               */
    "                                     \n"
    "    movl $copy_fail,gdb_trap_recover \n"
    "                                     \n"
    "    cld                              \n" /* count up                */
    "    movl %edx,%ecx                   \n" /* move by longwords first */
    "    shrl $2,%ecx                     \n"
    "    rep                              \n"
    "    movsl                            \n" /* move longwords          */
    "    movl %edx,%ecx                   \n" /* move remaining bytes    */
    "    andl $3,%ecx                     \n"
    "    rep                              \n"
    "    movsb                            \n"
    "                                     \n"
    "    xorl %eax,%eax                   \n" /* return 0 for success    */
    "                                     \n"
    "copy_ret:                            \n"
    "    movl $0,gdb_trap_recover         \n"
    "                                     \n"
    "    popl %edi                        \n" /* restore registers       */
    "    popl %esi                        \n"
    "    ret                              \n" /* and return              */
    "                                     \n"
    "copy_fail:                           \n"
    "    movl $-1,%eax                    \n" /* return -1 for failure   */
    "    jmp  copy_ret                    \n" /* pop frame and return    */
);

/***************************************************************************/
/* GDB stub                                                                */
/***************************************************************************/

/* Scan for the sequence $<data>#<checksum> */
static void getpacket(char *buffer)
{
    unsigned char checksum;
    unsigned char xmitcsum;
    int  i;
    int  count;
    char ch;

    do {
        /*
         *  Wait around for the start character;
         *  ignore all other characters.
         */
        while ((ch = (getDebugChar() & 0x7f)) != '$');
        checksum = 0;
        xmitcsum = -1;

        count = 0;

        /* Now, read until a # or end of buffer is found.  */
        while (count < BUFMAX) {
            ch = getDebugChar() & 0x7f;
            if (ch == '#') break;
            checksum = checksum + ch;
            buffer[count] = ch;
            count = count + 1;
        }
        buffer[count] = 0;

        if (ch == '#') {
            xmitcsum = hex(getDebugChar() & 0x7f) << 4;
            xmitcsum += hex(getDebugChar() & 0x7f);
            if (checksum != xmitcsum) {
                putDebugChar('-'); /* failed checksum */
            } else {
                putDebugChar('+'); /* successful transfer */

                /*
                 *  If a sequence char is present,
                 *  reply the sequence ID.
                 */
                if (buffer[2] == ':') {
                    putDebugChar( buffer[0] );
                    putDebugChar( buffer[1] );

                    /* remove sequence chars from buffer */
                    count = KLstringLength(buffer);
                    for (i=3; i <= count; i++)
                        buffer[i-3] = buffer[i];
                }
            }
        }
    } while (checksum != xmitcsum);
}

struct outfrag {
    const unsigned char *data;
    unsigned int len;
    int hexify;
};

static void
putfrags (const struct outfrag *frags, unsigned int nfrags)
{
    /*  $<packet info>#<checksum>. */
    do {
#define SEND(ch) do { putDebugChar(ch); checksum += (ch); } while (0)
        const struct outfrag *f;
        unsigned char checksum = 0;

        /*
         * Packets begin with '$'.
         */
        putDebugChar('$');

        for (f = frags; f < &frags[nfrags]; ++f) {
            const unsigned char *p = f->data;

            while (f->len ? (p < f->data + f->len) : (*p != '\0')) {
                /*
                 *  See if we have a run of more than three
                 *  of the same output character.
                 */
                if ((f->len == 0 ||
                    &p[f->hexify ? 1 : 3] < f->data + f->len)
                    && p[1] == p[0] && /* Two equal bytes. */
                    (f->hexify
                     /*
                      *  When hexifying, each input byte
                      *  turns into two output bytes, so we
                      *  will have four output bytes if the
                      *  two nibbles of this byte match.
                      */
                     ? ((*p >> 4) == (*p & 0xf))
                     : (p[2] == *p && p[3] == *p))) {
                    /*
                     * Run-length encode sequences of
                     * four or more repeated characters.
                     */
                    unsigned const char *q, *limit;
                    unsigned int rl;

                    if (f->hexify)
                        q = &p[2], limit = &p[98 / 2];
                    else
                        q = &p[4], limit = &p[98];

                    if (f->len && limit > f->data + f->len)
                        limit = f->data + f->len;

                    while (q < limit && *q == *p)
                        ++q;

                    if (f->hexify) {
                        rl = (q - p) * 2;
                        SEND(hexchars[*p & 0xf]);
                    } else {
                        rl = q - p;
                        SEND(*p);
                    }

                    SEND('*');
                    SEND(29 + rl - 1);
                    p = q;
                } else if (f->hexify) {
                    SEND(hexchars[*p >> 4]);
                    SEND(hexchars[*p & 0xf]);
                    ++p;
                } else {
                    /*
                     * Otherwise, just send
                     * the plain character.
                     */
                    SEND(*p);
                    ++p;
                }
            }
        }

        /*
         *  Packet data is followed by '#' and two hex digits
         *  of checksum.
         */
        putDebugChar('#');
        putDebugChar(hexchars[checksum >> 4]);
        putDebugChar(hexchars[checksum % 16]);

        /*
         * Repeat the whole packet until gdb likes it.
         */
    } while ((getDebugChar() & 0x7f) != '+');
}
#undef SEND

#define putpacket(frag...) do {                                       \
    struct outfrag frags[] = { frag };                                \
    putfrags (frags, sizeof frags / sizeof frags[0]);                 \
} while (0)

void gdb_set_trace_flag(int trace_enable, struct gdb_state *state)
{
    if (trace_enable)
        state->eflags |= 0x100;
    else
        state->eflags &= ~0x100;
}

enum gdb_action
gdb_serial_converse (struct gdb_params *p)
{
    char inbuf[BUFMAX+1];
    unsigned char errchar;
#define PUTERR(code)    do { errchar = (code); goto errpacket; } while (0)
    enum gdb_action retval = GDB_MORE;

    p->regs_changed = 0;    /* set this when we mutate P->regs */

    /*
     * Start with a spontaneous report to GDB of the arriving signal.
     */
    if(connected)
        goto report;

    do {
        /* Wait for a command from on high */
        getpacket(inbuf);

        /* OK, now we're officially connected */
        connected = 1;

        switch (inbuf[0]) {
        case 'H':    /* set thread for following operations */
        {
            goto unknown;  /* this is for now; elaborate later */

            /*
             *  Pseudocode (to be implemented):
             *
             *  IF isOK(thread_id)
             *  THEN
             *      switchTo(thread_id);
             *      copyRegs(thread_id,p->regs);
             *      p->thread_id = thread_id;
             *      p->regs_changed = 0;
             *  ENDIF
             *  goto ok;
             */
        }

        case 'T':    /* verify thread is alive */
        {
            goto unknown;  /* this is for now; elaborate later */
        }

        case 'g':    /* return the value of the CPU registers */
        {
            /* Send it over like a memory dump.  */
            putpacket({(void*)p->regs, sizeof(*p->regs), 1});
            break;
        }

        case 'G':    /* set the CPU registers - return OK */
        {
            /* Unpack the new register state.  */
            if (hex2bin(&inbuf[1], (void *) p->regs,
                    sizeof(*p->regs)))
                PUTERR(4);

            p->regs_changed = 1;
            goto ok;
        }

        case 'P':    /* set individual CPU register */
        {
            char *eqp;
            unsigned int reg;

            reg = (unsigned int) hex2int (&inbuf[1], &eqp);
            if (eqp == &inbuf[1] || *eqp != '=')
                PUTERR(4);

            /* Check for bogus register number. */
            if ((reg + 1) * sizeof(char *) > sizeof *p->regs
                /* Convert the value and modify the register state. */
                || hex2bin (eqp + 1,
                    (void *) &((char **) p->regs)[reg],
                    sizeof (char *)))
                PUTERR(4);

            /*
             *  Mark that the register state needs to be
             *  stored back into the thread before it continues.
             */
            p->regs_changed = 1;
            goto ok;
        }

        /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
        case 'm':
        {
            unsigned int addr, len;
            char *ptr, *ptr2;

            /*
             *  Read the start address and length.
             *  If invalid, send back an error code.
             */
            addr = (unsigned int) hex2int(&inbuf[1], &ptr);
            if ((ptr == &inbuf[1]) || (*ptr != ','))
                PUTERR (2);
            ptr++;

            len = (unsigned int) hex2int(ptr, &ptr2);
            if ((ptr2 == ptr) || (len*2 > BUFMAX))
                PUTERR (2);

            /*
             *  Copy the data into a kernel buffer.
             *  If a fault occurs, return an error code.
             */
            if (gdb_copyin(addr, inbuf, len))
                PUTERR(3);

            /* Convert it to hex data on the way out.  */
            putpacket({inbuf, len, 1});
            break;
        }

        /* MAA..AA,LLLL: Write LLLL bytes at AA.AA return OK */
        case 'M' :
        {
            unsigned int addr, len;
            char *ptr, *ptr2;

            /*
             *  Read the start address and length.
             *  If invalid, send back an error code.
             */
            addr = (unsigned int) hex2int(&inbuf[1], &ptr);
            if ((ptr == &inbuf[1]) || (*ptr != ','))
                PUTERR(2);
            ptr++;

            len = (unsigned int) hex2int(ptr, &ptr2);
            if ((ptr2 == ptr) || (*ptr2 != ':'))
                PUTERR(2);
            ptr2++;

            if (ptr2 + len*2 > &inbuf[BUFMAX])
                PUTERR(2);

            {
                char outbuf[len];

                /* Convert the hex input data to binary.  */
                if (hex2bin(ptr2, outbuf, len))
                    PUTERR(2);

                /*
                 *  Copy the data into its final place.
                 *  If a fault occurs, return an error code.
                 */
                if (gdb_copyout(outbuf, addr, len))
                    PUTERR(3);
            }
            goto ok;
        }

        /* cAA..AA Continue at address AA..AA(optional) */
        /* sAA..AA Step one instruction from AA..AA(optional) */
        case 'c' :
        case 's' :
        {
            unsigned int new_eip;
            char *ptr;

            /*
             *  Try to read optional parameter;
             *  leave EIP unchanged if none.
             */
            new_eip = (unsigned int) hex2int(&inbuf[1], &ptr);
            if (ptr > &inbuf[1] && new_eip != p->regs->eip) {
                p->regs->eip = new_eip;
                p->regs_changed = 1;
            }

            /* Set the trace flag appropriately.  */
            gdb_set_trace_flag(inbuf[0] == 's', p->regs);

            /* Return and "consume" the signal that caused us to be called. */
            p->signo = 0;
            retval = GDB_CONT;
            break;
        }

        /* CssAA..AA Continue with signal ss */
        /* SssAA..AA Step one instruction with signal ss */
        case 'C' :
        case 'S' :
        {
            unsigned int new_eip;
            int nsig;
            char *ptr;

            /* Read the new signal number.  */
            nsig = hex(inbuf[1]) << 4 | hex(inbuf[2]);
            if ((nsig <= 0) || (nsig >= 32))
                PUTERR (5);

            /*
             *  Try to read optional parameter;
             *  leave EIP unchanged if none.
             */
            new_eip = (unsigned int) hex2int(&inbuf[3], &ptr);
            if (ptr > &inbuf[3] && new_eip != p->regs->eip) {
                p->regs->eip = new_eip;
                p->regs_changed = 1;
            }

            /* Set the trace flag appropriately.  */
            gdb_set_trace_flag(inbuf[0] == 'S', p->regs);

            /* Return and "consume" the signal that caused us to be called. */
            p->signo = nsig;
            retval = GDB_CONT;
            break;
        }

        report:
        case '?':
            putpacket({"S"},{&p->signo,1,1});
            break;

            /*
             *  When threads are implemented, we need to send
             *  a 'T' reply in stead:
             *
             *      KLstringFormat(inbuf, "thread:%lx;",
             *                            (unsigned long int) p->thread_id);
             *      putpacket({"T"},{&p->signo,1,1},{inbuf});
             */

        case 'D':    /* debugger detaching from us */
            connected = 0;
            retval = GDB_DETACH;
            goto ok;

        case 'R':    /* restart the program */
            connected = 0;
            retval = GDB_RUN;
            /*
             * Send no response packet.
             * Once we've ACK'd receiving the restart packet,
             * gdb tries immediately to query the restarted state.
             */
            break;

        case 'k':    /* kill the program */
            connected = 0;
            retval = GDB_KILL;
            /*
             * Send no response packet.
             * Once we've ACK'd receiving the kill packet
             * (in getpacket, above), gdb ignores us.
             */
            break;

        case 'd':    /* toggle debug flag */
            /*
             * This toggles a `debug flag' for this stub itself.
             * If we ever add any debugging printfs, they should
             * be conditional on this.
             */
            gdb_serial_debug = ! gdb_serial_debug;
            goto ok;

#if 0
        /*
         *  Search memory: need a new callback for this.
         *  Don't bother, since as of 970202 gdb doesn't use `t' anyway.
         */
        case 't':    /* search memory */
        {
            oskit_addr_t addr, ptn, mask;
            char *ptr, *ptr2;

            /* Read the start address and length.
               If invalid, send back an error code.  */
            addr = strtoul(&inbuf[1], &ptr, 16);
            if ((ptr == &inbuf[1]) || (*ptr != ':'))
                PUTERR(2);
            ptr++;
            ptn = strtoul(ptr, &ptr2, 16);
            if ((ptr2 == ptr) || (*ptr2 != ','))
                PUTERR(2);
            ptr2++;
            mask = strtoul(ptr2, &ptr, 16);
            if (ptr == ptr2)
                PUTERR(2);

            goto ok;
        }
#endif

#if 0                /* XXX */
        case 'q':        /* generic "query" request */
            /* qOffsets -> Text=%x;Data=%x;Bss=%x */
        case 'Q':        /* generic "set" request */
#endif

        case '!':
            /*
             *  This is telling us gdb wants to use the
             *  "extended" protocol.  As far as I can tell, the
             *  extended protocol is backward compatible, so
             *  this is a no-op and we always grok it.
             */
            /* FALLTHROUGH */

        ok:
            putpacket({"OK"});
            break;

        errpacket:    /* error replies jump to here */
            putpacket({"E"},{&errchar,1,1});
            break;

        default:
        unknown:
            /*
             * Return a blank response to a packet we don't grok.
             */
            putpacket ({""});
            break;
        }
    } while (retval == GDB_MORE);

    return retval;
}

void gdb_serial_signal(int *inout_signo, struct gdb_state *state)
{
    struct gdb_params p = { -1, *inout_signo, state };

    switch (gdb_serial_converse(&p)) {
    case GDB_CONT:
    case GDB_DETACH:
        *inout_signo = p.signo;
        return;
    case GDB_KILL:
    case GDB_RUN:
        CKdebugPrint("gdbstub: terminated by debugger\n");
        break;
    default:
        CKdebugPrint("gdbstub: gdb_serial_signal confused\n");
    }
}

void gdb_serial_exit(int exit_code)
{
    const unsigned char exchar = exit_code;

    if (!connected)
        return;

    /* Indicate that we have terminated.  */
    putpacket({"W"},{&exchar,1,1});
    connected = 0;
}

void gdb_serial_killed(int signo)
{
    const unsigned char exchar = signo;

    if (!connected)
        return;

    /* Indicate that we have terminated.  */
    putpacket({"X"},{&exchar,1,1});
    connected = 0;
}

void gdb_serial_putchar(int ch)
{
    const unsigned char uch = ch;

    if (!connected) {
        putDebugChar(ch);
        return;
    }

    putpacket ({"O"}, {&uch,1,1});
}

void gdb_serial_puts(const char *s)
{
    if (!connected) {
        while (*s) putDebugChar(*s++);
        putDebugChar('\n');
        return;
    }

    putpacket({"O"}, {s,0,1}, {"\n",1,1});
}

int gdb_serial_getchar()
{
    static char inbuf[256];
    static int pos = 0;
    int c;

    /*
     * Unfortunately, the GDB protocol doesn't support console input.
     * However, we can simulate it with a rather horrible kludge:
     * we take a breakpoint and let the user fill the input buffer
     * with a command like 'call strcpy(inbuf, "hello\r")'.
     * The supplied characters will be returned
     * from successive calls to gdb_serial_getchar(),
     * until the inbuf is emptied,
     * at which point we hit a breakpoint again.
     */
    while (inbuf[pos] == 0) {
        inbuf[pos = 0] = 0;
        gdb_breakpoint();    /* input needed! */
    }                /* input needed! */
    c = inbuf[pos++];

    return c;
}


/***************************************************************************/
/* Initialisation / Interrupt stuff                                        */
/***************************************************************************/

/* The debugger stack */
#define STACKSIZE 10000
int remcomStack[STACKSIZE/sizeof(int)];
static int *stackPtr;

/* The register state */
struct gdb_state ts;

/* The trap recovery address */
unsigned long gdb_trap_recover = 0;

/*
 *  In gdb_i386_trap and gdb_i386_err we keep the interrupt
 *  number of the interrupt that occured and the error code
 *  that was pushed onto the stack (if no error, this is 0.)
 *  These are not used by the gdb stub; however, when in GDB,
 *  you can do
 *
 *      print gdb_i386_trap
 *      print gdb_i386_err
 *
 *  in order to get more information on the event that caused
 *  debugging to be initialised.
 */
unsigned int gdb_i386_trap = 0;
unsigned int gdb_i386_err  = 0;

#define CHECK_FAULT(x)        /* Check whether we need to recover   */   \
  asm ("cmpl $0, gdb_trap_recover"); /* from a trap in the GDB code */   \
  asm ("jne mem_fault" x );

#define SAVE_REGS1()          /* Save the registers in the GDB      */   \
  asm ("movl %eax, ts    ");  /* register structure; first store    */   \
  asm ("movl %ecx, ts+4  ");  /* the "normal" registers             */   \
  asm ("movl %edx, ts+8  ");                                             \
  asm ("movl %ebx, ts+12 ");                                             \
  asm ("movl %ebp, ts+20 ");                                             \
  asm ("movl %esi, ts+24 ");                                             \
  asm ("movl %edi, ts+28 ");                                             \
  asm ("popw %ax         ");  /* Retrieve user %ds                  */   \
  asm ("movw %ax,  ts+48 ");  /* Store the segment registers        */   \
  asm ("popw %ax         ");  /* Retrieve user %es                  */   \
  asm ("movw %ax,  ts+52 ");                                             \
  asm ("movw $0, %ax     ");  /* Put the segment padding in %ax     */   \
  asm ("movw %ax,  ts+50 ");  /* ... and pad them                   */   \
  asm ("movw %ax,  ts+54 ");                                             \
  asm ("movw %fs,  ts+56 ");                                             \
  asm ("movw %ax,  ts+58 ");                                             \
  asm ("movw %gs,  ts+60 ");                                             \
  asm ("movw %ax,  ts+62 ");

#define SKIP_ERRCODE()             /* Skip the error code           */   \
  asm ("popl %ebx             ");  /* Else we get wrong %eip etc.   */   \
  asm ("movl %ebx,gdb_i386_err");  /* Save error code for debugging */

#define SAVE_REGS2()          /* Save the registers in the GDB      */   \
  asm ("popl %ebx        ");  /* Get the old %eip off the stack     */   \
  asm ("movl %ebx, ts+32 ");  /* frame and store it                 */   \
  asm ("popl %ebx        ");  /* Get the old %cs                    */   \
  asm ("movl %ebx, ts+40 ");  /* Store it...                        */   \
  asm ("movw %ax,  ts+42 ");  /* ... and pad                        */   \
  asm ("popl %ebx        ");  /* Get the old %eflags off the stack  */   \
  asm ("movl %ebx, ts+36 ");  /* Store it...                        */   \
  asm ("movw %ss,  ts+44 ");  /* Store %ss                          */   \
  asm ("movw %ax,  ts+46 ");  /* ... and pad it                     */   \
  asm ("movl %esp, ts+16 ");  /* Finally, store the stack pointer   */

#define NERR_EXCEPTION(x)                   /* Exception stub        */  \
  extern void __catchException ## x ## ();  /* without error code    */  \
  asm(".text                        ");                                  \
  asm(".globl __catchException" #x "");                                  \
  asm("__catchException" #x ":      ");                                  \
  asm("    pushw %es                ");                                  \
  asm("    pushw %ds                ");                                  \
  asm("    pushw %ss                ");                                  \
  asm("    popw %ds                 ");                                  \
  asm("    pushw %ss                ");                                  \
  asm("    popw %es                 ");                                  \
  CHECK_FAULT("");                                                       \
  SAVE_REGS1();                                                          \
  SAVE_REGS2();                                                          \
  asm("movl $" #x ", %eax           ");                                  \
  asm("jmp remcomHandler            ");

#define ERR_EXCEPTION(x)                    /* Exception stub        */  \
  extern void __catchException ## x ## ();  /* with error code       */  \
  asm(".text                        ");                                  \
  asm(".globl __catchException" #x "");                                  \
  asm("__catchException" #x ":      ");                                  \
  asm("    pushw %es                ");                                  \
  asm("    pushw %ds                ");                                  \
  asm("    pushw %ss                ");                                  \
  asm("    popw %ds                 ");                                  \
  asm("    pushw %ss                ");                                  \
  asm("    popw %es                 ");                                  \
  CHECK_FAULT("_err");                                                   \
  SAVE_REGS1();                                                          \
  SKIP_ERRCODE();                                                        \
  SAVE_REGS2();                                                          \
  asm("movl $" #x ", %eax           ");                                  \
  asm("jmp remcomHandler            ");

NERR_EXCEPTION(0);
NERR_EXCEPTION(1);
NERR_EXCEPTION(2);
NERR_EXCEPTION(3);
NERR_EXCEPTION(4);
NERR_EXCEPTION(5);
NERR_EXCEPTION(6);
NERR_EXCEPTION(7);
ERR_EXCEPTION(8);
NERR_EXCEPTION(9);
ERR_EXCEPTION(10);
ERR_EXCEPTION(11);
ERR_EXCEPTION(12);
ERR_EXCEPTION(13);
ERR_EXCEPTION(14);
NERR_EXCEPTION(16);

  asm(".text                      ");
  asm("remcomHandler:             ");  /* Prepare to handle debug event */
  asm("    movl stackPtr, %esp    ");  /* Move to remcom stack area     */
  asm("    pushl %eax             ");  /* Push exception onto stack     */
  asm("    call gdb_trap          ");  /* Handle the debug event        */
  asm("    movw ts+44, %ss        ");  /* Restore the registers         */
  asm("    movl ts+16, %esp       ");  /* %esp: this gets us off of the */
  asm("    movl ts+4,  %ecx       ");  /* debugger stack automatically  */
  asm("    movl ts+8,  %edx       ");
  asm("    movl ts+12, %ebx       ");
  asm("    movl ts+20, %ebp       ");
  asm("    movl ts+24, %esi       ");
  asm("    movl ts+28, %edi       ");
  asm("    movw ts+52, %es        ");
  asm("    movw ts+56, %fs        ");
  asm("    movw ts+60, %gs        ");
  asm("    movl ts+36, %eax       ");
  asm("    pushl %eax             ");  /* saved eflags                  */
  asm("    movl ts+40, %eax       ");
  asm("    pushl %eax             ");  /* saved cs                      */
  asm("    movl ts+32, %eax       ");
  asm("    pushl %eax             ");  /* saved eip                     */
  asm("    movl ts,    %eax       ");
  asm("    movw ts+48, %ds        ");
  asm("    iret                   ");  /* whoo, let's go !              */

  asm(".text                      ");  /* Code to handle faults during  */
  asm("mem_fault_err:             ");  /* data copying                  */
  asm("    addl $4,%esp           ");  /* Discard saved %ds / %es       */
  asm("    popl %eax              ");  /* Skip the error code           */
  asm("    movl %eax,gdb_i386_err ");  /* We can clobber regs, we're    */
  asm("    jmp 1f                 ");  /* Skip to generic code          */
  asm("mem_fault:                 ");  /* going back to GDB code anyway */
  asm("    addl $4,%esp           ");  /* Discard saved %ds / %es       */
  asm("1:  popl %eax              ");  /* Get %eip                      */
  asm("    movl gdb_trap_recover,%eax");  /* Change to recovery code    */
  asm("    pushl %eax             ");  /* Restore %eip on stack         */
  asm("    movl $0, %eax          ");  /* Zero recovery routine         */
  asm("    movl %eax,gdb_trap_recover");
  asm("    iret                   ");  /* Go !!!!!                      */

void gdb_trap(int trapno)
{
    int sigval;

    gdb_i386_trap = trapno;

    /* Convert the x86 trap code into a generic signal number.  */
    /* XXX some of these are probably not really right.         */
    switch (trapno) {
//      case -1 : sigval = 2;  break; /* hardware interrupt          */
        case 0  : sigval = 8;  break; /* divide by zero              */
        case 1  : sigval = 5;  break; /* debug exception             */
        case 3  : sigval = 5;  break; /* breakpoint                  */
        case 4  : sigval = 16; break; /* into instruction (overflow) */
        case 5  : sigval = 16; break; /* bound instruction           */
        case 6  : sigval = 4;  break; /* Invalid opcode              */
        case 7  : sigval = 8;  break; /* coprocessor not available   */
        case 8  : sigval = 7;  break; /* double fault                */
        case 9  : sigval = 11; break; /* coprocessor segment overrun */
        case 10 : sigval = 11; break; /* Invalid TSS                 */
        case 11 : sigval = 11; break; /* Segment not present         */
        case 12 : sigval = 11; break; /* stack exception             */
        case 13 : sigval = 11; break; /* general protection          */
        case 14 : sigval = 11; break; /* page fault                  */
        case 16 : sigval = 7;  break; /* coprocessor error           */
        default : sigval = 7;         /* "software generated"        */
    }

    /* Call the GDB stub to do its thing.  */
    gdb_serial_signal(&sigval, &ts);

    if(sigval) {
        CKdebugPrint("gdbstub: whoops, gdb_serial_signal returned signal\n");
        for(;;);
    }

    gdb_i386_err = 0;

    return;
}

void set_debug_traps()
{
    /* Setup debugger stack */
    stackPtr  = &remcomStack[STACKSIZE/sizeof(int) - 1];

    /* Setup exception handlers */
    exceptionHandler (0,  __catchException0);  /* Divide by zero      */
    exceptionHandler (1,  __catchException1);  /* Debug exception     */
    exceptionHandler (2,  __catchException2);  /* NMI                 */
    exceptionHandler (3,  __catchException3);  /* Breakpoint          */
    exceptionHandler (4,  __catchException4);  /* INTO overflow       */
    exceptionHandler (5,  __catchException5);  /* BOUNDS check        */
    exceptionHandler (6,  __catchException6);  /* Invalid opcode      */
//  exceptionHandler (7,  __catchException7);  /* FPU exception       */
    exceptionHandler (8,  __catchException8);  /* Double fault        */
    exceptionHandler (9,  __catchException9);  /* Reserved            */
    exceptionHandler (10, __catchException10); /* Invalid TSS         */
    exceptionHandler (11, __catchException11); /* Segment not present */
    exceptionHandler (12, __catchException12); /* Stack exception     */
    exceptionHandler (13, __catchException13); /* General protection  */
    exceptionHandler (14, __catchException14); /* Page fault          */
    exceptionHandler (16, __catchException16); /* Coprocessor error   */
}

void breakpoint()
{
    gdb_breakpoint();
}
