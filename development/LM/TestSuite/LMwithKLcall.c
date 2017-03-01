/*
 * This file defines an LM which contains the (optional) LM functions
 * LMinit() and some Kernel Library calls in order to test full 
 * dynamic linking capabilities of LMs.
 */

#include <klibs.h>

#include <lm.h>

LOCAL STRING lmName[]="LM with a call to a KL..() function"; 


/* LM description structure */
PUBLIC LMDescription lmDescription=
{
	lmName,      
	0,
	LMIOSK1,
	{ 1, 1, 1 }, 
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	(BYTE *) NIL 
};

PUBLIC BOOLEAN LMinit(VOID)

{
	char *test="testMe";

	if(KLstringCompare("testMe", test)==0) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}
