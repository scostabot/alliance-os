/*
 * Dynamic linker shell command handler
 *
 * Implements all commands that are defined internally to the shell
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 10/11/03 scosta    1.0    Logical split between shell & command handling
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/* These are Alliance-specific includes */

#include <klibs.h>
#include <elf.h>

#include "elfloader.h"

/* These are UNIX-specific includes */

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Shell includes */

#include "shell.h"
#include <cmddynalinker.h>

LOCAL STRING entryPointSymName[MAX_SYM_NAME];/* The symbol name of program entry point */
LOCAL STRING fileName[MAX_NUM_ELF_FILES][MAX_PATH];
LOCAL STRING fileToBeLinked[MAX_PATH];  /* The filename of an ELF image */
LOCAL ELF ELFFileToBeLinked[MAX_NUM_ELF_FILES]; /* The ELF data structures
                                                   for all files that will be
                                                   linked toghether          */

LOCAL UWORD32 linkedFiles=0;
LOCAL UWORD32 numOfELFFiles=0;

/*
 * Command handler routines.
 *
 * Each function below is executed when the relative command is entered
 * as a command line input or as a script.
 *
 * The argument cmdLine holds the string that contains all characters
 * (except for initial blanks) after the command text itself (if any).
 *
 */

LOCAL WORD32 doChdir(INPUT STRING *cmdLine)

/*
 * Change current working directory
 *
 * INPUT:
 * cmdLine    the new working directory
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	extern STRING absolutePath[];
	STRING changeDir[MAX_PATH];
	STRING *p;

	switch(cmdLine[0]) {
		case '\0':
			/* If there is no argument to change directory command, we go $HOME */

			p=getenv("HOME");
			if(p!=NIL) {
				strcpy(changeDir, p);
			}

			break;

		case '/':
			/* If the cd command argument begins with a '/' means that the path
			 * is an absolute one, otherwise is relative and we make it absolute */

			strcpy(changeDir, cmdLine);
			break;

		default:
			/* Then it's a relative path */

			strcpy(changeDir, absolutePath);
			strcat(changeDir, "/");
			strcat(changeDir, cmdLine);
			break;
	}

	if(chdir(changeDir)==-1) {
		printf("%s\n",strerror(errno));
	}

	(VOID) getcwd(absolutePath, MAX_LINK_PATH);
	computePrompt(absolutePath);

	return(1);
}

LOCAL WORD32 doreloc(INPUT STRING *cmdLine)

/*
 * Enable relocation of ELF files while loading
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	allowBinaryPatch(TRUE);
	return(1);
}

LOCAL WORD32 doExit(INPUT STRING *cmdLine)

/*
 * Terminate the dynalinker shell session
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Never returns
 *
 */

{
	exit(1);
}

LOCAL WORD32 help(INPUT STRING *cmdLine)

