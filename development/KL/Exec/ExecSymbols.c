/*
 * Executable Loader Library
 *
 * Fetch from the executable pointed by the id the given symbols addresses.
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

PUBLIC UWORD32 KLexecSymbols(INPUT EHandle eId,
                             INPUT Esymbol symbol[], 
                             UBYTE *symbolAddr[])

/*
 * Get the addresses of the given symbol names in the executable 
 * pointed by eId.
 *
 * INPUT:
 * eId              the executable identifier
 * symbol[]         a table of structures Esymbol that holds the
 *                  symbol(s) names (up to MAX_SYMBOL_NAME chars)
 *                  and symbol semantics (EXEC_FUNC if the requested
 *                  symbol identifies a function, or EXEC_VAR if the 
 *                  symbol identifies a variable).
 *
 * OUTPUT:
 * symbolAddr       the addresses of the symbol requested. If the
 *                  n-th symbol is not found in the executable loaded,
 *                  the n-th entry of the table is set to NIL
 *
 * RETURNS:
 * the number of symbols defined in the executable
 *
 */

{
	UWORD32 i;
	UWORD32 symbolsFound=0;
	ELF *pELF;
	
	if(eId >= _maxExecutables) {
		return(EXEC_SYMBOL_INVID);
	} else {
		if(_eTable[eId].exec==NIL) {
			return(EXEC_SYMBOL_INVID);
		}
	}

	pELF=(ELF *) _eTable[eId].exec;

	/* Compute how much symbols are defined in the executable
	 * file loaded by KLexecLoad() call, and return to caller. */

	i=0;
	while(symbol[i].name[0]!='\0') {
		symbolAddr[i]=(UBYTE *) ELFsearchSymbol(pELF, &symbol[i]);
		if(symbolAddr[i]!=NIL) {
			symbolsFound++;
		}

		i++;
	}
	
	return(symbolsFound);
}

