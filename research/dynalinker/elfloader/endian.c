/*
 * Endianess handling routines            
 *
 * ELF files are architecture-independent, so we may load executable files
 * that are generated from a platform with endianess different from the one
 * of the CPU that executes this program. Here we change things in the
 * right format.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 01/06/99 scosta    1.0    First draft
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

extern Translation translationMode;

PUBLIC Endianess probeEndianess(VOID)
  
/*
 * Inquire which kind of endianess has the CPU that is running
 * the program.
 * 
 * INPUT:
 * None
 * 
 * RETURN:
 * ELFDATA2MSB 	  (SPARC, MIPS)
 * ELFDATA2LSB    (Intel 80x86, Pentiums, Xeons)
 */
  
{
	UBYTE testPattern[4]={ 0x1, 0x2, 0x3, 0x4 };
	UWORD32 *p=(UWORD32 *) testPattern;
  
	if(*p==0x01020304) {
		return(MSBENCODING);
	} else {
		return(LSBENCODING);
	}
}

PUBLIC VOID convert16BitData(UWORD16 *data, UWORD32 loops)
  
/*
 * Converts in the host CPU format a 16-bit word
 * 
 * INPUT:
 * data     the pointer to the 16-bit word to convert
 * loops    how many 16-bit words are to be converted
 * 
 * RETURNS:
 * data     the converted data is overwritten in original place
 */
  
{
	UBYTE *p0;
	UBYTE *p1;
	UWORD16 tmpdata;
	DATA i;
	
	p0=(UBYTE *) data;
	p1=p0++;
	
	if(translationMode==LSB2MSB) {
		for(i=0;i<loops;i++,data++,p0+=2,p1+=2) {
			tmpdata=(UWORD16) *p0;
			tmpdata<<=8;
			tmpdata|=*p1;
			*data=tmpdata;
		}	  
	}
}

PUBLIC VOID convert32BitData(UWORD32 *data, UWORD32 loops)
  
/*
 * Converts in the host CPU format a 32-bit word
 * 
 * INPUT:
 * data     the pointer to the 16-bit word to convert
 * loops    how many 16-bit words are to be converted
 * 
 * RETURNS:
 * data     the converted data is overwritten in original place
 */

{
	UBYTE *p4;
	UWORD32 tmpdata;
	DATA i;

	p4=(UBYTE *) data;
	p4+=3;
	
	if(translationMode==LSB2MSB) {
		for(i=0;i<loops;i++,data++,p4+=4) {
			tmpdata=(UWORD32) *p4;
			tmpdata<<=8;
			tmpdata|=*(p4-1);
			tmpdata<<=8;
			tmpdata|=*(p4-2);
			tmpdata<<=8;
			tmpdata|=*(p4-3);
			*data=tmpdata;
		}
	}
}

PUBLIC VOID decodeELFHeader(Elf32_Ehdr *header)
  
/*
 * Converts in the host CPU endianess format the ELF header
 * 
 * INPUT:
 * header    the pointer to the ELF file header
 * 
 * RETURNS:
 * the converted data is overwritten in the input argument
 */

{
	UBYTE *p;
	
	p=(UBYTE *) header;
	p+=EI_NIDENT;
	
	convert16BitData(&header->e_type, 2);
	convert32BitData(&header->e_version, 5);
	convert16BitData(&header->e_ehsize, 6);
}

PUBLIC VOID decodeProgramHeader(Elf32_Phdr *header)
  
/*
 * Converts in the host CPU endianess format the ELF program header
 * 
 * INPUT:
 * header    the pointer to the ELF program header
 * 
 * RETURNS:
 * the converted data is overwritten in the input argument
 */

{
	convert32BitData(&header->p_type, 10);
}

PUBLIC VOID decodeSectionHeader(Elf32_Shdr *header)
  
/*
 * This function takes care of converting in the host
 * CPU endianess format the Section Header give.
 * 
 * INPUT:
 * header    The pointer to a valid section header
 * 
 * OUTPUT:
 * None
 */
  
{
	convert32BitData(&header->sh_name, 10);
}

PUBLIC VOID decodeSymbolTable(Elf32_Sym *symbols, UWORD32 size)
  
/*
 * This function takes care of converting in the host
 * CPU endianess format the Symbol Table for size entries.
 * 
 * INPUT:
 * symbols    The pointer to a valid symbol table 
 * size       How many entries in the Symbol Table dump
 * 
 * OUTPUT:
 * None
 */
  
{
	UWORD32 i;
	
	for(i=0;i<size;i++,symbols++) {
		convert32BitData((UWORD32 *) symbols, 3);
		convert16BitData((UWORD16 *) &symbols->st_shndx, 1);
	}
}

PUBLIC VOID decodeRelocEntries(Elf32_Rel *entries, INPUT UWORD32 size)
  
/*
 * This function takes care of converting in the host
 * CPU endianess format the Relative Relocation Entry section
 * 
 * INPUT:
 * entries    Pointer to relocation entry data
 * size       The number of entries present
 * 
 * OUTPUT:
 * None
 */
  
{
	convert32BitData((UWORD32 *) entries, 2*size);
}

PUBLIC VOID decodeRelocEntriesAbs(Elf32_Rela *entries, INPUT UWORD32 size)

/*
 * This function takes care of converting in the host
 * CPU endianess format the Absolute Relocation Entry section
 * 
 * INPUT:
 * entries     Pointer to relocation entry data
 * size        The number of entries present
 * 
 * OUTPUT:
 * None
 */
  
{
	convert32BitData((UWORD32 *) entries, 3*size);
}

  
  
  


