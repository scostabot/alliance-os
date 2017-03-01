/*
 * Alliance OS LM library
 *
 * Unload an LM module from memory
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

extern UWORD32 _maxLMs;
extern LMTable *_lmTable;

PUBLIC VOID LMunload(INPUT LMHandle lmId)

/*
 * Unload the LM pointed by given id
 *
 * INPUT:
 * lmId     the id of LM to be unloaded from memory
 *
 * RETURNS:
 * None
 */

{
	/* Is the given id within valid range? */

	if(lmId<_maxLMs) {
		/* If so, really there is a loaded LM with given id? */

		if(_lmTable[lmId].usage) {
			_lmTable[lmId].usage--;
			/* Before unloading, call (if exists) the LMunInit function */

			if(_lmTable[lmId].entryPoints[1]!=NIL) {
				(VOID) (*_lmTable[lmId].entryPoints[1])();
			}
		}
	}
}
