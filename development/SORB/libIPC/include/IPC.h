/*
 * Definition of the 'page swapping' IPC interface
 *
 * Types, constants and functions for 'page swapping' IPC
 *
 * HISTORY
 * Date        Author Rev Notes
 * Oct, 9,1998 Luc     1  First version
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <typewrappers.h>
#include <processes.h>

#ifndef __IPC_H
#define __IPC_H

struct IPCBufferIdStruct {
  INT32     shmemId;  /* SysV identifier for the shared memory */
  BYTE*     addr;     /* Pointer to the first byte after the header */
  ProcessId owner;    /* Owner of the buffer */
};

typedef struct IPCBufferIdStruct IPCBufferId;
 
#define IPCNOBUFFER (IPCBufferId){(INT32)0x0, (BYTE*)NIL}
#define isIPCNOBUFFER(x) (((x).shmemId==0)&&((x).addr==NIL) ? TRUE : FALSE)
/*Returns address of 1st usable byte*/
#define ipcBufferGetAddr(x) ((x).addr)

/* sender allocates the buffer */
PUBLIC IPCBufferId ipcBufferAllocate(WORD32 size); 

/* sender sends the message and waits for the receiver to process it */
PUBLIC INT16 ipcBufferSendSync(IPCBufferId buffer, ProcessId target);

/* sender sends the message and continues execution */
PUBLIC INT16 ipcBufferSendAsync(IPCBufferId buffer, ProcessId target);

/* receiver gets the first message queued or waits for an incomming message */
PUBLIC IPCBufferId ipcBufferGetSync(VOID);

/* receiver gets the first message queued or continues execution */
PUBLIC IPCBufferId ipcBufferGetAsync(VOID);

/* Receiver notifies the sender after processing,  the sender can read the
 * response in the same buffer  
 * When both sender and receiver issue ipcBufferDone, the buffer is freed */
PUBLIC INT16 ipcBufferDone(IPCBufferId buffer);

#endif


