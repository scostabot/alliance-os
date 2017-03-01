/*
 * demo.c:  Linux alliance-emulation messaging library demo
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

/* Linux includes */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* Alliance includes */
#include <typewrappers.h>
#include "msg.h"

/* Global variables */
extern pid_t atthread;
MsgChannel chan;
MsgBuf buf;


VOID sigHand(DATA signal, DATA extra, pid_t caller)
/*
 * NOTE:  keep in mind that due to the limitations of the linux
 *        emulation, this is sometimes more simplistic than it
 *        can be.  SIGNLSHMRDY is simplified by the fact that we
 *        only have one message channel open.
 *        OTOH, it is slightly complicated because we have to pass
 *        the signal sender pid on to the messaging library.
 */
{
    switch(signal) {
        case SIGNLSHINVI:              /* Shmem invite signal           */
            atthread = caller;         /* Mark who we're talking to     */
            chan = KLmsgAccept(extra); /* Accept the invitation         */
            break;

        case SIGNLSHDEAD:              /* Shmem connect broken          */
            atthread = 0;              /* Stop communicating            */
            KLmsgDisconnect(chan);     /* Disconnect from the channel   */
            printf("demo:  channel owner closed connection\n");
            exit(0);

        case SIGNLSHMRDY:              /* Shmem message ready           */
            if(!extra)                 /* Break if this is a reject     */
                break;                 /* message, or an invalid one    */
            if((VOID *)extra == chan)  /* If it is an accept msg break  */
                break;                 /* and let the lib handle it     */
            if(!buf)                   /* We got a message! record its  */
                buf = (VOID *)extra;   /* location in memory if we're   */
            else                       /* waiting for one, else lose it */
                printf("demo:  lost incoming asynchronous message\n");
            break;                     /* location and return to app.   */

        default:
            printf("demo:  unknown signal %d... bug ?\n", (int)signal);
            exit(255);
    }
}


VOID server(VOID)
{
    initSignalEmu(sigHand);        /* Initialise the signal emulation   */
    buf = NIL;                     /* Mark no message ready             */
    for(;;) {                      /* Main server loop                  */
        waitSignal();              /* Wait for an incoming message      */
        if(!buf)                   /* A signal arrived, but is it the   */
            continue;              /* message ready signal ?            */

        printf("serv:  message received `%s'\n", buf->data);   /* Yup ! */

        strcpy(buf->data, "OK");   /* Reuse message buffer for reply    */
        KLmsgSendAsync(chan, buf); /* Send reply                        */

        buf = NIL;                 /* We finished handling this message */
    }
}


VOID client(pid_t srv)
{
    MsgBuf new;

    initSignalEmu(sigHand);        /* Initialise the signal emulation   */
    buf = NIL;                     /* Mark no message ready             */
    sleep(1);                      /* Sleep 1 second to give server the */
                                   /* time to initialise                */
    chan = KLopenMsgChannel(0);    /* Create a new message channel      */
    KLmsgConnect(chan, srv);       /* Connect to the server (assume ok) */
    new = KLallocMsgBuf(chan,10);  /* Allocate a message buffer         */
    strcpy(new->data,"Hi World!"); /* Write message into buffer         */
    KLmsgSendSync(chan, new);      /* Send message to server            */

    if(buf)
        printf("clnt:  reply message received `%s'\n", buf->data);
    else
        printf("clnt:  did not get reply from server\n");

    KLcloseMsgChannel(chan);       /* Close the connection with server  */
}


int main(VOID)
{
    pid_t child;

    if((child = fork())) {
        client(child);
        wait(NULL);
    } else {
        server();
    }

    return 0;
}
