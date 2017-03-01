/* 
 * Echo Object Implementation (Not Corba Aware)
 *
 * Just Some Example Object implementation To Test The Request System Used For
 * Alliance Sorb.
 *
 * History
 * Date          Author    Rev    Notes
 * Sep 10th,1998 Luc       1      Based On Orbit Example. 
 * Sep 28th,1998 Luc       2      Now uses typewrappers.h
 * 
 * This Program Is Free Software; You Can Redistribute It And/Or Modify
 * It Under The Terms Of The Gnu General Public License As Published By
 * The Free Software Foundation.
 * This Program Is Distributed But Without Any Warranty; Without Even The
 * Implied Warranty Of Merchantability Or Fitness For A Particular Purpose.
 * See The Gnu General Public License For More Details.
 */ 

#include <typewrappers.h>
#include <stdio.h>
#include "echo-skel.h"

/* Note that the the object implementation does not need to be CORBA aware
 * so every library can easily become a CORBA object. Also, any executable can
 * be converted to a CORBA object by defining an IDL for it and compiling
 * it with its CORBA skeleton.
 */

/* The only function implemented by this object is echoString */


VOID echoString (STRING * aString) {
/* This function echoes a string to the defult display
 *
 * INPUT:
 * str : String to be printed.
 *
 * RETURNS:
 * Nothing.
 */ 
  printf ("%s [na]\n", aString);
};

