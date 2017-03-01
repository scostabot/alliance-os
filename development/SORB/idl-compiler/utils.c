/*
 * Implementation of functions for easier parsion of the IDL trees.
 *
 * Functions with diverse utility that can't or weren't chosen to be 
 * implemented as macros.
 *
 * HISTORY
 * Date      Author Rev Notes
 * 31/Dec/98 Luc    1   Happy new year!
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <typewrappers.h>
#include <klibs.h>
#include <stdlib.h> /* malloc() */
#include <wchar.h>/* Required by IDL.h */
#include <glib.h> /* Required by IDL.h */
#include <IDL.h>
#include "include/utils.h"

STRING * getScopedName (CONST IDL_tree node) {
  /* Finds the scoped name of the node if it is of a scoped name (see the
   * IDL_NODE_IS_SCOPED macro on IDL.h)
   * INPUT:
   *  node : node whose scoped name we want to know.
   * OUTPUT:
   *  The scoped name of the node, if exists (it can be an empty string), or
   * NIL if the node isn't a scoped one.
   */

  STRING * result; /* The string to be returned */
  STRING * source; /* string to be processed */
  STRING * index;  /* Pointer to the character that is being processed */

  if (!IDL_NODE_IS_SCOPED(node))
    return NIL;

  /* Find out where is the source string */

  switch (IDL_NODE_TYPE(node)) {
  case IDLN_IDENT:
    source = node->u.idl_ident.repo_id;
    break;
  case IDLN_INTERFACE:
    source = node->u.idl_interface.ident->u.idl_ident.repo_id;
    break;
  case IDLN_MODULE:
    source = node->u.idl_module.ident->u.idl_ident.repo_id;
    break;
  case IDLN_EXCEPT_DCL:
    source = node->u.idl_except_dcl.ident->u.idl_ident.repo_id;
    break;
  case IDLN_OP_DCL:
    source = node->u.idl_op_dcl.ident->u.idl_ident.repo_id;
    break;
  case IDLN_TYPE_ENUM:
    source = node->u.idl_type_enum.ident->u.idl_ident.repo_id;
    break;
  case IDLN_TYPE_STRUCT:
    source = node->u.idl_type_struct.ident->u.idl_ident.repo_id;
    break;
  case IDLN_TYPE_UNION:
    source = node->u.idl_type_union.ident->u.idl_ident.repo_id;
    break;
  default:
    /* Can't get here */
    abort();
  }
  /* Allocate memory for the resulting string.*/
  /* We allocate the same ammount of memory for simplicity, we should not
   * allocate for the "IOR:" prefix and ":1.0" postfix */
  result = forceMAlloc(KLstringLength(source));

  /* Copy the scoped name changing slashes by underscores*/
  source += 4; /* skip "IOR:" */
  for (index = result;
       (*source) != ':';    /* stop when ":IOR" is found. Simplified. */
       index ++, source++)
    *(index) = (*source)=='/' ? '_' : (*source);

  /* Now mark the end of the string */
  *(index) = '\0';
  
  /* Go home :) */
  return result;
}


/* ----------------------------------------------------------------------
 * Miscelaneous
 */

VOID * forceMAlloc (ADDR size) {
  /* This function is a wrapper for the old malloc. It just gives an error
   * message and aborts if the memory can't be assigned
   * INPUT:
   *   size : amount of memory requested;
   * OUTPUT:
   *   returns a pointer to the allocated memory, or aborts if it is impossible
   */

  VOID * result = malloc(size);

  if (result != NIL) return result;
  else { 
    printf ("forceMAlloc: Could not allocate memory... aborting.");
    abort ();
  }
}
