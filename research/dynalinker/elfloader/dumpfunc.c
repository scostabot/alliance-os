/*
 * ELF file dump funtions
 * 
 * The functions defined here print on standard output the contents
 * of ELF file key structures and fields.
 * Each function defined here accepts a pointer to an ELF structure
 * and does not change in any way the data passed by caller.
 * In this way the functions are completely transparent to the
 * application.
 * 
 * HISTORY
 * Date     Author    Rev    Notes
 * 02/06/99 scosta    1.0    First draft
 * 11/10/99 scosta    1.1    Added SPARC dump informations
 * 15/12/99 scosta    1.2    Modified dump of symbol table and relocs
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

#include "klibs.h"
#include "elf.h"
#include "elfloader.h"

PUBLIC VOID initDump(INPUT UWORD32 traceLevel)
  
/*
 * Initialize the printout of ELF information.
 * 
 * INPUT:
 * traceLevel	The bitmask for trace to be output
 * 
 * OUTPUT:
 * None
 */
  
{
	KLsetTraceLevel(traceLevel);
	KLsetTraceOptions(0);
}

PUBLIC VOID dumpELFHeader(INPUT Elf32_Ehdr *ELFHeader, UWORD32 fileEncoding)
	
/*
 * Dump the contents of ELF file header
 * 
 * INPUT:
 * ELFHeader		The pointer to the ELF Header structure
 * 
 * RETURN:
 * None
 */
	
{
	KLtrace(ELF_HEADER "################################\n");
	KLtrace(ELF_HEADER "ELF Header\n----------\n");
	KLtrace(ELF_HEADER "e_type     =%X ", ELFHeader->e_type);
	switch(ELFHeader->e_type) {
		case ET_NONE:
			KLtrace(ELF_HEADER "(ET_NONE)\n");
			break;
		case ET_REL:
			KLtrace(ELF_HEADER "(ET_REL)\n");
			break;
		case ET_EXEC:
			KLtrace(ELF_HEADER "(ET_EXEC)\n");
			break;
		case ET_DYN:
			KLtrace(ELF_HEADER "(ET_DYN)\n");
			break;
		case ET_CORE:
			KLtrace(ELF_HEADER "(ET_CORE)\n");
			break;
		case ET_NUM:
			KLtrace(ELF_HEADER "(ET_NUM)\n");
			break;
		case ET_LOPROC:
			KLtrace(ELF_HEADER "(ET_LOPROC)\n");
			break;
		case ET_HIPROC:
			KLtrace(ELF_HEADER "(ET_HIPROC)\n");
			break;
		default:
			KLtrace(ELF_HEADER "(Not defined)\n");
			break;
	}

	KLtrace(ELF_HEADER "e_machine  =%X ", ELFHeader->e_machine);
	switch(ELFHeader->e_machine) {
		case EM_NONE:
			KLtrace(ELF_HEADER "(EM_NONE)\n");
			break;
		case EM_M32:
			KLtrace(ELF_HEADER "(EM_M32)\n");
			break;
		case EM_SPARC:
			KLtrace(ELF_HEADER "(EM_SPARC)\n");
			break;
		case EM_386:
			KLtrace(ELF_HEADER "(EM_386)\n");
			break;
		case EM_68K:
			KLtrace(ELF_HEADER "(EM_68K)\n");
			break;
		case EM_88K:
			KLtrace(ELF_HEADER "(EM_88K)\n");
			break;
		case EM_486:
			KLtrace(ELF_HEADER "(EM_486)\n");
			break;
		case EM_860:
			KLtrace(ELF_HEADER "(EM_860)\n");
			break;
		case EM_MIPS:
			KLtrace(ELF_HEADER "(EM_MIPS)\n");
			break;
		case EM_S370:
			KLtrace(ELF_HEADER "(EM_S370)\n");
			break;
		case EM_MIPS_RS4_BE:
			KLtrace(ELF_HEADER "(EM_MIPS_RS4_BE)\n");
			break;
		case EM_SPARC64:
			KLtrace(ELF_HEADER "(EM_SPARC64)\n");
			break;
		default:
			KLtrace(ELF_HEADER "(Not defined)\n");
			break;
	}

	KLtrace(ELF_HEADER "e_version  =%X ", ELFHeader->e_version);
	switch(ELFHeader->e_version) {
		case EV_NONE:
			KLtrace(ELF_HEADER "(EV_NONE)\n");
			break;
		case EV_CURRENT:
			KLtrace(ELF_HEADER "(EV_CURRENT)\n");
			break;
		default:
			KLtrace(ELF_HEADER "(Not defined)\n");
			break;
	}

	KLtrace(ELF_HEADER "e_entry    =%Xh\n", ELFHeader->e_entry);
	KLtrace(ELF_HEADER "e_phoff    =%Xh\n", ELFHeader->e_phoff);
	KLtrace(ELF_HEADER "e_shoff    =%Xh\n", ELFHeader->e_shoff);
	KLtrace(ELF_HEADER "e_flags    =%Xh\n", ELFHeader->e_flags);
	KLtrace(ELF_HEADER "e_ehsize   =%Xh\n", ELFHeader->e_ehsize);
	KLtrace(ELF_HEADER "e_phentsize=%Xh\n", ELFHeader->e_phentsize);
	KLtrace(ELF_HEADER "e_shnum    =%Xh\n", ELFHeader->e_shnum);
	KLtrace(ELF_HEADER "e_shstrndx =%Xh\n", ELFHeader->e_shstrndx);
	
	KLtrace(ELF_HEADER "File is encoded ");

	switch(fileEncoding) {
		case LSBENCODING:
			KLtrace(ELF_HEADER "ELFDATA2LSB");
			break;
		case MSBENCODING:
			KLtrace(ELF_HEADER "ELFDATA2MSB");
			break;
	}
	
	KLtrace(SK_INFO " host encoding is ");
	switch(probeEndianess()) {
		case LSBENCODING:
			KLtrace(ELF_HEADER "ELFDATA2LSB\n");
			break;
		case MSBENCODING:
			KLtrace(ELF_HEADER "ELFDATA2MSB\n");
			break;
	}
}

