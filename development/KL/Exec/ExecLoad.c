/*
 * Executable Loader Library
 *
 * This module takes care of actual executable loading in memory
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
extern UWORD32 _bssSection;

LoadStatus KLexecLoad(INPUT EHandle eId, ExecRecord *record)

/*
 * Load an executable in memory 
 *
 * INPUT
 * eId		executable identifier
 * record	contains the chunk of executable data fetched by caller
 *
 * OUTPUT:
 * record	the size and position of the next chunk of data to be read
 */

{
	ELF *pELF;
	LoadStatus nextFsmAction;
	FsmResult state;

	/* Is the executable Id a valid one? */

	if(eId >= _maxExecutables) {
		return(EXEC_LOAD_INVID);
	} else {
		if(_eTable[eId].type==none) {
			return(EXEC_LOAD_INVID);
		}
	}

	/* For now, we act as ELF is THE only executable format supported, */
	/* Finite state machine code */

	nextFsmAction=EXEC_LOAD_AGAIN; /* Otherwise noted, another transition */

	switch(_eTable[eId].fsm.state) {
		case saveElfHeader:
			pELF=(ELF *) _eTable[eId].exec;
				
			/* Now that we are reasonably sure that it is a 
			 * valid executable, initialize the executable table
			 * with the new entry.                                */

			(VOID) KLmemoryCopy(pELF, record->buffer, record->size);
		  	if(ELFSectionHeaderProcessing(pELF, record)==nextState) {
				_eTable[eId].fsm.state=elfSectionNames;
			} else {
				_eTable[eId].fsm.state=elfInvalid;
			}
			break;

		case elfSectionNames:
			pELF=(ELF *) _eTable[eId].exec;
				
			if(ELFSectionNamesProcessing(pELF, record)==nextState) {
				_eTable[eId].fsm.state=elfSection;
			} else {
				_eTable[eId].fsm.state=elfInvalid;
			}
			break;
	
		case elfSection:
			pELF=(ELF *) _eTable[eId].exec;
			
			state=ELFSectionProcessing(pELF, record, &_eTable[eId].fsm);
			if(state==stopIteration) {
				_eTable[eId].fsm.state=postLoad;
			} else {
				if(state!=continueIteration) {
					_eTable[eId].fsm.state=elfInvalid;
				}
			}
			break;

		case postLoad:
			pELF=(ELF *) _eTable[eId].exec;

			if(pELF->section[_bssSection].sh_size==0) {
				ELFsizeAndAssignMemToBSS(pELF);
			}

			if(ELFresolveRelocations(pELF)==nextState) {
				_eTable[eId].fsm.state=finalState;
			} else {
				_eTable[eId].fsm.state=elfInvalid;
			}

			record->buffer=NIL;
			break;

		case finalState:
			nextFsmAction=EXEC_LOAD_FINISHED;
			break;
			
		case elfInvalid:	
		default:
			nextFsmAction=EXEC_LOAD_ERROR;
	}

	return(nextFsmAction);
}