/*
 * Print out an help screen
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	printf("Available commands:\n\n");
	printf("cd <directory>          change current working directory\n");
	printf("doreloc                 enable relocation of object files (default=TRUE)\n");
	printf("exit                    exit from the interactive shell\n");
	printf("entrypoint=<symbolname> assign linked program entrypoint\n");
	printf("link                    resolve global symbols in loaded object files\n");
	printf("memory                  display dynamic memory usage\n");
	printf("noreloc                 inhibits relocation on loaded object files\n");
	printf("purge                   unload from memory previously loaded ELF files\n");
	printf("relocs                  display relocation informations\n");
	printf("run                     kickstart dynalinked program\n");
	printf("symbols                 display symbol table entries\n");
	printf("trace=<tracetype>       assign trace level.\n");
	printf("                        Available levels:\n");
	printf("                            all      display anything\n");
	printf("                            headers  display all ELF headers\n");
	printf("                            none     disable traces\n");
	printf("                            relocs   display relocation informations\n");
	printf("                            sections display section headers only\n");
	printf("                            symtab   display symbol table info\n");
	printf("vers                    display shell version information\n");
	printf("\nAll other things taken as command line input are first checked as\nobject/executable files to be loaded, and if no ELF files of that name\nare found, are processed as standard UNIX commands.\n");

	return(1);
}

LOCAL WORD32 doLink(INPUT STRING *cmdLine)

/*
 * Link two or more ELF executables loaded previously
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	if(numOfELFFiles) {
		linkedFiles=linkELFFiles(ELFFileToBeLinked, numOfELFFiles);
 		if(linkedFiles!=numOfELFFiles) {
		    	printf("The loaded file(s) do not contain all refs.\n");
		} else {
			printf("The loaded files resolves all global refs.\n");
		}
	} else {
		printf("No ELF files loaded!\n");
	}

	return(1);
}

LOCAL WORD32 memory(INPUT STRING *cmdLine)

/*
 * Print out memory usage of loaded ELF files
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	UWORD32 i;
	UWORD32 j;
	UWORD32 n;
	UWORD32 controlStructs;
	UWORD32 bytes;
	UWORD32 len;
	UWORD32 padding;
	STRING stringizedName[32];
	struct stat buf;
	ELFSize section[32];

	if(numOfELFFiles==0) {
		printf("No ELF files loaded.\n");
		return(1);
	}

	for(i=0;i<numOfELFFiles;i++) {
		printf("\nFile %s:\n", &fileName[i][0]);
		n=getELFMemoryFootPrint(section, &controlStructs, &ELFFileToBeLinked[i]);
		printf("Control structures:   %5dd\n\n", controlStructs);
		bytes=0;

		printf("Loaded sections         Size      Alignement   Address\n");
		printf("---------------         ------    ----------   ---------\n");

		for(j=0;j<n;j++) {
			printf("%2d:", section[j].id);
			len=sprintf(stringizedName, "%s", section[j].name); 
			if(len>16) {
				stringizedName[16]='\0';
			}
			printf("<%s>", stringizedName);
			padding=16-len;
			if(padding>0) {
				while(padding--) {
					printf(" ");
				}
			}
				
			printf("%8dd", section[j].size);
			bytes+=section[j].size;

			printf("          %4d", section[j].alignement);
			printf("   %8p\n", section[j].data);
		}
				
		printf("                        ------\n");
		printf("Dynamic memory total:%8dd\n",bytes);
		stat(&fileName[i][0], &buf);
		printf("File total size:     %8dd\n", (UWORD32) buf.st_size);
	}

	return(1);
}

LOCAL WORD32 noreloc(INPUT STRING *cmdLine)

/*
 * Allow ELF files relocations
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	allowBinaryPatch(FALSE);
	return(1);
}

LOCAL WORD32 purge(INPUT STRING *cmdLine)

/*
 * Remove all data that belongs to loaded ELF file from memory
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	if(numOfELFFiles!=0) {
		unloadELFFiles(ELFFileToBeLinked, numOfELFFiles);
		printf("%d file(s) purged from memory, all refs zeroed\n", numOfELFFiles);
		numOfELFFiles=0;
		entryPointSymName[0]='\0';
	} else {
	  	printf("Nothing to purge\n");
	}

	return(1);
}

LOCAL WORD32 relocs(INPUT STRING *cmdLine)

/*
 * Display information about relocation data of loaded ELF files
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	STRING stringizedResult[MAX_SYM_NAME];
	UWORD32 i;
	UWORD32 j;
	UWORD32 totalNum;
	UWORD32 nonNullNum;
	Rel *sym;

	if(numOfELFFiles==0) {
		printf("No ELF files loaded\n");
		return(1);
	}

	for(i=0;i<numOfELFFiles;i++) {
		printf("\nFile %s:\n", &fileName[i][0]);

		/* First get the number of symbols that the currently
		 * loaded ELF file has, then allocate enough memory and
		 * display symbol data.                                   */
		
		totalNum=getNumOfELFRelocs(&ELFFileToBeLinked[i]);
		if(totalNum) {
			printf("Symbol to relocate");
			for(j=0;j<MAX_SYM_NAME-18;j++) {
				printf(" ");
			}
					
			printf("r_info        r_offset\n");
			printf("------------------");

			for(j=0;j<MAX_SYM_NAME-18;j++) {
				printf(" ");
			}
   			printf("------        --------\n");
			
			sym=(Rel *) malloc(totalNum*sizeof(Rel));
			nonNullNum=getELFRelocs(sym, &ELFFileToBeLinked[i]);
			for(j=0;j<nonNullNum;j++) {
				UWORD32 len;
			
				len=strlen(sym[j].name);
				if(len>MAX_SYM_NAME-2) {
					strncpy(stringizedResult, sym[j].name, MAX_SYM_NAME-2);
					stringizedResult[MAX_SYM_NAME-2]='\0';
					printf("<%s]", stringizedResult);
				} else {
					printf("<%s>", sym[j].name);
					len=MAX_SYM_NAME-len-2;
					if(len) {
						while(len--) {
							printf(" ");
						}
					}
				}
						 
				dumpRelocType(stringizedResult, sym[j].sym.r_info,
				              ELFFileToBeLinked[i].header.e_machine);
				printf("%s", stringizedResult);
				
				len=strlen(stringizedResult);
				if(len<13) {
					len=13-len;
				} else {
					len=2;
				}

				while(len--) {
					printf(" ");
				}
						
				printf("%8Xh\n",  (UWORD32) sym[j].sym.r_offset);
			}
					
			free(sym);
					
			printf("\n");
			printf("Non-null symbols:                 %d\n",nonNullNum);
			printf("Total symbols in relocation table:%d\n", totalNum);
		} else {
			printf("No relocs\n");
		}
	}

	return(1);
}

