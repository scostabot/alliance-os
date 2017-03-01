/*
 * Alliance Kernel Library Functions prototypes
 *
 * This file defines function prototypes of Kernel Library, a collection
 * of general purpose function, many derived from standard C library.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 18/10/98 scosta    1.0    First draft
 * 18/11/98 ramon     1.1    Updated to Alliance Coding Style
 * 07/01/99 scosta    1.2    Include systemdefs.h instead of typewrappers.h
 * 06/02/99 scosta    1.3    Added monitoring facilities definitions
 * 14/03/99 jens      1.4    Added Alloc to KLlibs
 * 24/03/99 scosta    1.5    Fixed a bug in KLtrace macro declaration
 * 28/03/99 scosta    1.6    Added the call KLsetTraceOptions
 * 11/04/99 scosta    1.7    Fixed macro definition for KLtrace
 * 21/04/99 scosta    1.8    Added queue support
 * 18/12/99 scosta    1.9    Changed args for some Memory() ops
 * 12/02/00 scosta    2.0    Added prototype for KLprint()
 * 24/11/00 scosta    2.1    Added support for executable library
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifndef __KLIBS_H
#define __KLIBS_H

#include <systemdefs.h>   /* All definitions for function declaration   */
#include <stdio.h>        /* Required only by UNIX, to be removed later */
#include <stdarg.h>       /* Comes with egcs */

/* String module library functions */

PUBLIC STRING *KLstringAppend(OUTPUT STRING *dest, INPUT STRING *src);
PUBLIC DATA KLstringCompare(INPUT STRING *p1, INPUT STRING *p2);
PUBLIC STRING *KLstringCopy(OUTPUT STRING *dest, INPUT STRING *src);
PUBLIC STRING *KLstringNumCopy(OUTPUT STRING *s1, INPUT STRING *s2, INPUT ADDR n);
PUBLIC DATA KLstringCompareNoCase(INPUT STRING *s1, INPUT STRING *s2);
PUBLIC STRING *KLstringMatch(INPUT STRING *phaystack, INPUT STRING *pneedle);
PUBLIC ADDR KLstringLength(INPUT STRING *str);
PUBLIC DATA KLstringFormat(OUTPUT STRING *buffer, CONST STRING *formatString, ...);
PUBLIC DATA KLformatToString(OUTPUT STRING *buffer, CONST STRING *formatString, va_list arguments);
PUBLIC DATA KLasciiToInt(INPUT STRING *string);

/* Memory module library definitions */

PUBLIC VOID *KLmemorySet(OUTPUT VOID *dstpp, INPUT UBYTE c, INPUT ADDR len);
PUBLIC VOID *KLmemoryCopy(OUTPUT VOID *dstpp, INPUT VOID *srcpp, INPUT ADDR len);
PUBLIC DATA KLmemoryCompare(INPUT VOID *a, INPUT VOID *b, INPUT ADDR len);

/* Queue module library definitions */

typedef struct {
	VOID *data;
	UWORD32 size;
} Packet;			/* An individual item in the queue */

typedef enum { Q_NOQUEUE=-4, Q_OVERFLOW, Q_UNDERFLOW, Q_WRITEERR,
               Q_READOK, Q_WRITEOK } QueueState;

#define QVALID_HANDLE	0xFFFF0000L /* Handles > than this are error flags */

/* Queue error codes for KLqueueAlloc */

#define QNO_PACKETS     QVALID_HANDLE+1  /* Number of packets requested=0 */

typedef UWORD32 Qhandle; /* Queue descriptor type */

BOOLEAN KLqueueInit(INPUT UWORD32 numOfQueues);
Qhandle KLqueueAlloc(INPUT UWORD32 queueSize, INPUT UWORD32 maxPackets);
QueueState KLqueueInsert(INPUT Qhandle q, INPUT Packet *packet);
QueueState KLqueueRemove(INPUT Qhandle qId, Packet *packet);
BOOLEAN KLqueueFree(INPUT Qhandle qId);

/* Thread module library definitions */

#ifdef REAL_ALLIANCE

/* Thread and Mutex type definitions for running in the CK environment */

