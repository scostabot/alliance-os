/*
 * UNIX stub for startup and actually test the functional behaviour of LMs
 *
 * This is a simple emulation on LM funtionality that should be done by
 * IOSK when this will be modified.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 27/03/99 scosta    1.0    First draft
 * 24/04/99 scosta    1.1    Better tracing, channels count, various improvs
 * 04/05/99 scosta    1.2    Now displays LM name and version
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>   /* Definition common to any EK, so also for this stub */
#include <lm.h>
#include <iosklm.h>   /* LM for IOSK specific defines, since we act as an SK */

#include <stdlib.h>   /* For UNIX compat, to be removed */

extern LMDescription lmParallel; /* Temporary kludge: should be loaded
					dynamically by a linker            */

int main(int argc, char *argv[])
  
/*
 * SYNTAX:
 * 
 * LMparallel <channel number> <filename>
 * 
 * will write on LMparallel channel the filename given
 */

{
    UWORD32 i;
	UWORD32 numChannels;
	UWORD32 userChannel;
    STRING *probedString;
	STRING lmName[16];

	/* Step 0: init traces environment
	 * 
	 * We use here SK_INFO tracing level and no
	 * trace options.                           */
	
	KLsetTraceLevel(0xF);
	KLsetTraceOptions(0);

    /* Step 1: check user parameter and fetch LM channel
     * 
     * In order to simulate a bit better LM behaviour, we 
     * allow user to exercise a particular device from 
     * those available.                                    */
	
	if(argc==1) {
	    KLtrace(SK_INFO "Usage: LMparallel <channel number> <filename>\n");
		exit(-1);
	} else {
	    userChannel=atoi(argv[1]);
	}

    /* Step 2: Display LM description
     * 
     * Fetch the LMdescription structure, and display useful data */

    KLtrace(SK_INFO "Loading LM %s ", lmParallel.lmName);
	KLtrace(SK_INFO "ver %d.%d.%d\n", lmParallel.release.Major,
		                              lmParallel.release.Minor,
									  lmParallel.release.BugFix);
	
    /* Step 3: Init the LM linked with this module.
     * 
     * This function, toghether with LMprobe are the only
     * entry points that must be present in every IOSK LM. */

    KLtrace(SK_INFO "Initting LM....");  
    if(LMinit()==TRUE) {
		KLtrace(SK_INFO "OK\n");
	} else {
	    KLtrace(SK_INFO "ERROR!\n");
		exit(-1);
	}
		 
	/* Step 4: Discover what the LM does and which interfaces has.
	 * 
	 * Calling LMprobe we obtain a string coded following the
	 * probe syntax (see spec document "LM for IOSK rel 1").
	 * With this string, we see a) which kind of function LM
	 * implement b) in which way we can control it.             */
	
	KLtrace(SK_INFO "Probing....");
    probedString=LMprobe();
	if(probedString==NIL) {
	    KLtrace(SK_INFO "failed\n");
	    exit(-1);
	}

	KLtrace(SK_INFO "result:<%s>\n", probedString);
	
	/* Step 5: use the services that LM gives us.
	 * 
	 * Here we simply ask the user on which LM channel
	 * we operate our test. We determine the actual number of
	 * supported channels by LM counting the number of times
	 * the LM name is repeated. Then we check if user defined
	 * channel match those available in LM.                    */
	
	i=0;
	while(*probedString!='\0' && *probedString!='(') {
	    lmName[i++]=*probedString;
		probedString++;
	}

	lmName[i]='\0';
	KLtrace(SK_INFO "LM name is %s\n", lmName);
	
	numChannels=1;
	while(KLstringMatch(probedString, lmName)!=NIL) {
	    probedString+=i;
	    numChannels++;
	}
	
	if(userChannel>numChannels) {
	    KLtrace(SK_INFO "LM channel %d not available.\n", userChannel);
		exit(-2);
	}
	
	/* Step 6: write raw data on LM.
	 * 
	 * Now we exercise the true LM nature: process data. 
	 * We do so writing data from a file to LM, and see the 
	 * result.
	 * When dynamic linking of LM will be done, the actual
	 * LMwrite() function call will be a pointer to a function,
	 * whose pointer assigned by a real dynalinker, which 
	 * I'll write later...                                    */
	
	{
#include <stdio.h>

	    size_t byteRead;
		UBYTE myBuffer[4096];
	    FILE *fd;
		
    	if((fd=fopen(argv[2], "rb"))==NULL) {
	    	KLtrace(SK_FATAL "Missing filename\n");
	    	exit(-1);
		}
		
		while(byteRead!=0) {
			byteRead=fread(myBuffer, sizeof(myBuffer), 1, fd);
			if(LMwrite(myBuffer, byteRead, 0)==FALSE) {
			    KLtrace(SK_FATAL "LMwrite failed.\n");
			    exit(-1);
			}
		}
	}	

    KLtrace(SK_INFO "Data successfully written.\n");

	exit(0);
}