LOCAL WORD32 run(INPUT STRING *cmdLine)

/*
 * Bootstrap the loaded ELF files, calling the function set in entrypoint variable
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 * Always successfull
 *
 */

{
	if(numOfELFFiles==linkedFiles) {
		if(strlen(entryPointSymName)) {
			int rc;
					
			rc=runELFFile();
			printf("Execution terminated, exit value=%dd (%Xh).\n", rc, rc);
		} else {
			printf("No entry point defined!\n");
		}
	} else {
		printf("No files to being run!\n");
	}
	return(1);
}

LOCAL WORD32 symbols(INPUT STRING *cmdLine)

/*
 * Display informations about symbols defined in loaded ELF files
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 *
 */

{
	UWORD32 i;
	UWORD32 j;
	UWORD32 num;
	Sym *symbolList;

	if(numOfELFFiles==0) {
		printf("No ELF files loaded\n");
		return(1);
	}

	for(i=0;i<numOfELFFiles;i++) {
		printf("\nFile %s:\n", fileName[i]);

		/* Since we limit the length of the stringized name of
		 * the ELF symbol to MAX_SYM_NAME characters, we compute
		 * the padding needed to display it run-time.            */

		printf("Symbol name");
		for(j=0;j<MAX_SYM_NAME-11;j++) {
			printf(" ");
		}
		printf("st_info                  st_value  st_size  st_shndx\n");
		printf("-----------");
				
		for(j=0;j<MAX_SYM_NAME-11;j++) {
			printf(" ");
		}
		printf("-------                  --------  -------  --------\n");

		/* Now for each symbol defined in the file, display
		 * its name and symbol type. Again, display only the
		 * first MAX_SYM_NAME characters and pad it if necessary. */

		num=getNumOfELFSymbols(&ELFFileToBeLinked[i]);
		symbolList=malloc(num*sizeof(Sym));
		num=getELFSymbols(symbolList, &ELFFileToBeLinked[i]);
		for(j=0;j<num;j++) {
			STRING stringizedResult[MAX_SYM_NAME];
			UWORD32 len;
					
			len=strlen(symbolList[j].name);
			if(len>MAX_SYM_NAME-2) {
				strncpy(stringizedResult, symbolList[j].name, MAX_SYM_NAME-2);
				stringizedResult[MAX_SYM_NAME-2]='\0';
				printf("<%s]", stringizedResult);
			} else {
				printf("<%s>", symbolList[j].name);
				len=(MAX_SYM_NAME)-len-2;
				if(len) {
					while(len--) {
				    		printf(" ");
					}
				}
			}
					
			dumpSymbolBinding(stringizedResult, symbolList[j].sym.st_info);
			printf("%s", stringizedResult);
					
			len=24-strlen(stringizedResult);
			while(len--) {
				printf(" ");
			}
			printf("%8Xh", (UWORD32) symbolList[j].sym.st_value);
			printf(" %7dd", (UWORD32) symbolList[j].sym.st_size);
			printf("     %5d\n", (UWORD32) symbolList[j].sym.st_shndx);
		}

		free(symbolList);
		printf("Number of symbols in table:%d\n", num);
	}

	return(1);
}

LOCAL WORD32 vers(INPUT STRING *cmdLine)

/*
 * Display shell version information
 *
 * INPUT:
 * cmdLine    the command has no input, ignored
 *
 * RETURNS:
 *
 */

