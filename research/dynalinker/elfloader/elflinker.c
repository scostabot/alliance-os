/*
 * ELF linker library
 *
 * In this file are contained all those routines that find symbols and
 * their addresses in ELF sections.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 04/05/00 scosta    1.0    First draft
 * 
 * (c) 2000 Copyright Stefano Costa - The Alliance group
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
#include "elfloader.h"  /* The ELF file loader specific definitions */

PUBLIC Elf32_Sym * searchSymbol(UWORD32 *targetFile,
                                UWORD32 *targetIndex,
                                ELF *elf,
                                UWORD32 numOfELFFiles,
                                INPUT UWORD32 fileToAvoid, 
                                INPUT STRING *symbol,
                                INPUT UBYTE criteria)
  
/*
 * Look for a valid occurence of a valid symbol in a given list of ELF files.
 * 
 * INPUT:
 * elf              the pointer to an array of ELF files
 * numOfELFFiles    the number of ELF files in the array
 * fileToAvoid      the n-th ELF file that can be safely skipped. If no
 *                  file may be skipped, use ANY_FILE as input.
 * symbol           the ASCII, zero-terminated string of the symbol to search
 * criteria         if is equal to ANY_MATCH the search stop on first
 *                  entry found, otherwise checks for a match of criteria
 *                  and st_info field. Match criteria is a bitwise AND.
 * 
 * RETURN:
 * the pointer to the entry in the section of the given symbol or NIL
 * if no match is found. If and only if a match is found, return also the
 * symbol table entry (file and section index) of that symbol.
 */
  
{
	UWORD32 i;
	UWORD32 j;
	UWORD32 k;
	UWORD32 numOfEntries;
	STRING *pLinkedSec;
	Elf32_Word linkedSection;
	Elf32_Sym *pSectionData;
	Elf32_Sym *symbolEntry=NIL;

	for(i=0;i<numOfELFFiles;i++) { /* Scans all ELF files... */ 
 		if(i==fileToAvoid) { /* ...except the one that can be skipped */ 
			continue;
		}

		for(j=0;j<elf[i].header.e_shnum;j++) { /* Scan all sections */
	   		if(elf[i].sectionData[j]==NIL) { /* Skip unloaded sections */
				continue;
			}
		
			/* The starting point is the Symbol Table of the file. 
			 * Here we will look for our symbol.                   */
			
			switch(elf[i].section[j].sh_type) {
				case SHT_SYMTAB:
				case SHT_DYNSYM:
					/* Scan the symbol table and look for target symbol */
			  
					numOfEntries=elf[i].section[j].sh_size/elf[i].section[j].sh_entsize;

					/* Get the address of the String Table that holds the
					 * ASCIIZ names of all symbols entries present in the
					 * Symbol Table.                                      */
					
		   			linkedSection=elf[i].section[j].sh_link;
		   			pLinkedSec=(STRING *) elf[i].sectionData[linkedSection];
					pSectionData=(Elf32_Sym *) elf[i].sectionData[j];

					/* Now scan all entries and check if there is a 
					 * symbol named as our source symbol. If we find a
					 * symbol that is named identically in a ELF file
					 * different from the source one, than it's a probable
					 * target.                                            */
					
					for(k=0;k<numOfEntries;k++) {
				    	if(KLstringCompare(symbol, &pLinkedSec[pSectionData[k].st_name])==0) {
						  
					    	/* A definition of the symbol was found. 
					    	 * Does this definition match the criteria? */

					    	if(criteria==ANY_MATCH) {
								/* We don't mind, set output variables */
							  
								symbolEntry= &pSectionData[k];
								*targetFile=i;
								*targetIndex=j;
								break;						    
							} else {
								/* We must check that only if the symbol
								 * type of the target is equal to the
								 * specified one we will end successfully
								 * our search.                            */

								int symbolType;

								symbolType=ELF32_ST_TYPE(pSectionData[k].st_info);
								if(symbolType==criteria) {
									symbolEntry= &pSectionData[k];
									*targetFile=i;
									*targetIndex=j;
									break;
								}
							}
						}
					}
			    	break;
			}
		}
	}

    return(symbolEntry);
}

PUBLIC UWORD32 searchTextSection(INPUT ELF *elf)
  
/*
 * Search the .text section in the ELF file pointed by fileIndex
 * 
 * INPUT:
 * fileIndex		the index in the ELF file array of the file to look
 * 
 * RETURN:
 * the section index that points to the .text section
 */
  
{
	STRING *sectionName;
	UWORD32 i;
	UWORD32 index;
	
	sectionName=(STRING *) elf->sectionData[elf->header.e_shstrndx];
	
	/* I assume here that exists only one .text section in a given
	 * ELF file. So far, there are no exceptions.                    */

	for(i=0;i<elf->header.e_shnum;i++) {
		index=elf->section[i].sh_name;
		if(KLstringCompare(&sectionName[index], ".text")==0) {
			return(i);
		}
	}
	
	return(0);
}

PUBLIC UWORD32 linkELFFiles(ELF *elf, INPUT UWORD32 numOfELFFiles)
  
