/*
 * Local include for the IPC library
 *
 * Types and constants used by the IPC library internally. Also, #includes all
 * the required files.
 *
 * HISTORY
 * Date          Author Rev Notes
 * Oct, 9th,1998 Luc    1   For use with the SORB prototype
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <typewrappers.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <IPC.h>

#ifndef __LIBIPC_H
#define __LIBIPC_H

/* IPC Message header related definitions */
struct MsgHeaderStruct {
  BOOLEAN   senderDone;  /* sender has terminated using the buffer */
  BOOLEAN   receiverDone;/* receiver has terminated using the buffer */
  INT32     senderSem;   /* identifier of the SysV semaphore where the sender
			  * sleeps */
};
typedef struct MsgHeaderStruct MsgHeader;
#define HEADERSIZE sizeof(MsgHeader)

/* Other definitions*/
#define NODESC (struct shmid_ds *) (NIL)
#endif





