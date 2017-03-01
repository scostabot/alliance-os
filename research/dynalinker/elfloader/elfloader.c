/*
 * ELF loader library
 *
 * This module defines all routines that checks and extract data
 * contained in an ELF file and put it in memory.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 06/07/99 scosta    1.0    Creation
 * 08/09/99 scosta    1.1    Added support for debugging data in ELF files
 * 09/09/99 scosta    1.2    Improved speed for large section loads
 * 11/09/99 scosta    1.3    Now dump all sections, regardless if loaded or not
 * 10/11/99 scosta    1.4    Fixed little/big endian problem
 * 20/11/99 scosta    1.5    Added more SPARC support, various fixes
 * 20/12/99 scosta    1.6    First (abortive) attempt to support ELF dynalinking
 * 12/02/00 scosta    1.7    Second attempt
 * 04/05/00 scosta    1.8    Taken away linking code
 * 13/10/00 scosta    1.9    Added support for memory alignement
 * 23/10/00 scosta    2.0    Fixed relocation bug
 * 
 * (c) 1999-2000 Copyright Stefano Costa - the Alliance group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>		/* The basic Kernel Library definitions */
#include <elf.h>		/* The ELF file format structures definitions */
#include "elfloader.h"	/* The ELF file loader specific definitions */

/* Interface to the basic OS facilities.
 * This is a part that will be deleted when the loader will
 * run on top of Alliance OS IOSK, but for now....          */

PUBLIC BOOLEAN (*readCallBack)(VOID *, UWORD32);
PUBLIC BOOLEAN (*seekCallBack)(UWORD32);
PUBLIC VOID *(*allocCallBack)(UWORD32, UWORD32); 
PUBLIC VOID (*freeCallBack)(VOID *);

/* In this source is assumed that the reader is familiar with ELF
 * specifications document rel. 1.1.
 * 
 * ELF files loading is performed in a two-step fashion.
 * Firstly, the contents of ELF file data is copied into CPU memory,
 * with a minimal re-organization of ELF data, using arrays that points
 * to actual ELF control structures. All this stuff is done through call
 * of loadELFFile() function.
 * All ELF data is stored in the following arrays of structures: 
 *
 * typedef struct {
 *     Elf32_Ehdr header;
 *     Elf32_Phdr programHeader;
 *     Elf32_Shdr *section;
 *     UBYTE **sectionData;
 * } ELF;
 *
 * With loadELFFile() function you can succesfully load
 * ELF files originated from a different HW/SW environment, like 
 * SPARC CPUs executables, not only Intels. If the endianess of the host
 * CPU is different from that of the loaded file, the following variables
 * holds different values:                                                */

PUBLIC Translation translationMode;
LOCAL Endianess fileEncoding;

/* The loader perform all necessary data conversion tasks as needed 
 * from/to any endianess type.
 * 
 * After the ELF file is loaded in host CPU memory, you can inquire
 * for specific symbol(s) location and/or dynamically resolve addresses
 * as you like with ...(). It is important to know that this function
 * does *not* try to pass CPU execution to target program, but only 
 * resolves loading-time symbols. It is ELF loader caller duty to
 * kickstart the loaded program to execute it. This is deliberate,
 * since various tasks may be needed before actually run the program
 * (security, emulation of different CPUs...).
 * 
 * When caller want to thrash ELF program from memory, use removeELF()
 * function that frees all resources locked by the above functions in 
 * an orderly manner.
 * 
 */

#define NO_INDEX	1024	/* Define a placeholder to mark that no section
							   names section index is present in this file. */

PUBLIC UWORD32 textSection; /* The index in ELF structure of .text section */
PUBLIC UWORD32 myTraceMask; /* The run-time tracing mask currently in use */

LOCAL BOOLEAN binaryPatchingAllowed=TRUE;

LOCAL ELFLoadState loadELFInMemory(ELF *elf)

