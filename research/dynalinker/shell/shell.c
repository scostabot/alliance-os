/*
 * Experimental shell for LM dynamic link test
 *
 * This interactive shell is meant for testing purposes only, not for
 * actual Alliance OS uses. Nevertheless it is useful in order to
 * play with LMs and make testing of them and their functionality.
 * Note that this will run only over a UNIX box, and for this
 * reason this file does not follow strictly Alliance Coding Guide.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 02/07/99 scosta    1.0    First draft
 * 20/11/99 scosta    1.1    Added memory & link commands
 * 20/12/99 scosta    1.2    Added relocs and symbols commands
 * 12/01/00 scosta    1.3    Fixed various errors in symbols handling
 * 22/04/00 scosta    1.4    Added doreloc and noreloc command
 * 13/10/00 scosta    1.5    Added support for memory alignement
 * 03/01/03 scosta    1.6    More user-friendliness everywhere
 * 
 * (c) 1999 Copyright Stefano Costa - the Alliance group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/* Include Alliance OS specific typedefs, so we will be as 
   compliant as possible with programming guidelines.      */

#include <typewrappers.h>

/* This program is a research study on how load and manage ELF programs.
 * Since there is no Alliance OS at this stage, we use the host UNIX
 * platform and so we include these files.                                */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "shell.h"
#include <cmddynalinker.h>

PUBLIC FILE *fd;   /* Descriptor used for generic I/O in ELF loader */
PUBLIC STRING absolutePath[MAX_LINK_PATH]; /* Current working directory for shell */

LOCAL STRING prompt[MAX_PATH];  /* This is the prompt in interactive shell */

PUBLIC VOID computePrompt(STRING *path)

/*
 * Compute the string that will make up the prompt of shell
 * 
 * INPUT:
 * path	     the current working directory (absolute path)
 * 
 * RETURNS:
 * None
 */

{
	char *p;
	UWORD32 homePathLength;

	strcpy(prompt, "dynlink:");

	p=getenv("HOME");
	if(p!=NIL) {
		homePathLength=strlen(p);
		if(strncmp(path, p, homePathLength)==0) {
			strcat(prompt, "~");
			strcat(prompt, &path[homePathLength]);
		} else {
			strcat(prompt, path);
		}
	} else {
		strcat(prompt, path);
	}
			
	strcat(prompt, ">");
}

LOCAL UBYTE *mallocedBlock[MAX_ALLOCATABLE_BLOCKS];
LOCAL UBYTE *returnedAddress[MAX_ALLOCATABLE_BLOCKS];

/* Callback functions.
 * These function are passed as pointers to the actual ELF loader, so
 * a clear, no-nonsense interface to basic OS primitives is given.     */

extern BOOLEAN (*readCallBack)(VOID *, UWORD32);
extern BOOLEAN (*seekCallBack)(UWORD32);
extern VOID *(*allocCallBack)(UWORD32, UWORD32); 
extern VOID (*freeCallBack)(VOID *);

BOOLEAN readFile(VOID *buffer, UWORD32 size)
	
/*
 * readFile
 * 
 * INPUT:
 * buffer		the pointer to a memopry buffer
 * size			the number of bytes to be read
 * 
 * OUTPUT:
 * buffer		the data read from device
 * 
 * RETURNS:
 * TRUE if all is gone well, FALSE otherwise
 * 
 */

{
  	if(fread(buffer, size, 1, fd)==0) {
		if(feof(fd)) {
			printf("Unexpected EOF\n");
	  	}
		return(FALSE);
	} else {
		return(TRUE);
	}
}

BOOLEAN seekFile(UWORD32 position)
  
/*
 * seekFile
 * 
 * INPUT:
 * position		the file pointer, in bytes from beginning
 * 
 * RETURNS:
 * Guess that
 */
	
{
	if(fseek(fd, (long) position, SEEK_SET)==-1) {
		return(FALSE);
	} else {
		return(TRUE);
	}
}

VOID *allocBuffer(UWORD32 size, UWORD32 alignement)
  
/*
 * allocBuffer
 * 
 * INPUT:
 * size 		the number of bytes to be allocated
 * alignement	the required memory alignement for the memory allocation (bytes)
 * 
 * RETURNS:
 * the pointer to a memory area of size bytes aligned as needed
 */
  
{
  	UBYTE *p;
	UWORD32 blockIndex=0;
	
	if(alignement>1) {
		size+=alignement;
	}
	
	p=(UBYTE *) malloc(size);

	while(mallocedBlock[blockIndex]!=0) {
		blockIndex++;
		if(blockIndex>MAX_ALLOCATABLE_BLOCKS) {
			exit(-3);
		}
	}
	
	mallocedBlock[blockIndex]=p;
		
	while(((UWORD32) p) & alignement) {
		p++;
	}

	returnedAddress[blockIndex]=p;	
	return(p);
}

VOID freeBuffer(VOID *buffer)
  
/*
 * freeBuffer
 * 
 * INPUT:
 * buffer		the pointer to the memory area previously allocated
 * 
 * RETURNS:
 * None
 */
  
{
	UWORD32 blockIndex=0;
  	
	while(returnedAddress[blockIndex]!=buffer) {
		blockIndex++;
	}

	free(mallocedBlock[blockIndex]);

	mallocedBlock[blockIndex]=(UBYTE *) 0;
	returnedAddress[blockIndex]=(UBYTE *) 0;
}

