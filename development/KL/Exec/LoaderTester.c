/*
 * Executable Loader Library test program
 *
 * This program is a test suite for Executable Loader Library behaviour
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/06/00 scosta    1.0    First draft
 * 27/12/02 scosta    1.1    Improved test reporting, cleaned up
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>
#include <testreport.h>
#include <teststubs.h>		/* Include all defs that compile machine 
                             needs for basic stuff in order to work */

LOCAL EHandle loadExecutableUntested(INPUT FILE *fd)

/*
 * Loads the executable file pointed by fileName, without so many checks
 *
 * INPUT:
 * fileName			absolute path of the file to be loaded
 *
 * RETURNS:
 * the executable handle of the loaded file
 */

{
	EHandle handle;
	ExecRecord record;
	LoadStatus loadState;
	FILE *myFd=(FILE *) fd;			/* Get rid of warning */
	
	handle=KLexecOpen(&record);
	if(fseek(myFd, record.position, SEEK_SET)==-1) {
		exit(-1);
	}

	if(fread(record.buffer, record.size, 1, myFd)==0) {
		exit(-1);
	}
	
	loadState=KLexecLoad(handle, &record);
	while(loadState==EXEC_LOAD_AGAIN) {
		if(record.buffer!=NIL) {
			if(fseek(myFd, record.position, SEEK_SET)==-1) {
				break;
			}
	
			if(fread(record.buffer, record.size, 1, myFd)==0) { 
				break;
			}
		}

  		loadState=KLexecLoad(handle, &record);		
	}

	if(loadState==EXEC_LOAD_FINISHED) {
		return(handle);
	} else {
		KLexecClose(handle);
		return(ENO_MOREEXECS);
	}
}

PUBLIC UWORD32 fakeKLFunction(STRING *a, STRING *b)

{
	return(KLstringCompare(a, b));
}

UBYTE originalContent[64];

/* This static data mirror the symbol table of each file under /Testsuite.
 * If you change these ones, you must update also this table, otherwise
 * tests will fail!                                                       */

#define NFILES 9
#define MAX_SYMBOLS 8

STRING *fileName[NFILES]= { 
	"ExternFunctionCalled", "AssignedGlobalData",
	"StaticLocalData", "StaticGlobalData", "PointerTest", 
	"LocalFunctionCallWithFiller", "LocalFunctionCallPostfix",
	"FunctionPointer", ""
};

STRING *extFileName[NFILES]= {
	"FunctionLinkage", "" 
};
 
STRING objectFileName[NFILES][80]; /* Absolute pathname for the ELF files */

/* In this table are stored the relationship between ELF files and
 * the symbols defined inside each of them. For each file exist one
 * row that contains a list of strings, terminated with a NULL element,
 * that lists the symbol names for the i-th file.
 * IMPORTANT: put the ELF entry point symbol name as the first entry,
 * otherwise the tester will crash immediately on ELF execution.      */

typedef struct {
	STRING *name;
	SymbolType type;
} SYMBOL;

SYMBOL symbolTable[NFILES][MAX_SYMBOLS]= 
{ { { "calledFunction", EXEC_FUNC }, { "", EXEC_NIL } }, 
  { { "main", EXEC_FUNC }, { "i", EXEC_VAR }, { "", EXEC_NIL } },
  { { "main", EXEC_FUNC }, { "", EXEC_NIL } },
  { { "main", EXEC_FUNC }, { "i", EXEC_VAR }, { "", EXEC_NIL } },
  { { "main", EXEC_FUNC }, { "table" , EXEC_VAR }, { "", EXEC_NIL } }, 
  { { "main", EXEC_FUNC }, { "functionCalled", EXEC_FUNC }, { "unusedCode", EXEC_FUNC }, { "", EXEC_NIL } },
  { { "main", EXEC_FUNC }, { "functionCalled", EXEC_FUNC }, { "unusedCode", EXEC_FUNC }, { "", EXEC_NIL } },
  { { "main", EXEC_FUNC }, { "functionCalled", EXEC_FUNC }, { "", EXEC_NIL } }, 
  { { "", EXEC_NIL } } };

/* The following is the symbol table definition for programs that calls
 * functions defined in the executable loader tester, instead of the reverse */

SYMBOL symbolExtLink[][MAX_SYMBOLS]=
  { { { "main", EXEC_FUNC }, { "functionCall", EXEC_FUNC }, { "", EXEC_NIL } },
  { { "", EXEC_NIL } } };

Esymbol funcToBeCalledSym[]=
  { { "functionCall", EXEC_FUNC }, { "", EXEC_NIL } };

UBYTE *funcAddr[2]; 

Esymbol listOfSymbols[MAX_SYMBOLS];
UBYTE *addresses[MAX_SYMBOLS];