/* These are temporary. They will be replaced when the interface to
 * the CK threading code is solid.
 */
typedef int Thread;
typedef int Mutex;

#else /* not REAL_ALLIANCE */

/* Thread and mutex type definition for running over POSIX */

#include <pthread.h>  /* Posix threads. _REENTRANT should be defined as well */

typedef pthread_t Thread;
typedef pthread_mutex_t Mutex;

#endif  /* REAL_ALLIANCE */

PUBLIC DATA KLthreadCreate(Thread* thread,
			   /* INPUT */ DATA (*threadMain) (VOID*),
			   VOID* mainParam);
PUBLIC DATA KLthreadJoin(INPUT Thread* thread, DATA* threadReturn);
PUBLIC VOID KLthreadExit(INPUT DATA returnValue);
PUBLIC DATA KLthreadSleep(INPUT DATA microseconds);
PUBLIC DATA KLthreadYield();

PUBLIC DATA KLmutexCreate(Mutex* mutex);
PUBLIC DATA KLmutexLock(Mutex* mutex);
PUBLIC DATA KLmutexUnlock(Mutex* mutex);
/* PUBLIC BOOLEAN KLmutexIsLocked(INPUT Mutex* mutex); */
/* The 'is mutex locked?' operation depends on a function which is
 * apparently not included in the Linux pthreads library. We need a
 * workaround.
 */

/* Monitoring facilities definitions */

#ifdef SKTRACE

/* Since monitoring facilities are required to be (possibly) left out
 * of the code at compilation time, we "fake" standard Alliance
 * function names with a macro, that is in turn defined differently
 * if the symbol SKTRACE is defined or not.
 * In addition, if monitoring code is compiled, for sake of clarity, 
 * we also put trasparently from the user the file name and row when
 * the KLtrace call is done inside the tracing message.
 * To do that, we redefine another macro "faking" again function
 * entry points.                                                     */

PUBLIC VOID KLsetTraceLevel(INPUT UWORD16);
PUBLIC UWORD16 KLgetTraceLevel(VOID);
PUBLIC VOID KLhighLevelTrace(STRING *msg, ...);
PUBLIC inline VOID KLlowLevelTrace(INPUT STRING *msg);
PUBLIC VOID KLsetTraceOptions(INPUT UWORD32);

/* These are allowed bitfields for KLsetTraceOptions() call */

#define MONITOR_FILE 0x80000000L   /* Display full filename for eack KLtrace */
#define MONITOR_LINE 0x40000000L   /* Display line number for each KLtrace */

#define KLtrace \
{\
	extern UWORD32 tracedLine;\
	extern STRING tracedFile[40];\
	tracedLine=__LINE__;\
	KLstringCopy(tracedFile,__FILE__);\
}\
	KLhighLevelTrace

/* Defines the system-wide tracing levels */
	
#define SK_FATAL	"0|"   /* The trace refers to an unrecoverable error */
#define SK_WARNING  "1|"   /* The trace refers to a recoverable error */
#define SK_INFO		"2|"   /* The trace is informational and unharmful */

#else
#define KLtrace(str, addargs...)
#define KLsetTraceLevel(int)
#define KLsetTraceOptions(opt)
#endif /* SKTRACE */

PUBLIC VOID KLprint(STRING *, ...);

/* allocation facilities definitions */

typedef struct _KLfreeListEntry {
    struct _KLfreeListEntry *prev;
    struct _KLfreeListEntry *next;
    struct _KLfreeListEntry *prevsize;
    struct _KLfreeListEntry *nextsize;
	UADDR size;
	DATA lookupsize;
} KLfreeListEntry;

typedef struct _KLfreeListDescriptor {
	KLfreeListEntry *FirstFreeEntry;
	KLfreeListEntry *FirstFreeSizeEntry[8];
	UADDR FreeMemory;
	UADDR (*AllocChunk)(UADDR *size);
	BOOLEAN (*PrepareFreeChunk)(UADDR *addr,UADDR *size);
	VOID (*FreeChunk)(UADDR addr,UADDR size);
	BOOLEAN DisallowFreeChunk;
} KLfreeListDescriptor;

