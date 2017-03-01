/*
 * Dynamic linker shell definitions
 *
 * In this file is defined all shell-wide stuff
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 14/10/00 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#define MAX_CMD_LINE		129 /* Max number of characters in a cmd line */
#define MAX_LINK_PATH		99  /* Max number of characters for environment
                                       variable DYNLINKPATH                     */
#define MAX_FILE_PATH		41  /* Max number of characters for filename
                                     * as input from user                       */

#define MAX_PATH		MAX_LINK_PATH+MAX_FILE_PATH	

#define MAX_NUM_ELF_FILES	8   /* The maximum number of ELF files linkable */

#define MAX_ALLOCATABLE_BLOCKS	32

/* Shell command structure */

typedef struct {
	STRING *string;                       /* Command keyword */
	WORD32 (*entryPoint)(INPUT STRING *); /* Command handler */
} Command;

PUBLIC VOID computePrompt(STRING *path);