PUBLIC VOID dumpProgramHeader(INPUT Elf32_Phdr *programHeader)
	
/*
 * Dump the contents of the (optional) ELF Program Header
 * 
 * INPUT:
 * programHeader	The pointer to the Program Header
 * 
 * RETURN:
 * None
 */
	
{
	KLtrace(ELF_PHEADER "\nELF Program Header\n------------------\n");
	KLtrace(ELF_PHEADER "p_type    =%X ", programHeader->p_type);
	switch(programHeader->p_type) {
		case PT_NULL:
			KLtrace(ELF_PHEADER "(PT_NULL)\n");
			break;
		case PT_LOAD:
			KLtrace(ELF_PHEADER "(PT_LOAD)\n");
			break;
		case PT_DYNAMIC:
			KLtrace(ELF_PHEADER "(PT_DYNAMIC)\n");
			break;
		case PT_INTERP:
			KLtrace(ELF_PHEADER "(PT_INTERP)\n");
			break;
		case PT_NOTE:
			KLtrace(ELF_PHEADER "(PT_NOTE)\n");
			break;
		case PT_SHLIB:
			KLtrace(ELF_PHEADER "(PT_SHLIB)\n");
			break;
		case PT_PHDR:
			KLtrace(ELF_PHEADER "(PT_PHDR)\n");
			break;
		case PT_NUM:
			KLtrace(ELF_PHEADER "(PT_NUM)\n");
			break;
		default:
			KLtrace(ELF_PHEADER "(Not defined)\n");
			break;
	}

	KLtrace(ELF_PHEADER "p_offset  =%Xh\n", programHeader->p_offset);
	KLtrace(ELF_PHEADER "p_vaddr   =%Xh\n", programHeader->p_vaddr);
	KLtrace(ELF_PHEADER "p_paddr   =%Xh\n", programHeader->p_paddr);
	KLtrace(ELF_PHEADER "p_filesz  =%Xh\n", programHeader->p_filesz);
	KLtrace(ELF_PHEADER "p_memsz   =%Xh\n", programHeader->p_memsz);
	KLtrace(ELF_PHEADER "p_flags   =%Xh", programHeader->p_flags);
	
	if(programHeader->p_flags) {
		KLtrace(ELF_PHEADER " (");
	}
	if(programHeader->p_flags & PF_X) {
		KLtrace(ELF_PHEADER "PF_X");
		if(programHeader->p_flags & (PF_W|PF_R)) {
			KLtrace(ELF_PHEADER "|");
		}
	}
	if(programHeader->p_flags & PF_W) {
		KLtrace(ELF_PHEADER "PF_W");
		if(programHeader->p_flags & (PF_R)) {
			KLtrace(ELF_PHEADER "|");
		}
	}
	if(programHeader->p_flags & PF_R) {
		KLtrace(ELF_PHEADER "PF_R");
	}

	if(programHeader->p_flags) {
		KLtrace(ELF_PHEADER ")\n");
	}
	
	KLtrace(ELF_PHEADER "p_align   =%X\n", programHeader->p_align);
}

