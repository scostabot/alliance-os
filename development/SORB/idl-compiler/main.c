/*
 * Starting point of the IDL compiler.
 *
 * This file contains the structure of the basic parsing sequence of an
 * IDL file. It also interprets the command line arguments.
 *
 * HISTORY
 * Date      Author Rev Notes
 * 24/Dec/98 Luc    1   First contact with libIDL. Merry Christmas!
 * 22/Feb/99 Luc    1.1 Modified to match new idlLIB(0.5.8) syntax.y
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#include <glib.h>
#include <wchar.h> /* needed by IDL.h */
#include <IDL.h>
#include <typewrappers.h>
#include <klibs.h>
#include <orb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/utils.h" /* Helpful macros */
#include "include/hFile.h" /* Definitions for the generation of .h files */

typedef enum { headerFile = 0, stubFile, skeletonFile } OutputFileKind;

typedef struct {
  FILE* file[3];
} OutputFiles;

gboolean printCProtos (IDL_tree root,
		       IDL_tree parent,
		       gpointer files) {
  /* This function is called as an entry point to the parsing and
   * recursivelly to determine the scoped name of each function.
   *
   * INPUT:
   *  root ; root node of th IDL tree to parse.
   *  parent : parent node of the one being processed.
   *  user_data : additional data, in this case the current scope.
   * OUTPUT
   *  Returns TRUE if "tree" has to be processed, FALSE otherwise
   */

#define _OUTFILES ((OutputFiles *)(files))

  switch (IDL_NODE_TYPE(root)) {
  case IDLN_INTERFACE:
    /* Keep traversing the tree */
    return TRUE;
  case IDLN_MODULE :
    /* Keep traversing the tree */
    return TRUE;
  case IDLN_OP_DCL: /* Declation of operations */
    /* generate some output... */
    hMapOperations(_OUTFILES->file[headerFile], root); /* .h file */
    /* ... for stubs and skeletons shoul be here too [Luc] *** */
    return FALSE;
  case IDLN_TYPE_DCL: /* Declaration of types */
    /* generate some output */
    hMapTypeDef(_OUTFILES->file[headerFile], root); /* .h file */
    /* Missing skeletons and stubs [Luc] *** */
    return FALSE;
  case IDLN_TYPE_ENUM: /* enumeration */
    hMapEnum (_OUTFILES->file[headerFile], root); /* write to .h file */
    return FALSE;
  case IDLN_EXCEPT_DCL: /* exception */
    hMapException (_OUTFILES->file[headerFile],root); /* write to .h file */
  default :
    return TRUE;
  }
}    

OutputFiles openOutputFiles (INPUT STRING *idlFile) {
  /* Opens for writing the generated output files. The names are generated
   * from the idlFile like this: file.idl -> file.h, file-stub.c, file-skel.c
   * A verification on the idlFile is also done (it must end with ".idl")
   * INPUT:
   *  idlFile : name of the file to be parsed.
   * OUTPUT:
   *  Of everything goes well, returns an strucuture with the files opened. 
   * Otherwise a message is printed and the abort() called.
   */

  CONST STRING * SUFIX[] = {".h", "-stub.c", "-skel.c"};
  CONST DATA     SUFIXLEN[] = { 2, 7, 7 };

  STRING * fileName = NIL;  /* Temporary holder of each file name */
  STRING * sourceName;      /* Name (w/o extension) of the source idl file */
  DATA     nameLength;      /* Length of the current file name */
  OutputFileKind which;     /* Loop variable used as array index */
  OutputFiles result;
 
  /* Verify we have a nice name */
  nameLength = KLstringLength(idlFile);
  sourceName = forceMAlloc(nameLength);
  sourceName = KLstringCopy (sourceName, idlFile);
  if (KLstringLength(KLstringMatch(idlFile, ".idl"))!=4) {
    printf ("The source file name must end in \".idl\"\n");
    abort ();
  }

  /* Now strip the ".idl" extension */
  *(sourceName+nameLength-4)='\0'; /* Truncate at the "." */
  nameLength -= 4;
  
  /* Open the output files */
  for (which = headerFile; which <= skeletonFile; which ++) {
    if ((fileName = realloc(fileName, nameLength + SUFIXLEN[which]))==NIL) {
      printf ("Not enough memory.\n");
      abort();
    }
    fileName = KLstringAppend (KLstringCopy(fileName,sourceName), SUFIX[which]);
    
    if ((result.file[which] = fopen (fileName, "w"))==NIL) {
      printf ("Could not open %s for writing. Check permissions and disk "
	      "space/quota\n", fileName);
      abort();
    }
  }
  return result;
}

int main (int argc, char * argv[]) {
  int parseResult = 0;
  IDL_tree tree; /* here we will have the parse tree */
  IDL_ns   ns;   /* here we will have the name space :-? */
  STRING * idlFile = argv[1];
  OutputFiles files;
  unsigned long flags =0;

  IDL_check_cast_enable (TRUE);

  /* Check we have been given an IDL file to process */
  if (argc < 2) {
    printf("Error, should give a file to parse.\n");
    exit(1);
  }

  parseResult = IDL_parse_filename (idlFile, NIL, NIL, &tree, &ns,
				    flags, IDL_WARNING1);
  /* The NULL are for the callback functions for each node processed (tree
   * and name space). The "0" is for not expanding constants. */

  if (parseResult == IDL_SUCCESS) {
    files = openOutputFiles (idlFile);
    IDL_tree_walk_in_order (tree, printCProtos, &files);
    IDL_ns_free(ns); /* Deallocate the namespace tree */
    IDL_tree_free(tree); /* Deallocate the IDL tree */
  }
  else if (parseResult == IDL_ERROR) {
    fprintf(stderr, "tstidl: IDL_ERROR\n");
    exit(1);
  }
  else if (parseResult < 0) {
    perror(idlFile); /* this behaviour of libIDL is not very nice... */
    exit(1);
  }
    
  return 0;
}
