/*
 * Executable Loader Library
 *
 * Dynamically resolve symbol references which are define externally
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 13/12/03 scosta    1.0    First draft
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

PUBLIC UWORD32 KLexecSetSymbols(INPUT EHandle eId, 
                                INPUT Esymbol symbol[], 
                                INPUT UBYTE *symbolAddr[])

/*
 * Define a list of symbols names and addresses that will be used as base
 * to resolve undefined symbols in the executable pointed by eId.
 *
 * INPUT:
 * eId              the executable identifier
 * symbol[]         a table of structures Esymbol that holds the
 *                  symbol(s) names (up to MAX_SYMBOL_NAME chars)
 *                  and symbol semantics (EXEC_FUNC if the requested
 *                  symbol identifies a function, or EXEC_VAR if the 
 *                  symbol identifies a variable).
 * symbolAddr[]     the table of addresses that points to the symbols
 *                  defined above
 *
 * RETURNS:
 * The number of symbols resolved
 */

{
	ELF *pELF;

	if(eId >= _maxExecutables) {
		return(EXEC_SYMBOL_INVID);
	} else {
		if(_eTable[eId].exec==NIL) {
			return(EXEC_SYMBOL_INVID);
		}
	}

	pELF=(ELF *) _eTable[eId].exec;

	return(ELFsetSymbols(pELF, symbol, symbolAddr));
}

