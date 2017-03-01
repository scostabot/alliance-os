/*
 * Alliance OS monitoring Facilities
 *
 * This module contain the HW-dependant part of Monitoring facilities
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 10/02/99 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>
#include <stdio.h> /* Only for Unix compatibility, should be removed */

PUBLIC inline VOID KLlowLevelTrace(INPUT STRING *msg)

/*
 * Low-level string output to tracing device.
 * 
 * INPUT:
 * msg    The string to be displayed (as is, no expansion/substitution)
 * 
 * OUTPUT:
 * None
 */
		
{
	/* For now, write on stdout. Later on, we must support gprof/gdb
	 * remote device or some form of more advanced device scheme.    */

	(VOID) fputs(msg, stdout);
	(VOID) fflush(stdout);
}