PUBLIC VOID dumpSectionHeader(INPUT Elf32_Shdr *sectionHeader, 
	                          INPUT UWORD32 i,
							  INPUT STRING *sectionName)
	
/*
 * Dump the contents of an ELF Section header.
 * 
 * INPUT:
 * sectionHeader   The pointer to the Section Header
 * i               The section index number to be displayed with the dump
 * 					
 * RETURN:
 * None
 */
	
{
	KLtrace(ELF_SHEADER "\nELF Section Header (%d) <%s>\n", i, sectionName);
	KLtrace(ELF_SHEADER "----------------------\n");
	KLtrace(ELF_SHEADER "sh_name     =%Xh\n", sectionHeader->sh_name);
	KLtrace(ELF_SHEADER "sh_type     =%Xh ", sectionHeader->sh_type);

	switch(sectionHeader->sh_type) {
		case SHT_NULL:
			KLtrace(ELF_SHEADER "(SHT_NULL)\n");
			break;
		case SHT_PROGBITS:
			KLtrace(ELF_SHEADER "(SHT_PROGBITS)\n");
			break;
		case SHT_SYMTAB:
			KLtrace(ELF_SHEADER "(SHT_SYMTAB)\n");
			break;
		case SHT_STRTAB:
			KLtrace(ELF_SHEADER "(SHT_STRTAB)\n");
			break;
		case SHT_RELA:
			KLtrace(ELF_SHEADER "(SHT_RELA)\n");
			break;
		case SHT_HASH:
			KLtrace(ELF_SHEADER "(SHT_HASH)\n");
			break;
		case SHT_DYNAMIC:
			KLtrace(ELF_SHEADER "(SHT_DYNAMIC)\n");
			break;
		case SHT_NOTE:
			KLtrace(ELF_SHEADER "(SHT_NOTE)\n");
			break;
		case SHT_NOBITS:
			KLtrace(ELF_SHEADER "(SHT_NOBITS)\n");
			break;
		case SHT_REL:
			KLtrace(ELF_SHEADER "(SHT_REL)\n");
			break;
		case SHT_SHLIB:
			KLtrace(ELF_SHEADER "(SHT_SHLIB)\n");
			break;
		case SHT_DYNSYM:
			KLtrace(ELF_SHEADER "(SHT_DYNSYM)\n");
			break;
		default:
			KLtrace(ELF_SHEADER "(Not defined)\n");
			break;
	}

	KLtrace(ELF_SHEADER "sh_flags    =%Xh ", sectionHeader->sh_flags);
	if(sectionHeader->sh_flags) {
		KLtrace(ELF_SHEADER "(");
	}
	if(sectionHeader->sh_flags & SHF_MASKPROC) {
		KLtrace(ELF_SHEADER "SHF_MASKPROC");
		if(sectionHeader->sh_flags & ~SHF_MASKPROC) {
			KLtrace(ELF_SHEADER "|");
		}
	}
	if(sectionHeader->sh_flags & SHF_EXECINSTR) {
		KLtrace(ELF_SHEADER "SHF_EXECINSTR");
		if(sectionHeader->sh_flags & ~SHF_EXECINSTR) {
			KLtrace(ELF_SHEADER "|");
		}
	}
	if(sectionHeader->sh_flags & SHF_ALLOC) {
		KLtrace(ELF_SHEADER "SHF_ALLOC");
		if(sectionHeader->sh_flags & SHF_WRITE) {
			KLtrace(ELF_SHEADER "|");
		}
	}
	if(sectionHeader->sh_flags & SHF_WRITE) {
		KLtrace(ELF_SHEADER "SHF_WRITE");
	}
	if(sectionHeader->sh_flags) {
		KLtrace(ELF_SHEADER ")");
	}
	
	KLtrace(ELF_SHEADER "\n");

	KLtrace(ELF_SHEADER "sh_addr     =%Xh\n", sectionHeader->sh_addr);
	KLtrace(ELF_SHEADER "sh_offset   =%Xh\n", sectionHeader->sh_offset);
	KLtrace(ELF_SHEADER "sh_size     =%Xh\n", sectionHeader->sh_size);
	KLtrace(ELF_SHEADER "sh_link     =%Xh\n", sectionHeader->sh_link);
	KLtrace(ELF_SHEADER "sh_info     =%Xh\n", sectionHeader->sh_info);
	KLtrace(ELF_SHEADER "sh_addralign=%Xh\n", sectionHeader->sh_addralign);
	KLtrace(ELF_SHEADER "sh_entsize  =%Xh\n", sectionHeader->sh_entsize);
}