/*
 * Create a memory image of an ELF file.
 * 
 * INPUT:
 * elfFile		the pointer to a ELF file structure
 * 
 * RETURNS:
 * If file is loaded successfully returns ELF_LOADED_OK.
 * On error returns:
 * ELF_READ_ERROR			the reading op has failed
 * ELF_ALLOC_ERROR			the allocation op has failed
 * ELF_SEEK_ERROR			the seek op has failed
 * ELF_FORMAT_ERROR 		there is a bad value in ELF control structure
 * ELF_NOT_SUPPORTED_ERROR  the value of an ELF field is valid but
 *                          it is not supported by this loader
 */

{
	STRING *sectionName;
	Elf32_Half sectionNamesIndex;
	UWORD32 i;
	Endianess hostEncoding;
	
	/* Read ELF file header */

	if((*readCallBack)(&elf->header, sizeof(Elf32_Ehdr))==FALSE) {
		return(ELF_READ_ERROR);
	}
	
	/* Check ELF signature */
	
	if(elf->header.e_ident[EI_MAG0]!=ELFMAG0 ||
		elf->header.e_ident[EI_MAG1]!=ELFMAG1 ||
		elf->header.e_ident[EI_MAG2]!=ELFMAG2 ||
		elf->header.e_ident[EI_MAG3]!=ELFMAG3) {
			return(ELF_FORMAT_ERROR);
	}

	/* Reject abnormally formed executables */

	if(elf->header.e_machine==EM_NONE) {
	    return(ELF_FORMAT_ERROR);
	}

	/* Look for ELF data format class: we support only 32-bit ELF for now */

	if(elf->header.e_ident[EI_CLASS]!=ELFCLASS32) {
	    return(ELF_NOT_SUPPORTED_ERROR);
	}

	/* Look for data encoding and endianess issues, rejecting invalid cases */
	
	switch(elf->header.e_ident[EI_DATA]) {
		case ELFDATANONE: /* This is classified as invalid field */
			return(ELF_FORMAT_ERROR);
			
		case ELFDATA2LSB:
			fileEncoding=LSBENCODING;
			break;
			
		case ELFDATA2MSB:
			fileEncoding=MSBENCODING;
			break;
			
		default:         /* I prefer to sort out unknown stuff */
			return(ELF_FORMAT_ERROR);
	}

	hostEncoding=probeEndianess();
	if(hostEncoding!=fileEncoding)	{
		/* We are loading an ELF executable written for a CPU with
		 * a different endianess than the one that we are hosted to,
		 * so all subsequent read of ELF data structures must be 
		 * interpreted using a translation mode depending on the 
		 * architectures involved. Mark the needed translation.     */
	 
		if(hostEncoding==MSBENCODING) { 
	    		translationMode=LSB2MSB;
		} else {
			translationMode=MSB2LSB;
		}
	
		/* Now resolve endianess issues and decode ELF header structure
		 * so our host CPU can interpretate correctly the data in it.   */

		decodeELFHeader(&elf->header);
	} else {
		translationMode=NONE;
	}
	
	dumpELFHeader(&elf->header, fileEncoding);
	
	/* If Program file header is present, read it */

	if(elf->header.e_phoff) {
		if((*seekCallBack)(elf->header.e_phoff)==FALSE) {
			return(ELF_SEEK_ERROR);
		}
		
		if((*readCallBack)(&elf->programHeader, sizeof(Elf32_Phdr))==FALSE) {
			return(ELF_READ_ERROR);
		}
	
		/* Again, if endianess is involved, decode ELF Program Header */
		
		if(translationMode!=NONE) {
			decodeProgramHeader(&elf->programHeader);
		}
	
		/* Dump informations if needed */
		
	   	dumpProgramHeader(&elf->programHeader);
	}

	/* Section headers handling. If they are present, load them */

	if(elf->header.e_shoff) {
		UWORD32 sizeOfSectionHeaders;

		/* Allocate space for section data entries */
		
		elf->sectionData=(UBYTE **) (*allocCallBack)(sizeof(UBYTE *)*elf->header.e_shnum, 1);
		if(elf->sectionData==(UBYTE **) NIL) {
			return(ELF_ALLOC_ERROR);
		}
		
		for(i=0;i<elf->header.e_shnum;i++) { /* Mark as unloaded the sections */
			elf->sectionData[i]=NIL;
		}
	
		/* Read all section Headers at once in memory */
		
		if((*seekCallBack)(elf->header.e_shoff)==FALSE) {
			return(ELF_SEEK_ERROR);
		}

		sizeOfSectionHeaders=sizeof(Elf32_Shdr)*elf->header.e_shnum;
		elf->section=(Elf32_Shdr *) (*allocCallBack)(sizeOfSectionHeaders, 1);
		if(elf->section==(Elf32_Shdr *) NIL) {
			return(ELF_ALLOC_ERROR);
		}
					
		if((*readCallBack)(elf->section, sizeOfSectionHeaders)==FALSE) {
			return(ELF_READ_ERROR);
		}
		
		/* In order to selectevely load only the sections we are
		 * interested to, we /always/ load the section (if present)
		 * that holds stringized section names. Later on, we will
		 * check and use this data.                               */
			
		if(elf->header.e_shstrndx!=SHN_UNDEF) {
			sectionNamesIndex=elf->header.e_shstrndx;
			
			/* Check if we have to cope with different CPU endianess.. */
			
			if(translationMode!=NONE) {
				decodeSectionHeader(&elf->section[sectionNamesIndex]);
			}
			  
			elf->sectionData[sectionNamesIndex]=(UBYTE *) (*allocCallBack)(elf->section[sectionNamesIndex].sh_size, elf->section[sectionNamesIndex].sh_addralign);
			if(elf->sectionData[sectionNamesIndex]==NIL) {
				return(ELF_ALLOC_ERROR);
			} else {
				sectionName=(STRING *) elf->sectionData[sectionNamesIndex];
				if((*seekCallBack)(elf->section[sectionNamesIndex].sh_offset)==FALSE) {
					return(ELF_SEEK_ERROR);
				}
					
				if((*readCallBack)(sectionName, 
					               elf->section[sectionNamesIndex].sh_size)==FALSE) {
			   		return(ELF_READ_ERROR);
				}
			}
		} else {
			sectionNamesIndex=NO_INDEX;
		}

		/* Now process individually each Section Header present.
		 * If endianess of the loaded program is different from
		 * that of the host program also decode each header also. */

		for(i=0;i<elf->header.e_shnum;i++) {
			/* If endianess needs to be adjusted, do so now so we can
			 * correctly interpretate section data later.
			 * Remember that we have already loaded the section containing
			 * the stringized section names, so we don't have to adjust it. */

			if(translationMode!=NONE && i!=sectionNamesIndex) {
				decodeSectionHeader(&elf->section[i]);
			}

			/* Avoid to process the unused Section Header code */
	
			if(elf->section[i].sh_type==SHT_NULL) {
				continue;
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
					continue;
				} else {
					/* As you may remember, we have already loaded
					 * the section that holds section string names,
					 * so avoid to reload it again.                 */
				  
					if(i==sectionNamesIndex) {
						continue;
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
							continue;
						}
					} 
					
					if(sectionType==SHT_STRTAB) {
						if(KLstringMatch(&sectionName[elf->section[i].sh_name],
							             "stabstr")!=NIL) {
							continue;
						}
					}
				}
			}
	
			/* We save the index of the .text section, since it will be
			 * useful later. IMPORTANT: we assume that only one .text
			 * section exist in the ELF file!                           */

			if(KLstringCompare(&sectionName[elf->section[i].sh_name], ".text")==0) {
				textSection=i;
			}

			/* It is legal that a Section is declared, but with no
			 * section data referenced. Check for that condition
			 * before allocating space and reading it.              */
		
			if(elf->section[i].sh_size) {
				/* Now allocate memory and load the contents of i-th section */
			
				elf->sectionData[i]=(UBYTE *) (*allocCallBack)(elf->section[i].sh_size, elf->section[i].sh_addralign);
				if(elf->sectionData[i]==(UBYTE *) NIL) {
					return(ELF_ALLOC_ERROR);
				}

				/* A special case is a section of type SHT_NOBITS.
				 * These sections may have valid sh_offset and/or
				 * sh_size values, but still no space is occupied
				 * for it in the file. For this reason, this kind
				 * of data is called "conceptual data".
				 * So, if this is the case, we avoid to read from 
				 * file the section data, while we keep the reference
				 * to conceptual data in sectionData[] array.         */
			
				if(elf->section[i].sh_type!=SHT_NOBITS) {
					if((*seekCallBack)(elf->section[i].sh_offset)==FALSE) {
						return(ELF_SEEK_ERROR);
					}
					
					if((*readCallBack)(elf->sectionData[i], 
						               elf->section[i].sh_size)==FALSE) {
						return(ELF_READ_ERROR);
					}

					/* Check if we need to get rid of endianess inside
					 * section data. We don't translate all ELF file 
					 * data, only those relative to sections of interest. */
					
					if(translationMode!=NONE) {
						UWORD32 numOfEntries;

						switch(elf->section[i].sh_type) {
							case SHT_DYNSYM:
							case SHT_SYMTAB:
								numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
								decodeSymbolTable((Elf32_Sym *) elf->sectionData[i], numOfEntries);
								break;
							case SHT_REL:
								numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
								decodeRelocEntries((Elf32_Rel *) elf->sectionData[i], numOfEntries);
								break;
							case SHT_RELA:
								numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
								decodeRelocEntriesAbs((Elf32_Rela *) elf->sectionData[i], numOfEntries);
								break;
						}
					}
				}
			} else {
				elf->sectionData[i]=(UBYTE *) NIL;
			}
		}
	}

	/* We save current trace level, since we want to avoid to do the
	 * check with KLtrace() many times and this slow down considerably 
	 * the ELF file loading when section data is dumped out.           */

	myTraceMask=KLgetTraceLevel();

	/* Dump out all informations about the ELF file just loaded.
	 * We do that now, after the load has taken place, because we can
	 * cross-reference data and we can dump out information in a 
	 * human-understandable form.                                      */
	
	for(i=0;i<elf->header.e_shnum;i++) {
		dumpSectionHeader(&elf->section[i], i, 
			              &sectionName[elf->section[i].sh_name]);
		
		/* Avoid to waste time to dump out large section data if we
		 * don't need it.                                            */

		if(myTraceMask & NELF_SDATA) {
			if(elf->sectionData[i]!=NIL) {
		 		dumpSectionData(elf->sectionData[i], elf->section[i].sh_size);
			} else {
				UBYTE *tempStorage;
				Elf32_Word sectionSize;

				/* As memory optimization we haven't loaded all sections,
				 * and this is a Right Thing (TM), but it is nevertheless
				 * useful to see them in a debugging dump, so in this
				 * case we load, display them and discard.                 */
			 
				if(elf->section[i].sh_size==0) {
					continue;
				}

				sectionSize=elf->section[i].sh_size;
				tempStorage=(UBYTE *) (*allocCallBack)(sectionSize, elf->section[i].sh_addralign);
				if(tempStorage==(UBYTE *) NIL) {
				 	return(ELF_ALLOC_ERROR);
				}

				if((*seekCallBack)(elf->section[i].sh_offset)==FALSE) {
					return(ELF_SEEK_ERROR);
				}
					
				if((*readCallBack)(tempStorage, sectionSize)==FALSE) {
			   		return(ELF_READ_ERROR);
				}
			  
		 		dumpSectionData(tempStorage, sectionSize);
				(*freeCallBack)(tempStorage);
			}
		}
	}

	return(ELF_LOADED_OK);
}

