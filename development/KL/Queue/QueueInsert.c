/*
 * Queue handling module
 *
 * In this module are defined the functions to insert/remove a packet
 * in a queue.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 30/11/98 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>
#include <qdefs.h>

extern Queue *_qTable;          /* The table of queues control data */
extern UWORD32 _maxQueues;      /* Number of entries of the above table */

PUBLIC QueueState KLqueueInsert(INPUT Qhandle qId,
								INPUT Packet *packet)
	
/*
 * Insert a packet in the given queue
 * 
 * INPUT:
 * qId	   The queue where insert the packet
 * packet  The data to be inserted in the queue
 * 
 * RETURN:
 * On successfull completion Q_WRITEOK, otherwise Q_OVERFLOW or Q_NOQUEUE
 */
	
{
	UWORD32 upperLimit;
	UWORD32 sizeWithThisPacket;
	UBYTE **packetPtr;
	Queue *queue;

	/* Be sure that we don't access a inexistant array element */

	if(qId<_maxQueues) {	
	  
		queue=&_qTable[qId];

		/* Is really a valid queue? */
		
		if(queue->maxPackets==0) {
		    return(Q_NOQUEUE);
		}

		/* Queue is implemented as a circular list: check for wrapping */

		upperLimit=queue->maxPackets-1;
	  
		if(queue->r==upperLimit) {
			queue->r=0;
		} else {
			queue->r++;
		}

		/* If an overflow is occurred, don't insert the packet and return */

		if(queue->r==queue->f) { 
			if(queue->r) {
				queue->r--;
			} else {
				queue->r=upperLimit;
			}

			return(Q_OVERFLOW);
		}
	
		/* There are two distinct ways to insert data in a queue:
		 * copy physically it inside the queue buffer or put in the
		 * queue the pointer to the actual data to be put in queue.
		 * If maxsize is greater than zero, we must copy data.     */

		if(queue->maxSize) {
			sizeWithThisPacket=queue->currentSize+packet->size;
			if(sizeWithThisPacket>queue->maxSize) {
				return(Q_WRITEERR);
			}

			queue->currentSize=sizeWithThisPacket;
		} else {
			packetPtr=(UBYTE **) queue->packetPtr;
			packetPtr[queue->r]=(UBYTE *) packet->data;
		}

		queue->packetSize[queue->r]=packet->size;

		/* Insert the packet in queue buffer. If the queue was
		 * allocated by reference, insert in the buffer only the
		 * pointer to packet data, and save packet size.          */

		return(Q_WRITEOK);
	} else {
		return(Q_NOQUEUE);
	}
}