PUBLIC VOID dumpSectionData(INPUT UBYTE *sectionData, UWORD32 size)
  
/*
 * This procedure display actual section data contents in an
 * hexadecimal/ASCII dump.
 * 
 * INPUT:
 * sectionHeader    The pointer to the Section Header structure
 * sectionData      The pointer to the section data buffer
 * 
 * RETURN:
 * None
 */
  
{
	UBYTE binBuffer[64];
	UBYTE asciiBuffer[64];
	UBYTE tmp[5];
	UBYTE tmpas[5];
	UWORD32 i;
	UWORD32 padding;

	binBuffer[0]='\0';
	asciiBuffer[0]='\0';

	KLtrace(ELF_SDATA "\n");

	for(i=1;i<=size;i++) {
		KLstringFormat(tmp, "%.2X ", sectionData[i-1]);
		if(sectionData[i-1]>=32 && sectionData[i-1]<=127) {
			KLstringFormat(tmpas, "%c ", sectionData[i-1]);
		} else {
			KLstringFormat(tmpas, "? ");
		}
	
		KLstringAppend(binBuffer, tmp);
		KLstringAppend(asciiBuffer, tmpas);
		
	 	if(((i % 16)==0)) {
			KLtrace(ELF_SDATA "%s  %s\n", binBuffer, asciiBuffer);
			binBuffer[0]='\0';
			asciiBuffer[0]='\0';	
		}
	}
	
	KLtrace(ELF_SDATA "%s", binBuffer);
	padding=48-KLstringLength(binBuffer);
	for(i=0;i<padding;i++) {
		binBuffer[i]=' ';
	}
	binBuffer[i]='\0';
	KLtrace(ELF_SDATA "%s  %s\n", binBuffer, asciiBuffer);
} 

