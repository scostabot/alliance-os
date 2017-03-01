/*
 * ELF executable loader library
 *
 * All stuff needed to link different ELF files toghether is defined here
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 29/10/00 scosta    1.0    First draft
 * 15/12/03 scosta    1.1    Added support for external symbols link
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

extern UWORD32 _textSection;
extern UWORD32 _symTabSection;
extern UWORD32 _relocTableSection;

PUBLIC UBYTE * ELFsearchSymbol(INPUT ELF *elf, 
                               INPUT Esymbol *symbol)

/*
 * Fetch symbol location in memory of the loaded file pointed by elf.
 *
 * INPUT:
 * elf		The ELF header structure of the file to be searched
 * symbol	The symbol definition to be searched
 *
 * RETURN:
 * The address of the symbol in memory. If not found NIL.
 */

  
{
	UWORD32 i;
	UWORD32 numOfEntries;
	UBYTE *symbolAddr=NIL;
	STRING *pLinkedSec;
	Elf32_Word linkedSection;
	Elf32_Section targetIndex;
	Elf32_Sym *pSectionData;
	Elf32_Sym *symbolEntry;

	/* Get the address of the String Table that holds the
	 * ASCIIZ names of all symbols entries present in the
	 * Symbol Table.                                      */
					
	linkedSection=elf->section[_symTabSection].sh_link;
	pLinkedSec=(STRING *) elf->sectionData[linkedSection];
	pSectionData=(Elf32_Sym *) elf->sectionData[_symTabSection];

	/* Now scan all entries and check if there is a 
	 * symbol named as our source symbol.             */
				
	numOfEntries=elf->section[_symTabSection].sh_size/elf->section[_symTabSection].sh_entsize;

	for(i=0;i<numOfEntries;i++) {
	 	if(KLstringCompare(symbol->name, 
			               &pLinkedSec[pSectionData[i].st_name])==0) {
			int symbolType;
						
			/* Check if the symbol named as the one we are
			 * looking for also points to the kind of object 
			 * that we want to know of, otherwise skip.      */
						
			symbolType=ELF32_ST_TYPE(pSectionData[i].st_info);
						
	/*		if(symbol->type==EXEC_FUNC && symbolType!=STT_FUNC) {
				continue;
			}

			if(symbol->type==EXEC_VAR && symbolType!=STT_OBJECT) {
				continue;
			}*/
						
			/* A definition of the symbol was found. 
			 * Return the pointer of the symbol in memory */

			symbolEntry= &pSectionData[i];

			/* Another undocumented and empirically-found
			 * feature: the st_shndx is a negative number
			 * if the symbol table item defines a target
			 * defined in the .text section of the ELF file
			 * where the symbol table belongs to.
			 * Otherwise points (as defined) to the section
			 * that contains the reference.                  */
						  
			if(symbolEntry->st_shndx>elf->header.e_shnum) {
				targetIndex=_textSection;
			} else {
				targetIndex=symbolEntry->st_shndx;
			}

			symbolAddr=elf->sectionData[targetIndex]+symbolEntry->st_value;
			break;
		} 
	}

	return(symbolAddr);
}

LOCAL UWORD32 linkToExternalReference(INPUT ELF *elf, 
                                      INPUT Elf32_Sym *symbolEntry, 
                                      INPUT UWORD32 symbolAddr)

/* 
 * Resolve the symbol reference assigning the address symbolAddr to given
 * symbol entry
 *
 * INPUT:
 * elf            the ELF file control structure
 * symbolEntry    the symbol entry in ELF Symbol Table that needs to be linked
 * symbolAddr     the symbol address that is valid for linking 
 *
 * RETURN:
 * the number of references linked
 *
 */

