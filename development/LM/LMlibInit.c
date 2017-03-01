/*
 * Alliance OS LM library
 *
 * Initialize LM library
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 26/04/03 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>    
#include <lm.h>

#include "lmsys.h"

PUBLIC UWORD32 _maxLMs=0;
PUBLIC LMTable *_lmTable=NIL;

PUBLIC BOOLEAN LMlibInit(INPUT UWORD32 numOfLMs)

/*
 * Initialize LM library data structures
 *
 * INPUT:
 * numOfLMs   the maximum number of loadable LMs by the library
 *
 * RETURNS:
 * TRUE if all is OK, FALSE otherwise
 */

{
	UBYTE *p;
	UWORD32 lmTableAllocatedSize;

	if(_lmTable==NIL) {
		lmTableAllocatedSize=numOfLMs*sizeof(LMTable);
		p=(UBYTE *) KLmalloc(lmTableAllocatedSize);

		if(p!=NIL) {
			KLmemorySet(p, 0, lmTableAllocatedSize);
			_lmTable=(LMTable *) p;
			_maxLMs=numOfLMs;
			KLexecInit(numOfLMs);
			return(TRUE);
		} else {
			return(FALSE);
		}
	} else {
		return(FALSE);
	}
}
