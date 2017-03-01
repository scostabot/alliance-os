/*
 * ELF linker library
 *
 * Search and compute address of the ELF program entry point
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 04/05/00 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>      /* The basic Kernel Library definitions */
#include <elf.h>        /* The ELF file format structures definitions */
#include "elfloader.h"	/* The ELF file loader specific definitions */

LOCAL WORD32 (*entryPoint)(); /* The ELF program entry point.
                                 We run it as a pointer to a function that
                                 returns an integer.                        */

PUBLIC BOOLEAN setELFEntryPoint(ELF *elfFile, 
                               INPUT UWORD32 numOfELFFiles,
	                       INPUT STRING *entryPointSymbolicName)
  
/*
 * Set the entry point for the dynalinked program.
 * 
 * INPUT:
 * entryPointSymbolicName   the string that contains the entry point symbolic name.
 * 							
 * RETURN:
 * TRUE if the symbol is present in the symbol table of previously
 * loaded files, FALSE if not.
 */
  
{
	UWORD32 targetFile;
	UWORD32 targetIndex;
	Elf32_Sym *sym;

	/* Does exist a defined procedure named as input in the 
	 * previously loaded ELF files?                         */

	sym=searchSymbol(&targetFile, &targetIndex, elfFile, numOfELFFiles,
	                 ANY_FILE, entryPointSymbolicName, STT_FUNC);
	if(sym!=NIL) {
		UBYTE *addr;
		UWORD32 textSection;
		
		/* We have found it, so we gather where code is and compute
		 * the symbol physical address.                              */

		textSection=searchTextSection(elfFile);
		addr=(UBYTE *) elfFile[targetFile].sectionData[textSection];
		addr+=sym->st_value;
		
		entryPoint=(WORD32 (*)()) addr;
		
		/* The above method of address resolution works if and only if
		 * the symbol given as entry point is defined, i.e., already
		 * resolved as ELF symbol, like the main() procedure in a
		 * C object file.                                             */

		return(TRUE);
	} else {
		/* The entry point symbolic name does not exists, so we exit
		 * with an error code.                                       */

		return(FALSE);
	}
}

PUBLIC WORD32 runELFFile(VOID)
  
/*
 * Pass CPU to the loaded ELF file executing the routine pointed 
 * by a previous call to function setELFEntryPoint().
 * 
 * INPUT:
 * None
 * 
 * OUTPUT:
 * None
 */
  
{
	return((*entryPoint)());	
}

