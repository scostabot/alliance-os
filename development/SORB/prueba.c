/*
 * Test program for converting to and from object references and strings
 *
 * This file contains code to define an object reference (by hand) and to
 * translate it to a string, print it, and convert the string back to an
 * object reference, printing the values stored on it in a readable way
 * BTW: prueba means "test" in Spanish, but is already an executable with that
 * name ;-)
 *
 * HISTORY
 * Date          Author    Rev    Notes
 * ???           Luc       0      Only for personal testing
 * Sep 28th,1998 Luc       1      Added GPL notice and some comments
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */


#include <orb.h>
#include <stdio.h>

int main () { /* kept without typewrappers becouse is a standard */
	struct IOR obj_ior;
	CORBA_Object obj = &obj_ior; /* the object reference */
	CORBA_string str;            /* the string to convert to/from */

	/* Filling the object reference by hand */
	obj->local = CORBA_TRUE;  /* Local call */
	obj->ip[0] = 127;         /* loopback address */
	obj->ip[1] = 0;
	obj->ip[2] = 0;
	obj->ip[3] = 1;
	obj->sn = "/usr/local/orb/ir/echo"; /* scoped name...not really used */
	obj->flags = orbISSORB | orbISLIBRARY | orbISCORBAAWARE;

	str =  CORBA_ORB_object_to_string(obj); /* obj. ref to string */
	printf ("%s\n\n",str); /* print the resulting string */

	obj = CORBA_ORB_string_to_object(str); /* back to an object */

	/* print the contents of the object reference for validation */
	printf ("Local: %d\n", obj->local);
	printf ("ip: %d.%d.%d.%d\n", obj->ip[0], obj->ip[1],
		 obj->ip[2], obj->ip[3]);
	printf ("Scoped name: %s\n", obj->sn);
	printf ("Flags: %d\n", obj->flags);
	
	return 0;
}
