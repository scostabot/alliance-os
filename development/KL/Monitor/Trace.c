/*
 * Monitoring module for Alliance
 *
 * This module defines the tracing function callable anywhere in SKs.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 06/02/99 scosta    1.0    First draft
 * 11/04/99 scosta    1.1    Removed "No mask" message
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>  /* Monitoring Facilites definitions */

/* This module relies on successful inclusion of klibs.h with SKTRACE
 * symbol #defined. User of KLtrace really use a macro which calls 
 * these functions. See klibs.h and /docs for details.                 */

extern UWORD16 traceMask;  /* The value of the desired tracing level */

PUBLIC UWORD32 tracedLine;	  /* This is the variable that contains
							     the KLtrace line (it's set in the macro)  */
PUBLIC STRING tracedFile[40]; /* This is the variable that corntains the
								 KLtrace file name (It's set in the macro) */

PUBLIC VOID KLhighLevelTrace(STRING *msg, ...)

/*
 * Display a debugging message on current debugging output device,
 * with a variable number of (optional) variables to be printed out.
 *
 * INPUT:
 * msg	The pointer to the string to be displayed, formatted as printf()
 *
 * OUTPUT:
 * None
 */

{
	WORD16 i;	
	WORD16 length;	
	UWORD16 traceLevel=0;
	STRING *stringNum;
	STRING *ptrDelimiter;
	STRING outputString[80];
	va_list arguments;
	
	/* The syntax of KLtrace string is: "xxxx'|'<the string to output>".
	 * Let's first transform the string-coded hex number to a number.
	 * To do that correctly, first get the length of the stringized number
	 * (look for '|') and then use this value to compute the number.      */
 
	ptrDelimiter=msg;	
	while(*ptrDelimiter!='|' && *ptrDelimiter!='\0')
		ptrDelimiter++;
	
	if(*ptrDelimiter=='\0') {
		return;
	}

	length=(WORD16) (ptrDelimiter-msg);

	/* Next, compute the value of the tracing level */
	 
	stringNum=msg;	
	for(i=(length-1)*4;i>=0;i-=4, stringNum++) {
	    if(*stringNum>='A') {
			traceLevel += ((*stringNum - ('A'-10))<<i); 
		} else {
			traceLevel += ((*stringNum - '0')<<i);	  
		}
	}

	/* Check if the given tracing level matches the one to display */

	if(traceLevel & traceMask) {
	    extern UWORD32 traceMonFlags;
		DATA writtenChars;
		STRING *traceString=ptrDelimiter;

		outputString[0]='\0';

		/* The trace must be output, parse the presence of trace options */
		
		if(traceMonFlags & MONITOR_FILE) {
		    KLstringAppend(outputString, tracedFile);
			if(traceMonFlags & MONITOR_LINE) {
		    	KLstringAppend(outputString, ":");
			}
		}
		
		if(traceMonFlags & MONITOR_LINE) {
		    STRING tempString[7];

		    KLstringFormat(tempString, "%d", tracedLine);
		    KLstringAppend(outputString, tempString);
		}

		if(traceMonFlags) {
		    KLstringAppend(outputString, "|");
		}

		/* Display the optional trace options and trace message */

		KLlowLevelTrace(outputString);
		va_start(arguments, msg); 	
		writtenChars=KLformatToString(outputString, ++traceString, arguments);
		KLlowLevelTrace(outputString);
	}
}

