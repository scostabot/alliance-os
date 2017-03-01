/*
 * Executable Loader Library
 *
 * This function do all things needed to release memory claimed by executable
 * loading and do housekeeping.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/06/00 scosta    1.0    First draft
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>

#include "exec.h"

extern UWORD32 _maxExecutables;
extern ExecutableTable *_eTable;

PUBLIC BOOLEAN KLexecClose(INPUT EHandle eId)

/*
 * Release all data claimed for the executable pointed by eId
 *
 * INPUT:
 * eId		handle of the executable
 *
 * RETURN:
 * The state of the closing process: TRUE or FALSE
 */

{
	/* Reject trivial error case, like impossible descriptors */

	if(eId>=_maxExecutables) {
		return(FALSE);
	}

	if(_eTable[eId].exec==NIL) {
		return(FALSE);
	}

	if(_eTable[eId].type==elf32) {
		ELFclose(_eTable[eId].exec);
	}

	_eTable[eId].type=none;

	/* Release memory associated to the executable loaded */

	KLfree(_eTable[eId].exec);
	_eTable[eId].exec=NIL;

	return(TRUE);
}

