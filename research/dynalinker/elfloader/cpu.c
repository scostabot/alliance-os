/*
 * Architecture-dependant part of dynamic linker
 *
 * In this file is put all code that is specific to each hardware platform
 * in order to compute valid symbol addresses and relocations.
 * Currently supports only Intel platform.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 29/4/00  scosta    1.0    First draft
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

PUBLIC VOID relocateSymbol(ELF *elf, 
                           INPUT Elf32_Sym *symbol, 
                           INPUT Elf32_Rel *pRelData,
                           INPUT BOOLEAN binaryPatchingAllowed)
  
/*
 * Relocate given symbol.
 *
 * INPUT:
 * elf       the pointer to the ELF file structures
 * symbol    the pointer to the symbol to be relocated
 * pRelData  the pointer to the relocation data entry for the above symbol
 * 
 * binaryPatchingAllowed is a flag that allow/deny the actual relocation
 * write in ELF memory image.
 *
 * OUTPUT:
 * None
 */
  
{
	extern UWORD32 textSection;
	extern UWORD32 myTraceMask;

	UWORD32 relocation;
	UWORD32 targetSection;
	UWORD32 *machineWord;
	WORD32 targetOffset;
	UBYTE *symbolAddrInMemory;
	UBYTE *targetAddrInMemory; 

	/* Note that here we mean for relocation a stricter definition that
	 * the one specced in ELF docs: only offsets defined in assembly code
	 * and NOT addresses used for access a functions or procedures call.
	 * For addresses used for function calls, we define the process as 
	 * linking and we manage it in a separate function.                  */
	
	/* Step 1: compute the address in the ELF file image of the 
	 * symbol to be relocated. We assume that the symbol is contained
	 * in the .text section of that ELF file.                          */

	symbolAddrInMemory=elf->sectionData[textSection];
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
			break;

	  	case R_386_PC32:
			/* Program counter relocation entry, i.e. we are talking
			   about relative addresses used in CALL, JMP....        */

			machineWord=(UWORD32 *) symbolAddrInMemory;
			targetSection=textSection;
			targetOffset = (symbol->st_value-pRelData->r_offset-4);	
			relocation=(UWORD32) targetOffset;
			break;

		default:
			/* Unknown relocation code, print out and exit */

			if(myTraceMask & NELF_REL) {
				printf("relocation=%X\n", pRelData->r_info);
			}
			return;

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

	/* Dump info if needed */
	
	if(myTraceMask & NELF_REL) {
		dumpSymbolRelocation(elf, pRelData, symbol->st_value, targetSection);
	}
	
	/* Step 3: write the symbol relocation value in memory image */

	if(binaryPatchingAllowed==TRUE) {
		*machineWord=(UWORD32) relocation;
	}
			
}

PUBLIC VOID linkSymbol(ELF *elf,
                       INPUT Elf32_Rel *symbolEntry, 
                       INPUT UWORD32 originFileIndex, UWORD32 originSectionIndex,
                       INPUT Elf32_Sym *target,
                       INPUT UWORD32 targetFileIndex, UWORD32 targetSectionIndex)
  
/*
 * Link (compute the symbol memory address and write in the actual assembly
 * code) the given symbol origin (i.e. the caller of a function) and its
 * target (i.e. the called function).
 * 
 * INPUT:
 * symbolEntry	        The pointer to the relocation entry of the symbol that
 *                      USE the symbol reference (the point wherre the function
 *                      is called, a memory location accessed...)
 * originFileIndex      The array index of the ELF file table where the symbol
 *                      pointed by symbolEntry is defined.
 * originSectionIndex	The array index of the ELF file array structure that
 *                      point to the section that contains the actual symbol
 *                      memory location.
 * targe                The pointer to the Symbol Table entry of the target
 *                      symbol definition.
 * targetFileIndex      The array index of the ELF file table where the symbol
 *                      pointed by target is defined.
 * targetSectionIndex	The array index of the ELF file array structure that
 *                      point to the section that contains the actual symbol
 *                      memory location.
 * 						
 * RETURNS:
 * None
 */

{
	UBYTE *symbolAddrInMemory;
	UBYTE *targetAddrInMemory;
	
	/* Here we assume that we load /only/ 32-bit ELF files, and *not* 64 */

	UWORD32 *machineWord;
	
	/* 80x86 family of microprocessors has two way of defining addresses:
	 * absolute and signed relative. These variables defines the twos.     */

	UWORD32 displacement;
	WORD32 sDisplacement;

	/* We compute the "physical" address in memory where the symbol is
	 * used (read the function call point) and where is in memory the 
	 * target (read the function definition). 
	 * Since we work on top of malloc() for memory allocation now,
	 * these addresses are virtual and not physical, but in the future
	 * may change.......                                              */

	symbolAddrInMemory=elf[originFileIndex].sectionData[originSectionIndex];
	symbolAddrInMemory+=symbolEntry->r_offset;
	
	targetAddrInMemory=elf[targetFileIndex].sectionData[targetSectionIndex];
	targetAddrInMemory+=target->st_value;

	/* This is the only place where is hardcoded the CPU architecture,
	 * so we can't execute the code originated from a CPU different than
	 * the one we are currenlty running, even if we load it just fine.
	 * But for now it's not a limitation :).                             */

	switch(elf->header.e_machine) {
		case EM_386:
		case EM_486:
		
			/* Compute relative address displacement for 32-bit opcodes.
			 * In ELF specs this is the most obscured and darkest specced
			 * feature. I don't know why complicate things....           */

			switch(ELF32_R_TYPE(symbolEntry->r_info)) {
	  			case R_386_PC32:
					/* Program counter displacement relocation entry, i.e.
					 * the addresses we are talking about are used by a CALL.
					 * ELF specs say about: offset is computed as
					 * S+A-P where A is the optional addend present only 
					 * in SHT_RELA entry, and P the  relocation offset.
					 * In Intel files I never seen SHT_RELA entries, so A=0.
					 * The +4 addend take into account the fact that EIP
					 * register contents (on which the relative offset is
					 * computed) points to the /next/ opcode, so after the
					 * displacement offset (4 bytes in ELF32).              */

		 			machineWord=(UWORD32 *) symbolAddrInMemory;
					sDisplacement=targetAddrInMemory-(symbolAddrInMemory+4);
					displacement=(UWORD32) sDisplacement;
		  			break;
			 	case R_386_32:
					/* Data offset relocation entry, i.e. the addresses we
					 * are talking about are used by a MOV, AND, XOR, NOT..
					 * and on opcodes. These are simple relative offsets
	 				 * in Intel family processors in this memory model. */

					machineWord=(UWORD32 *) symbolAddrInMemory;
					displacement=(UWORD32) targetAddrInMemory;
					break;
				default:
					KLtrace(ELF_REL "Unsupported Relocation type %d\n", 
						    ELF32_R_TYPE(symbolEntry->r_info));
					return;
			}
			break;
	}

	/* Write the symbol address */

	*machineWord=(UWORD32) displacement;
}
