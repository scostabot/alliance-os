/*
 * LM library definitions
 * 
 * In this file is defined the function needed to parse correctly the 
 * LM init string. 
 * LMs init string are the (optional) parameters that a generic LM
 * module require to init properly. The loaded LM receives in the
 * init function a pointer to the parameter(s). It is LM task to
 * interpret the ASCII, null-terminated string in a way that is 
 * meaningful, i.e., this routine does not make any modification other
 * than stripping delimiters and blanks.
 * Init string also defines the shape of LM chain, and this data is
 * used by an SK to init correctly LM environment. This function 
 * only interpret LM init string syntax, and does nothing with LM
 * load/unload.
 * 
 * Syntax
 * 
 * <LMname>([LMarg] [[','][LMarg]]*)
 * 
 * HISTORY
 * Date     Author    Rev    Notes
 * 5/11/98  S. Costa  1.0    First draft
 * 18/11/98 Ramon     1.1    Updated to Alliance Coding Style
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h>
#include <lm.h>

LOCAL WORD32 LMIndex=0;

PUBLIC VOID LMparseInitString(INPUT STRING *Text, Symbol LMchainInitStrings[])	
{
    STRING *ScanPointer;
    STRING *EndPointer;
    STRING *StartOfLMArgs;
    UWORD32 Length;
    UWORD32 i,j;

    /* Look for LM module name, eg. from "ide(0, 1024, 512)" is ide */

    if((StartOfLMArgs=KLstringMatch(Text, "("))!=(STRING *) NIL) {
        Length=(StartOfLMArgs - Text);
        KLstringNumCopy(LMchainInitStrings[LMIndex].Name, Text, Length);
        LMchainInitStrings[LMIndex].Name[Length]='\0';
        LMIndex++;
        LMchainInitStrings[LMIndex].Name[0]='\0';

        /* Syntax is recursive, to allow chaining of different LM modules:
         * "cache(ide(0, 16, 1024), 512)" means that there are two LMs,
         * and that cache sends its output to ide, and vice-versa, so
         * call ourselves with the pointer to first char of second init
         * string, "ide(0, 16, 1024), 512)" in our example.
         */

         ScanPointer=StartOfLMArgs;
         LMparseInitString(++ScanPointer, LMchainInitStrings);
    } else {
         return;
    }

    /* Process LM arguments string. First set boundaries */

    EndPointer=KLstringMatch(ScanPointer, ")");
		
    /* We are beginning scanning arguments from the LAST LM module
     * in the init string, eg., LM init string is like a call stack
     * analogue to a C listing, so we begin from ide() arguments
     * following our example.  
     */

    LMIndex--;
    i=0; j=0;

    /* Collect arguments, preserving order, a character at a time.
     * Arguments separator is the comma ','. Blanks are ignored.  
     */

    while(ScanPointer<EndPointer) {
        if(*ScanPointer!=' ') {
            if(*ScanPointer==',') {
                LMchainInitStrings[LMIndex].Parameter[j][i]='\0';
                j++;
                i=0;
            } else {
                LMchainInitStrings[LMIndex].Parameter[j][i]=*ScanPointer;
                i++;
            }
        }
        ScanPointer++;
    }
				

    /* Keep last argument too */

    while(*ScanPointer++!=',');

    LMchainInitStrings[LMIndex].Parameter[j][i]='\0';
}
	
