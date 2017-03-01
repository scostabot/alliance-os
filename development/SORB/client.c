/* 
 * Sample client for the Sample Echo Object on the Sample SORB =)
 *
 * This client just performs 10.000 calls to the object to allow profiling.
 *
 * History
 * Date          Author    Rev    Notes
 * Sep,16th,1998 Luc       1      Should use typewrappers.h
 * Sep,28th,1998 Luc       2      Uses typewrappers.h
 * 
 * This Program Is Free Software; You Can Redistribute It And/Or Modify
 * It Under The Terms Of The Gnu General Public License As Published By
 * The Free Software Foundation.
 * This Program Is Distributed But Without Any Warranty; Without Even The
 * Implied Warranty Of Merchantability Or Fitness For A Particular Purpose.
 * See The Gnu General Public License For More Details.
 */ 

#include <orb.h>
#include "echo.h"
#include <typewrappers.h>
#include <stdio.h>

int main (int argc, char **argv) {/* left without wrappers as it is standard */
  /* creates an object reference and perform the requests
   *
   * INPUT:
   *   Standard command line parameters (see README file)
   *
   * OUTPUT:
   *   0 on success, other if any error (heppefully with a nice message)
   */

  struct IOR obj_ior;
  Echo obj = &obj_ior;
  CORBA_Environment ev;
  INT16 loop;
  STRING * helloStr = "Hello world";

/* We do not have any Naming sevice, so we use a dirty trick and code the
 * Object reference ourselves */

  obj->local = CORBA_TRUE;
  obj->flags = orbISSORB | orbISLIBRARY | orbISCORBAAWARE;

  for (loop=0; loop<1; loop ++) {
    printf ("Calling implementation...\n\t");
    Echo_echoString (obj, helloStr, &ev);
  }

  return 0;
}










