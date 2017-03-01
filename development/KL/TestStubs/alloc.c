/*
 * Code stub that fakes basic Alliance OS services 
 *
 * This code is a stub that helps testing in the building environment
 * faking some services into UNIX standard calls, without the need to
 * have real OS services in place.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 28/09/05 scosta    1.0    First draft
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>          /* We act as a true Kernel Library */

#include <teststubs.h>      /* Import all UNIX defs needed */

/* These functions wraps Alliance OS calls for memory allocation to 
 * standard UNIX counterparts. 
 * These calls are normally defined in the EKs code. 
 * For testing purposes we make UNIX stubs here a library.
 */

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

	if(align==0)   /* Just to get rid of warning...*/
		return(0);
	else
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
	free(p);
}
