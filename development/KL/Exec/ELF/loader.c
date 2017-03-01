/*
 * ELF executable loader library
 *
 * All stuff needed to load an ELF file in memory is defined here.
 * I suggest to read the listing under /research/dynalinker first in order
 * to have a gentler start in the studying.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 29/10/00 scosta    1.0    First draft
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

/* These variables caches the section indexes of .text, .bss, symbol and
   relocation tables in the currently loaded ELF file.                   */

PUBLIC UWORD32 _textSection;
PUBLIC UWORD32 _bssSection;
PUBLIC UWORD32 _symTabSection;
PUBLIC UWORD32 _relocTableSection=NOT_LOADED;

/* Each of the following routines is a state of a Finite State Machine
 * (FSM) that defines the behaviour of the executable loader. Each state
 * process binary data that contains a portion of the binary file being
 * loaded. The size, the position of the data relative to executable file
 * start and the pointer to the buffer that holds the data is defined in
 * a structure called Executable Record. In the N-th state of the FSM is
 * initialized the Executable Record, then in the N+1-th step the
 * caller will put in the Executable Record the needed data, which will be
 * processed and after the Executable Record is set....
 * The state of a finite state machine element can be one of the following:
 *
 * nextState          the FSM can advance to next defined state
 * stateError         the FSM found invalid data and cannot proceed
 * continueIteration  the FSM can continue looping in current state
 * stopIteration      the FSM can advance to next defined state after 
 *                    an iteration                                      */

PUBLIC FsmResult ELFSectionHeaderProcessing(ELF *elf, 
                                            OUTPUT ExecRecord *record)

/*
 * This step allocates memory to hold all ELF section headers 
 * of the executable file
 *
 * INPUT:
 * elf        pointer to ELF header informations
 *
 * OUTPUT:
 * record     where needed info is located and how big it is
 * elf        updates section data info
 *
 * RETURNS:
 * nextState if all OK, stateError otherwise
 */

{
	UWORD32 sizeOfSectionHeaders;
	FsmResult newState;

	/* Compute the size of Section Headers, tell to caller what to 
	 * fetch, allocate memory for that data                         */

	sizeOfSectionHeaders=sizeof(Elf32_Shdr)*elf->header.e_shnum;
	
	record->size=sizeOfSectionHeaders;
	record->position=elf->header.e_shoff;
	record->buffer=KLmalloc(sizeOfSectionHeaders);
	if(record->buffer!=NIL) {
		elf->section=(Elf32_Shdr *) record->buffer;
		elf->sectionData=(UBYTE **) KLmalloc(sizeof(UBYTE *)*elf->header.e_shnum);
		if(elf->sectionData==NIL) {
			newState=stateError;
		} else {
			UWORD32 i;

			for(i=0;i<elf->header.e_shnum;i++) {
				elf->sectionData[i]=NIL;
			}
			newState=nextState;
		}
	} else {
		newState=stateError;
	}

	return(newState);	
}

PUBLIC FsmResult ELFSectionNamesProcessing(INPUT ELF *elf, 
                                           OUTPUT ExecRecord *record)

/*
 * This step allocates memory for section names and tell to caller
 * where it is located and the dimension.
 *
 * INPUT:
 * elf       pointer to ELF header informations
 *
 * OUTPUT:
 * record    where needed info is located and how big it is
 * elf       updates section data info
 *
 * RETURNS:
 * nextState if all OK, stateError otherwise
 */

{
	FsmResult newState;
	
	/* In order to selectevely load only the sections we are
	 * interested to, we /always/ load the section (if present)
	 * that holds stringized section names. Later on, we will
	 * check and use this data.                               */
			
	if(elf->header.e_shstrndx!=SHN_UNDEF) {
		/* Tell to caller where is positioned and how many bytes is
		 * sized the section that holds section names.              */

		record->size=elf->section[elf->header.e_shstrndx].sh_size;
		record->position=elf->section[elf->header.e_shstrndx].sh_offset;
		record->buffer=KLmalloc(record->size);
		if(record->buffer!=NIL) {
			elf->sectionData[elf->header.e_shstrndx]=(UBYTE *) record->buffer;
			newState=nextState;	
		} else {
			newState=stateError;	
		}
	} else {
		newState=stateError;
	}

	return(newState);
}

PUBLIC FsmResult ELFSectionProcessing(INOUT ELF *elf, 
                                      INOUT ExecRecord *record, 
                                      OUTPUT ExecFSM *fsm)

/*
 * This step process an ELF section at a time. If the section has
 * to be loaded then fill Executable Record structure with needed
 * section data, otherwise set to NIL the pointer to buffer data.
 * When no other section has to be loaded, advance to next state.
 *
 * INPUT:
 * elf       pointer to ELF header informations
 * fsm       pointer to the FSM state data (we use number of iterations)
 * record    the pointer to data requested in the previous step
 *  
 * OUTPUT:
 * record    where needed info is located and how big it is
 * elf       updates section data info
 *
 * RETURNS:
 * nextState if all OK, stateError otherwise
 */