/* Test program code */

int main()

{
	unsigned int i,j;
	int rc;
	int (*entryPoint)();
	FILE *fd;

	UWORD32 numOfSymbols;
	EHandle handle,handle2;
	ExecRecord record;
	ExecType executableType;
	ProbeStatus status;
	LoadStatus loadState;
	UWORD32 testSummary=1;
	BOOLEAN returnFlag;

	ReportTitle("Executable loader library test suite");

	/* Firstly init the Executable Loader Library, and check that all
	 * things went well in the minimum configuration (only one executable
	 * loadable as maximum).                                              */

	TestTitle("Initialization");

	returnFlag=KLexecInit(1);
	testSummary&=ReportResult(returnFlag, TRUE);
	KLexecUnInit();
	returnFlag=KLexecInit(1);
	testSummary&=ReportResult(returnFlag, TRUE);

	TestTitle("Opening/closing");

	/* After that check that the upper bound check is performed on ExecOpen */

	handle=KLexecOpen(&record);
	testSummary&=ReportResult((handle<EVALID_HANDLE), TRUE);

	handle2=KLexecOpen(&record);
	testSummary&=ReportResult((handle2<EVALID_HANDLE), FALSE);

	/* Checks correct handle release */

	returnFlag=KLexecClose(handle2);
	testSummary&=ReportResult(returnFlag, FALSE);

	returnFlag=KLexecClose(handle);
	testSummary&=ReportResult(returnFlag, TRUE);

	returnFlag=KLexecClose(handle);
	testSummary&=ReportResult(returnFlag, FALSE);

	TestTitle("Executable probing");

	/* Build object files list for the correct target first */

	for(i=0;*fileName[i]!='\0';i++) {
		KLstringCopy(objectFileName[i], "TestSuite/objects/architecture/" );
		KLstringAppend(objectFileName[i], PLATFORM);
		KLstringAppend(objectFileName[i], "/");
		KLstringAppend(objectFileName[i], fileName[i]);
	}

	/* In order to test probing we have to read a real executable file */

	if((fd=fopen(objectFileName[0], "rb+"))==NULL) {
		perror("TestLoader");
		exit(-1);
	}

	/* Now do the standard stuff: get an handle, do a read of the
	 * file as requested by KLexecOpen, then give data to KLexecProbe. */
	
	handle=KLexecOpen(&record);
	testSummary&=ReportResult((handle<EVALID_HANDLE), TRUE);

	if(fseek(fd, record.position, SEEK_SET)==-1) {
		exit(-1);
	}

	if(fread(record.buffer, record.size, 1, fd)==0) {
		exit(-1);
	}

	status=KLexecProbe(handle, &executableType, &record);
	if(status==EXEC_PROBE_OK) {
		testSummary&=ReportResult((executableType==elf32), TRUE);
	} else {
		testSummary&=ReportResult(FALSE, TRUE);
	}

	/* We patch arbitrarely the executable program in oder to generate an
	 * abnormally formed executable, something not (usually ;)) possible
	 * with normal compiling/linking.									
	 * First we read and backup original ELF file data.                   */
	
	if(fseek(fd, record.position, SEEK_SET)==-1) {
		exit(-1);
	}

	rc=fread(originalContent, sizeof(originalContent), 1, fd);
	if(rc==0) {
		exit(-1);
	}

	/* Abnormally formed executable case 1: invalid signature. */

	record.buffer[1]='\0';
	status=KLexecProbe(handle, &executableType, &record);
	testSummary&=ReportResult((status==EXEC_PROBE_OK),FALSE);

	/* Restore original data */
	
	record.buffer[1]=originalContent[1];
	
	/* Abnormally formed executable case 2: invalid data */

	record.buffer[5]=0;

	status=KLexecProbe(handle, &executableType, &record);
	testSummary&=ReportResult((status==EXEC_PROBE_OK),FALSE);

	/* Restore original data */
	
	record.buffer[5]=originalContent[5];

	TestTitle("Executable loading");

	/* The loading process is a series of call to KLexecLoad()
	 * until return value is EXEC_LOAD_AGAIN. 
	 * The function asks for data to be read through the structure
	 * ExecRecord. If the memory buffer pointed by it is NIL
	 * it means that no data has to be read.
	 * The loading process ends when KLexecLoad() function return
	 * EXEC_LOAD_FINISHED if the entire executable file is loaded
	 * correctly, or a different value in case of error.           */
	 
	loadState=KLexecLoad(handle, &record);
	while(loadState==EXEC_LOAD_AGAIN) {
		testSummary&=ReportResult(TRUE, TRUE);
		if(record.buffer!=NIL) {
			if(fseek(fd, record.position, SEEK_SET)==-1) {
				break;
			}
	
			if(fread(record.buffer, record.size, 1, fd)==0) { 
				break;
			}
		}

  		loadState=KLexecLoad(handle, &record);		
	}
	
	testSummary&=ReportResult((loadState==EXEC_LOAD_FINISHED), TRUE);

	KLexecClose(handle);
	fclose(fd);
			  
	TestTitle("Symbols handling");

	/* This part of the test suite check that the process that use
	 * the Executable Loader library get the correct addresses of
	 * the symbols defined in the ELF file just loaded, variables
	 * or function procedures, differencing between the two.      */
 
	/* Scan the list of the ELF test object files to be loaded.
	 * For each file, we load it with Executable Library, build
	 * a static list of symbol for each one, fetch from Executable
	 * Library the list of the symbols from loaded file, compare 
	 * them and see if the two matches. Then, we resolve symbol
	 * references in the file loaded (linking process) and finally
	 * run the program just loaded.                                 */

	i=0;
	while(*objectFileName[i]!='\0') {

		if((fd=fopen(objectFileName[i], "rb"))==NULL) {
			perror(objectFileName[i]);
			exit(-1);
		}
		
		handle=loadExecutableUntested(fd);
		if(handle>EVALID_HANDLE) {
			exit(-1);
		}

		TestInfo(objectFileName[i]);

		/* Build static list */
	
		j=0;
		while(*symbolTable[i][j].name!='\0') {
			KLstringCopy(listOfSymbols[j].name, symbolTable[i][j].name);
			listOfSymbols[j].type=symbolTable[i][j].type;
			j++;
		}

		*listOfSymbols[j].name='\0';

		/* Firstly check that the correct number of symbols are returned */
		
		numOfSymbols=KLexecSymbols(handle, listOfSymbols, addresses);
		testSummary&=ReportResult((numOfSymbols==j), TRUE);

		/* Execute the program just loaded. All test cases have main()
		 * as the entry point, and return 0x5A if the program run 
		 * flawlessy. We also assume that entry point symbol is the first 
		 * element in listOfSymbols table (and so in addresses table)     */

		entryPoint=(WORD32 (*)()) addresses[0];
		testSummary&=ReportResult(((*entryPoint)()==0x5A), TRUE);
		
		KLexecClose(handle);
		fclose(fd);

		/* Of each sample ELF program there are two versions: one
		 * with no debug info, and one compiled with debug info
		 * (the same filename but with "Debug" at filename end).
		 * We should test both, so we don't advance to next file
		 * unless the debugging info version is tested.            */

		if(KLstringMatch(objectFileName[i], "Debug")==NIL) {
			KLstringAppend(objectFileName[i], "Debug");
		} else {
			i++;
		}
	}

	TestTitle("External linking");

	/* This part of the test suite check that the executable modules
	 * loaded with the Executable Loader Library can correctly access
	 * to symbols that are defined in the process that has loaded them. */

	/* Build object files list for the correct target first */

	for(i=0;*extFileName[i]!='\0';i++) {
		KLstringCopy(objectFileName[i], "TestSuite/objects/architecture/" );
		KLstringAppend(objectFileName[i], PLATFORM);
		KLstringAppend(objectFileName[i], "/");
		KLstringAppend(objectFileName[i], extFileName[i]);
	}

	/* Now test the N-th file */

	i=0;
	while(*objectFileName[i]!='\0') {

		if((fd=fopen(objectFileName[i], "rb"))==NULL) {
			perror(objectFileName[i]);
			exit(-1);
		}
		
		handle=loadExecutableUntested(fd);
		if(handle>EVALID_HANDLE) {
			exit(-1);
		}
	
		/* Build static list */
	
		j=0;
		while(*symbolExtLink[i][j].name!='\0') {
			KLstringCopy(listOfSymbols[j].name, symbolExtLink[i][j].name);
			listOfSymbols[j].type=symbolExtLink[i][j].type;
			j++;
		}

		*listOfSymbols[j].name='\0';

		/* Fetch the address of the function to be called */

		numOfSymbols=KLexecSymbols(handle, listOfSymbols, addresses);
	/*	if(numOfSymbols!=j) {
			testSummary&=ReportResult(FALSE, TRUE);
			break;
		}*/	

		/* Then give to the executable module the address of needed sym */

		funcAddr[0]=(UBYTE *) fakeKLFunction;
		numOfSymbols=KLexecSetSymbols(handle, funcToBeCalledSym, funcAddr);

		/* Then execute the target routine and check the result */

		entryPoint=(WORD32 (*)()) addresses[0];
		testSummary&=ReportResult(((*entryPoint)()==0x5A), TRUE);
		
		KLexecClose(handle);
		fclose(fd);
	}

	EndReport();

	if(testSummary) {	
		exit(0);
	} else {
		exit(1);
	}
}
