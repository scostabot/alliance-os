/*
 * Functions for creating the header files,
 *
 * Implementation of the callbacks used by the IDL compiler's main code
 * to generate the include files needed by the C languaje mapping for the
 * CORBA 2.2 especification.
 *
 * HISTORY
 * Date      Author Rev Notes
 * 25/Dec/98 Luc    1   Merry Christmas!
 * 21/Feb/99 Luc    1.1 Adpated to new libIDL(0.5.8) syntax.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <klibs.h>
#include "include/utils.h"
#include "include/toString.h"
#include "include/hFile.h"

/*
 * Helper macros
 */
/* writes the #ifndef, #define block */
#define openProtectBlock(file,name) fprintf(file, "#ifndef %s\n#define %s\n",\
                                            name, name)
/* writes the #endif block */
#define closeProtectBlock(file) fprintf(file, "#endif\n\n")

/*
 * Mapping for operations
 */
  
PUBLIC VOID hMapOperations (FILE * hFile, IDL_tree root) {
  /* Outputs to the pre-opened header file the C mapping of an operation 
   * INPUT:
   *  hFile : Poiner to the file descriptor to which the output will be sent
   *  root: IDL tree containing the function to parse
   *  fName: function name (scoped).
   * OUTPUT:
   *  Outputs to the header file the results of the parsing.
   */
  CONST STRING * opType;
  StringList     params;
  DATA           paramCounter = 0;
  
  /* root is of type  struct _IDL_OP_DCL (see ../libIDL/IDL.h) */
  assert (IDL_NODE_TYPE(root) == IDLN_OP_DCL);
  /* Oneway field is ignored in the header file */

  /* op_type_spec (get the return type) */
  opType = typeToString (IDL_OP_DCL(root).op_type_spec);
  /* parameter_decls (get the definition of parameters) */
  utilsInitStringList(params);
  IDL_tree_walk_in_order (IDL_OP_DCL(root).parameter_dcls, 
			  paramsToStringList, &params);
  /* raises_expr is not needed in the header file */
  /* context_expr is only useful for DII... skip by now [Luc] *** */
  fprintf (hFile, "PUBLIC extern %s\n", opType);
  fprintf (hFile, "%s %s", getScopedName(root), IMPLICITOBJECT);
  
  while (paramCounter < params.count) {
    fprintf (hFile, "\t%s,\n", params.line[paramCounter]);
    paramCounter += 2;
  }

  /* Should check for context [Luc] *** */
  fprintf (hFile, "\t%s;\n", IMPLICITENVIRONMENT);
  /* done */
  fprintf (hFile, "\n");
}


/*
 * Mapping for type deffinitions
 */

VOID hMapTypeDef (FILE* hFile, IDL_tree root) {
  /* Outputs to the pre-opened header file passed the C mapping of the type
   * declaration in the IDL tree.
   * INPUT:
   *   hFile : header file descriptor to which the output will be sent.
   *   root  : root node of the IDL tree holding the type definition.
   * OUPUT:
   *   Outputs to the given file the C mapping of the type declaration passed.
   */
  
  /* NOTE: we'll be calling identsToStringList to get all the names for the
   * given type (as libIDL allows it). The IDL grammar does only allow one
   * name in each type definition, thougt [Luc] *** */

  CONST STRING * typeStr;     /* Type declaration */
  StringList     typeNames;   /* List of names assigned to that declaration */
  DATA           nameIndex=1; /* index to process all the names */

  assert (IDL_NODE_TYPE(root) == IDLN_TYPE_DCL);

  /* get the type deffinition */
  typeStr = typeToString (IDL_TYPE_DCL(root).type_spec);
  /* get the names of the type */
  utilsInitStringList(typeNames);
  IDL_tree_walk_in_order (IDL_TYPE_DCL(root).dcls, 
			  identsToStringList, &typeNames);

  while (nameIndex < typeNames.count) {
    openProtectBlock(hFile, typeNames.line[nameIndex]);
    fprintf (hFile, "typedef %s %s;\n", typeStr, typeNames.line[nameIndex]);
    closeProtectBlock(hFile);
    nameIndex +=2;
  }
}


/*
 * Mapping for enumerations
 */
  
