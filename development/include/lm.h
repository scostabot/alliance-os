/*
 * LMs system-wide symbol definitions
 *
 * In this file are listed all those definitions pertinent to all sorts of LMs.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 04/05/99 scosta    1.0    First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifndef __LM_H
#define __LM_H

#include <typewrappers.h>  /* System-wide types definitions */
#include <systemdefs.h>    /* System-wide definitions */

/* LMType and LMOwner definition */ 

typedef UWORD16 LMOwner; 
typedef enum { 
	LMIOSK, LMFSSK, LMNETSK, LMGRSK,
   	LMSNDSK, LMSSK, LMUISK          } LMType;
	
typedef struct {
	STRING * lmName;        /* Descriptive name of this LM */
	STRING * lmDescription; /* Any description the LM wants to provide */
	LMOwner  lmOwner;       /* Identifier of the SK owning the LM */ 
	Version  release;       /* Release number of the LM */ 
} LMDescription;

typedef UWORD32 LMHandle;

#define LM_ERROR_BASE		0xFFFF0000L

#define LM_NOMORE_LMS       LM_ERROR_BASE+1
#define LM_FILE_ERR         LM_ERROR_BASE+2
#define LM_WRONG_PLATFORM   LM_ERROR_BASE+3
#define LM_NODESCRIPTION    LM_ERROR_BASE+4
#define LM_WRONG_VERS       LM_ERROR_BASE+5
#define LM_NO_ENTRYPOINTS   LM_ERROR_BASE+6
#define LM_INIT_ERROR       LM_ERROR_BASE+7

PUBLIC BOOLEAN LMlibInit(INPUT UWORD32 numOfLMs);
PUBLIC LMHandle LMload(INPUT STRING *moduleName, INPUT Version *required);
PUBLIC VOID LMunload(INPUT LMHandle lmId);
PUBLIC VOID LMlibUninit(VOID);

PUBLIC UWORD32 lowLevelOpen(INPUT STRING *);
PUBLIC UWORD32 lowLevelRead(INPUT UWORD32, ExecRecord *);
PUBLIC UWORD32 lowLevelClose(INPUT UWORD32);

#endif
