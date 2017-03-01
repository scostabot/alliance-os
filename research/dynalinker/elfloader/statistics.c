/*
 * ELF dynamic link library
 *
 * Defines functions that are not directly related to ELF file loading
 * or linking, but makes useful (optional) stuff in keeping a watchful eye.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 20/11/99 scosta    1.0    First draft
 * 19/12/99 scosta    1.1    Improved getELFSymbols
 * 20/12/99 scosta    1.2    Added functions that return number of symbols
 * 
 * (c) 1999 Copyright Stefano Costa - The Alliance group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>
#include <elf.h>
#include "elfloader.h"

#include <memory.h>

PUBLIC UWORD32 getELFMemoryFootPrint(ELFSize *sec,
                                     UWORD32 *memoryForHeaders,
                                     INPUT ELF *elf)
/*
 * Return dynamic memory allocation information about the pointed ELF file.
 *
 * INPUT:
 * elf               the pointer to the ELF file in memory
 *
 * OUTPUT:
 * memoryForHeaders  the memory used (in bytes) to store ELF structs
 * sec               the memory used (in bytes), the section id and
 *                   name for each loaded section of ELF file in input.
 */

{
	UWORD32 i;
	UWORD32 j;
	STRING *sectionName;
	Elf32_Half sectionNamesIndex;
	
	if(elf==NIL) {
		return(0);
	}
	
	sectionNamesIndex=elf->header.e_shstrndx;
	sectionName=(STRING *) elf->sectionData[sectionNamesIndex];

	j=0;	
	for(i=1;i<elf->header.e_shnum;i++) {
		if(elf->sectionData[i]!=NIL) { /* Skip unloaded sections */
			sec[j].size=elf->section[i].sh_size;
			sec[j].id=i;
			sec[j].alignement=elf->section[i].sh_addralign;
			if(sectionName!=NIL) {
				KLstringCopy(sec[j].name, &sectionName[elf->section[i].sh_name]);
			}
			sec[j].data=(UBYTE *) elf->sectionData[i];
			j++;
		}
	}

	*memoryForHeaders=sizeof(Elf32_Shdr)*elf->header.e_shnum;

	return(j);
}

PUBLIC UWORD32 getNumOfELFRelocs(INPUT ELF *elf)
  
/*
 * Get the number of relocation entries for a given ELF file.
 * 
 * INPUT:
 * elf		the pointer to an ELF file structure
 * 
 * RETURN:
 * the number of relocation entries contained in input file
 */
  
{
	UWORD32 i;
	UWORD32 numOfEntries=0;

	if(elf==NIL) {
		return(0);
	}
	
	for(i=0;i<elf->header.e_shnum;i++) {
	   	if(elf->sectionData[i]==NIL) { /* Skip unloaded sections */
			continue;
		}
		
		if(elf->section[i].sh_type==SHT_REL ||
		   elf->section[i].sh_type==SHT_RELA) {
		
			/* Compute how much symbols are defined in this section */

			numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
			break;
		}
	}
	
	return(numOfEntries);
}

LOCAL STRING *alert="***INVALID NAME***";

PUBLIC UWORD32 getELFRelocs(Rel *reloc, INPUT ELF *elf)
  
/*
 * Copy in an array of structures the contents of relocation records of
 * a given ELF file.
 * 
 * INPUT:
 * elf      the pointer to an ELF file structure
 * 
 * RETURN:
 * reloc    a pointer to an aray of relocation entry dump structure.
 *          The array must be sized at least with a number of elements 
 *          equal to the number returned by getNumOfELFRelocs() function.
 */
  
{
	UWORD32 i;
	UWORD32 j;
	UWORD32 k;
	UWORD32 numOfEntries;
	Elf32_Rel *pRelData;
	STRING *strTab;
	UWORD32 strTabSection;

	if(elf==NIL) {
		return(0);
	}
	
	for(i=0;i<elf->header.e_shnum;i++) {
	   	if(elf->sectionData[i]==NIL) { /* Skip unloaded sections */
			continue;
		}
		
		if(elf->section[i].sh_type==SHT_REL ||
		   elf->section[i].sh_type==SHT_RELA) {
		
			/* Compute how much symbols are defined in this section */

			numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
			
			pRelData=(Elf32_Rel *) elf->sectionData[i];

			strTabSection=elf->section[elf->section[i].sh_link].sh_link;
			strTab=elf->sectionData[strTabSection];

			k=0;
			for(j=0;j<numOfEntries;j++, pRelData++, k++) {
				STRING *symbolName;
				UWORD32 symbolIndex;
				Elf32_Sym *symbol;

				reloc[k].sym.r_info=pRelData->r_info;
				symbol=(Elf32_Sym *) elf->sectionData[elf->section[i].sh_link];
				symbolIndex=ELF32_R_SYM(pRelData->r_info);

				/* I have noted during my tests that on SPARC platform the
				 * ELF code generated has invalid symbol index data, that
				 * is, an outragely high number, greater than the size of
				 * symbol table. If this case happens, put a warning in
				 * the displayed data.                                     */

				if(symbolIndex>elf->section[elf->section[i].sh_link].sh_entsize) {
					reloc[k].name=alert;
					reloc[k].sym.r_offset=0;
				} else { 
					symbol+=symbolIndex;
					symbolName=&strTab[symbol->st_name];
					reloc[k].name=symbolName;
					reloc[k].sym.r_offset=pRelData->r_offset;
				}
			}
		}		
	}
	
	return(numOfEntries);
}

PUBLIC UWORD32 getNumOfELFSymbols(INPUT ELF *elf)
  
/*
 * Get the number of symbols contained in the given ELF file.
 * 
 * INPUT:
 * elf    the pointer to an ELF file structure
 * 
 * RETURN:
 * the number of symbol entries of the input ELF file
 */

{
	UWORD32 i;
	UWORD32 numOfEntries;
	
	if(elf==NIL) {
		return(0);
	}
	
	for(i=0;i<elf->header.e_shnum;i++) {
	   	if(elf->sectionData[i]==NIL) { /* Skip unloaded sections */
			continue;
		}

		if(elf->section[i].sh_type==SHT_SYMTAB) {
			numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
			break;
		}
	}
	
	return(numOfEntries);
}

PUBLIC UWORD32 getELFSymbols(Sym *symbolList, INPUT ELF *elf)

/*
 * Return a list of symbols declared in the input ELF file.
 *
 * INPUT:
 * elf     the pointer to an ELF file structure.
 *
 * RETURNS
 *
 */
 
{
	UWORD32 i;
	UWORD32 j;
	UWORD32 k;
	UWORD32 numOfEntries;
	Elf32_Sym *pSymbol;
	STRING *strTab;
	STRING *symbolName;
	UWORD32 strTabSection;

	if(elf==NIL) {
		return(0);
	}
	
	for(i=0;i<elf->header.e_shnum;i++) {
	   	if(elf->sectionData[i]==NIL) { /* Skip unloaded sections */
			continue;
		}

		switch(elf->section[i].sh_type) {
			case SHT_SYMTAB:
				numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
				pSymbol=(Elf32_Sym *) elf->sectionData[i];

				strTabSection=elf->section[i].sh_link;
				strTab=elf->sectionData[strTabSection];
				k=0;

				for(j=0;j<numOfEntries;j++, pSymbol++) {
					symbolName=&strTab[pSymbol->st_name];
					if(*symbolName!='\0') {
						memcpy(&symbolList[k].sym, pSymbol, sizeof(Elf32_Sym));
						symbolList[k].name=symbolName;
						k++;
					}
				}
				break;
		}	
	}

	return(k);
}