{
	UWORD32 i;
	UWORD32 numOfSymbolsLinked=0;
	UWORD32 numOfEntries;
	Elf32_Sym *symbolTable;
	Elf32_Rel *pRelData;

	/* We start linking process scanning the Relocation Table entries */

	pRelData=(Elf32_Rel *) elf->sectionData[_relocTableSection];
	symbolTable=(Elf32_Sym *) elf->sectionData[elf->section[_relocTableSection].sh_link];

	/* Compute how much symbols are defined in this section */

	numOfEntries=elf->section[_relocTableSection].sh_size/elf->section[_relocTableSection].sh_entsize;
	
	/* Now we look for a match in the Relocation Table with the symbol
	 * given in input of this function.                                */
		
	for(i=0;i<numOfEntries;i++, pRelData++) {
		UWORD32 symbolIndex;
		Elf32_Sym *symbol;

		symbolIndex=ELF32_R_SYM(pRelData->r_info);
		symbol=&symbolTable[symbolIndex];

		/* If the relocation entry points to the same symbol defined 
		 * in input, then we have found the entry to be linked.      */
		
		if(symbol->st_name==symbolEntry->st_name) {
			UBYTE *symbolAddrInMemory;
			UWORD32 *machineWord;
			WORD32 relocation;
			WORD32 temporary;

			/* We have relocation offset of the symbol to be linked.
			   Compute logical address in memory                     */  

			symbolAddrInMemory= elf->sectionData[_textSection];
			symbolAddrInMemory+=pRelData->r_offset;
			/* *symbolAddrInMemory=symbolAddr; */

            machineWord=(UWORD32 *) symbolAddrInMemory;
			if(machineWord>symbolAddr) {
				temporary=(symbolAddr-machineWord)-4;
			} else {
				temporary=(symbolAddr-machineWord)-4;
			}
			
			*machineWord=temporary;

			/* relocateSymbol(elf, symbolEntry, pRelData); */
			numOfSymbolsLinked++;
		}
	}

	return(numOfSymbolsLinked);
}


PUBLIC UWORD32 ELFsetSymbols(INPUT ELF *elf, 
                             INPUT Esymbol *symbol,
                             INPUT UBYTE *symbolAddr[])

/*
 * Set a list of symbols to be considered as globally defined in ELF linking
 *
 * INPUT:
 * elf          The ELF header structure of the file to be linked with
 *              the list of given symbols
 * symbol       The list of symbols globally defined
 * symbolAddr   The symbols addresses in memory
 *
 * RETURNS:
 * The number of symbols actually linked in the given executable
 *
 */

{
	UWORD32 i;
	UWORD32 resolved=0;
	UWORD32 numOfEntries;
	STRING *pLinkedSec;
	Elf32_Sym *pSectionData;
	Elf32_Word linkedSection;

	/* Get the address of the String Table that holds the
	 * ASCIIZ names of all symbols entries present in the
	 * Symbol Table.                                      */
					
	linkedSection=elf->section[_symTabSection].sh_link;
	pLinkedSec=(STRING *) elf->sectionData[linkedSection];
	pSectionData=(Elf32_Sym *) elf->sectionData[_symTabSection];

	/* Now scan all entries and check if there is an 
	 * undefined symbol in the loaded executable.  These are symbols to
	 * be linked, we just look if there is one that has the same name of
	 * the symbols(s) in input of this function.                         */
				
	numOfEntries=elf->section[_symTabSection].sh_size/elf->section[_symTabSection].sh_entsize;

	for(i=0;i<numOfEntries;i++) {
	    int symbolType;

		symbolType=ELF32_ST_TYPE(pSectionData[i].st_info);
		if(symbolType==STT_NOTYPE) {
			STRING *undefinedSymName;
			UWORD32 j=0;

			/* An undefined symbol is found. Now check if it's contained 
			 * in the input list of globally defined symbols to be linked to. */

			undefinedSymName=&pLinkedSec[pSectionData[i].st_name];
			while(symbol[j].name[0]!='\0') {
				if(KLstringCompare(undefinedSymName, symbol[j].name)==0) {
					/* Found in the list of globally defined symbols. 
					 * Now we can link it with given address. */

					resolved+=linkToExternalReference(elf, &pSectionData[i], 
					                        (UWORD32) symbolAddr[j]);
				}

				j++;
			}
		}
	}

	return(resolved);
}

