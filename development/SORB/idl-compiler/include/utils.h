/*
 * Macros,types and functions for easier IDL tree parsing.
 *
 * Helpful macro definitions for obtaining frecuently used data from a node
 * without traversing the whole tree (well, more or less :-). Also functions
 * to obtain data from the name space.
 *
 * HISTORY
 * Date      Author Rev Notes
 * 25/Dec/98 Luc    1   Merry Christmas!
 * 31/Dec/98 Luc    2   And Happy new year! Added function declarations.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#ifndef __UTILS_H
#define __UTILS_H

#include <typewrappers.h>
#include <wchar.h> /* needed by IDL.h */
#include <glib.h>  /* needed by IDL.h */
#include <IDL.h>

/* Get the name of the following elements by accessing their "ident" node */
/* Operation declarations */
#define getOperationName(node)  (node->u.idl_op_dcl.ident->u.idl_ident.str)
/* Interfaces */
#define getInterfaceName(node)  (node->u.idl_interface.ident->u.idl_ident.str)
/* Modules */
#define getModuleName(node)     (node->u.idl_module.ident->u.idl_ident.str)
/* Parameter declarations */
#define getParamName(node)      (node->u.idl_param_dcl.simple_declarator->\
                                 u.idl_ident.str)
/* Identifiers */
#define getIdentName(node)      (node->u.idl_ident.str)
/* Exceptions */
#define getExceptionName(node)  (node->u.idl_except_dcl.ident->u.idl_ident.str)


/* Get the scoped name of the given node */
PUBLIC STRING * getScopedName (CONST IDL_tree node);

/* Get the scope of the given node */
PUBLIC STRING * getScope (CONST IDL_tree node);

/* Get the specified contents of the node */
#define getInterfaceBody(node)     (node->u.idl_interface.body)
#define getModuleDefinitions(node) (node->u.idl_module.definition_list)


/* Useful constants */
#define IMPLICITOBJECT ((STRING*)("(CORBA_Object anObject,\n"))
#define IMPLICITENVIRONMENT ((STRING*)("CORBA_Environment *ev)"))
#define IMPLICITCONTEXT ((STRING*)("CORBA_Context context,\n"))

#define ENUMMAPPING ((STRING*)("UINT32"))


/* This type is a temporary (?) substitute of linked lists [Luc] *** */
struct StringListStruct {
  DATA     count;      /* number of items in the list = first free entry */
  STRING * line[255];  /* This is a random limit, but should be enough */
};

typedef struct StringListStruct StringList;
#define utilsInitStringList(list) (list.count=0)
#define utilsFreeStringList(list) while (list.count > 0) { \
                                    list.count--; \
                                    free (list.line[list.count]); \
                                  }
/*
 * Miscelaneous functions
 */

/* Allocates memory or aborts */
PUBLIC VOID * forceMAlloc (ADDR size);

#endif
