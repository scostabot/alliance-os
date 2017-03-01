/*
 * Common translations from IDL_Tree to string.
 *
 * The translations performed by the functions in this file are common to, at
 * least two of the generated files by the idl-compiler. These functions just
 * return a value (a string or string list), and do not write to disk.
 *
 * HISTORY
 * Date          Author Rev Notes
 * Jan 4th, 1999 Luc    1.0 In the beginning...
 * Feb 21st,1999 Luc    1.1 Modified to match new libIDL(0.5.8) syntax.
 * Mar 13th,1999 Luc    1.2 Added support for sequences
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <wchar.h> /* needed by IDL.h */
#include <glib.h>  /* needed by IDL.h */
#include <IDL.h>
#include <stdlib.h> /* for malloc */

#include <typewrappers.h>
#include <klibs.h>
#include "include/utils.h"
#include "include/toString.h"

/*
 * Private functions
 */

STRING * sequenceToString(CONST IDL_tree sequence) {
  /* Returns the parsing of the given sequence IDL definition
   * INPUT:
   *   sequence : The sequence to be parsed.
   * OUTPUT:
   *   returns the parsed string.
   */
  CONST STRING * firstPart = 
    "typedef struct {\n"
    "\tCORBA_unsigned_long __maximum;\n"
    "\tCORBA_unsigned_long _length;\n\t"; /* Here goes the sequence type */
  CONST STRING * endingPart =
    " *_buffer;\n"
    "\tUBYTE _release : 1; /* 1-bit release flag */\n"
    "} "; /* Here goes the sequence name */
  STRING * sequenceType = 
    typeToString(IDL_TYPE_SEQUENCE(sequence).simple_type_spec);
  int size = KLstringLength(firstPart) + KLstringLength(sequenceType) +
    KLstringLength(endingPart);
  STRING * result = (STRING*)malloc(size);

  result = KLstringCopy(result, firstPart);
  result = KLstringAppend(result, sequenceType);
  result = KLstringAppend(result, endingPart);

  return result;
}  

/*
 * Functions published in the header file
 */
STRING * typeToString (CONST IDL_tree typeDef) {
  /* Returns a pointer to a static string representing the C 
   * equivalent of the  CORBA type defined by the passed node.
   * INPUT:
   *  typeDef : Node specifying the CORBA type to map.
   * OUTPUT:
   *  returns the allocated string.
   */

  /* Currently only implemented for basic types [Luc] *** */
  if (typeDef == NULL) /* no type */
    return "VOID"; 

  switch (IDL_NODE_TYPE(typeDef)) {
  case IDLN_TYPE_FLOAT:
    /* Find out which kind of float */
    switch (IDL_TYPE_FLOAT(typeDef).f_type) {
    case IDL_FLOAT_TYPE_FLOAT:
      return "CORBA_float";
    case IDL_FLOAT_TYPE_DOUBLE:
      return "CORBA_doble";
    case IDL_FLOAT_TYPE_LONGDOUBLE:
      return "CORBA_long_double";
    default:
      printf ("\nUnknown float type.\n");
      abort();
      break;
    };    
  case IDLN_TYPE_FIXED:
    /* NYI *** */
    return "fixed_not_yet_implemented";
  case IDLN_TYPE_INTEGER:
    /* Find out which kind of integer */
    switch (IDL_TYPE_INTEGER(typeDef).f_type) {
    case IDL_INTEGER_TYPE_SHORT:
      return ((IDL_TYPE_INTEGER(typeDef).f_signed) ?
	"CORBA_short" : "CORBA_unsigned_short");
    case IDL_INTEGER_TYPE_LONG:
      return (IDL_TYPE_INTEGER(typeDef).f_signed) ?
	"CORBA_long" : "CORBA_unsigned_long";
    case IDL_INTEGER_TYPE_LONGLONG:
      return (IDL_TYPE_INTEGER(typeDef).f_signed) ?
	"CORBA_long_long" : "CORBA_unsigned_long_long";
    default:
      printf ("\nUnknown integer type.\n");
      abort();
    }
  case IDLN_TYPE_CHAR:
    return "CORBA_char";
  case IDLN_TYPE_WIDE_CHAR:
    return "CORBA_wchar";
  case IDLN_TYPE_STRING:
    return "CORBA_char *"; /* Use CORBA_string instead? [Luc] *** */
  case IDLN_TYPE_BOOLEAN:
    return "CORBA_boolean";
  case IDLN_TYPE_OCTET:
    /* Could not find it in the C languaje mappings. *** */
    return "CORBA_octet";
  case IDLN_TYPE_ANY:
    return "CORBA_any";
  case IDLN_TYPE_OBJECT: /* *** */
    return "Only_basic_types_implemented";
  case IDLN_TYPE_ENUM: /* *** */
    return "Only_basic_types_implemented";
  case IDLN_TYPE_SEQUENCE:
    return sequenceToString(typeDef);
  case IDLN_TYPE_ARRAY: /* *** */
    return "Only_basic_types_implemented";
  case IDLN_TYPE_STRUCT: /* *** */
    return "Only_basic_types_implemented";
  case IDLN_TYPE_UNION: /* *** */
    return "Only_basic_types_implemented";
  case IDLN_IDENT: /* user-defined type */
    return getScopedName (typeDef);
  default: /* Should never happen, but must be here to avoid egcs complains */
    return "REALLY_WEIRD_THINGS_HAPPEN";
  }
}


