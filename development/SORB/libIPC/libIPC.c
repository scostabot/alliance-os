/*
 * Implementation of 'page swapping' IPC under Linux
 *
 * This file hides the details of IPC (queuing messages, managing semaphores
 * for sleeping/awaking processes, filling the message header...) from the user
 *
 * HISTORY
 * Date        Author Rev Notes
 * Oct, 9,1998 Luc    1   For use in the SORB prototype.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */


#include <IPC.h>
#include <processes.h>
#include "ipcqueue.h"
#include "libIPC.h"  /* local definitions for the library */
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

/* Definition of the global array of queues */

#define MAXIPCQUEUES 5   /* randomly chosen ***
			  * must by 8-bit to work *** */
#define IDSTRING "~/IPC/SharedMemory" /* string to get the shmem identifier */

struct ipcQueueStruct {
  ProcessId owner; /* proces id of owner of the queue */
  INT32     semId; /* SysV id for the receiver's semaphore */
  IPCQueue  queue; /* The queue itself   */
} * ipcQueues; /* will be set to an array of MAXIPC queues in the sh memory */

DATA * users; /* this will hold the number of programs using the library */

/*
 *  Private functions for managing the public structure of queues 
 */

BYTE ipcBufferFindEntry(ProcessId pid) {
  /* Finds the entry in the IPCQueues list for the current process, if it does
   * not exists, then create it.
   * INPUT:
   *   None
   * OUTPUT
   *   The entry. If there is no free entry, notify and exit.
   */
  UBYTE     i;
  BYTE      firstFree=-1;

  /* search for an assigned entry */
  for (i=0; (i<MAXIPCQUEUES)&&(ipcQueues[i].owner!=pid); i++) {
    if ((ipcQueues[i].owner==NOPROCESS) && (firstFree==-1))
      firstFree=i; /* update the first free entry cache */
  }
  if (i<MAXIPCQUEUES) /* we have found the entry */
    return i; /* return it */
  else { /* this pid has not an ipc queue, se we create one */
    if (firstFree != -1) { /* there is a free entry in the cache */
      ipcQueues[firstFree].owner=pid;
      return firstFree;  /* return it */
    }
    else{ /* Ooops! */
      printf ("libIPC: No more free IPC buffers, exiting...\n");
      exit (1);
      return -1; /* just to avoid a warning at compile time */
    }
  }
}

/*
 *  Public functions exprted by the header file <IPC.h> 
 */

IPCBufferId ipcBufferAllocate(WORD32 size) {
  /* Allocates the required number of pages for the given size and attaches
   * them to the current process.
   * INPUT:
   *  size  :  number of bytes to be allocated
   * OUTPUT
   *  An IPCBuffer identifier (needed for other functions), IPCNOBUFFER on
   * error
   */
  IPCBufferId      result;  /* temporary var to build the resturned value */
  MsgHeader *      header;  /* simplifies the reading of the code */ 
  union semun      semInfo; /* sysV information of a semaphore array */
  BYTE             error=0; /* error value */

  size += HEADERSIZE; /* total size = data + header */
  result.shmemId = shmget (IPC_PRIVATE, size, IPC_CREAT|0666); /*rw for all*/
  if (result.shmemId != -1) { 
    /* shmget suceed, now attach the pages to the current process */
    result.addr=shmat (result.shmemId, 0, 0);
    if (result.addr != (BYTE*)(-1)) {
      /* shared memory successfully attached */
      header=(MsgHeader *) result.addr;
      /* create the semaphore for the sender */
      header->senderSem = semget (IPC_PRIVATE, 1, IPC_CREAT|0666);
      semInfo.val=0; /* no messages queued in the semaphore */
      if ((header->senderSem!=-1) &&
	  (semctl (header->senderSem, 0, SETVAL, semInfo) !=-1)) {
	/* semaphore created and initialized */
	/* Now the header has been filled */
	result.owner = getpid();
	result.addr += HEADERSIZE; /* skip the header */
      } else error = -4;  /* error creating the semaphore */
    } else error = -2;    /* shmat failed */
  } else error = -1;      /* shmget failed */

  switch (error) {
  case -4 : /* could not create & initialize the semaphore */
  case -2 : /* shared memory attachment failed */
    shmctl(result.shmemId, IPC_RMID, NODESC); /* destroy the shared memory */
  case -1 : /* Shared memory creation failed */
    return IPCNOBUFFER;       /* return an error */
    break;
  default :
  }
  return result;
}
  
