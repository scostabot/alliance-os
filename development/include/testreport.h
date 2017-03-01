/*
 * Test report functions
 *
 * This include file defines a standard framework for display
 * internal test report in a clean and consistent form.
 * For every logical module (KL library, SKs...) a tester module
 * should exist that verify correct implementation of code
 * exercising the functionality implemented. The result should
 * be output to user using these functions, called by make test.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 26/12/02 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>

LOCAL inline VOID ReportTitle(STRING *msg)
/*
 * Nicely format the beginning of a test section.
 *
 * INPUT:
 * msg  a string with report title
 *
 * RETURNS:
 * None.
 * 
 */
{
	KLprint("\n%s\n", msg);
}

LOCAL inline VOID TestTitle(STRING *msg)
/*
 * Nicely format the beginning of a test section.
 *
 * INPUT:
 * msg  a string with report title
 *
 * RETURNS:
 * None.
 * 
 */
{
	KLprint("\n%s", msg);
}

LOCAL inline VOID TestInfo(STRING *msg)
  
{
	KLprint("\n  %s", msg);
}

LOCAL VOID EndReport(VOID)
/*
 * Nicely format the end of a test section.
 *
 * INPUT:
 * None
 *
 * RETURNS:
 * None.
 * 
 */
{
	KLprint("\n");
}

LOCAL UWORD32 ReportResult(UWORD32 ReturnedValue, UWORD32 ExpectedValue)

/*
 * Check if the value returned from a function is what is expected
 * printing a message accordingly.
 *
 * INPUT:
 * ReturnedValue the value returned
 * ExpectedValue the value we expect
 *
 * OUTPUT:
 * Test result
 */
		
{
	if(ReturnedValue==ExpectedValue) {
		KLprint(" OK");
		return(1);
	} else {
		KLprint(" ERROR!");
		return(0);
	}
}

