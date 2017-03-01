/*
 * This file represents an LM module with the (optional) function
 * LMprobe() defined.
 */

#include <lm.h>

PUBLIC LMDescription lmDescription=
{
	"LMprobe test",                  /* Descriptive name for this LM */
	"An LM module with LMprobe()",   /* A more complete description of it */
	LMIOSK|LMFSSK|LMNETSK|LMGRSK|LMSNDSK|LMSSK|LMUISK, /* Any SK can load it */
	{ 1, 0, 0 },                     /* LM version */
};


PUBLIC BOOLEAN LMprobe(VOID)

{
	return(TRUE);
}

