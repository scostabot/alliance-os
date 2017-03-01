/*
 * Interface for managing the IPC queues
 *
 * Uses INT32 (shmemIds) elements as the data on the queue, defines the index
 * type for the queue and the basic functions (isFull, Init, Write and Read)
 *
 * HISTORY
 * Date          Author Rev Notes
 * Oct,11th,1998 Luc    1   Uses fixed length, circular queues
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <typewrappers.h>
#include <string.h>  /* for memset() */
#ifndef __IPCQUEUE_H
#define __IPCQUEUE_H

#define NOPROCESS 0   /* pid for no process owning a queue */

#ifndef QUEUESIZE
#define QUEUESIZE 250 /* max queued messages, configurable at compile
time */
#endif

#if QUEUESIZE < 0xFF
typedef UBYTE QIndex; /* 8 bit indexes */
#else
#if QUEUESIZE >= 0xFFFF
#error Size of message queue is too big.
#endif /* error checking */ 
typedef UWORD16 QIndex; /* 16 bit indexes */
#endif /* QIndex type definition */

struct IPCQueueStruct {
  QIndex read;
  QIndex write;
  INT32  shmemId[QUEUESIZE+1];
};

typedef struct IPCQueueStruct IPCQueue;

/* Functions and macros */
#define IPCQueueIsFull(q) ((((q)->Write==(q)->Read-1) || \
                            (((q)->Read==0) && ((q)->Write==QUEUESIZE))) ? \
                           TRUE : FALSE)
/* Returns TRUE if the parameter is a full circular queue
 * INPUT
 *  q : a pointer to a queue
 * OUTPUT
 *  TRUE if *q is a full queue, FALSE otherwise
 */

#define IPCQueueInit(q) {(q)->read=0; (q)->write=0; \
                         memset((q)->shmemId,0,(QUEUESIZE+1)*sizeof(QIndex));}
/* Initilizes the IPCQueue* argument to an empty queue
 * INPUT
 *  q : a pointer to a (hopefully uninitialized) queue
 * OUTPUT
 *  None
 */


/* The following could also be macros... */
PUBLIC INT16 IPCQueueWrite (IPCQueue* queue, INT32 shmId);

PUBLIC INT32 IPCQueueRead (IPCQueue* queue);

#endif