{
	UWORD32 i;
	STRING *sectionName;

	i=fsm->iterator; /* This is the n-th iteration on this step */

	if(elf->section[i].sh_type!=SHT_NULL) {
		/* Set pointer to stringized section names */

		sectionName=(STRING *) elf->sectionData[elf->header.e_shstrndx];

		/* Caches the section indexes for .text, .bss, symbol 
		 * and relocation table future reference.              */

		if(elf->section[i].sh_flags & SHF_EXECINSTR) {
			_textSection=i;
		} else {
			if(KLstringCompare(&sectionName[elf->section[i].sh_name], ".bss")==0) {
				_bssSection=i;
			} else {
				switch(elf->section[i].sh_type) {
					case SHT_SYMTAB:
						_symTabSection=i;
						break;
					case SHT_REL:
						if(KLstringCompare(&sectionName[elf->section[i].sh_name], ".rel.text")==0) {
							_relocTableSection=i;
						}
						break;
				}
			}
		}

		/* In ELF files, the sections marked as SHF_ALLOC are
		 * defined as data or code essential to the program to
		 * being run. Conversely, the section not marked this way
		 * should not be loaded in memory since should be 
		 * auxiliary informations (like debugger data, for
		 * example). 
		 * Unfortunately, I have seen that the semantic of ELF
		 * files generated in different environments is rather
		 * dissimilar, and we can't rely on the above definition
		 * in order to load symbol data. Practically gcc on
		 * Linux for Intel generate symbol data in a section
		 * not marked as SHF_ALLOC. The same compiler for 
		 * SPARC instead use this flag for the same data.
		 * So since we need to have in memory symbol data,
		 * we look also for section type and force loading
		 * of SHT_SYMTAB,SHT_DYNSYM,SHT_STRTAB and SHT_REL anyway. */
	 
		if((elf->section[i].sh_flags & SHF_ALLOC)==0) {
			Elf32_Word sectionType;
			
			sectionType=elf->section[i].sh_type;
			if(sectionType!=SHT_SYMTAB &&
			   sectionType!=SHT_DYNSYM &&
			   sectionType!=SHT_STRTAB &&
			   sectionType!=SHT_REL &&
			   sectionType!=SHT_RELA) {
				fsm->iterator++;
				record->buffer=NIL;
				return(continueIteration);
			} else {
				/* As you may remember, we have already loaded
				 * the section that holds section string names,
				 * so avoid to reload it again.                 */
			  
				if(i==elf->header.e_shstrndx) {
					fsm->iterator++;
					record->buffer=NIL;
					return(continueIteration);
				}

				/* Now we are going to read one of the above 
				 * sections. If the source program was compiled
				 * with debugging informations, then a replica
				 * of SHT_REL and SHT_RELA will be generated for
		 		 * debugger-related symbol informations. 
		 		 * We don't want to process debugging info here,
		 		 * so we throw them out if present. Also the
				 * rest of the code assumes that the only symbol
				 * data available is relative to the executable
				 * code, and not to other entities.              */
			   
				if(sectionType==SHT_REL || sectionType==SHT_RELA) {
					if(KLstringMatch(&sectionName[elf->section[i].sh_name],
						             "stab")!=NIL) {
						fsm->iterator++;
						record->buffer=NIL;
				 		return(continueIteration);
					}
				} 
					
				if(sectionType==SHT_STRTAB) {
				    if(KLstringMatch(&sectionName[elf->section[i].sh_name],
						             "stabstr")!=NIL) {
						fsm->iterator++;
						record->buffer=NIL;
						return(continueIteration);
					}
				}
			}
		}

		/* It is legal that a Section is declared, but with no
		 * section data referenced. Check for that condition
		 * before allocating space and reading it.              */

		if(elf->section[i].sh_size) {
			/* A special case is a section of type SHT_NOBITS.
			 * These sections may have valid sh_offset and/or
			 * sh_size values, but still no space is occupied
			 * for it in the file. For this reason, this kind
			 * of data is called "conceptual data".
			 * So, if this is the case, we avoid to read from 
			 * file the section data, while we keep the reference
			 * to conceptual data in sectionData[] array.         */
			
			if(elf->section[i].sh_type!=SHT_NOBITS) {
				/* Now allocate memory and load the contents of i-th section */
		
	  			record->size=elf->section[i].sh_size;
				record->position=elf->section[i].sh_offset;
				record->buffer=KLmallocAligned(record->size,
				                               elf->section[i].sh_addralign);
				if(record->buffer==(UBYTE *) NIL) {
					fsm->iterator++;
					return(stateError);
				}

				elf->sectionData[i]=record->buffer;
			} else {
				elf->sectionData[i]=KLmallocAligned(record->size,
				                                    elf->section[i].sh_addralign);
				if(elf->sectionData[i]==NIL) {
					return(stateError);
				}
			}
		} else {
			elf->sectionData[i]=(UBYTE *) NIL;
			record->buffer=NIL;
		}
	} else {
		record->buffer=NIL;
	}

	fsm->iterator++;
	if(elf->header.e_shnum==fsm->iterator) {
		return(stopIteration);
	} else {
		return(continueIteration);	
	}
}

