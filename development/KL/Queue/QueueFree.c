/*
 * Queue library definitions
 *
 * Deallocates all data previously allocated for queue init
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 01/12/98 scosta    1.0    First draft
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

extern Queue *_qTable;
extern UWORD32 _maxQueues;

PUBLIC BOOLEAN KLqueueFree(INPUT Qhandle qId)

/*
 * Free resources allocated by EKqueueAlloc()
 *
 * INPUT:
 * qId    the handle of the queue to be freed
 *
 * OUTPUT:
 * TRUE if the deallocation is completed correctly, FALSE otherwise
 */
	
{
	Queue *qToBeDeallocated;

	/* Be sure that we don't access a inexistant array element */

	if(qId<_maxQueues) {
        qToBeDeallocated= &_qTable[qId];

		/* If there are no packets defined for this queue, it's
		 * meaningless to deallocate.                           */

		if(qToBeDeallocated->maxPackets) {
			/* When the Alliance equivalent of malloc/free will be 
			 * implemented, should be checked for correct deallocation,
			 * and from the result return correctly. Now we hardcode
			 * always TRUE for free, but it's not (always) the case    */

	  		if(qToBeDeallocated->buffer!=NIL) {	  
				KLfree(qToBeDeallocated->buffer);
			}
			
			if(qToBeDeallocated->packetPtr!=NIL) {
				KLfree(qToBeDeallocated->packetPtr);
			}
			
			if(qToBeDeallocated->packetSize!=NIL) {
				KLfree(qToBeDeallocated->packetSize);
			}
			
			qToBeDeallocated->currentSize=0;
			qToBeDeallocated->maxSize=0;
			qToBeDeallocated->maxPackets=0;
		
			return(TRUE);
		} else {
			return(FALSE);
		}
	} else {
		return(FALSE);
	}
}