LOCAL VOID resolveRelocations(ELF *elf)

/*
 * Compute the address for all relocatable local/global symbols and
 * write it in memory image of the given ELF file.
 *
 * INPUT:
 * elf       the pointer to the ELF file to be relocated
 *
 * OUTPUT:
 * NONE
 */

{
	UWORD32 i;
	UWORD32 j;
	UWORD32 numOfEntries;
	Elf32_Word sectionType;
   
	/* Scan all ELF file sections */

	for(i=0;i<elf->header.e_shnum;i++) {
		if(elf->sectionData[i]==NIL) { /* Skip unloaded sections */
		   	continue;
		}

		/* Look for relocation table section */

		sectionType=elf->section[i].sh_type;
		if(sectionType==SHT_REL) {
			UWORD32 symbolIndex;
			Elf32_Rel *pRelData;
			Elf32_Sym *symbol;
			UWORD32 symbolTableIndex;

			/* Compute how much symbols are defined in this section */

			numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
			
			pRelData=(Elf32_Rel *) elf->sectionData[i];
				
			/* For each symbol in the relocation table, try to resolve it. */ 

			symbolTableIndex=elf->section[i].sh_link;
			if(numOfEntries>0) {
				KLtrace(ELF_REL "\nSymbol relocation (section=%d)\n", i);
			 	KLtrace(ELF_REL "------------------------------------\n");
			}

			for(j=0;j<numOfEntries;j++, pRelData++) {
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

				relocateSymbol(elf, symbol, pRelData, binaryPatchingAllowed);
			}
			
			if(numOfEntries>0) {
				KLtrace(ELF_REL "\n");
			}
		}
	}
}

