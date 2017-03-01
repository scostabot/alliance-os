/*
 * Monitoring Facilities
 *
 * Set the option flags for KLtrace function call.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/03/99 scosta    1.0    First draft
 * 11/04/99 scosta    1.1    Added more options
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>   /* Monitor facilities defnitions */

/* This is the default value for KLsetTraceOptions, that is, display
 * both filename and line number where KLtrace is called.                */

PUBLIC UWORD32 traceMonFlags=MONITOR_FILE | MONITOR_LINE;

PUBLIC VOID KLsetTraceOptions(UWORD32 flags)
	
/*
 * Append src after dest
 * 
 * INPUT:
 * flags: the bit fields for KLtrace options.
 *        These are defined fields:
 *        MONITOR_FILE    display filename from which KLtrace function
 *                        is called before the real trace message
 *        MONITOR_LINE    display the line number from which KLtrace
 *                        function is called before the real trace message
 *        
 * RETURNS:
 * None
 */
	
{
	traceMonFlags=flags;
}

