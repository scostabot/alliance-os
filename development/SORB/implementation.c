/* 
 * Echo object implementation
 *
 * Just some example objectimplementation to test the request system used for
 * Alliance SORB.
 *
 * HISTORY
 * Date          Author    Rev    Notes
 * Sep 14th,1998 Luc       1      Based on ORBit example. 
 * Sep 28th,1998 Luc       2      Now uses typewrappers.h
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */ 

#include <typewrappers.h>
#include <orb.h>
#include "echo-skel.h"
#include <stdio.h>

/* The only function implemented by this object is echoString */

VOID echoString (CORBA_string aString, CORBA_Environment * ev) {
/* This function echoes a string to the defult display
 *
 * INPUT:
 * aString : String to be printed.
 *
 * RETURNS:
 * Nothing.
 */ 
  printf ("%s [ca]\n", aString);
};






