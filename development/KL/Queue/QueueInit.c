/*
 * Queue module library
 *
 * Init the queueing environment
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 07/12/98 scosta    1.0    First draft
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

PUBLIC UWORD32 _maxQueues=0;
PUBLIC Queue *_qTable=NIL;

PUBLIC BOOLEAN KLqueueInit(INPUT UWORD32 numOfQueues)
  
/*
 * Init the queue library. Should be called prior to any
 * other Queue function.
 * 
 * INPUT:
 * NumOfQueues   The maximun number of queues that the app will use
 * 
 * OUTPUT:
 * TRUE if all goes well, FALSE otherwise.
 */

{
	UBYTE *p;
	UWORD32 queueAllocatedSize;

	/* For now, this is a one-time-in-the-life call that allocate
	 * memory for control structures for queues. At a later stage,
	 * when the real memory manager is in place, we may allow
	 * to call this func whenever you want to change the total
	 * number of queues do you want to use, providing additional
	 * argument.                                                   */
 
	if(_qTable==NIL) {
		if(numOfQueues==0) {
			return(FALSE);
		}

		queueAllocatedSize=numOfQueues * sizeof(Queue);

		p=(UBYTE *) KLmalloc(queueAllocatedSize);

		if(p!=NIL) {
		 	KLmemorySet(p, 0, queueAllocatedSize); 
			_qTable=(Queue *) p;
			_maxQueues=numOfQueues;
			return(TRUE);
		} else {
			return(FALSE);
		}
	} else {
		return(FALSE);
	}
}