PUBLIC VOID ELFsizeAndAssignMemToBSS(INPUT ELF *elf)

/*
 * After the loading process we must compute the amount of memory needed
 * to hold all uninitialized data in the executable.
 *
 * INPUT:
 * elf          pointer to ELF header informations
 *
 * RETURNS:
 * always nextState 
 */

{
	UWORD32 i;
	UWORD32 numOfEntries;
	UWORD32 totalBSSSize;
	Elf32_Sym *pSymbol;
	STRING *pLinkedSec;
	Elf32_Word linkedSection;

	/* Look for all occurences of unassigned variables in the Symbol
	 * Table that should made up the .bss section, which for some
	 * strange reasons, it is always generated empty by the GCC.     */

	totalBSSSize=0;

	numOfEntries=elf->section[_symTabSection].sh_size/elf->section[_symTabSection].sh_entsize;
	pSymbol=(Elf32_Sym *) elf->sectionData[_symTabSection];
	linkedSection=elf->section[_symTabSection].sh_link; 
	pLinkedSec=(STRING *) elf->sectionData[linkedSection];

	for(i=0;i<numOfEntries;i++,pSymbol++) {
			  
		/* Another quirk: the unassigned variables section indexes
		 * in the symbol table points to invalid section number.
		 * We use this empirical fact to find what put into bss.   */
			  
		if(pSymbol->st_shndx>elf->header.e_shnum) {
			if(ELF32_ST_TYPE(pSymbol->st_info)!=STT_FILE) {
			    	totalBSSSize+=pSymbol->st_size;
				pSymbol->st_shndx=_bssSection;
			}
		}
	}

	/* Finally put in the .bss section header the pointer to a memory
	 * block large enough to contain all unitialized data and save its
	 * size in memory for further reference.                            */

	if(totalBSSSize>0) {	
		elf->sectionData[_bssSection]=(UBYTE *) KLmallocAligned(totalBSSSize, elf->section[_bssSection].sh_addralign);
		elf->section[_bssSection].sh_size=totalBSSSize;
	}

}

PUBLIC FsmResult ELFresolveRelocations(INPUT ELF *elf)

/*
 * After the loading process we must resove all the relocations local
 * to the file being loaded.
 *
 * INPUT:
 * elf        pointer to ELF header informations
 *
 * RETURNS:
 * always nextState 
 */

{
	UWORD32 i;
	UWORD32 numOfEntries;
	UWORD32 symbolIndex;
	Elf32_Rel *pRelData;
	Elf32_Sym *symbol;
	UWORD32 symbolTableIndex;

	/* Reject trivial case of an empty relocation section */

	if(_relocTableSection==NOT_LOADED) {
		return(nextState);
	}
  
	/* Compute how much symbols are defined in this section */

	numOfEntries=elf->section[_relocTableSection].sh_size/elf->section[_relocTableSection].sh_entsize;
			
	pRelData=(Elf32_Rel *) elf->sectionData[_relocTableSection];
				
	/* For each symbol in the relocation table, try to resolve it. */ 

	symbolTableIndex=elf->section[_relocTableSection].sh_link;

	for(i=0;i<numOfEntries;i++, pRelData++) {
		/* First find in the symbol table the entry for the
		 * symbol to be relocated.                           */
					
		symbol=(Elf32_Sym *) elf->sectionData[symbolTableIndex];
		symbolIndex=ELF32_R_SYM(pRelData->r_info);
		symbol+=symbolIndex;
				
		/* Check if this relocation entry applies to a type
		 * of symbol that actually needs relocation, since
		 * there are relocation entres which are of no use for
		 * our purposes. STT_FILE for instance is useless,
		 * since it marks the filename of the source file that
		 * has generated the ELF file image.                  */

		if(ELF32_ST_TYPE(symbol->st_info)==STT_FILE) {
			continue;
		}

		if(ELF32_ST_TYPE(symbol->st_info)==STT_NOTYPE) {
			continue;
		}

		/* Ok, it's a symbol to be relocated: do it following
		 * target CPU conventions for this task.              */

		relocateSymbol(elf, symbol, pRelData);
	}
	
	return(nextState);	
}

PUBLIC VOID ELFclose(INPUT ELF *elf)

/*
 * Free all resources locked by support for specific ELF features
 *
 * INPUT:
 * elf       pointer to ELF header informations
 *
 * RETURNS:
 * None
 */

{
	UWORD32 i;

	for(i=0;i<elf->header.e_shnum;i++) {
		if(elf->sectionData[i]!=NIL) {
			KLfree(elf->sectionData[i]);
		}
	}

	if(elf->sectionData!=NIL) {
		KLfree(elf->sectionData);
	}

	if(elf->section!=NIL) {
		KLfree(elf->section);
	}
}