PUBLIC VOID dumpSymbolBinding(STRING *stringizedDump, INPUT UBYTE st_info)
  
/*
 * Dump in an ASCII string the content of st_info field of Elf32_Sym
 * 
 * INPUT:
 * st_info		the field st_info of symbol table structure
 * 
 * OUTPUT:
 * stringizedDump	the ASCIIZ strng with dump
 */
  
{
	KLstringCopy(stringizedDump, "(");
	switch(ELF32_ST_BIND(st_info)) {
		case STB_LOCAL:
			KLstringAppend(stringizedDump, "STB_LOCAL");
			break;
		case STB_GLOBAL:
			KLstringAppend(stringizedDump, "STB_GLOBAL");
			break;
		case STB_WEAK:
			KLstringAppend(stringizedDump, "STB_WEAK");
			break;
		default:
			KLtrace(stringizedDump, "Other");
			break;
	}
	
	KLstringAppend(stringizedDump, "|");

	switch(ELF32_ST_TYPE(st_info)) {
		case STT_NOTYPE:
			KLstringAppend(stringizedDump, "STT_NOTYPE");
			break;
		case STT_OBJECT:
			KLstringAppend(stringizedDump, "STT_OBJECT");
			break;
		case STT_FUNC:
			KLstringAppend(stringizedDump, "STT_FUNC");
			break;
		case STT_SECTION:
			KLstringAppend(stringizedDump, "STT_SECTION");
			break;
		case STT_FILE:
			KLstringAppend(stringizedDump, "STT_FILE");
			break;
		case STT_NUM:
			KLstringAppend(stringizedDump, "STT_NUM");
			break;
		case STT_LOPROC:
			KLstringAppend(stringizedDump, "STT_LOPROC");
			break;
		case STT_HIPROC:
			KLstringAppend(stringizedDump, "STT_HIPROC");
			break;
		default:
			KLstringAppend(stringizedDump, "Other");
			break;
	}
		
	KLstringAppend(stringizedDump, ")");
}

PUBLIC VOID dumpSymbolTable(INPUT Elf32_Sym *p)
  
/*
 * Dumps Symbol Table entry
 * 
 * INPUT:
 * p	the pointer to a symbol table entry
 * 
 * OUTPUT:
 * None
 */
  
{
	STRING stringizedResult[32];

	KLtrace(ELF_SYMTAB "st_name=%Xh\n", p->st_name);
	KLtrace(ELF_SYMTAB "st_value=%Xh\n", p->st_value);
	KLtrace(ELF_SYMTAB "st_size=%Xh\n", p->st_size);
	KLtrace(ELF_SYMTAB "st_other=%Xh\n", p->st_other);
	KLtrace(ELF_SYMTAB "st_shndx=%Xh\n", p->st_shndx);
	KLtrace(ELF_SYMTAB "st_info=%Xh ", p->st_info);

	if(p->st_info) {
		dumpSymbolBinding(stringizedResult, p->st_info);
		KLtrace(ELF_SYMTAB "%s", stringizedResult);
	}

	KLtrace(ELF_SYMTAB "\n");
}

PUBLIC VOID dumpRelocType(STRING *stringizedResult, 
                          INPUT UWORD32 info, INPUT UWORD32 cpuType)

