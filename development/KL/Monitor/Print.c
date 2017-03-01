/*
 * Monitoring module for Alliance
 *
 * Unconditional trace print.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 10/02/00 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>  /* Monitoring Facilites definitions */

PUBLIC VOID KLprint(STRING *msg, ...)

/*
 * Print (currently on console only)
 *
 * INPUT:
 * msg		the null-terminated string to be printed
 *
 * OUTPUT:
 * None
 */

{
	STRING outputString[80];
	DATA writtenChars;
	va_list arguments;

	va_start(arguments, msg); 	
	writtenChars=KLformatToString(outputString, msg, arguments);
	KLlowLevelTrace(outputString);
}

