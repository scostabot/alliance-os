/*
 * Monitoring facilities testing code
 *
 * This module is written with the purpose to test and illustrate the usage
 * of Alliance OS Monitoring Facilities.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 10/02/99 scosta    1.0    First draft
 * 11/04/99 scosta    1.1    A more serious test
 * 11/02/00 scosta    1.2    KLprint instead of KLlowLevelTrace 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <stdlib.h>  /* For exit() */

#include <klibs.h>   /* Monitoring facilities definitions */

int main()

{
	DATA i=1234;

	/* Start printing welcome stuff.... */

	KLprint("Monitor facilities Test\n\n");
	
	/* First sets the trace that we want to see. We want all. */
	 
	KLsetTraceLevel(0xF);
	KLprint("The two following lines must be equal:\n");
	KLprint("MonitorTester.c:39|Variable=1234\n");
	 
	/* Now start output things */

	KLtrace(SK_INFO "Variable=%d\n", i);

	/* This one should be invisible, since no mask is present */

	KLsetTraceLevel(0x0);
	KLtrace(SK_INFO "Should be invisible variable=%d\n", i);

	/* Let's test other options as well */
	
	KLsetTraceLevel(0xF);
	KLsetTraceOptions(0);
	 
	KLprint("\n");
	KLprint("The two following lines must be equal:\n");
	KLprint("Variable=1234\n");
	KLtrace(SK_INFO "Variable=%d\n", i);

	KLprint("\n");
	KLsetTraceOptions(MONITOR_LINE);
	 
	KLprint("The two following lines must be equal:\n");
	KLprint("58|Variable=1234\n");
	KLtrace(SK_INFO "Variable=%d\n", i);

	KLlowLevelTrace("\n");
	KLsetTraceOptions(MONITOR_FILE);
	 
	KLprint("The two following lines must be equal:\n");
	KLprint("MonitorTester.c|Variable=1234\n");
	KLtrace(SK_INFO "Variable=%d\n", i);

	exit(0);	
}


