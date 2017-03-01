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

#ifndef __LMSYS_H
#define __LMSYS_H

#include <typewrappers.h>  /* System-wide types definitions */
#include <systemdefs.h>    /* System-wide definitions */

typedef struct {
	UWORD32 usage;         /* Usage count for that LM */
	EHandle execHandle;    /* Exexutable loader handle allocated for that LM */
	UWORD32 (*entryPoints[3])(); /* Common optional LM entry points pointers */
} LMTable;

/* Checking for ownership of LMs */ 

#define isIOSK1LM(x)  (((x).lmOwner==LMIOSK1) ? TRUE : FALSE) 
#define isFSSK1LM(x)  (((x).lmOwner==LMSFSK1) ? TRUE : FALSE) 
#define isNetSK1LM(x) (((x).lmOwner==LMNETSK1) ? TRUE : FALSE) 
#define isGrSK1LM(x)  (((x).lmOwner==LMGRSK1) ? TRUE : FALSE) 
#define isSndSK1LM(x) (((x).lmOwner==LMSNDSK1) ? TRUE : FALSE) 
#define isSSK1LM(x)   (((x).lmOwner==LMSSK1) ? TRUE : FALSE) 
#define isUISK1LM(x)  (((x).lmOwner==LMUISK1) ? TRUE : FALSE) 
#define isASH1LM(x)   (((x).lmOwner==LMASH1) ? TRUE : FALSE) 
#define isAnySKLM(x)  (((x).lmOwner==LMASH1) ? TRUE : FALSE) 
#define isAnyLM(x)    (((x).lmOwner==LMANY) ? TRUE : FALSE) 

 
#endif