/*
 * Resolve all unresolved symbols addresses between ELF files 
 * given as input.
 * 
 * INPUT:
 * elf             the pointer to an array of ELF structures as given by
 *                 loadELFFile() function.
 * numOfELFFiles   the number of ELF files structures in the table
 * 
 * RETURNS:
 * non-zero        the number of files actually linked
 * 0               error, unresolved references.
 */
  
{
	STRING *strTab;
	UWORD32 i;
	UWORD32 j;
	UWORD32 k;
	UWORD32 numOfEntries;
	UWORD32 symbolTableSection;
	UWORD32 strTabSection;
	UWORD32 symbolIndex;
	UWORD32 traceMask;
	Elf32_Sym *sourceSymbol;
	Elf32_Rel *pRelData;

	/* Reject trivially invalid input cases */
	
	if(elf==NIL || numOfELFFiles<=0) {
		return(0);
	}

	/* If there is only one file, no linking is necessary, obviously! */

	if(numOfELFFiles==1) {
		return(1);
	}

	/* In order to avoid to waste a lot of CPU time we decode ELF data
	 * in a human-understanble form if and only if user has asked to do so. */

	traceMask=KLgetTraceLevel();

	/* The array of ELF structure given in input contains all useful data
	 * needed in order to successfully link the files each other and 
	 * execute them.
	 * We process each ELF file one at a time, looking for unresolved
	 * symbols, and for each unresolved symbol we look for a match in
	 * the remaining ELF files.                                          */
	
	for(i=0;i<numOfELFFiles;i++) {
		/* Scans all sections in the ELF file currently read */
	
		for(j=0;j<elf[i].header.e_shnum;j++) {
			if(elf[i].sectionData[j]==NIL) { /* Skip unloaded sections */
				continue;
			}

			/* The first step in the search of unresolved symbols is to 
			 * gather all relocation entries, since ANY unresolved symbol
			 * has one (and only one) relocation entry (the type of 
			 * relocation may vary, but it's always a relocation).        */ 

			switch(elf[i].section[j].sh_type) {
				case SHT_REL:
					/* Compute how much relocation entries are present */
				  
					numOfEntries=elf[i].section[j].sh_size/
					             elf[i].section[j].sh_entsize;

					/* Fetch pointers to relocation and symbol tables */
					
					pRelData=(Elf32_Rel *) elf[i].sectionData[j];
					symbolTableSection=elf[i].section[j].sh_link;

					/* The pointer to the symbol table that holds the 
					 * symbol to be linked to a target.                 */

					sourceSymbol=(Elf32_Sym *) elf[i].sectionData[symbolTableSection];

					/* Symbol search is performed with symbol name as
					 * key, so we gather also the String Table section 
					 * index. The String Table holds all ELF symbols names. */

					strTabSection=elf[i].section[symbolTableSection].sh_link;
					strTab=(STRING *) elf[i].sectionData[strTabSection];

					if(traceMask & NELF_SYMTAB) {	
						dumpLinkInfoHeader();
					}
					
					/* For all relocation entries check for an unresolved
					 * entry. These entries are marked STT_NOTYPE (no known
					 * type) since it is not possible for the compiler to
					 * determine the symbol type unambiguosly.            */

					for(k=0;k<numOfEntries;k++,pRelData++) {
						symbolIndex=ELF32_R_SYM(pRelData->r_info);
						if((ELF32_ST_TYPE(sourceSymbol[symbolIndex].st_info)==STT_NOTYPE)) {
							UWORD32 targetIndex;
							UWORD32 targetFile;
							STRING *symbolName;
							Elf32_Sym *targetSymbol;

							symbolName=&strTab[sourceSymbol[symbolIndex].st_name];

							/* Dump information about linking process. */
						
							if(traceMask & NELF_SYMTAB) {	
								dumpLinkInfo(&elf[i], symbolTableSection, 
							                     symbolIndex);
							}

							/* Now look for an occurence of a symbol named 
							 * identically (case IS significant) in all
							 * ELF files given as input EXCEPT the one
							 * that contains the unresolved symbol reference
							 * of course (the i parameter).                 */

							targetSymbol=searchSymbol(&targetFile, &targetIndex,
							             		  elf, numOfELFFiles, 
							                          i, symbolName, ANY_MATCH);
							if(targetSymbol==NIL) {
								KLtrace(ELF_SYMTAB "<Not found>\n");
								continue;
							}

							KLtrace(ELF_SYMTAB "%s\n", elf[targetFile].name);

							/* Another undocumented and empirically-found
							 * feature: the st_shndx is a negative number
							 * if the symbol table item defines a target
							 * defined in the .text section of the ELF file
							 * where the symbol table belongs to.
							 * Otherwise points (as defined) to the section
							 * that contains the reference.                  */
						  
							if(targetSymbol->st_shndx>elf[targetFile].header.e_shnum) {
								targetIndex=searchTextSection(&elf[targetFile]);
							} else {
								targetIndex=targetSymbol->st_shndx;
							}

							/* Now try to link symbol and its target. */

							linkSymbol(elf, pRelData, i, 
							           searchTextSection(&elf[i]),
							           targetSymbol, targetFile,
							           targetIndex);
						}
								
					}
				    break;
			}
		}
	}

	/* All files linked successfully, exit */

  	return(numOfELFFiles);
}

