/*
 * Executable Loader Library
 *
 * In this module are kept all those things that are specific to 80X86 CPUs
 * support in executables.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/06/00 scosta    1.0    First draft
 * 25/04/03 scosta    1.1    Finalized relocs code
 * 26/05/03 scosta    1.2    Now it is possible to load ELF without code
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

PUBLIC BOOLEAN cpuDependantCheck(INPUT Elf32_Ehdr *elfHeader)

/*
 * Makes all CPU-dependant checks needed in order to probe executableness
 *
 * INPUT:
 * elfHeader		the pointer to ELF header
 *
 * RETURN:
 * TRUE if the ELF executable matches our target environment, FALSE otherwise.
 */

{
	/* At this time our only need is to check if the CPU matches our target */

	switch(elfHeader->e_machine) {
		case EM_386:
		case EM_486:
			return(TRUE);
		default:
			return(FALSE);
	}
}

PUBLIC VOID relocateSymbol(INPUT ELF *elf, 
                           INPUT Elf32_Sym *symbol, 
                           INPUT Elf32_Rel *pRelData)
  
/*
 * Relocate given symbol.
 *
 * INPUT:
 * elf       the pointer to the ELF file structures
 * symbol    the pointer to the symbol to be relocated
 * pRelData  the pointer to the relocation data entry for the above symbol
 * 
 * OUTPUT:
 * None
 */
  
{
	extern UWORD32 _textSection;

	UWORD32 relocation;
	UWORD32 targetSection;
	UWORD32 *machineWord;
	UWORD32 textSection;
	WORD32 targetOffset;
	UBYTE *symbolAddrInMemory;
	UBYTE *targetAddrInMemory; 

	/* It is possible that an ELF executable contains no code
	   to be executed but only data definitions (like an empty LM).
	   We check this condition so we avoid to reference NULL pointers. */

	if(elf->section[_textSection].sh_size==0) {
		return;
	}

	/* Step 1: compute the address in the ELF file image of the 
	 * symbol to be relocated. We assume that the symbol is contained
	 * in the .text section of that ELF file.                          */

	symbolAddrInMemory=elf->sectionData[_textSection];
	symbolAddrInMemory+=pRelData->r_offset;
	
	/* Step 2: compute the symbol relocation. */
				
	switch(ELF32_R_TYPE(pRelData->r_info)) {
		case R_386_32:
			/* Data offset relocation entry, i.e. the addresses we
			 * are talking about are used by a MOV, AND, XOR, NOT..
    			 * and on opcodes. These are simple relative offsets
    			 * in Intel family processors for this memory model. */

		 	machineWord=(UWORD32 *) symbolAddrInMemory;

			/* Normally st_shndx holds the index to the section
			 * that contains the symbol data, but when this
			 * field contains an obviously invalid number we
			 * treat it as a reference to the .text section.     */
			
			if(symbol->st_shndx<elf->header.e_shnum) {
				targetSection=symbol->st_shndx;
			} else {
				targetSection=textSection;
			}

			targetAddrInMemory=(UBYTE *) elf->sectionData[targetSection];	
			targetAddrInMemory+=symbol->st_value;
			relocation=(UWORD32) targetAddrInMemory;

			/* Additional undocumented thing: in the file another "relocation"
			 * info is present in the ELF file executable image. Add whatever
			 * we have computed with relocation table with what is contained 
			 * in the target section.                                          */

			*machineWord=(UWORD32) relocation + *machineWord; 
			break;

	  	case R_386_PC32:
			/* Program counter relocation entry, i.e. we are talking
			   about relative addresses used in CALL, JMP....        */

			machineWord=(UWORD32 *) symbolAddrInMemory;
			targetSection=textSection;
			targetOffset = (symbol->st_value-pRelData->r_offset-4);	
			relocation=(UWORD32) targetOffset;
			*machineWord=(UWORD32) relocation;
			break;

			/* Post scriptum: please note that there are for Intel
			 * 80X86 CPUs also other relocation types, namely 
			 * R_386_GOT32, R_386_PLT32, R_386_COPY, R_386_GLOB_DAT
			 * and others.In my experience these entries are present
			 * only if you link as shared library the compiled
			 * object. Since in my dynalinking scheme we don't use
			 * this option, but I allow trasparent linking of all
			 * symbols in any case, I think that these entries are
			 * useless in our environment.
			 * This is good because they are practically not 
			 * documented ;).                                        */
	}

			
}