{
	switch(cpuType) {
		case EM_386:
		case EM_486:
			switch(ELF32_R_TYPE(info)) {
	    			case R_386_NONE:
		    			KLstringCopy(stringizedResult, "R_386_NONE");
		    			break;
				case R_386_32:
					KLstringCopy(stringizedResult, "R_386_32");
		    			break;
				case R_386_PC32:
					KLstringCopy(stringizedResult, "R_386_PC32");
		    			break;
				case R_386_GOT32:
					KLstringCopy(stringizedResult, "R_386_GOT32");
		    			break;
				case R_386_PLT32:
					KLstringCopy(stringizedResult, "R_386_PLT32");
		    			break;
				case R_386_COPY:
					KLstringCopy(stringizedResult, "R_386_COPY");
					break;
				case R_386_GLOB_DAT:
					KLstringCopy(stringizedResult, "R_386_GLOB_DAT");
		 			break;
				case R_386_JMP_SLOT:
					KLstringCopy(stringizedResult, "R_386_JMP_SLOT");
					break;
				case R_386_RELATIVE:
					KLstringCopy(stringizedResult, "R_386_RELATIVE");
					break;
				case R_386_GOTOFF:
					KLstringCopy(stringizedResult, "R_386_GOTOFF");
					break;
				case R_386_GOTPC:
					KLstringCopy(stringizedResult, "R_386_GOTPC");
					break;
			}
			break;
		case EM_SPARC:
			switch(ELF32_R_TYPE(info)) {
				case R_SPARC_NONE:
					KLstringCopy(stringizedResult, "R_SPARC_NONE");
					break;
				case R_SPARC_8:
					KLstringCopy(stringizedResult, "R_SPARC_8");
					break;
				case R_SPARC_16:
					KLstringCopy(stringizedResult, "R_SPARC_16");
					break;
				case R_SPARC_32:
					KLstringCopy(stringizedResult, "R_SPARC_32");
					break;
				case R_SPARC_DISP8:
					KLstringCopy(stringizedResult, "R_SPARC_DISP8");
					break;
				case R_SPARC_DISP16:
					KLstringCopy(stringizedResult, "R_SPARC_DISP16");
					break;
				case R_SPARC_DISP32:
					KLstringCopy(stringizedResult, "R_SPARC_DISP32");
					break;
				case R_SPARC_WDISP30:
					KLstringCopy(stringizedResult, "R_SPARC_WDISP30");
					break;
				case R_SPARC_WDISP22:
					KLstringCopy(stringizedResult, "R_SPARC_WDISP22");
					break;
				case R_SPARC_HI22:
					KLstringCopy(stringizedResult, "R_SPARC_HI22");
					break;
				case R_SPARC_22:
				    KLstringCopy(stringizedResult, "R_SPARC_22");
					break;
				case R_SPARC_13:
					KLstringCopy(stringizedResult, "R_SPARC_13");
					break;
				case R_SPARC_LO10:
					KLstringCopy(stringizedResult, "R_SPARC_LO10");
					break;
				case R_SPARC_GOT10:
					KLstringCopy(stringizedResult, "R_SPARC_GOT10");
					break;
				case R_SPARC_GOT13:
					KLstringCopy(stringizedResult, "R_SPARC_GOT13");
					break;
				case R_SPARC_GOT22:
					KLstringCopy(stringizedResult, "R_SPARC_GOT22");
					break;
				case R_SPARC_PC10:
					KLstringCopy(stringizedResult, "R_SPARC_PC10");
					break;
				case R_SPARC_PC22:
					KLstringCopy(stringizedResult, "R_SPARC_PC22");
					break;
				case R_SPARC_WPLT30:
					KLstringCopy(stringizedResult, "R_SPARC_WPLT30");
					break;
				case R_SPARC_COPY:
					KLstringCopy(stringizedResult, "R_SPARC_COPY");
					break;
				case R_SPARC_GLOB_DAT:
					KLstringCopy(stringizedResult, "R_SPARC_GLOB_DAT");
					break;
				case R_SPARC_JMP_SLOT:
					KLstringCopy(stringizedResult, "R_SPARC_JMP_SLOT");
					break;
				case R_SPARC_RELATIVE:
					KLstringCopy(stringizedResult, "R_SPARC_RELATIVE");
					break;
			}
	}
}

PUBLIC VOID dumpSymbolReloc(INPUT UWORD32 info, INPUT UWORD32 cpuType)

