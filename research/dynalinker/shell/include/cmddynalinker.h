/*
 * Dynamic linker shell definitions
 *
 * In this file are defined all those functions that are specific to link interface
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 15/11/03 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

UWORD32 isInternalCommand(INPUT STRING *cmdLine);
PUBLIC VOID isExternalCommand(INPUT STRING *cmdLine);
PUBLIC VOID uninteractiveShell(STRING *argv[], UWORD32 argc, STRING *entryPointSymName);
