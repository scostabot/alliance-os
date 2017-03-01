/*
 * Queue library definitions
 *
 * Initialize a queue memory space and data structures.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 17/11/98 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>    /* Public interface for all queue routines */
#include <qdefs.h>    /* Implementation-dependant symbols for queues */

extern Queue *_qTable;

PUBLIC Qhandle KLqueueAlloc(INPUT UWORD32 queueSize, INPUT UWORD32 maxPackets)
  
/*
 * Init a queue with given characteristics.
 * 
 * INPUT:
 * queueSize    The maximum size in bytes for all outstanding
 *              packets in this queue. If zero, it is assumed
 *              that the data that will be inserted/removed from
 *              the queue is limited to a pointer that reference
 *              actual data. If nonzero, actual data will be
 *              copied in the queue buffer when a packet is inserted.
 * maxPackets   The maximum number of packets that this queue
 *              can handle
 * 
 * RETURN:
 * An unique handle that identify the queue. Valid handles are those
 * that are less than QVALID_HANDLE. If the returned handle is equal
 * or greater than this value, then the call is failed.
 * 
 * KLqueueFree must be called when this function return a valid handle.
 * 
 */
  
{
	UWORD32 computedSize;
	Queue *qToBeSetup;
	Qhandle qId;

 	/* Don't allow meaningless cases: zero-packets */

	if(maxPackets==0) {
		return((Qhandle) QNO_PACKETS);
	}	

	/* It is theoretically possible to init the queue:get a valid handle */
	 
	qId=0;
	while(_qTable[qId].maxPackets!=0) {
	    qId++;
	}

	qToBeSetup= &_qTable[qId];

	/* Init the qId queue data structure to a correct starting state */
	
	qToBeSetup->maxPackets=maxPackets+1; /* Max number of outstanding packets */
	
	/* Init the queue buffer if data needs to be copied in the queue,
	 * otherwise init an array of pointers to queue packet data.      */
	
	if(queueSize!=0) {
		/* This is the actual size of queue data buffer */
	     
		qToBeSetup->buffer=(UBYTE *) KLmalloc(queueSize);
		qToBeSetup->packetPtr=NIL;
	} else {
		/* There is no queue buffer */
	  
		qToBeSetup->buffer=NIL;

		/* Compute the number of bytes needed to store pointers to
		 * actual packet data inside the queue buffer.
		 * Since the last element in the array is used by queue
		 * insertion routine as placeholder, allocate space for
		 * one extra element.                                    */
	  
		computedSize=sizeof(UBYTE *)*(maxPackets+1);
		qToBeSetup->packetPtr=(UBYTE *) KLmalloc(computedSize);
	}
		
	/* Compute the number of bytes needed to store packet sizes */
		
	computedSize= sizeof(UWORD32)*qToBeSetup->maxPackets;
	qToBeSetup->packetSize= (UWORD32 *) KLmalloc(computedSize);
		
	/* Initially the queue is empty, so buffer is empty too */

	qToBeSetup->currentSize=0;

	return(qId);
}

