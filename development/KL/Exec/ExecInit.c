/*
 * Executable Loader Library
 *
 * This module contains the one-time-in-the-life init of Alliance OS
 * executable loader library.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/06/00 scosta    1.0    First draft
 * 29/12/02 scosta    1.1    Added KLexecUnInit()
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

PUBLIC UWORD32 _maxExecutables=0;
PUBLIC ExecutableTable *_eTable=NIL;

PUBLIC BOOLEAN KLexecInit(INPUT UWORD32 numOfExecutables)

/*
 * Initialize the Executable table
 *
 * INPUT:
 * numOfExecutables		the maximum number of loaded executables
 *
 * RETURN:
 * TRUE					if all is ok
 * FALSE				if something is wrong
 */

{
	UBYTE *p;
	UWORD32 executableTableSize;

	/* Checks if the loaded exewcutable table is already initted 
	 * (i.e. if KLexecInit() was already called before).          */

	if(_eTable==NIL) {
		executableTableSize=numOfExecutables*sizeof(ExecutableTable);
		p=(UBYTE *) KLmalloc(executableTableSize);
		if(p!=NIL) {
		    /* Init the allocated space for executable table to a known state */

			KLmemorySet(p, 0, executableTableSize);
			_maxExecutables=numOfExecutables;
			_eTable=(ExecutableTable *) p;
			return(TRUE);
		} else {
			return(FALSE);
		}
	} else {
		return(FALSE);
	}
}

PUBLIC VOID KLexecUnInit()

/*
 * Orderdly deallocation of Executable Loader library
 *
 * INPUT:
 * None
 *
 * RETURN:
 * None
 */

{
	if(_eTable!=NIL) {
		KLfree(_eTable);
	}

	_eTable=NIL;
}
