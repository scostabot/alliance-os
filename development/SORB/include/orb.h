/* 
 * SORB library header file
 *
 * This file defines the required functions/types/constants for using SORB
 * calls.
 *
 * HISTORY
 * Date          Author    Rev    Notes
 * Sep 14th,1998 Luc       1.0    Should be using typewrappers.h.
 * Sep 28th,1998 Luc       1.1    Uses typewrappers.h
 * Jan 4th, 1999 Luc       1.2    Added CORBA_octect, fixed CORBA_string
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */ 

#ifndef __ORB_H
#define __ORB_H

#include <typewrappers.h>

/* Incomplete or Non-Compliant definitions are marked with *** */

/* Mapping for basic CORBA types */
/* I'm not too sure that they all are correct */

typedef INT16 CORBA_short;                /* signed 16 bits */
typedef INT32 CORBA_long;                 /* signed 32 bits */
typedef INT64 CORBA_long_long;            /* signed 64 bits */
typedef UINT16 CORBA_unsigned_short;      /* unsigned 16 bits */
typedef UINT32 CORBA_unsigned_long;       /* unsigned 32 bits */
typedef UINT64 CORBA_unsigned_long_long;  /* unsigned 64 bits */
typedef FLOAT32 CORBA_float;              /* IEEE simple precission */
typedef FLOAT64 CORBA_double;             /* IEEE double precision */
typedef long double CORBA_long_double;    /* IEEE double extended */
typedef UBYTE CORBA_char;                 /* 8 bits */
typedef WORD16 CORBA_wchar;               /* Implementation dependant */
typedef UBYTE CORBA_octet;                /* 8 bits w/o modification */
#define CORBA_TRUE  1
#define CORBA_FALSE 0
typedef BOOLEAN CORBA_boolean;   /* Only 0 (false) or 1 (true) allowed */

/* CORBA_string is not defined in the C languaje mapping, but is kept to be
 * consistent with the Coding Guidelines [Luc] *** */
typedef CORBA_char * CORBA_string;
typedef VOID CORBA_void;

/* Managing exceptions*** */
struct CorbaEnvironment { INT32 _major; };
typedef struct CorbaEnvironment CORBA_Environment;

/* Interface Repository*** */
/* This should be in a diferent source file... [Luc] */
typedef CORBA_string CORBA_Scoped_Name;
typedef CORBA_string CORBA_Identifier;
typedef CORBA_string CORBA_RepositoryID;

struct IOR { 
  CORBA_boolean local; /* the implementation is in the local machine     */
  BYTE    ip[4];   /* IP address of the machine that can process the request */
  CORBA_Scoped_Name sn;  /* Scoped name of the object in the IR              */
  BYTE    flags;   /* 0x1 set if the target ORB is a SORB 
		    * 0x2 set if the implementation is a library 
		    * 0x4 set if the implementation is CORBA aware */
}; 

#define orbISSORB       0x1
#define orbISLIBRARY    0x2
#define orbISCORBAAWARE 0x4
#define orbIsLocal(x)      ((x)->local==CORBA_TRUE)
#define orbIsSORB(x)       ((x)->flags&0x1)
#define orbIsLibrary(x)    ((x)->flags&0x2)
#define orbIsCorbaAware(x) ((x)->flags&0x4)
  
/* Other types */
typedef struct IOR * CORBA_Object;
#define CORBA_OBJECT_NIL NIL

/* Operations on object references */
extern CORBA_string CORBA_ORB_object_to_string (CORBA_Object);
extern CORBA_Object CORBA_ORB_string_to_object (CORBA_string);

#endif