/*
 * Dump symbol relocation type, based on CPU.
 *
 * INPUT:
 * info		the st_info field of SHT_REL(A)
 * cpuType	the CPU ELF #define code
 *
 * RETURN:
 * None
 */
  
{
	STRING stringizedResult[20];
	
	KLtrace(ELF_REL "(symbol table id=%d", ELF32_R_SYM(info));
	KLtrace(ELF_REL "|");
	dumpRelocType(stringizedResult, info, cpuType);
	KLtrace(ELF_REL "%s)\n", stringizedResult);
}

PUBLIC VOID dumpSymbolRelocation(INPUT ELF *elf,
                                 INPUT Elf32_Rel *pRelData,
                                 INPUT UWORD32 offset,
                                 INPUT UWORD32 targetSection)
  
/*
 * Dump symbol relocation data and where points to
 * 
 * INPUT:
 * elf             the pointer to the array of ELF structure
 * pRelData        the pointer to relocation data of the symbol
 * symbol          the pointer to the symbol data to be relocated
 * targetSection   the index of ELF section that holds target symbol
 * 
 * OUTPUT:
 * None
 */
  
{
	STRING *sectionName;

	sectionName=(STRING *) elf->sectionData[elf->header.e_shstrndx];
	sectionName+=elf->section[targetSection].sh_name;

	KLtrace(ELF_REL "<.text>");
	KLtrace(ELF_REL "+%.9Xh --> ", pRelData->r_offset);	
	KLtrace(ELF_REL "<%s>", sectionName);	
	KLtrace(ELF_REL "+%.9Xh\n", offset);
}

PUBLIC VOID dumpLinkInfoHeader(VOID)

/*
 * Print a human-readable title for linking informational traces
 *
 * INPUT:
 * None
 *
 * OUTPUT:
 * NONE
 */

{
	KLtrace(ELF_SYMTAB "Source                  Symbol name                  Target\n");
	KLtrace(ELF_SYMTAB "------                  -----------                  ------\n");
}

PUBLIC VOID dumpLinkInfo(ELF *elf, 
                         INPUT UWORD32 symbolTableSection,
                         INPUT UWORD32 symbolIndex)

/*
 * Dump information about symbols being linked toghether.
 *
 * INPUT:
 * elf                  the pointer to the ELF file that contain
 * symbolTableSection   the index in ELF file that pointd to symbol table
 * symbolIndex          the index of the symbol to dump in the symbol table
 *
 * RETURN:
 * None
 */

{
	STRING *strTab;
	STRING *symbolName;
	Elf32_Sym *targetSymbol;
	STRING clippedName[MAX_SYM_NAME+1];
	WORD32 padding;

	strTab=(STRING *) elf->sectionData[elf->section[symbolTableSection].sh_link];
	targetSymbol=(Elf32_Sym *) elf->sectionData[symbolTableSection];
	symbolName=&strTab[targetSymbol[symbolIndex].st_name];
   	
	padding=24-KLstringLength(elf->name);
	if(padding>=0) {
   		KLtrace(ELF_SYMTAB "%s", elf->name);
		while(padding>0) {
			KLtrace(ELF_SYMTAB " ");
			padding--;
		}
	} else {
		KLstringNumCopy(clippedName, elf->name, 23);
		clippedName[22]='\0';
		KLtrace(ELF_SYMTAB "%s] ", clippedName);
	}

	padding=MAX_SYM_NAME-KLstringLength(symbolName)-2;
	if(padding>=0) {
		KLtrace(ELF_SYMTAB "<%s>", symbolName);
		while(padding>0) {
			KLtrace(ELF_SYMTAB " ");
			padding--;
		}
	} else {
		KLstringNumCopy(clippedName, symbolName, MAX_SYM_NAME-2);
		clippedName[MAX_SYM_NAME-2]='\0';
		KLtrace(ELF_SYMTAB "<%s]", clippedName);
	}
}
