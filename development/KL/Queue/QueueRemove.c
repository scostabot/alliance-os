/*
 * Queue module library 
 *
 * Read a packet from a generic queue.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 06/12/98 scosta    1.0    First draft
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

extern Queue *_qTable;        /* The table of queues control data */
extern UWORD32 _maxQueues;    /* Number of entries of the above table */

PUBLIC QueueState KLqueueRemove(INPUT Qhandle qId, Packet *packet)

/*
 * Remove a packet from the queue
 * 
 * INPUT:
 * qId     The handle of the queue to be read
 * 
 * OUTPUT:
 * packet  The pointer to the data packet
 * size    The size of the data packet
 * 
 * The function returns Q_OK if the data is valid, otherwise Q_UNDERFLOW
 * 
 */
	
{
	Queue *queue;
	UBYTE **ptrPacket;

	/* Be sure that we don't access a inexistant array element */

	if(qId<_maxQueues) {
	  
		queue=&_qTable[qId];
		
		/* Is a valid queue? */
		
		if(queue->maxPackets==0) {
		    return(Q_NOQUEUE);
		}

		/* Test underflow condition before proceed to read */

		if(queue->f==queue->r) {
			return(Q_UNDERFLOW);
		}

		/* Queues are implemented as a circular list: check for wrapping */

		if(queue->f==queue->maxPackets-1) {
			queue->f=0;
		} else {
			queue->f++;
		}

		/* Output packet data to caller */

		packet->size= queue->packetSize[queue->f];
		packet->data=(UBYTE *) ptrPacket[queue->f];

		return(Q_READOK);
	} else {
		return(Q_NOQUEUE);
	}
}
