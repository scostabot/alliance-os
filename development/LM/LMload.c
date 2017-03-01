/*
 * Alliance OS LM library
 *
 * Load an LM module in memory
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 26/04/03 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>    
#include <lm.h>

#include "lmsys.h"

extern UWORD32 _maxLMs;
extern LMTable *_lmTable;

PUBLIC LMHandle LMload(INPUT STRING *moduleName, INPUT Version *required)

/*
 * Load a Loadable Module in memory, checking that mandatory symbols are
 * present and manage all optional ones.
 *
 * INPUT:
 * moduleName:   the LM filename to be loaded
 * required:     the minimum supported version to be loaded
 *
 * RETURNS:
 * The handle of the loaded LM if < LM_ERROR_BASE, otherwise an error
 * is occurred.
 */

{
	UBYTE *address[1];
	STRING absolutePath[64];
	UWORD32 readHandle;
	EHandle execHandle;
	ExecRecord record;
	ExecType executableType;
	Esymbol listOfSymbols[2];
	ProbeStatus status;
	LoadStatus loadState;
	LMHandle lmId;

	/* Determine LM module file name location in file system */

	KLstringCopy(absolutePath, "");
	KLstringAppend(absolutePath, moduleName);

	/* Load the executable file that contains LM module in memory.
	 * First open the file and init Executable Library.             */

	readHandle=lowLevelOpen(absolutePath);

	if((execHandle=KLexecOpen(&record))>EVALID_HANDLE) {
		lowLevelClose(readHandle);
		return((LMHandle) LM_NOMORE_LMS);
	}

	if(lowLevelRead(readHandle, &record)==0) {
		lowLevelClose(readHandle);
		return((LMHandle) LM_FILE_ERR);
	}

	/* Is the executable file of the supported HW platform? */

	status=KLexecProbe(execHandle, &executableType, &record);
	if(status!=EXEC_PROBE_OK) {
		lowLevelClose(readHandle);
		KLexecClose(execHandle);
		return((LMHandle) LM_WRONG_PLATFORM);
	}

	/* If so, start to load LM in memory a chunk at a time */

	loadState=KLexecLoad(execHandle, &record);
	while(loadState==EXEC_LOAD_AGAIN) {
		if(record.buffer!=NIL) {
			if(lowLevelRead(readHandle, &record)==0) { 
				break;
			}
		}

  		loadState=KLexecLoad(execHandle, &record);		
	}

	/* The executable image is stored in memory now, no more
	 * reads from the given device                           */

	lowLevelClose(readHandle);

	/* Check that mandatory LM symbol <lmDescription> is present
	 * in the LM file. If not, return error code.                */

	KLstringCopy(listOfSymbols[0].name, "lmDescription");
	listOfSymbols[0].type=EXEC_VAR;
	listOfSymbols[1].name[0]='\0';
	listOfSymbols[1].type=EXEC_NIL;

	if(KLexecSymbols(execHandle, listOfSymbols, address)!=1) {
		KLexecClose(execHandle);
		return((LMHandle) LM_NODESCRIPTION);
	}

	/* Version of loaded LM control: is the LM release
	 * compatible to user needs?                       */
   
	{
		LMDescription *lmDesc;

		lmDesc=(LMDescription *) address[0];
		if(lmDesc->release.Major<required->Major) {
			KLexecClose(execHandle);
			return((LMHandle) LM_WRONG_VERS);
		}

		if(lmDesc->release.Minor<required->Minor) {
			KLexecClose(execHandle);
			return((LMHandle) LM_WRONG_VERS);
		}
	}

	/* Check optional LM symbols presence in the LM file.
	 * Currently common to every kind of LM in Alliance OS
	 * there are three optional symbols: LMinit(),LMunInit()
	 * and LMprobe().
	 * The three functions, if defined in the LM, contains
     * one-time-in-LM-life code to be executed at LM load
	 * and unload, plus LM functions probing respectevely.    */
			
	KLstringCopy(listOfSymbols[0].name, "LMinit");
	listOfSymbols[0].type=EXEC_FUNC;
	KLstringCopy(listOfSymbols[1].name, "LMunInit");
	listOfSymbols[1].type=EXEC_FUNC;
	KLstringCopy(listOfSymbols[2].name, "LMprobe");
	listOfSymbols[2].type=EXEC_FUNC;
	listOfSymbols[3].name[0]='\0';
	listOfSymbols[3].type=EXEC_NIL;

	/* Assign an handle to the loaded LM */

	lmId=0;
	while(_lmTable[lmId].usage!=0) {
	    lmId++;
	}

	if(lmId<_maxLMs) {
		(VOID) KLexecSymbols(execHandle, listOfSymbols, (UBYTE **) _lmTable[lmId].entryPoints);

		/* If LMinit() symbol is defined, must be called right after loading */

		if(_lmTable[lmId].entryPoints[0]!=NIL) {
			if((*_lmTable[lmId].entryPoints[0])()!=TRUE) {
				/* LM init function returned error, flag to user */

				lmId=LM_INIT_ERROR;
				KLexecClose(execHandle);
				return(lmId);
			}
		}

		_lmTable[lmId].execHandle=execHandle;
		_lmTable[lmId].usage++;
	} else {
		lmId=LM_NOMORE_LMS;
	}

	KLexecClose(execHandle);

	return(lmId);	
}

