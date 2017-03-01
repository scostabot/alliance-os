/*
 * This file represents an Alliance OS LM module
 * with the optional one-time init function LMinit().
 */

#include <lm.h>

PUBLIC LMDescription lmDescription=
{
	"LMinit test",                   /* Descriptive name for this LM */
	"An LM module with LMinit()",    /* A more complete description of it */
	LMIOSK|LMFSSK|LMNETSK|LMGRSK|LMSNDSK|LMSSK|LMUISK, /* Any SK can load it */
	{ 1, 0, 0 },                     /* LM version */
};

PUBLIC BOOLEAN LMinit(VOID)

{
	return(TRUE);
}
