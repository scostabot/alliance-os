/*
 * <One-line LM description>
 *
 * <High level description of the functions implemented in this LM>
 * (Enclosed in parenthesis are metacomments, all those comments that
 *  are pertinent to an explanation of standard rules to follow in
 *  writing an LM for IOSK source code, and that should be deleted
 *  in the actual, real LM code you will write.                     )
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h> 	/* This LM belong to an SK environment */
#include <lm.h>         /* Generic LM definitions */
#include <iosklm.h>     /* Definitions specific to IOSK LMs */

/*
 * LM local defines and typedefs.
 * (If your LM is composed by this file only, put this stuff here.
 *  Otherwise, you can delete the whole comment and put in an
 *  include file the same stuff.                                   )
 */

/*
 * LM Global Data.
 * (If your LM needs global variables declaration, put them here.
 *  If you don't need them, delete this comment block entirely.
 *  If your LM will be composed of more than one file and you need
 *  global variables, put them in a separate file.                  )
 */

/*
 * Standard LM entry points.
 * (These functions must be always defined, even if empty.)
 */

PUBLIC BOOLEAN LMinit(LMinitData *data)
  
/*
 * (Init LM module. If your LM needs a one-time-in-process-life actions,
 *  put it here. If you don't need such things, you leave the statement
 *  return(TRUE), otherwise the IOSK won't use this LM.
 *  It is FORBIDDEN to access hardware resources here.                  )
 */

{
    return(TRUE);
}

PUBLIC STRING *LMprobe(VOID)
  
/*
 * (LM probe function.
 *  This function, if LM in question is a device driver, sniff out HW 
 *  configuration. If the LM is a software-only module, you declare 
 *  here what you do.                                                 )
 */
  
{
}

/* 
 * (Optional LM entry points. Depending on what LMprobe reports, 
 *  there will be the following functions or not.                )
 */