{
	printf("Dynamic Linker Shell ver 1.4\n");
	return(1);
}

/*
 * Command table
 *
 * Each command has its own textual defition and a function which control
 * its execution.
 */

LOCAL Command internalCmd[]=
{
	{ "cd", doChdir },
	{ "doreloc", doreloc },
	{ "exit", doExit },
	{ "help", help },
	{ "link", doLink },
	{ "memory", memory },
	{ "noreloc", noreloc },
	{ "purge", purge },
	{ "relocs", relocs },
	{ "run", run },
	{ "symbols", symbols },
	{ "vers", vers },
	{ "", NIL }
};

UWORD32 isInternalCommand(INPUT STRING *cmdLine)
  
/*
 * Check if the user has typed an internal command. If recognized, the
 * command is executed immediately.
 * 
 * INPUT:
 * cmdLine     the pointer to the user-typed string
 * 
 * RETURNS:
 * 1           the command is recognized and executed
 * 0           no command is recognized
 * -1          the syntax of command is ok, but does not exist
 */
  
{
	STRING *op;
	STRING traceLevel[9];

	/* Is the user command line a global variable assignement? */

	if((op=strchr(cmdLine, (int) '='))!=NULL) { /* Assignement op */
		op++; /* Advance pointer to the next character after '=' */
		while(*op==' ') { /* Skip blanks after '=' */
			op++;
		}

		/* Check if it is the definition of the entry point
		 * of the program that will be linked soon.         */
				
		if(strstr(cmdLine, "entrypoint")!=NULL) {
			if(numOfELFFiles==0) {
				printf("No ELF files loaded!\n");
				return(1);
			}
			strcpy(entryPointSymName, op);
			op=entryPointSymName;
			while(*op!=' ' && *op!='\0') {
				op++;
			}
			*op='\0';

			if(setELFEntryPoint(ELFFileToBeLinked, numOfELFFiles, 
				                entryPointSymName)==FALSE) {
				printf("Symbol <%s> is not present in ELF files loaded\n",
					   entryPointSymName);
			} else {
				printf("Program entry point is set to <%s>\n", entryPointSymName);
			}
			return(1);
		}
	
		/* Check if it is the definition of tracing level */

		if(strstr(cmdLine, "trace")!=NULL) {
			strcpy(traceLevel, op);
			op=traceLevel;
			while(*op!=' ' && *op!='\0') {
				op++;
			}
			*op='\0';
			
			if(strcmp(traceLevel, "all")==0) {
			    initDump(NELF_HEADER|NELF_PHEADER|NELF_SHEADER|NELF_SDATA|NELF_SYMTAB|NELF_REL);
				return(1);
			}
			
			if(strcmp(traceLevel, "relocs")==0) {
				initDump(NELF_REL);
				return(1);
			}
			
			if(strcmp(traceLevel, "symtab")==0) {
				initDump(NELF_SYMTAB);
				return(1);
			}
			
			if(strcmp(traceLevel, "sections")==0) {
				initDump(NELF_SHEADER);
				return(1);
			}
			
			if(strcmp(traceLevel, "headers")==0) {
				initDump(NELF_HEADER|NELF_PHEADER|NELF_SHEADER);
				return(1);
			}
			
			if(strcmp(traceLevel, "none")==0) {
				initDump(0);
				return(1);
			} 
		}
		
		/* Does not match any internal variable */
		
		return(0);
	} else {
		/* No, check if is an argumentless internal command */
		UWORD32 i=0;
		UWORD32 cmdLength;
		STRING *p;

		while(*internalCmd[i].string!='\0')
		{
			cmdLength=strlen(internalCmd[i].string);
			if(strncmp(internalCmd[i].string, cmdLine, cmdLength)==0) {
				p=(STRING *) &cmdLine[cmdLength];
				while(*p==' ' || *p=='\t')
					p++;

				return((*internalCmd[i].entryPoint)(p));
			}

			i++;
		}	
	  
	}
	
	return(-1);
}

/* These are textual description of ELF loader return codes */

LOCAL STRING *returnStates[]={ "ELF_LOADED_OK",
                               "ELF_FORMAT_ERROR",
                               "ELF_NOT_SUPPORTED_ERROR",
                               "ELF_SEEK_ERROR",
                               "ELF_READ_ERROR",
                               "ELF_ALLOC_ERROR" };


