/*
 * Queue module library
 *
 * In this file are defined all data structure needed to handle queueing.
 * This file contains ONLY symbol definitions needed by the library
 * itself, and it is NOT needed by a generic Queue Library user.
 * A generic user MUST use eklibs.h only.
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

typedef struct {
	UWORD32 maxPackets;  /* Maximum number of outstanding packets */
	UWORD32 maxSize;     /* Memory allocated for the queue */	
	UWORD32 currentSize; /* Amount of memory used so far in queue */
	UWORD32 r;           /* Queue rear index */
	UWORD32 f;           /* Queue front index */
	UBYTE *buffer;       /* Pointer to the actual queue buffer */
	UBYTE *packetPtr;    /* Table of pointers to individual packets */
	UWORD32 *packetSize; /* Table of packet sizes */
} Queue;

typedef enum {
	ByCopy,       /* Copy the data associated to the packet in queue */   
	ByReference   /* Simply use a pointer to packet data in queue */
} QueueFlag;

