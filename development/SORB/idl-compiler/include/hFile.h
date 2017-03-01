/*
 * Definitions for the generation of header files by the IDL compiler
 *
 * Declares the functions needed to be known by the core of the idl compiler
 * to generate the header files needed by the C languaje mapping of the
 * CORBA 2.2 specification.
 *
 * HISTORY
 * Date      Author Rev Notes
 * 25/Dec/98 Luc    1   Coding started... Merry Christmas!
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. 
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. 
 */

#ifndef __HFILE_H
#define __HFILE_H

#include <glib.h>
#include <IDL.h>

#include <typewrappers.h>

#include <stdio.h>

/* 
 * Entrypoints for main.c to generate the output.
 */
PUBLIC VOID hMapOperations (FILE* hFile, IDL_tree root);

PUBLIC VOID hMapTypeDef (FILE* hFile, IDL_tree root);

PUBLIC VOID hMapEnum (FILE* hFile, IDL_tree root);

PUBLIC void hMapException (FILE* hFile, IDL_tree root);

#endif
