/*
 * Test module for Alliance Kernel string handling functions
 *
 * The purpose of this program is to exercise and test the behaviour of
 * string handling functions.
 * The test are copied from equivalent module in GNU glibc, but it is
 * NOT the same code since the interface of Alliance routines is quite
 * different from those of UNIX.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 26/10/98 scosta    1.0    First draft
 * 18/11/98 ramon     1.1    Updated to Alliance Coding Style
 * 11/02/99 scosta    1.2    Added support for KLtrace
 * 20/04/99 scosta    1.3    Enhanced KLtrace usage
 * 02/02/00 scosta    1.4    KLtrace->KLprint for unconditional print
 * 24/11/02 scosta    1.5    Better self-test handling
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <stdlib.h>  /* For exit() */

#include <klibs.h>
#include <testreport.h>
								
int main()
{
    WORD32 ReturnCode;
    UWORD32 testSummary=1;
    STRING One[50];
    STRING Two[50];
	
    ReportTitle("String test suite");

    TestTitle("Testing KLstringCompare()");

    ReturnCode=KLstringCompare("", "");          /* ==0 */
    testSummary&=ReportResult(ReturnCode, 0);
    ReturnCode=KLstringCompare("a", "a");        /* ==0 */
    testSummary&=ReportResult(ReturnCode, 0);
    ReturnCode=KLstringCompare("abc", "abc");    /* ==0 */
    testSummary&=ReportResult(ReturnCode, 0);
    ReturnCode=KLstringCompare("abc", "abcd");   /* <0 */
    testSummary&=ReportResult((ReturnCode<0), 1);
    ReturnCode=KLstringCompare("bcd", "abc");    /* >0 */
    testSummary&=ReportResult((ReturnCode>0), 1);
    ReturnCode=KLstringCompare("abcd", "abce");  /* <0 */
    testSummary&=ReportResult((ReturnCode<0), 1);
    ReturnCode=KLstringCompare("abce", "abcd");  /* >0 */
    testSummary&=ReportResult((ReturnCode>0), 1);

    TestTitle("Testing KLstringCopy()");

    if(KLstringCopy(One, "abcd")==One) {
        ReturnCode=KLstringCompare(One, "abcd");
    } else {
        ReturnCode=1;
    }
    testSummary&=ReportResult(ReturnCode, 0);
    KLstringCopy(One, "x");
    ReturnCode=KLstringCompare(One+2, "cd");
    testSummary&=ReportResult(ReturnCode, 0);
    KLstringCopy(Two, "Hi there");
    KLstringCopy(One, Two);
    ReturnCode=(KLstringCompare(One, "Hi there")==0 && 
                KLstringCompare(Two, "Hi there")==0);
    testSummary&=ReportResult(ReturnCode, 1);
    KLstringCopy(One, "");
    ReturnCode=KLstringCompare(One, "");
    testSummary&=ReportResult(ReturnCode, 0);

    TestTitle("Testing KLstringAppend()");

    KLstringCopy(One, "x");
    KLstringAppend(One, "yz");
    ReturnCode=KLstringCompare(One, "xyz");
    testSummary&=ReportResult(ReturnCode, 0);
    ReturnCode=KLstringCompare(One+4, "here");
    testSummary&=ReportResult(ReturnCode, 0);
    KLstringCopy(One, "");
    KLstringAppend(One, "");
    ReturnCode=KLstringCompare(One, "");
    testSummary&=ReportResult(ReturnCode, 0);
    KLstringCopy(One, "");
    KLstringAppend(One, "ab");
    ReturnCode=KLstringCompare(One, "ab");
    testSummary&=ReportResult(ReturnCode, 0);

    TestTitle("Testing KLstringNumCopy()");
	
    ReturnCode=(KLstringNumCopy(One, "abc", 4)==One);
    testSummary&=ReportResult(ReturnCode, 1);
    ReturnCode=KLstringCompare(One, "abc");
    testSummary&=ReportResult(ReturnCode, 0);
    KLstringCopy(One, "abcdefgh");
    KLstringNumCopy(One, "xyz", 3);
    ReturnCode=KLstringCompare(One, "xyzdefgh");
    testSummary&=ReportResult(ReturnCode, 0);
		
    TestTitle("Testing KLstringLength()");
		
    ReturnCode=KLstringLength("");
    testSummary&=ReportResult(ReturnCode, 0);
    ReturnCode=KLstringLength("a");
    testSummary&=ReportResult(ReturnCode, 1);
    ReturnCode=KLstringLength("abcd");
    testSummary&=ReportResult(ReturnCode, 4);

    TestTitle("Testing KLstringMatch()");

    ReturnCode=(WORD32) KLstringMatch("abcd", "z");
    testSummary&=ReportResult(ReturnCode, (WORD32) NIL);
    ReturnCode=(WORD32) KLstringMatch("abcd", "abx");
    testSummary&=ReportResult(ReturnCode, (WORD32) NIL);
    KLstringCopy(One, "abcd");
    ReturnCode=(KLstringMatch(One, "c")==One+2);
    testSummary&=ReportResult(ReturnCode, 1);
		
    TestTitle("Testing KLstringCompareNoCase()");
		
    ReturnCode=(KLstringCompareNoCase("abc", "ABC")==0);
    testSummary&=ReportResult(ReturnCode, 1);
    ReturnCode=(KLstringCompareNoCase("abcd", "ABC")==0);
    testSummary&=ReportResult(ReturnCode, 0);
    ReturnCode=(KLstringCompareNoCase("ab01", "AB01")==0);
    testSummary&=ReportResult(ReturnCode, 1);

    TestTitle("Testing KLstringFormat()");

    KLstringFormat(One, "This is a test");
    KLprint("%s\n", One);
    KLstringFormat(One, "Number:  %d (should be 11)\n",11);
    KLprint("%s", One);
    KLstringFormat(One, "Hex:     0x%x (should be 0xdead)\n",0xdead);
    KLprint("%s", One);
    KLstringFormat(One, "Pointer: %p (address of One)\n",One);
    KLprint("%s", One);
    KLstringFormat(One, "Another one: %+07d ", 64);
    KLprint("%s\n", One);

    TestTitle("Testing KLasciiToInt()");
    ReturnCode = KLasciiToInt("10573");    
    KLstringFormat(One, "Number:  %d (should be 10573)",ReturnCode);
    KLprint("%s", One);

    KLstringFormat(One, "\nThat's it folks !! ");
    KLprint("%s", One);

    EndReport();

    if(testSummary) {
    	exit(0);
    } else {
		exit(1);
    }
}