PUBLIC VOID isExternalCommand(INPUT STRING *cmdLine)

/*
 * Control the behaviour of external (to dyanlinker shell) commands
 *
 * INPUT:
 * cmdLine      the command as typed by user
 *
 * RETURNS:
 * None
 *
 */

{
	extern STRING absolutePath[];
	extern FILE *fd;
	STRING *p;
	ELFLoadState rc;

	/* Commands external to the shell may be of two kinds: commands
	 * of the UNIX host where the shell run on top, or ELF files
	 * to be loaded and analized. 
	 * We consider that the commands are targeted at the UNIX host
	 * when there is not a file in the current working directory
	 * that is named as the user input.                              */
	 
	strcpy(fileToBeLinked, absolutePath);
	strcat(fileToBeLinked, "/");
	strcat(fileToBeLinked, cmdLine);
					
	if((fd=fopen(fileToBeLinked, "rb"))==NULL) {
		/* Try as UNIX command */

		system(cmdLine);
		return;
	} else {
		/* Put in name field of ELF structure the filename
		 * We take only the file name and cut off the rest. */

		p=&fileToBeLinked[strlen(fileToBeLinked)];
		while(*p!='/' && p!=fileToBeLinked) {
			p--;
		}

		p++;
		strcpy(ELFFileToBeLinked[numOfELFFiles].name, p);
		rc=loadELFFile(&ELFFileToBeLinked[numOfELFFiles]);
		if(rc!=ELF_LOADED_OK) {
			printf("Error %s loading %s\n", returnStates[rc], fileToBeLinked);
			unloadELFFiles(&ELFFileToBeLinked[numOfELFFiles], 1);
		} else {
			strcpy(fileName[numOfELFFiles], fileToBeLinked);
			printf("File <%s> loaded\n", fileToBeLinked);
			numOfELFFiles++;
		}
	}

	while((p=strtok(0, "\t "))!=NULL) {
		strcpy(fileToBeLinked, absolutePath);
		strcat(fileToBeLinked, p);
		if((fd=fopen(fileToBeLinked, "rb"))==NULL) {
			printf("File %s does not exist.\n", fileToBeLinked);
			unloadELFFiles(&ELFFileToBeLinked[numOfELFFiles], 1);
		} else {
			rc=loadELFFile(&ELFFileToBeLinked[numOfELFFiles]);
			if(rc!=ELF_LOADED_OK) {
				printf("Error %d loading %s\n", rc, fileToBeLinked);
				unloadELFFiles(&ELFFileToBeLinked[numOfELFFiles], 1);
			} else {
				strcpy(fileName[numOfELFFiles], fileToBeLinked);
				numOfELFFiles++;
			}
							
			fclose(fd);
		}
	}
}

PUBLIC VOID uninteractiveShell(STRING *argv[], UWORD32 argc, STRING *entryPointSymName)

/*
 * Control the behaviour of non-interactive dynalinker shells
 *
 * INPUT:
 * argv         the command-line argument array
 * argc         the number of (remaining) arguments to be scanned
 * 
 *
 */

{
	BOOLEAN errorDuringLoad;
	ELFLoadState rc;
	FILE *fd;
	WORD32 returnCode;
	UWORD32 i;
	UWORD32 j;

	/* Process file to be linked */
 
	errorDuringLoad=FALSE;
	j=0;	
	for(i=0;i<argc;i++) {
		if((fd=fopen(argv[i], "rb"))==NULL) {
			printf("File %s does not exist.\n", argv[i]);
			errorDuringLoad=TRUE;
			break;
		} else {
			rc=loadELFFile(&ELFFileToBeLinked[j++]);
			if(rc!=ELF_LOADED_OK) {
				printf("Error %s loading %s\n", returnStates[rc], argv[i]);
			}
			
			fclose(fd);
		}
	}
		
	/* If file loading was correct, try to link those files */
	
	if(errorDuringLoad==FALSE) {
		if(linkELFFiles(ELFFileToBeLinked, j)!=j) {
			printf("Error during linking\n");
		}

		if(setELFEntryPoint(ELFFileToBeLinked, j, entryPointSymName)==TRUE) {
			returnCode=runELFFile();
			printf("rc=%d\n", returnCode);
		} else {
			printf("Symbol <%s> is not present in ELF files loaded\n", entryPointSymName);
		}
	}
}
