/* 
 * Sample stub for the Echo Object
 *
 * This file delivers the requests from a client to an Echo object
 * implementation
 *
 * HISTORY
 * Date          Author    Rev    Notes
 * Sep 14th,1998 Luc       1      Should be using typewrappers.h. 
 * Sep 28th,1998 Luc       2      Uses typewrappers.h
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/* This stub supports delivery of requests to different object implementations
 * but does not support mixing calls to corba-aware and not aware objects
 * (which, anyway, should never happen). It supports mixing calls to a library
 * implementation and to a threaded server (but I can't imagine an environment
 * where this could happen...) [Luc]
 *
 * Can be optimized if the implementation and Corba-awareness of the object is
 * well known, by removing some of the comparations in each function, which
 * will then follow this schema
 * if (orbIsLocal(obj)) DO_LOCAL_HANDLING;
 * else SEND_REQUEST_TO_REMOTE_HOST;
 * 
 */
#include <stdio.h>
#include <stdlib.h>/* For NULL =)                                        */
#include <orb.h>
#include "echo.h"  /* In case we need using special constants or types   */
#include <typewrappers.h>
#include <dlfcn.h> /* Will be used if the implementation is in a library */

#define libNameCorbaAware "libEcho.so" /* Library containing corba aware 
					* implementation */
#define libNameNotCorbaAware "libEcho-na.so" /* Library containing non corba 
					      * aware implementation */
#define threadedServer "Echo-server"   /* Executable to run before sending IPC
					* messages to the implementation */

/* GLOBAL VARIABLES */
VOID * globalEchoHandle = NIL; /* This is needed for calling the dynamic
				* linker just once (at most) */

/* Types for pointers to funcions (needed if impl. is a library) */
typedef VOID (*Echo_echoStringType) (CORBA_string, ...);

/* Locally defined functions and constants*/
#define ERRNOLIB 1
#define ERRNOFUNC 2

LOCAL VOID stubFatalError (INT16 errNo) {
  /* Displays an error message and exits with an error code
   *
   * INPUT:
   * errNo : Error number to use
   *
   * RETURNS:
   * Nothing
   */
  printf ("\nSORB Stub for Echo object error: ");
  switch (errNo) {
  case 1 : 
  case 2 :
    printf ("Dynamic linker returned an error:\n%s", dlerror());
    break;
  default :
    printf ("Undefined error.");
  }
  printf ("\n\n");
  if (globalEchoHandle!=NIL) dlclose(globalEchoHandle);
  exit (errNo);
}


/* Implementation of interface functions  */
CORBA_void
Echo_echoString (Echo obj, CORBA_string aString, CORBA_Environment* ev) {
  /* Stub for locating and executing echoString method of the Echo object
   *
   * INPUT:
   * obj     : Object reference
   * aString : String to be printed.
   * ev      : Corba environment
   *
   * RETURNS:
   * ev : if an exception is raised, it is notified by the ev variable (NYI)
   */

  LOCAL Echo_echoStringType echoString=NIL;

  /* To make this more readable, I was thinking in replace the inner
   * conditions with #defined macros. Opinions? [Luc] *** */
  if (orbIsLocal(obj)) {        /* Implementation is local */
    if (orbIsLibrary(obj)) {    /* implementation is a library */
      if (echoString==NIL) {   /* have we linked before to the lib funtion?*/
	if (globalEchoHandle==NIL) /* Check if we need to open the library */
	  if (orbIsCorbaAware(obj)) /* If so, check which one and load it   */
	    globalEchoHandle = dlopen(libNameCorbaAware, RTLD_NOW);
	  else
	    globalEchoHandle = dlopen(libNameNotCorbaAware, RTLD_NOW);
	if (globalEchoHandle==NIL) /* Error opening the library */
	  stubFatalError(ERRNOLIB);

	/* NOTE: dlopen() always return the same handle for an already loaded
	 * library, so many stubs can dlopen the same library at the "same"
	 * time (and share the same handle) without interferences. dlopen, has
	 * to be thread safe, of course. */
	echoString=(Echo_echoStringType) dlsym(globalEchoHandle, "echoString");
	if (echoString==NIL) /* Error linking to the function */
	  stubFatalError (ERRNOFUNC);
      }
      /* At this point we have a valid reference to the function implementing
       * the request */
      if (orbIsCorbaAware (obj))
	echoString(aString, ev);
      else
	echoString(aString);
    } else { /* implementation is a thread */
      /* Deliver the request to the object server by IPC (NYI)*** */
    }			  
  }  else { /* implementation is not local */
    /* Forward the request (NYI)*** */
  }
}

/* What is the type of the _init anf _fini functions? [Luc] *** */
INT32 _init (VOID) {
  return 0;
}

INT32 _fini (VOID) {
  /* Final cleanup
   *
   * INPUT: Nothing
   *
   * OUTPUT: Nothing
   */
  if (globalEchoHandle!=NIL)
    dlclose (globalEchoHandle); /* We are no longer using the library */
  return 0;
}




