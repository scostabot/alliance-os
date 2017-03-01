/*
 * Monitoring module for Alliance
 *
 * Set the current tracing level for the debugged process.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 07/02/99 scosta    1.0    First draft
 * 29/03/99 scosta    1.1    Forced recompilation with #define
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifndef SKTRACE
#define SKTRACE       /* Force definition of Monitoring Facilities */
#endif

#include <klibs.h>   /* Monitoring Facilities definitions */

PUBLIC UWORD16 traceMask;

PUBLIC VOID KLsetTraceLevel(INPUT UWORD16 userTraceMask)
  
/*
 * Set current trace level.
 * 
 * INPUT:
 * userTraceMask   Which KLtrace() the user want to see displayed
 * 
 * OUTPUT:
 * None
 */
  
{
	/* Very simplistically, we set a global variable as user tells us */

	traceMask=userTraceMask;
}

PUBLIC UWORD16 KLgetTraceLevel(VOID)
  
/*
 * Get current trace level.
 * 
 * INPUT:
 * None
 * 
 * OUTPUT:
 * Current trace level
 */
  
{
	return(traceMask); 
}