INT16 ipcBufferSendSync(IPCBufferId buffer, ProcessId target) {
  /* Puts the pages of buffer in the queue of the target process and waits
   * for it to read them.
   * INPUT:
   *  buffer : Identifier of the ipc buffer containing the outgoing message
   *  target : PID of the receiver of the message
   * OUTPUT:
   *  Zero on success, a negative number if any error ocurred.
   */
  MsgHeader * header = (MsgHeader *)(buffer.addr - HEADERSIZE);
  UBYTE       entry  = ipcBufferFindEntry(target);
  struct sembuf  semOps;

  semOps.sem_num=  0; /* operate on the first semaphore */
  semOps.sem_op = +1; /* add 1 to the semaphore value */
  semOps.sem_flg=  0; /* No flags */
 
  /* sender has to process the result */
  header->senderDone = FALSE;
  /* receiver has to process the message */
  header->receiverDone = FALSE;
  /* add the message to the receiver's queue */
  IPCQueueWrite (&(ipcQueues[entry].queue), buffer.shmemId);
  /* Awake the receiver */
  if (semop(ipcQueues[entry].semId, &semOps, 1)==-1) /* error */
    return -1; /* could not awake the receiver */
  /* Now sleep the sender in the semaphore */
  semOps.sem_op = -1; /* modify first semaphore in -1, with no flags */
  if (semop(header->senderSem, &semOps, 1)==0) /* ok */
    return 0;
  else
    return -2; /* could not sleep */
}

INT16 ipcBufferSendAsync(IPCBufferId buffer, ProcessId target) {
  /* Puts the pages of buffer in the queue of the target process and continues
   * INPUT:
   *  buffer : Identifier of the ipc buffer containing the outgoing message
   *  target : PID of the receiver of the message
   * OUTPUT:
   *  Zero on success, a negative number if any error ocurred.
   */
  MsgHeader * header = (MsgHeader *)(buffer.addr - HEADERSIZE);
  UBYTE       entry  = ipcBufferFindEntry(target);
  struct sembuf  semOps;

  semOps.sem_num=  0; /* operate on the first semaphore */
  semOps.sem_op = +1; /* add 1 to the semaphore value */
  semOps.sem_flg=  IPC_NOWAIT; /* Do not block */
 
  /* sender has to process the result */
  header->senderDone = FALSE;
  /* receiver has to process the message */
  header->receiverDone = FALSE;
  /* add the message to the receiver's queue */
  IPCQueueWrite (&(ipcQueues[entry].queue), buffer.shmemId);
  /* Awake the receiver */
  if (semop(ipcQueues[entry].semId, &semOps, 1)==-1) /* error */
    return -1; /* could not awake the receiver */
  /* Now modify the sender's semaphore w/o sleeping*/
  semOps.sem_op = -1; /* modify first semaphore in -1, with no flags */
  if (semop(header->senderSem, &semOps, 1)==0) /* ok */
    return 0;
  else
    return -2; /* could not sleep */
}