PUBLIC VOID hMapEnum (FILE* hFile, IDL_tree root) {
  /* Outputs to the pre-opened header file passed the C mapping of the
   * enumeration declaration in the IDL tree.
   * INPUT:
   *   hFile : header file descriptor to which the output will be sent.
   *   root  : root node of the IDL tree holding the enumeration declaration.
   * OUPUT:
   *   Outputs to the given file the C mapping of the enumeration declaration
   *  passed.
   */
  CONST STRING * enumName;    /* Name of the declared enum */
  StringList     enumerators; /* Name of each item in the enumeration */
  UINT32         enumIndex=0; /* Index for processing the enumerators. Is
			       * defines as UINT32 becouse the CORBA specs
			       * (see page 3.27, "enumerations"). */

  assert (IDL_NODE_TYPE(root) == IDLN_TYPE_ENUM);

  enumName = getScopedName (root); /* get the type name */

  /* get names of the enumerators */
  utilsInitStringList(enumerators);
  IDL_tree_walk_in_order (IDL_TYPE_ENUM(root).enumerator_list,
			  identsToStringList, &enumerators);
  /* now really echo the output */
  openProtectBlock(hFile, enumName);
  fprintf (hFile, "typedef %s %s;\n", ENUMMAPPING, enumName);
  while (enumIndex < enumerators.count) {
    fprintf (hFile, "#define %s (UINT32)( 0x%08lx )\n", 
	     enumerators.line[enumIndex+1], enumIndex/2);
    enumIndex += 2;
  }
  /* done */
  closeProtectBlock(hFile);
}
  

/*
 * Mapping for Exceptions
 */

gboolean hMapMember (IDL_tree root,
		     IDL_tree parent, 
		     VOID * hFile) {
  /* Helper function for producing the output of the member declarations
   * within an exception or structure.
   * INPUT:
   *   root : root node of the member declaration
   *   parent : Parent of the node being processed.
   *   hFile : pre-opened file to receive the output
   * OUTPUT:
   *   returns TRUE if the tree needs to be processed, FALSE if that was
   *  done.
   */

  StringList     declarations; /* list of variables declared for this type */
  DATA           declIndex=2;  /* Index to process the declarations */

  if (IDL_NODE_TYPE(root) == IDLN_MEMBER) {
    utilsInitStringList(declarations);
    IDL_tree_walk_in_order (IDL_MEMBER(root).dcls,
			    identsToStringList, &declarations);
    fprintf ((FILE*)(hFile), "\t%s %s", 
	     typeToString(IDL_MEMBER(root).type_spec), 
	     declarations.line[0]);
    while (declIndex < declarations.count) {
      fprintf ((FILE*)(hFile), ", %s", declarations.line[declIndex]);
      declIndex +=2;
    }
    fprintf ((FILE*)(hFile), ";\n");
    return FALSE;
  } else 
    return TRUE;
}

  

PUBLIC void hMapException (FILE* hFile, IDL_tree root) {
  /* Outputs to the pre-opened header file passed the C mapping of the
   * exception declaration in the IDL tree.
   * INPUT:
   *   hFile : header file descriptor to which the output will be sent.
   *   root  : root node of the IDL tree holding the exception declaration.
   * OUPUT:
   *   Outputs to the given file the C mapping of the exception declaration
   *  passed.
   */

  CONST STRING * exceptionName;
 
  assert (IDL_NODE_TYPE(root) == IDLN_EXCEPT_DCL);

  exceptionName = getScopedName(root);

  /* print the boundaries */
  openProtectBlock(hFile, exceptionName);
  fprintf (hFile, "typedef struct %s {\n", exceptionName);
  /* print the contents */
  IDL_tree_walk_in_order (IDL_EXCEPT_DCL(root).members, hMapMember, hFile);
  /* close the definition */
  fprintf (hFile, "} %s;\n", exceptionName);

  /* unique ID for the exception */
  fprintf (hFile, "#define ex_%s \"%s\"\n", exceptionName, 
	   root->u.idl_except_dcl.ident->u.idl_ident.repo_id);
  /* allocation function */
  fprintf (hFile, "#define %s__alloc() (%s *)\\\n\t(malloc(sizeof(%s)))\n",
	   exceptionName, exceptionName, exceptionName);
  /* done */
  closeProtectBlock(hFile);
}
	   
