/*
 * Declarations for/of the IDL_tree to string conversion functions
 *
 * The functions declared in this file are used by the file generation ones to
 * get the string representation of common structures.
 *
 * HISTORY
 * Date      Author Rev Notes
 * ??/???/98 Luc    1   First coded.
 * 21/Feb/99 Luc    1.1 Modified to match new libIDL(0.5.8) syntax.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

/* Converts a type definition (NOT a type declaration!) to a string */
PUBLIC STRING * typeToString  (IDL_tree type_def);

/* Converts a parameter list into a string list */
PUBLIC gboolean paramsToStringList (IDL_tree node, 
				    IDL_tree parent,
				    void * strParams);

/* Converts a list of identifiers into a string list */
PUBLIC gboolean identsToStringList (IDL_tree node,
				    IDL_tree parent,
				    void * strIdents);
