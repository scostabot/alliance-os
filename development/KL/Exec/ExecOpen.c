/*
 * Executable Loader Library
 *
 * Get a valid handle for loading an executable
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

PUBLIC EHandle KLexecOpen(ExecRecord *p)

/*
 * Open the executable for reading
 *
 * INPUT:
 * p		the pointer to the executable record that the caller must provide
 *
 * OUTPUT:
 * p		a new request of data in the executable record
 *
 * RETURN:
 * the handle. The handle is valid if is less than EVALID_HANDLE, otherwise
 * an error is occurred. Defined errors are:
 *
 * ENO_EXECTABLE	there is no Executable Table (i.e. KLexecInit wasn't called)
 * ENO_MOREEXECS	the Executable Table is full.
 *
 */

{
	EHandle eId;
	ELF *pELF;
	
	/* Reject trivial cases */

	if(p==NIL) {
		return((EHandle) ENO_EXECTABLE);
	}

	/* Get a valid handle */

	eId=0;
	while(_eTable[eId].exec!=NIL) {
		eId++;
	}

	/* Check if we have reached the upper limit */

	if(eId>=_maxExecutables) {
		return((EHandle) ENO_MOREEXECS);
	}

	/* Allocate space for ELF executable management in the executable table */

	_eTable[eId].exec=(ELF *) KLmalloc(sizeof(ELF));
	if(_eTable[eId].exec==NIL) {
		return((EHandle) ENO_EXECTABLE);
	}
	pELF=(ELF *) _eTable[eId].exec;

	/* Set the executable loader finite state machine status */
	
	_eTable[eId].fsm.state=saveElfHeader;
	_eTable[eId].fsm.iterator=0;

	/* Since KLexecProbe() is an optional call, we assume here that the
	 * executable is of type elf32. If user call KLexecProbe an active
	 * check on executable format will be done, otherwise no check.    */

	_eTable[eId].type=elf32;

	/* The caller must provide to the library the header of the executable */

	p->position=0;
	p->size=sizeof(Elf32_Ehdr);
	p->buffer=(UBYTE *) &pELF->header;

	return(eId);
}
