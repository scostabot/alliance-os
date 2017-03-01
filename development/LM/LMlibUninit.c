/*
 * Alliance OS LM library
 *
 * Uninitialize the LM library
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 04/05/03 scosta    1.0    First draft
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

extern LMTable *_lmTable;

PUBLIC VOID LMlibUninit(VOID)

/*
 * Unload from memory all data structures allocated for LM library
 *
 * INPUT:
 * None
 *
 * RETURNS:
 * None
 */

{
	if(_lmTable!=NIL) {
		KLfree(_lmTable);
		_lmTable=NIL;
	}
}
