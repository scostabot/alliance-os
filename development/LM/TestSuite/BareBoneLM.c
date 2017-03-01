/*
 * This file represents the smallest possible Alliance
 * OS LM module.
 */

#include <lm.h>

/* LM description structure */

PUBLIC LMDescription lmDescription=
{
	"Bare bone LM",                  /* Descriptive name for this LM */
	"The null element in LM field",  /* A more complete description of it */
	LMIOSK|LMFSSK|LMNETSK|LMGRSK|LMSNDSK|LMSSK|LMUISK, /* Any SK can load it */
	{ 1, 0, 0 },                     /* LM version */
};