LOCAL VOID sizeAndAssignMemToBSS(INPUT ELF *elf)

/*
 * Compute the total size of BSS section,  allocate memory for it and 
 * set appropriately the ELF section hedaer for BSS section.
 *
 * INPUT:
 * elf		the pointer to an ELF memory image
 *
 * RETURN:
 * None
 */ 

{
	STRING *sectionName;
	UWORD32 i;
	UWORD32 j;
	UWORD32 numOfEntries;
	UWORD32 bssIndex;
	UWORD32 totalBSSSize;
	Elf32_Sym *pSymbol;

	/* Fetch the pointer to the table that holds the section names */
	
	sectionName=(STRING *) elf->sectionData[elf->header.e_shstrndx];
	
	/* I assume here that exists only one .bss section in a given
	 * ELF file. So far, there are no exceptions.                    */

	for(i=0;i<elf->header.e_shnum;i++) {
		if(KLstringCompare(&sectionName[elf->section[i].sh_name], ".bss")==0) {
			bssIndex=i;
			break;
		}
	}

	KLtrace(ELF_REL "\nBSS symbol                Size\n");
	KLtrace(ELF_REL "---------------------     ----------\n");

	/* Look for all occurences of unassigned variables in the Symbol
	 * Table that should made up the .bss section, which for some
	 * strange reasons, it is always generated empty by the GCC.     */

	totalBSSSize=0;
	for(i=0;i<elf->header.e_shnum;i++) {
		if(elf->sectionData[i]==NIL) { /* Skip unloaded sections */
			continue;
		}

		if(elf->section[i].sh_type==SHT_SYMTAB) {
			STRING *pLinkedSec;
			Elf32_Word linkedSection;

			numOfEntries=elf->section[i].sh_size/elf->section[i].sh_entsize;
			pSymbol=(Elf32_Sym *) elf->sectionData[i];
			linkedSection=elf->section[i].sh_link; 
			pLinkedSec=(STRING *) elf->sectionData[linkedSection];

			for(j=0;j<numOfEntries;j++,pSymbol++) {
			  
				/* Another quirk: the unassigned variables section indexes
				 * in the symbol table points to invalid section number.
				 * We use this empirical fact to find what put into bss   */
			  
			    if(pSymbol->st_shndx>elf->header.e_shnum) {
					if(ELF32_ST_TYPE(pSymbol->st_info)!=STT_FILE) {
				    	totalBSSSize+=pSymbol->st_size;
						pSymbol->st_shndx=bssIndex;
						
						if(myTraceMask & NELF_REL) {
						  	UWORD32 padding;
							STRING clippedName[24];
							STRING *symbolName;

							symbolName=&pLinkedSec[pSymbol->st_name];
							padding=24-KLstringLength(symbolName);
							if(padding>=0) {
   								KLtrace(ELF_REL "<%s>", symbolName);
								while(padding>0) {
									KLtrace(ELF_REL " ");
									padding--;
								}
							} else {
								KLstringNumCopy(clippedName, symbolName, 23);
								clippedName[22]='\0';
								KLtrace(ELF_REL "<%s] ", clippedName);
							}

							KLtrace(ELF_REL "0%.8xh\n", pSymbol->st_size);
						}
					}
				}
			}
		}
	}

	/* Finally put in the .bss section header the pointer to a memory
	 * block large enough to contain all unitialized data and save its
	 * size in memory for further reference.                            */

	if(totalBSSSize>0) {	
		elf->sectionData[bssIndex]=(UBYTE *) (*allocCallBack)(totalBSSSize, 4);	
		elf->section[bssIndex].sh_size=totalBSSSize;

		KLtrace(ELF_REL "                          ----------\n");
		KLtrace(ELF_REL "                          0%.8xh\n", totalBSSSize);
	} else {
		KLtrace(ELF_REL "No symbols\n");
	}
}