int main(int argc, char *argv[])
  
/*
 * This is the main module of the dynamic linker shell.
 * The shell syntax is:
 * 
 * dynsh [files to be linked]
 * 
 * If called with no arguments, dynsh start in interactive mode.
 * A shell with "dynlink>" prompt is started for user interaction.
 * Inside the shell it is possible to input internal commands or
 * object filenames to be linked.
 * 
 * Internal commands
 * 
 * It is possible to change the shell behaviour with internal
 * commands. Currently internal commands are all in the form of
 * 
 * <variable name>=<value> 
 * 
 * or
 * 
 * <keyword>
 * 
 * where <variable name> can be:
 * 
 * Name	        Scope								
 * trace        set the trace level of dynalinker shell
 *              Valid <value> for <trace> are: 
 *                all        print out all traces available
 *                headers    print out all ELF headers values
 *                none       disable traces
 *                symtab     print out all symbol-related informations
 *                sections   print out all sections headers
 *  
 * entrypoint   mark the name of the function for
 *              bootstrapping dynalinked program
 * 				
 * and <keyword> can be:
 * 
 * memory		display dynamic memory usage
 * run			kickstart loaded program
 * purge		deletes previously loaded files
 * vers			display version informations
 * 				
 * All command lines that are not internal commands are treated as a list
 * of ELF files to be loaded and linked.
 * For instance the prompt
 * 
 * dynlink>caller.o called.o
 * 
 * load ELF files caller.o and called.o, and try to link them togheher.
 * 
 * Environment
 * 
 * The dynalink shell reads DYNLINKPATH environment variable when 
 * it is needed to determine the path of ELF object loading,
 * so you can avoid to prepend a constant directory to any file.
 * For instance if DYNLINKPATH is "/home/myObjects" and at "dynlink>"
 * prompt you write "caller.o called.o" the shell looks for
 * /home/myObjects/caller.o and /home/myObjects/called.o.
 */
  
{
	char *p;
	UWORD32 userInput;
	STRING cmdLine[MAX_CMD_LINE];

	/* Get an environment setting in order to find out where to
	 * to look for files containing symbols to be linked. If no
	 * environment variable is given, set the path to current 
	 * working directory.                                      */

	p=getenv("DYNLINKPATH");
	if(p!=NIL) {
		if(strlen(p)<MAX_LINK_PATH) {
		    	strcpy(absolutePath, p);
			/* Check if trailing '/' is present. If not, add it */

			if(absolutePath[strlen(absolutePath)-1]!='/') {
		  		strcat(absolutePath, "/");
			}
		} else {
			printf("DYNLINKPATH too long. Max path is %d\n", MAX_LINK_PATH);
			p=getcwd(absolutePath, MAX_LINK_PATH);
			if(p==NIL) {
				exit(-1);
			}
		}
	} else {
		p=getcwd(absolutePath, MAX_LINK_PATH);
		if(p==NIL) {
			exit(-1);
		}
	}

	computePrompt(absolutePath);

	/* Init the I/O interface routines pointers */
	
	readCallBack=readFile;
	seekCallBack=seekFile;
	allocCallBack=allocBuffer;
	freeCallBack=freeBuffer;

	/* If user gives no arguments, use an interactive shell 
	 * approach using standard input,                        */
	
	if(argc==1) {
		FILE *descriptor;

	  	descriptor=stdin;

		/* Print out the shell prompt */

		printf("%s", prompt);
		
		/* Here we control the interactive shell behaviour.
		 * First of all get user input.                     */
	  
		while(fgets(cmdLine, sizeof(cmdLine), descriptor)!=NULL) {
			WORD32 length;

			/* Reject excessevely long command lines */

			length=strlen(cmdLine);
			if(length>MAX_CMD_LINE) {
				printf("Command line too long.\n");
				printf("%s", prompt); 
				continue;
			}

			/* Reject trivial case of null command line */
			
			if(length!=1) {
				UWORD32 i=0;

			 	cmdLine[length-1]='\0'; /* Strip off trailing CR */

				/* Strip off spaces at beginning, if any */

				while(cmdLine[i]==' ' || cmdLine[i]=='\t')
					i++;

				/* Is the user string a shell internal command? */
		
				if((userInput=isInternalCommand(&cmdLine[i]))==-1) {
					isExternalCommand(&cmdLine[i]);	
				} else {
					/* Yes, was an internal command, but was correctly
					 * recognized and executed?                         */

					if(userInput==0) {
						printf("command <%s> not recognized.\n", &cmdLine[i]);
					}
				}
			}
		
			/* Print out the shell prompt */

			printf("%s", prompt); 
		}
	} else {
		/* Uninteractive, command-line invoked dynalinker shell.
		 * The default dynalinked program entry point is main.   */

		extern char *optarg;
		extern int optind;
		STRING *entryPointSymName="";
		UWORD32 c;

		/* Process command line options (if any) */

		while((c=getopt(argc, argv, "e:"))!=EOF) {
			switch(c) {
				case 'e':
					entryPointSymName=optarg;
					break;
				default:
					printf("Unrecognized option -%d\n", c);
					break;
			}
		}

		uninteractiveShell(&argv[optind], argc-optind, entryPointSymName);
	}

	/* Exits gracefully */
	
	printf("\n");
	exit(0);
}