VOID *KLfreeListMemAlloc(KLfreeListDescriptor *Desc,UADDR size);
VOID *KLfreeListMemAllocAlign(KLfreeListDescriptor *Desc,UADDR size,DATA alignment);
VOID *KLfreeListMemAllocRegion(KLfreeListDescriptor *Desc,UADDR data,UADDR size);
VOID KLfreeListMemFree(KLfreeListDescriptor *Desc,VOID *data,UADDR size);

typedef struct {
	UWORD32 size;
	UWORD32 firstFree;
	UWORD32 bitmap[0];
} KLbitmapStruct;

PUBLIC VOID KLbitmapCreateLookupTable(VOID);
PUBLIC VOID KLbitmapInitAlloc(VOID *bitmap,DATA size);
PUBLIC DATA KLbitmapAlloc(VOID *bitmap,DATA nobits);
PUBLIC VOID KLbitmapMarkUsed(VOID *bitmap,DATA bit);
PUBLIC VOID KLbitmapFree(VOID *bitmap,DATA bit);

/* Executable Loader Library definitions */

typedef UWORD32 EHandle;	/* The handle for any library operation */

typedef struct {		/* Executable record. Tell caller what have to load */
	UWORD32 position;
	UWORD32 size;
	UBYTE *buffer;
} ExecRecord;

typedef enum {			/* Supported executable types */
	none, elf32
} ExecType;

typedef enum {			/* Return status of KLexecProbe() call */
	EXEC_PROBE_OK,
	EXEC_PROBE_INVID,
	EXEC_PROBE_NOTOK,
	EXEC_PROBE_NOT_SUPPORTED,
	EXEC_PROBE_NONE
} ProbeStatus;

typedef enum {			/* Return status of KLexecProbe() call */
	EXEC_LOAD_AGAIN,
	EXEC_LOAD_INVID,
	EXEC_LOAD_FINISHED,
	EXEC_LOAD_ERROR
} LoadStatus;

#define MAX_SYMBOL_NAME 32 /* Maximum symbol name length */

typedef enum {
	EXEC_NIL=0,
	EXEC_VAR=1,
	EXEC_FUNC
} SymbolType;			/* KLexecSymbol() can retrieve these kind of symbols */

typedef struct {		/* Definition of a symbol in Exec library */
	STRING name[MAX_SYMBOL_NAME+1];
	SymbolType type;
} Esymbol;

#define EXEC_SYMBOL_INVID	 0xFFFF0000L

/* Function prototypes */

PUBLIC BOOLEAN KLexecInit(INPUT UWORD32 numOfExecutables);
PUBLIC VOID KLexecUnInit(VOID);
PUBLIC EHandle KLexecOpen(ExecRecord *p);
PUBLIC ProbeStatus KLexecProbe(INPUT EHandle eId,ExecType *,INPUT ExecRecord *);
PUBLIC LoadStatus KLexecLoad(INPUT EHandle eId, ExecRecord *record);
PUBLIC BOOLEAN KLexecClose(INPUT EHandle eId);
PUBLIC UWORD32 KLexecSymbols(INPUT EHandle eId, INPUT Esymbol symbol[], UBYTE *[]);
PUBLIC UWORD32 KLexecSetSymbols(INPUT EHandle eId, INPUT Esymbol symbol[], INPUT UBYTE *[]);

#define EVALID_HANDLE	0xFFFF0000L		/* Valid handles are < of this value */
#define ENO_EXECTABLE	EVALID_HANDLE+1 /* No executable table allocated */
#define ENO_MOREEXECS	EVALID_HANDLE+2 /* No space in executable table */

/* Memory allocation library routines.
 * These functions are defined as a standard Kernel Library function,
 * but they are slightly different, since each EK may implement whatever
 * memory allocation strategy wants to. These functions are really defined
 * inside EKs rather than KL, even if they logically belongs to KL. 
 * This is a trick that saves us efficiency and various headaches.        */

PUBLIC VOID *KLmalloc(UWORD32 size);
PUBLIC VOID *KLmallocAligned(UWORD32 size, UWORD32 alignment);
PUBLIC VOID KLfree(VOID *p);

#endif /* __KLIBS_H */
