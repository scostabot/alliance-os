/*
 * Definitions for the dynamic linking loader for ELF files
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 12/07/99 scosta    1.0    First draft
 * 19/11/99 scosta    1.1    Something more guidelines-compliant 
 * 19/12/99 scosta    1.2    Added structures for convenient dump
 * 03/01/00 scosta    1.3    Added MAX_PATH and MAX_NUM_ELF_FILES
 * 03/05/00 scosta    1.4    Various changes and better organizations
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#define MAX_FILE_PATH		41  /* Max number of characters for filename
                                     * as input from user                       */

#define MAX_SYM_NAME		29  /* Max length of a symbol for entry point */
#define MAX_NUM_ELF_FILES	8   /* The maximum number of ELF files linkable */

/* Trace levels definitions: the first set is relative to KLtrace()
 * macro usage, and the second for initDump() function call.         */

#define ELF_HEADER               "1|"
#define ELF_PHEADER              "2|"
#define ELF_SHEADER              "4|"
#define ELF_SDATA      	         "8|"
#define ELF_REL                 "10|"
#define SPECIFIC_SECTION_HEADER "20|"
#define ELF_SYMTAB              "40|"

#define NELF_HEADER                 1
#define NELF_PHEADER                2
#define NELF_SHEADER                4
#define NELF_SDATA                  8
#define NELF_REL                 0x10
#define NELF_SYMTAB              0x40
#define NSPECIFIC_SECTION_HEADER 0x20

PUBLIC VOID initDump(INPUT UWORD32);

/* Definition of ELF dynalinker basic types for human-understandable
 * ELF file dumps.                                                   */

typedef struct {
	UWORD32 size;
	UWORD32 id;
	UWORD32 alignement;
	STRING name[MAX_FILE_PATH];
	UBYTE *data;
} ELFSize;

typedef struct {
	Elf32_Sym sym;
	STRING *name;
} Sym;

typedef struct {
	Elf32_Rel sym;
	STRING *name;
} Rel;

/* Types definition for ELF loading and linking routines */

typedef struct {
	STRING name[80];                /* The filename of the loaded file */
	Elf32_Ehdr header;              /* The ELF header */
	Elf32_Phdr programHeader;       /* If present, the ELF Program Header */
	Elf32_Shdr *section;            /* The pointer to the array of sections */
	UBYTE **sectionData;            /* The pointer to an array of section data */
} ELF;

typedef enum { NONE, LSB2MSB, MSB2LSB } Translation;
typedef enum { ELF_LOADED_OK, 
               ELF_FORMAT_ERROR, ELF_NOT_SUPPORTED_ERROR,
               ELF_SEEK_ERROR, ELF_READ_ERROR, ELF_ALLOC_ERROR } ELFLoadState;
typedef enum { GLOBAL_NOT_RESOLVED, GLOBAL_RESOLVED } LinkageState;

/* Endianess management routines */

PUBLIC VOID convert16BitData(UWORD16 *data, INPUT UWORD32 loops);
PUBLIC VOID convert32BitData(UWORD32 *data, INPUT UWORD32 loops);

PUBLIC VOID decodeELFHeader(Elf32_Ehdr *header);
PUBLIC VOID decodeProgramHeader(Elf32_Phdr *header);
PUBLIC VOID decodeSectionHeader(Elf32_Shdr *header);
PUBLIC VOID decodeSymbolTable(Elf32_Sym *symbols, UWORD32 size);
PUBLIC VOID decodeRelocEntries(Elf32_Rel *, UWORD32 size);
PUBLIC VOID decodeRelocEntriesAbs(Elf32_Rela *, UWORD32 size);

typedef enum { MSBENCODING, LSBENCODING } Endianess;

PUBLIC Endianess probeEndianess(VOID);

/* Dump functions routines */

PUBLIC VOID dumpELFHeader(INPUT Elf32_Ehdr *header, INPUT UWORD32 i);
PUBLIC VOID dumpProgramHeader(INPUT Elf32_Phdr *header);
PUBLIC VOID dumpSectionHeader(INPUT Elf32_Shdr *, INPUT UWORD32, INPUT STRING *);
PUBLIC VOID dumpSectionData(INPUT UBYTE *, UWORD32);
PUBLIC VOID dumpSymbolTable(INPUT Elf32_Sym *);
PUBLIC VOID dumpSymbolBinding(STRING *stringizedResult, INPUT UBYTE st_info);
PUBLIC VOID dumpSymbolReloc(INPUT UWORD32 info, INPUT UWORD32 cpuType);
PUBLIC VOID dumpRelocType(STRING *, INPUT UWORD32, INPUT UWORD32);
PUBLIC VOID dumpSymbolRelocation(INPUT ELF *, INPUT Elf32_Rel *, INPUT UWORD32, INPUT UWORD32);
PUBLIC VOID dumpLinkInfoHeader(VOID);
PUBLIC VOID dumpLinkInfo(ELF *, INPUT UWORD32, INPUT UWORD32);

/* Those functions are internally used by the dynalinker library */

PUBLIC Elf32_Sym * searchSymbol(UWORD32 *, UWORD32 *, ELF *, UWORD32, INPUT UWORD32, INPUT STRING *, INPUT UBYTE);
PUBLIC UWORD32 searchTextSection(INPUT ELF *elf);
PUBLIC VOID relocateSymbol(ELF *, INPUT Elf32_Sym *, INPUT Elf32_Rel *, INPUT BOOLEAN);
PUBLIC VOID linkSymbol(ELF *, INPUT Elf32_Rel *, INPUT UWORD32, INPUT UWORD32, INPUT Elf32_Sym *, INPUT UWORD32, UWORD32);

/* Dynalinker library interface definitions */

PUBLIC ELFLoadState loadELFFile(ELF *);
PUBLIC UWORD32 linkELFFiles(ELF *, INPUT UWORD32 numberOfElfFiles);
PUBLIC VOID unloadELFFiles(ELF *elf, INPUT UWORD32 numOfELFFiles);
PUBLIC BOOLEAN setELFEntryPoint(ELF *elf, UWORD32, INPUT STRING *);
PUBLIC WORD32 runELFFile(VOID);
PUBLIC UWORD32 getELFMemoryFootPrint(ELFSize *sec, UWORD32 *, INPUT ELF *elf);
PUBLIC UWORD32 getNumOfELFSymbols(INPUT ELF *elf);
PUBLIC UWORD32 getELFSymbols(Sym *, INPUT ELF *elf);
PUBLIC UWORD32 getNumOfELFRelocs(INPUT ELF *elf);
PUBLIC UWORD32 getELFRelocs(Rel *, INPUT ELF *elf);
PUBLIC VOID allowBinaryPatch(BOOLEAN flag);

#define ANY_MATCH 0xFF	/* "Any symbol with given name matches the search
                           in the Symbol Table, whatever attributes have" */
#define ANY_FILE 0xFF /* Search the symbol in any section of the ELF file */
