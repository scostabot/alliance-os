/*
 * IOSK internal definitions
 * 
 * HISTORY
 * Date     Author      Rev    Notes
 * 20/20/98 S. Costa    1      First draft
 * ^^ ^^ ??????
 * 98/12/4  djarb       2      Revised for compatability with current code and
 *                             compatability with the coding spec.
 * 09/12/98 djarb       3      *Seek functions
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include "iosk.h"
#include <klibs.h>
#include <typewrappers.h>
#include <lm.h>

#ifndef __INTERNAL_H
#define __INTERNAL_H

/* These are defined in the iosk.h header.
typedef STRING CORBA_char;
typedef UWORD32 Alliance_IOSK_Session;
typedef UWORD32 Alliance_IOSK_Caller;
typedef UWORD32 Alliance_IOSK_LockLevel;
*/

typedef struct {
     UWORD32 chainNumber;
} IOSKSession;

/* This doesn't exist.
  typedef UWORD32 IOSKHandle; 
*/


VOID IOSKinit(VOID);

Alliance_IOSK_Session IOSKopenSession(CORBA_char* deviceName,
                                      Alliance_IOSK_Permission perm,
                                      Alliance_IOSK_Caller caller);

#define NOMOREHANDLES 0xFFFFFFFFL

BOOLEAN IOSKcloseSession(Alliance_IOSK_Session session,
                         Alliance_IOSK_Caller caller);

BOOLEAN IOSKlock(Alliance_IOSK_Session session,
		 Alliance_IOSK_Permission perm,
		 Alliance_IOSK_Caller caller);

UWORD32 IOSKread(Alliance_IOSK_Session handle,
		 Alliance_IOSK_bytes* data,
		 UWORD32* length,
		 BOOLEAN block,
		 Alliance_IOSK_Caller caller);
  
UWORD32 IOSKwrite(Alliance_IOSK_Session handle,
		  Alliance_IOSK_bytes* data,
		  UWORD32 size,
		  Alliance_IOSK_Caller caller);

BOOLEAN IOSKreadSeek(Alliance_IOSK_Session handle,
                     Alliance_IOSK_Address where,
                     Alliance_IOSK_SeekDir dir,
                     Alliance_IOSK_Caller caller);

BOOLEAN IOSKwriteSeek(Alliance_IOSK_Session handle,
                      Alliance_IOSK_Address where,
                      Alliance_IOSK_SeekDir dir,
                      Alliance_IOSK_Caller caller);


typedef enum {
    BLOCKDEVICE, CHARDEVICE, DEVUNDEFINED
} DevCategory;

typedef enum {
    LMLOADED, LMUNLOADED
} LMState;

typedef struct {
     DevCategory category;  /* Which kind of device we are dealing with:
                               BLOCKDEVICE, CHARDEVICE                   */
     LM deviceLM;        /* The LM data structure (must be copied from Luc) */
     LMState stateLM;    /* Is the above LM loaded (may should be inside LM)?*/
} Chain;

/* These are a bit low. Perhaps dynamic allocation would be better? */
#define MAXCHAINLENGTH 4 /* Max allowed elements in a LM chain */
#define MAXCHAINS      8 /* Max number of LM chains in IOSK */ 
#define MAXSESSIONS   32 /* Max number of concurrent sessions */

/* Defined values for Permission field */

/* Not needed here. Use Alliance_IOSK_Permission instead. 
typedef enum {
    RW, RO, WO, PERMUNDEFINED
} Perm;
*/

typedef struct {
     Alliance_IOSK_Permission permission; /* What a user can do with this */
     Chain element[MAXCHAINLENGTH]; /* The full chain data */
     Queue chainQ;  /* The queue data relative to a chain */
     UWORD32 last;  /* Index of last LM in the chain (for read operations) */
} IOSKDeviceTable;

#endif
