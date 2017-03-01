/*
 * signal.c:  Alliance valued signal emulation on top of Linux
 *
 * (C) 1999 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 08/05/99  ramon       1.0    First release
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

/* Use BSD-style signals */
#define __USE_BSD_SIGNAL

/* Linux includes */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/* Alliance includes */
#include <typewrappers.h>

/* Constant definitions */
#define ALLOSSIGQ 0xdeaf
#define SIGMSGTYP 0xdeaf
#define MSGDATASZ sizeof(struct sigdata)


/* Data structures */
struct sigdata {
    DATA  signal;
    DATA  extra;
    pid_t caller;
};

struct amsgbuf {
    DATA type;
    struct sigdata signal;
};


/* Global variables */
LOCAL VOID  (*sigHand)(DATA signal, DATA extra, pid_t caller);
LOCAL DATA  sigmsgq;
LOCAL pid_t whoami;


LOCAL VOID unixSigHand(int dummy)
/*
 * The UNIX signal handler that is used to receive alliance signals
 *
 * INPUT:
 *     none used
 *
 * RETURNS:
 *     none
 */
{
    struct amsgbuf msgbuf;

    /*
     * Try to get a message with the alliance signal parameters for
     * this signal.
     */

    if(msgrcv(sigmsgq, (struct msgbuf *) &msgbuf, MSGDATASZ, 0, MSG_NOERROR) == -1) {
        fprintf(stderr, "sigemu:  signal message could not be received\n");
        exit(255);
    }

    /*
     * Check signal message for validity
     */

    if(msgbuf.type != SIGMSGTYP) {
        fprintf(stderr, "sigemu:  received invalid message\n");
        exit(255);
    }

    /*
     * Let the `alliance signal handler' handle the signal
     */

    sigHand(msgbuf.signal.signal, msgbuf.signal.extra, msgbuf.signal.caller);
}


PUBLIC VOID sendSignal(pid_t thread, DATA signal, DATA extra)
/*
 * Function used to send an alliance signal to a thread
 *
 * INPUT:
 *     thread:  UNIX pid of thread to send signal to
 *     signal:  Signal number to send
 *     extra:   The signal parameter
 *
 * RETURNS:
 *     none
 */
{
    /*
     * The signal message buffer is used to transfer the alliance
     * signal parameters to the receiving UNIX signal handler
     */

    struct amsgbuf msgbuf = { SIGMSGTYP, { signal, extra, whoami } };

    /*
     * Put the alliance signal data in an IPC message and enqueue it
     * on the signal queue
     */

    if(msgsnd(sigmsgq, (struct msgbuf *) &msgbuf, MSGDATASZ, 0)) {
        fprintf(stderr, "sigemu:  signal message could not be sent\n");
        exit(255);
    }

    /*
     * Now send the receiving thread a UNIX signal to wake up its
     * alliance signal emulation signal handler.  The UNIX signal
     * sending call is called kill(). Braindead...
     */

    kill(thread, SIGUSR1);
}


PUBLIC VOID waitSignal(VOID)
/*
 * Function used to block until a signal is received
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    /* Wait for a UNIX signal to come in */
    pause();
}


PUBLIC VOID initSignalEmu(VOID *handl)
/*
 * Function used to initialise the UNIX alliance signal emulation
 *
 * INPUT:
 *     handl:  The alliance signal handler address for this thread
 *
 * RETURNS:
 *     none
 */
{
    /* Get our own pid and remember it */
    whoami = getpid();

    /* Set up the Alliance emulated signal handler location */
    sigHand = handl;

    /* Set up the handler for receiving UNIX signals */
    signal(SIGUSR1, unixSigHand);

    /* Initialise the message queue used to transmit signal parameters */
    sigmsgq = msgget(ALLOSSIGQ, 0777 | IPC_CREAT /* | IPC_EXCL */);
}