gboolean paramsToStringList (IDL_tree node,
			     IDL_tree parent,
			     void * strParams) {
  /* Called on each node of a parameter declaration. It builds a list of
   * parameter declaration strings on the result parameter.
   * INPUT:
   *  node : the IDL tree node being processed.
   *  parent : Parent of the node being processed.
   *  result : StringList with the actual mapped parameters.
   * OUTPUT:
   *  returns TRUE when libIDL has to navigate the tree, FALSE otherwise.
   *  result : If the processed node is a parameter declaration one, its
   *          mapping is added, otherwise it is kept the same.
   */
  STRING * paramType; /* string containing the type of the parameter */
  DATA     declSize;  /* size of the mapped paramter declaration */

#define STRPARAMS (*(StringList *)(strParams))

  switch (IDL_NODE_TYPE(node)) {
  case IDLN_LIST :
    return TRUE; /* don't do a thing */
  case IDLN_PARAM_DCL:
    paramType = typeToString(IDL_PARAM_DCL(node).param_type_spec);
    if (IDL_PARAM_DCL(node).attr != IDL_PARAM_IN) {
      declSize = (KLstringLength(paramType) +         /* Length of the type */
		  3 +                                 /* length of " * "    */
		  KLstringLength(getParamName(node))+ /* Length of the name */
		  1);                                 /* Termination char   */
      STRPARAMS.line[STRPARAMS.count] = (STRING*)(forceMAlloc(declSize));
      sprintf (STRPARAMS.line[STRPARAMS.count], "%s * %s", 
	       paramType, getParamName(node));
      STRPARAMS.count ++;
    } else {
      declSize = (6 +                          /* length of "INPUT "   */
		  KLstringLength(paramType) +  /* Length of the type   */
		  1+                           /* Length of whitespace */
		  KLstringLength(getParamName(node))+ /* Length of the name */
		  1);                          /* Termination char     */
      STRPARAMS.line[STRPARAMS.count] = (STRING*)(forceMAlloc(declSize));
      sprintf (STRPARAMS.line[STRPARAMS.count], "INPUT %s %s", 
	       paramType, getParamName(node));
      STRPARAMS.count ++;
    }
    return FALSE;
  default: /* Never should get here */
    abort();
    return FALSE; /* to avoid warnings */
  }
}

gboolean identsToStringList (IDL_tree node,
			     IDL_tree parent,
			     void * strIdents) {
  /* Called on each node of a identifier's list (like in typedefs). It builds
   * a list of parameter declaration strings on the result parameter.
   * INPUT:
   *  node : the IDL tree node being processed.
   *  parent : Parent of the node being processed.
   *  result : StringList with the actual mapped identifiers.
   * OUTPUT:
   *  returns TRUE when libIDL has to navigate the tree, FALSE otherwise.
   *  result : If the processed node is a identifier one, its
   *          mapping is added, otherwise it is kept the same.
   */

#define STRIDENTS (*(StringList *)(strIdents))

  switch (IDL_NODE_TYPE(node)) {
  case IDLN_LIST :
    return TRUE; /* don't do a thing */
  case IDLN_IDENT:
    /* trick to get scoped and no scoped names in the same function */
    STRIDENTS.line[STRIDENTS.count] = getIdentName(node);
    STRIDENTS.line[STRIDENTS.count+1] = getScopedName(node);
    STRIDENTS.count += 2;
    return FALSE;
  default: /* Never should get here */
    abort();
    return FALSE; /* to avoid warnings */
  }
}
