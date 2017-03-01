/*
 * Executable Loader Library
 *
 * This module tests if the file is really an executable and that it is
 * in a format supported by the library.
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

PUBLIC ProbeStatus KLexecProbe(INPUT EHandle eId,
							   ExecType *type,
							   INPUT ExecRecord *record)

/*
 * Probe if the portion of executable stored in <record> is valid
 *
 * INPUT:
 * eId			the executable identifier given by KLexecOpen()
 * record		the pointer to a memory buffer that holds requested data 
 *
 * OUTPUT:
 * type			the type of probed executable (none or elf)
 *
 * RETURN:
 * the probe status (one of the following):
 * EXEC_PROBE_NOTOK,EXEC_PROBE_INVID,EXEC_PROBE_NOT_SUPPORTED,
 * EXEC_PROBE_NONE,EXEC_PROBE_OK
 *
 */

{
	Elf32_Ehdr *elfHeader;

	/* Is the executable Id a valid one? */

	if(eId >= _maxExecutables) {
		return(EXEC_PROBE_INVID);
	} else {
		if(_eTable[eId].exec==NIL) {
			return(EXEC_PROBE_INVID);
		}
	}

	/* For now, we act as ELF is THE only executable format supported,
	 * by all the stuff conceived under the Executable Loader Library
	 * was crafted to support multiple executable formats, given some
	 * changes here and there. Keep in mind when doing patches!        */

	elfHeader=(Elf32_Ehdr *) record->buffer;
	
	/* Check ELF signature */
	
	if(elfHeader->e_ident[EI_MAG0]!=ELFMAG0 ||
		elfHeader->e_ident[EI_MAG1]!=ELFMAG1 ||
		elfHeader->e_ident[EI_MAG2]!=ELFMAG2 ||
		elfHeader->e_ident[EI_MAG3]!=ELFMAG3) {
			return(EXEC_PROBE_NOTOK);
	}

	/* Reject abnormally formed executables */

	if(elfHeader->e_machine==EM_NONE) {
	    return(EXEC_PROBE_NOTOK);
	}

	if(elfHeader->e_ident[EI_DATA]==ELFDATANONE) {
		return(EXEC_PROBE_NOTOK);
	}

    /* Look for ELF data format class: we support only 32-bit ELF for now */

	if(elfHeader->e_ident[EI_CLASS]!=ELFCLASS32) {
	    return(EXEC_PROBE_NOT_SUPPORTED);
	}

	/* Look for platform-specific issues about executable format support */

	if(cpuDependantCheck(elfHeader)==TRUE) {
    	/* The next step is load the section headers, if they are present */

    	if(elfHeader->e_shoff) {
			/* Init the first stage of the finite state machine */
		  
			_eTable[eId].fsm.state=saveElfHeader;

			/* Mark the executable as ELF in executable table and to caller */

			_eTable[eId].type= *type =elf32;
		} else {
			return(EXEC_PROBE_NOTOK);
		}
	} else {
	  	*type=none;
		_eTable[eId].fsm.state=elfInvalid;
		return(EXEC_PROBE_NONE);
	}

	return(EXEC_PROBE_OK);	
}