PUBLIC ELFLoadState loadELFFile(ELF *elf)
  
/*
 * Load the ELF file, and prepare it for dynamic linking.
 * 
 * INPUT:
 * elfFile                 the pointer to a ELF file structure
 * 
 * RETURNS:
 * If file is loaded successfully returns ELF_LOADED_OK.
 * On error returns:
 * ELF_READ_ERROR          the reading op has failed
 * ELF_ALLOC_ERROR         the allocation op has failed
 * ELF_SEEK_ERROR          the seek op has failed
 * ELF_FORMAT_ERROR        there is a bad value in ELF control structure
 * ELF_NOT_SUPPORTED_ERROR the value of an ELF field is valid but
 *                         it is not supported by this loader
 */
	
{
	ELFLoadState loadState;

	/* First load the ELF file in question in memory */
	
	loadState=loadELFInMemory(elf);

	/* If load is correct, do all necessary relocations on it and
	 * size appropriately the BSS section data in order to hold
	 * all uninitialized variables references of the loaded file.
	 * This pre-processing approach simplify  the actual link process. */
	 
	if(loadState==ELF_LOADED_OK) {
		sizeAndAssignMemToBSS(elf);
		resolveRelocations(elf);
	}

	return(loadState);
}

PUBLIC VOID unloadELFFiles(ELF *elf, INPUT UWORD32 numOfELFFiles)
  
/*
 * Deallocates all resources owned to LoadELFFile
 * 
 * INPUT:
 * elf              the pointer to an array of ELF structures as given by 
 *                  loadELFFile
 * numOfELFFiles    the number of ELF files structures in the table
 * 
 * OUTPUT:
 * None
 */		 
  
{
	UWORD32 i;
	UWORD32 j;
	
	for(i=0;i<numOfELFFiles;i++) {
		if(elf[i].sectionData!=NIL) {
			for(j=0;j<elf[i].header.e_shnum;j++) {
				if(elf[i].sectionData[j]!=NIL) {
					(*freeCallBack)(elf[i].sectionData[j]);
				}
			}

			(*freeCallBack)(elf[i].sectionData);	
	  		(*freeCallBack)(elf[i].section);
			elf[i].sectionData=NIL;
			elf[i].section=NIL;
		} 
	}
}

PUBLIC VOID allowBinaryPatch(BOOLEAN flag)
  
/*
 * Flag if the loaded ELF files are to be patched for relocation or not
 * 
 * INPUT:
 * flag       TRUE or FALSE depending if relocation will be performed 
 *            or not
 * 
 * OUTPUT:
 * None
 */

{
	binaryPatchingAllowed=flag;
}

