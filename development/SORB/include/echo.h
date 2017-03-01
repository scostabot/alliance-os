/* 
 * Sample Echo object definition file
 *
 * This file defines the functions/types/constants needed for using the Echo
 * object.
 *
 * HISTORY
 * Date          Author    Rev    Notes
 * Sep 14th,1998 Luc       1      Should be using typewrappers.h.
 * sep 28th,1998 Luc       2      There is no need for typewrappers (yet)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <orb.h> /* For types */

#ifndef __ECHO_H
#define __ECHO_H

typedef CORBA_Object Echo;

#endif

extern CORBA_void Echo_echoString (Echo obj, CORBA_string aString, 
				   CORBA_Environment* ev);

