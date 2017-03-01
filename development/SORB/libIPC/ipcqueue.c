/*
 * Implememntation of the IPC queues
 *
 * Implementation of the functions to manage the message queues used for
 * 'page swapping' IPC.
 *
 * HISTORY
 * Date          Author Rev Notes
 * Oct,11th,1998 Luc    1   Uses fixed-length, circular queues
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <typewrappers.h>
#include "ipcqueue.h"

INT16 IPCQueueWrite (IPCQueue* queue, INT32 shmId) {
  /* Adds an element to the queue. If it was a full queue, the older datum is
   * overwrote and lost. For faster execution, only a pointer to the queue is
   * passed, instead of copying the structure,
   * INPUT:
   *  queue : pointer to the queue to which the element should be added
   *  shmId : shared memory identifier to be added
   * OUTPUT
   *  Zero on success (always?) a negative number otherwise
   */

  queue->shmemId[queue->write]=shmId;
  queue->write= (queue->write +1) % QUEUESIZE;
  if (queue->write == queue->read)
    queue->read = (queue->read +1) % QUEUESIZE;
  return 0;
}

INT32 IPCQueueRead (IPCQueue* queue) {
  /* Reads the first element in the queue returning and discarding it.
   *  For faster execution, only a pointer to the queue is  passed, instead of
   * copying the structure,
   * INPUT:
   *  queue : pointer to the queue from where the element is to be read
   * OUTPUT:
   *  The shmId stored in the first element of the queue, or -1 if there is
   * no meesage queued.
   */
  if (queue->read != queue->write) { /* not an empty queue */
    queue->read = (queue->read + 1) % QUEUESIZE;
    return (queue->shmemId[(queue->read-1) % QUEUESIZE]);
  } else return -1;

}
