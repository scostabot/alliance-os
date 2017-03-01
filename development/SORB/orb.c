/* 
 * Implementation of SORB functions not covered by stubs
 *
 * This file implements basic functions that are intended to be shared by many
 * stubs/skeletons.
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

#include <typewrappers.h>
#include <orb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*--------------------------------------------------------*/
/* Local Stuff                                            */
/*--------------------------------------------------------*/

#define UNCHECKEDFLAGS "........" /* 8 unchecked flags <-> 8 dots */
#define CHECKEDFLAG   'X'        /* 1 checked flag <-> 1 X */

/* byte to string */
/* extrange but I haven't found this in the man pages [Luc]*** */
STRING * btos (BYTE value, STRING * str) {
  /* This functions converts a byte to its string representation
   *
   * INPUT
   * value : byte value to be converted
   *
   * RETURNS
   * pointer to the resulting string
   */
  str[0]='0'+(value / 100);
  str[1]='0'+((value % 100)/10);
  str[2]='0'+(value % 10);
  str[3]='\0';

  return str;
}


/* Error handling */
#define ERRNOMEM 1

VOID orbFatalError (INT32 errNo) {
/* Displays an error message and terminates execution
 *
 * INPUT:
 * errNo : Error number to be used
 *
 * RETURNS:
 * Nothing
 */
  printf ("SORB library error: ");
  switch (errNo) {
  case 1 :
    printf ("Could not allocate memory.");
    break;
  default :
    printf ("Undefined error.");
  }
  printf ("\n\n");
  exit (errNo);
}

/*--------------------------------------------------------*/
/* Exported stuff                                         */
/*--------------------------------------------------------*/

CORBA_string CORBA_ORB_object_to_string (CORBA_Object anObject) {
/* Translates an object reference to a printable string
 *
 * INPUT:
 * anObject : object reference to be translated
 *
 * RETURNS:
 * The stringfied representation of the object
 *
 * NOTE: In shake of speed and simplicity , and as CORBA_string maps to 
 * STRING *, I will use standard allocation and string functions [Luc]
 */
  CORBA_string result;
  STRING strFlags[9];
  STRING strByte[4];
  INT32  size= 4 +                      /* size of "IOR:" */
	       3*4 +                    /* size of stringfied ip */	       
	       8 +                      /* 8 chars for the flags field */ 
               strlen(anObject->sn) +   /* size of the Scoped Name */
  	       1;                       /* 1 char for \0 */
  /* The local flag is not stored becouse it is filled based on the IP each
   * time the string is converted back to an object reference */
  strcpy (strFlags, UNCHECKEDFLAGS);
  if ((result =(STRING *) malloc (size))!=NULL) {
    /* flags are printed in reverse order... it is simpler */
    if (orbIsSORB(anObject))    
      strFlags[0] = CHECKEDFLAG;
    if (orbIsLibrary(anObject)) 
      strFlags[1] = CHECKEDFLAG;
    if (orbIsCorbaAware(anObject))
      strFlags[2] = CHECKEDFLAG;

    /* Can be probably optimized if we copy all "by hand" [Luc] */
    strcat(result,"IOR:");
    strcat(result, btos(anObject->ip[0], strByte));
    strcat(result, btos(anObject->ip[1], strByte));
    strcat(result, btos(anObject->ip[2], strByte));
    strcat(result, btos(anObject->ip[3], strByte)); 
    /* I should have used a "for" [Luc] */
    strcat(result, strFlags);
    strcat(result, anObject->sn);
  } else orbFatalError (ERRNOMEM);
  return result;
}
  

CORBA_Object CORBA_ORB_string_to_object (CORBA_string aString) {
/* A IOR string into an object reference
 *
 * INPUT:
 * aString : The string to be converted
 *
 * RETURNS:
 * The generated object reference
 *
 * NOTE: In shake of speed and simplicity , and as CORBA_string maps to 
 * STRING *, I will use standard allocation and string functions [Luc]
 */

  STRING buf[4];/* buffer   */
  INT16  i, j;  /* counters */
  CORBA_Object obj; /* The object reference */
  
  /* set up the Object Reference */
  if ( (obj = (CORBA_Object) malloc (sizeof(struct IOR)))==NULL)
    orbFatalError(ERRNOMEM);
  
  /* get the IP of target machine */
  buf[3]='\0';
  for (i=0; i<4; i++) {               /* read the 4 bytes of the ip    */
    for (j=0; j<3; j++)               /* read the 3 chars of each byte */
      buf[j]=*(aString+4+j+i*3);      /* Add 4 to skip "IOR:" */
    obj->ip[i]=(BYTE)(atoi(buf));
  };

  /* Check if it is a local call, simplified for testing *** */
  if ((obj->ip[0]==127) && (obj->ip[1]==0) && 
      (obj->ip[2]==0)   && (obj->ip[3]==1))
    obj->local = CORBA_TRUE;
  else
    obj->local = CORBA_FALSE;

  /* Fill the flags */
  obj->flags = 0;
  for (i=16; i<24; i++)
    if (*(aString+i)=='X')  obj->flags = obj->flags | (1<<(i-16));
  /* The bit shift is fast but  machine dependant [Luc]*** */

  /* From here, we've got the Scoped Name */
  if ((obj->sn = (STRING *) malloc (strlen(aString+24)))==NULL)
    orbFatalError(ERRNOMEM);
  strcpy (obj->sn, (aString+24));

  /* Everithing done */
  return obj;
}


