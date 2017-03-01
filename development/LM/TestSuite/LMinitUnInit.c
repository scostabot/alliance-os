/*
 * This file defines an LM which contains the (optional) LM functions
 * LMinit() and LMunInit().
 */

#include <lm.h>

/* LM description structure */

PUBLIC LMDescription lmDescription=
{
	"LM(un)init",                     /* Descriptive name for this LM */
	"An LM with both LMinit/LMuninit",/* more complete description of it */
	LMIOSK|LMFSSK|LMNETSK|LMGRSK|LMSNDSK|LMSSK|LMUISK, /* Any SK can load it */
	{ 1, 0, 0 },                      /* LM version */
};

PUBLIC BOOLEAN LMinit(VOID)

{
	return(TRUE);
}

PUBLIC VOID LMunInit(VOID)

{
	return;
}