IPCBufferId ipcBufferGetSync(VOID) {
  /* Gets the identifier of the first IPC buffer in the caller's queue. If no 
   * buffer is queued, waits for the first to arrive
   * INPUT:
   *  Nothing
   * OUTPUT:
   *  The IPC buffer identifier of the first queued message or IPCNOBUFFER if 
   * an error ocurred
   */

  UBYTE       entry  = ipcBufferFindEntry(getpid()); /* find caller's queue */
  IPCBufferId buffer;
  struct sembuf  semOps;

  semOps.sem_num=  0; /* operate on the first semaphore */
  semOps.sem_op = -1; /* add 1 to the semaphore value */
  semOps.sem_flg=  0; /* No flags */
 
  /* Decrement the semaphore and sleep if there is no message (the semaphore
   * acts as a counter */
  if (semop(ipcQueues[entry].semId, &semOps, 1)==-1) {/* error */
    return IPCNOBUFFER; /* could not modify the semaphore */  
  }
  /* read the message from the receiver's queue */
  buffer.shmemId = IPCQueueRead (&(ipcQueues[entry].queue));
  if (buffer.shmemId==-1) {
    return IPCNOBUFFER;
  }
  /* attach the shared memory to the current process */
  buffer.addr=shmat(buffer.shmemId,0,0666);
  if (buffer.addr!=(BYTE*)(-1)) { /* on successfully attach... */
    buffer.addr += HEADERSIZE;
    return buffer;
  } else {
    return IPCNOBUFFER;
  }
}

IPCBufferId ipcBufferGetAsync(VOID) {
  /* Gets the identifier of the first (if any) IPC buffer in the caller's 
   * queue. The receiver can now write to de buffer, and the
   * INPUT:
   *  Nothing
   * OUTPUT:
   *  The IPC buffer identifier of the first queued message or IPCNOBUFFER if
   * there was no messages in the queue
   */
  UBYTE       entry  = ipcBufferFindEntry(getpid()); /* find caller's queue */
  IPCBufferId buffer;
  struct sembuf  semOps;

  semOps.sem_num=  0; /* operate on the first semaphore */
  semOps.sem_op = -1; /* add 1 to the semaphore value */
  semOps.sem_flg=  IPC_NOWAIT; /* Asyncronous */
 
  /* Decrement the semaphore and sleep if there is no message (the semaphore
   * acts as a counter */
  if (semop(ipcQueues[entry].semId, &semOps, 1)==-1) {/* error */
    return IPCNOBUFFER; /* could not modify the semaphore or
			 * semaphore forced a wait */  
  }
  /* read the message from the receiver's queue */
  buffer.shmemId = IPCQueueRead (&(ipcQueues[entry].queue));
  if (buffer.shmemId==-1) {
    return IPCNOBUFFER;
  }
  /* attach the shared memory to the current process */
  buffer.addr=shmat(buffer.shmemId,0,0666);
  if (buffer.addr!=(BYTE*)(-1)) { /* on successfully attach... */
    buffer.addr += HEADERSIZE;
    return buffer;
  } else {
    return IPCNOBUFFER;
  }
}

INT16 ipcBufferDone(IPCBufferId buffer) {
  /* When called by the receiver, detach and notify the sender (if needed).
   * When called by the sender, detach the buffer pages.
   * When both have called this function, destroy the buffer
   * INPUT:
   *  buffer : buffer to be destroyed.
   * OUTPUT:
   *  Zero on success, a negative number on error:
   *  -1 : error with semaphores
   *  -2 : error detaching shared memory
   *  -4 : error destroying shared memory
   *  or any combitation of them.
   */
  MsgHeader *     header = (MsgHeader *)(buffer.addr - HEADERSIZE);
  struct sembuf   semOps; /* sysV structure for semaphore info. */
  union semun     semInfo;
  ProcessId       pid = getpid();
  INT16           result;
  BOOLEAN         destroy=FALSE;
  
  if ((buffer.owner)!=pid) { /* if the caller is the receiver */
    semOps.sem_num = 0;
    semOps.sem_op  = +1;
    semOps.sem_flg = 0;
    header->receiverDone = TRUE; /* receiver finished processing */
    /* destroy shared memory */
    if ((header->senderDone == TRUE) && (header->receiverDone == TRUE))
      destroy = TRUE;
    if (header->senderSem != 0) /* the semaphore exists */
      result = semop (header->senderSem, &semOps, 1);
    else
      result = 0;
    /* detach shared memory */ 
    if (shmdt (header)!=0)
      result -= 2; /* something went gone */

  } else  {/* if the caller is the sender */
    header->senderDone = TRUE; /* sender finished processing */
    /* destroy shared memory */
    if ((header->senderDone == TRUE) && (header->receiverDone == TRUE))
      destroy = TRUE;
    if (/* destroy the semaphore */
	semctl (header->senderSem, 0, IPC_RMID, semInfo)==0) {
      header->senderSem=0;
      result = 0;
    } else
      result = -1;
    
    if (/* detach shared memory */
	shmdt  (header)!=0)
      result -= -2; /* Could not detach shared memory */
  }
  if ((destroy == TRUE) &&
      (shmctl(buffer.shmemId, IPC_RMID, NODESC) < 0))
    result -= -4;
  return result;
}

