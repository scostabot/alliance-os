/*
 * Test functions for LM library
 *
 * Perform various tests to check LM library proper functioning.
 * These test are to be performed anytime you change them or you
 * port it to a new environment.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 10/11/98 S. Costa  1.0    First draft
 * 25/05/03 scosta    1.1    Very rewritten witn new LM structure
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

#include <lm.h>

#include <stdio.h>		/* For testing we use UNIX standard file I/O */
#include <stdlib.h>

/* These functions wraps Alliance OS calls for memory allocation to 
 * standard UNIX counterparts. 
 * These calls are normally defined in the EKs code. Since this testing
 * program acts as an EK, we define here those functions.                */

LOCAL FILE *fd;

PUBLIC UWORD32 lowLevelOpen(INPUT STRING *fileName)
{
	if((fd=fopen(fileName, "rb+"))==NULL) {
		perror("lowLevelOpen");
		exit(-1);
	}

	return(1);
}

PUBLIC UWORD32 lowLevelRead(INPUT UWORD32 descriptor, ExecRecord *record)
{
	if(fseek(fd, record->position, SEEK_SET)==-1) {
		perror("lowLevelRead (seek)");
		exit(-1);
	}

	if(fread(record->buffer, record->size, 1, fd)==0) {
		perror("lowLevelRead (read)");
		exit(-1);
	}

	return(1);
}

PUBLIC UWORD32 lowLevelClose(INPUT UWORD32 descriptor)
{
	return(fclose(fd));
}

PUBLIC VOID *KLmalloc(UWORD32 size)

/*
 * Allocates a memory region of the given size
 *
 * INPUT:
 * size		the size in bytes
 *
 * RETURN:
 * the pointer to the first byte of allocated region
 */

{
	UBYTE *p;
	p=malloc(size);
	/*printf("\nMalloc:%X, size=%X", p, size); */
	return(p);
}

PUBLIC VOID *KLmallocAligned(UWORD32 size, UWORD32 align)

/*
 * Allocates a memory region of the given size and alignement
 *
 * INPUT:
 * size     the size in bytes
 * align	the alignement in bytes
 *
 * RETURN: 
 * the pointer to the first byte of allocated region
 */

{
	UBYTE *p;
	p=malloc(size);
	/*printf("\nMalloc:%X,size=%X", p, size);*/
	return(p);
}

PUBLIC VOID KLfree(VOID *p)

/*
 * Free the memory region previously allocated
 *
 * INPUT:
 * p		the pointer to the first byte memory region
 *
 * RETURN:
 * None
 */

{
	/*printf("\nFree:%X", p);*/
	free(p);
}

Version Ver= { 1, 1, 0 };

int main(int argc, char *argv[])
{
	BOOLEAN returnFlag;
	UWORD32 testSummary=1;
	LMHandle lmHandle, lmHandle2;

	ReportTitle("LM library test suite");

	TestTitle("Initialization");

	returnFlag=LMlibInit(1);
	testSummary&=ReportResult(returnFlag, TRUE);
	LMlibUninit();
	returnFlag=LMlibInit(1);
	testSummary&=ReportResult(returnFlag, TRUE);
	returnFlag=LMlibInit(1);
	testSummary&=ReportResult(returnFlag, FALSE);
	LMlibUninit();
	returnFlag=LMlibInit(1);

	TestTitle("Loading");

	lmHandle=LMload("TestSuite/objects/BareBoneLM", &Ver);
	testSummary&=ReportResult(lmHandle<LM_ERROR_BASE, TRUE);

	lmHandle2=LMload("TestSuite/objects/BareBoneLM", &Ver);
	testSummary&=ReportResult(lmHandle2==LM_NOMORE_LMS, TRUE);

	LMunload(lmHandle);

	lmHandle=LMload("TestSuite/objects/BareBoneLM", &Ver);
	testSummary&=ReportResult(lmHandle<LM_ERROR_BASE, TRUE);

	LMunload(lmHandle);

	Ver.Minor=4;
	lmHandle=LMload("TestSuite/objects/BareBoneLM", &Ver);
	testSummary&=ReportResult(lmHandle==LM_WRONG_VERS, TRUE);

	LMlibUninit();
	(VOID) LMlibInit(4);

	Ver.Minor=1;
	lmHandle=LMload("TestSuite/objects/InvalidLM", &Ver);
	testSummary&=ReportResult(lmHandle==LM_NODESCRIPTION, TRUE);

	lmHandle=LMload("TestSuite/objects/LMinit", &Ver);
	testSummary&=ReportResult(lmHandle<LM_ERROR_BASE, TRUE);

	lmHandle=LMload("TestSuite/objects/LMinitUnInit", &Ver);
	testSummary&=ReportResult(lmHandle<LM_ERROR_BASE, TRUE);

	lmHandle=LMload("TestSuite/objects/LMprobe", &Ver);
	testSummary&=ReportResult(lmHandle<LM_ERROR_BASE, TRUE);

	EndReport();

	if(testSummary) {	
		exit(0);
	} else {
		exit(1);
	}
}