void _init (void) {
  /* Function to initilize the global queue array, called by the dynamic linker
   * after loading the library.
   * INPUT:
   *  None
   * OUTPUT:
   *  None
   */
  UBYTE           i;
  union semun     semInfo;
  INT32           shmId;
  struct shmid_ds description;
  BYTE            errno;
  
  /* get a reference to the shared memory with the list of queues */
  shmId = shmget (ftok(IDSTRING,'A'),
		  sizeof(struct ipcQueueStruct)*MAXIPCQUEUES+sizeof(INT32),
		  IPC_CREAT|0666);
  /* attach the shared memory */
  ipcQueues = (struct ipcQueueStruct *)(shmat(shmId, 0, 0666) + sizeof(INT32));
  users = ((DATA *) ipcQueues) - 1;

  errno =(shmId==0?-1:0)  + (ipcQueues==(struct ipcQueueStruct*)(-1)?-1:0);
  /* get the shmem info */
  errno +=shmctl (shmId, IPC_STAT, &description);

  if ((description.shm_cpid&0xFFFF)==getpid()) { /*if we are the creators... */
    /* Init the queues ... */
    for (i=0; i<MAXIPCQUEUES; i++) {
      ipcQueues[i].owner = NOPROCESS;        /* Queue has no owner */
      IPCQueueInit(&(ipcQueues[i].queue));  /* Init the queue     */
      
      /* create and init the receiver's semaphore */
      ipcQueues[i].semId = semget (IPC_PRIVATE, 1, IPC_CREAT|0666);
      semInfo.val=0; /* no messages queued in the semaphore */
      if ((ipcQueues[i].semId==-1) ||
	  (semctl (ipcQueues[i].semId, 0, SETVAL, semInfo) ==-1)) {
	/* Error, could not create and init the semaphore... */
	printf ("libIPC: Could not create and init semaphores, exiting...\n");
	exit (2);
      }
    }
  } else; /* everything should be already initialized by other process */
  (*users)++;
}
    
void _fini (void) {
  /* Destroys all allocated semaphores, executed automatically by the dynamic
   * linker when the library is unloaded
   * INPUT:
   *  none.
   * OUTPUT
   *  none.
   */
  UBYTE           i;
  union semun     semInfo;
  INT32           shmemId;
  struct shmid_ds description;

  /* get the shared memory reference */
  shmemId = shmget (ftok(IDSTRING, 'A'), 0, 0666);

  /* if this is the last process using IPC, free the list of queues */
  if (*users==1) {
    for (i=0; i<MAXIPCQUEUES; i++) /* remove the semaphores */
      semctl (ipcQueues[i].semId, 0, IPC_RMID, semInfo);
    /* mark the shared memory as deleted, will be freed on exit */
    shmctl (shmemId, IPC_RMID, &description);
  }
  (*users)--;
  /* the segment is detached automatically on exit */
}
