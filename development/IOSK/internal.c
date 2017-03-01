/*
 * IOSK main module
 * 
 * HISTORY
 * Date     Author      Rev    Notes
 * 10/10/98 D. Arbuckle 1      First draft
 * 20/10/98 S. Costa    2      Rewritten with new implementation
 * 18/11/98 R. vHandel  3      Updated to Alliance Coding Style (kind of)
 * 20/11/98 D. Arbuckle 4      Restore various things dropped in rev 2
 *                             Bring closer to coding style
 *                             Bring some things more in line with spec
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <typewrappers.h>
#include <klibs.h>
#include <lm.h>               /* Needed to interact with LMs */
#include <internal.h>         /* Our definitions */
#include <iosk.h>	      /* Our CORBA header */

#ifndef REAL_ALLIANCE
#include <stdio.h>
#define TRACE(a, b) printf(a, b)
#else
#define TRACE(a, b)
#endif

/* This is a fake SystemDevice file, put here only to avoid to write
 * a lot of code to read it from a file, useless in IOSK because 
 * at startup IOSK cannot read a file from a disk that does not know
 * to exist :)! In this way we simulate also the behaviour of 
 * Alliance boot procedure, since someone must probe the HW configuration,
 * and tell us (IOSK) which peripheral want to use. Since we cannot
 * use ordinary files (since IOSK is not initted!) we must pass the
 * database in memory. So it's nice way to start for a prototype.
 * But when an Alliance filesystem is active, a file named 
 * SystemDevices will be present under /alliance/config, with
 * contents syntax as the following one:
 * 
 * # System configuration for my home machine
 * #
 * # Syntax
 * #
 * # <DeviceCategory>:<LMName>(<LM Init data>)=<IOSK attributes>
 * #
 * # <DeviceCategory>= [BLOCKDEVICE|CHARDEVICE]
 * # <LMName>= Uh, well, whatever :) (without prepending LM)
 * # <LM Init data>=Specific to particular LM. Use comma ',' as a separator
 * between
 * # each data field. Blanks are ignored. Case sensitive.
 * # <IOSK attributes>= [Max Msgs],[Max memory],[RO|RW|WO]
 * 
 */

/* No need for a mutex, unless something changes it. Maybe later that'll be
 * possible.
 */
STRING *FakeSystemDevicesFile[]={ 
    "Blockdevice:Cache(IDE(0, 16, 1024), 128K)=512, 64K, \"RW\"",
    "Blockdevice:IDE(1)=RW",
    "Blockdevice:SCSI(3)=RO",
    "Blockdevice:SCSI(4)=WO",
    "Chardevice:Serial(0)=RW",
    "" 
};

typedef struct {
    DevCategory Category;
    STRING LMInit[32];
    STRING IOSKAttributes[32];
} DBQuery;

/* i starts at 0 */
/* i is set to the number of lines in the init data, by KLgetSystemConfig */
/* nothing accesses i, except KLgetSystemConfig. */
/* if i is already at the end, KLgetSystemConfig fails. */
/* I'll lock it, so that KLgetSystemDevices cannot run simultaneously */
/* Once i has been advanced to the end, it acts as an inverted spinlock for
   this function. */
static Mutex iMutex;
static WORD32 i=0;

STRING **Ptr=FakeSystemDevicesFile; /* Temporary kludge */

PUBLIC WORD32 KLgetSystemConfig(INPUT STRING *Query, DBQuery *Result)
    
    /*
     * Support interrogation of SystemDevices minidatabase.
     * 
     * INPUT:
     * Query: What we want to extract from minidatabase. If the string
     *        is empty, anything is extracted from database. Otherwise, only
     *        those entries in minidatabase that matches the input string
     *        will be returned.
     *        
     * OUTPUT:
     * Result: the output of the query      
     * The function returns 
     */
  
{
    BOOLEAN EndQuery=FALSE;
    STRING *Delimiter;
    STRING *Delimiter2;
    STRING Temp[32];
    UWORD32 Length;

    if(*Ptr[i]=='\0') {
        return(0);
    }
    
    KLmutexLock(&iMutex);
    
    while(*Ptr[i]!='\0' && EndQuery==FALSE) {
        if(KLstringMatch(Ptr[i], Query)!=NIL) {
            Delimiter=KLstringMatch(Ptr[i], ":");
            Length=(UWORD32) (Delimiter-Ptr[i]);
            KLstringNumCopy(Temp, Ptr[i], Length);
            Temp[Length]='\0';

            if(KLstringCompare(Temp, "Blockdevice")==0) {
                Result->Category=BLOCKDEVICE;
            } else {
                if(KLstringCompare(Temp, "Chardevice")==0) {
                    Result->Category=CHARDEVICE;
                } else {
                    Result->Category=DEVUNDEFINED;
                }
            }

            Delimiter2=KLstringMatch(Ptr[i], "=");
            Length=((UWORD32) (Delimiter2-Delimiter))-1;
            KLstringNumCopy(Temp, Delimiter+1, Length);
            Temp[Length]='\0';
            KLstringCopy(Result->LMInit, Temp);
            
            KLstringCopy(Result->IOSKAttributes, Delimiter2+1);

            EndQuery=TRUE;
        }
        
        i++;
    }
    
    KLmutexUnlock(&iMutex);
    
    if(*Ptr[i]=='\0') {
        return(0);
    }
    
    return(1);
}

PUBLIC WORD32 KLgetNumericParameter(UWORD32 *Integer, INPUT STRING *ArgString)
  
    /*
     * From an argument string, take first numeric value and convert to
     * an integer type <Output>.
     * 
     * INPUT:
     * ArgString: the argument string to be parsed
     * 
     * OUTPUT:
     * Integer: the integer value of the string (if in the string the suffix
     *         'K' for kilobytes and 'M' for megabytes is used, it is the 
     *         actual immediate value)
     * Output: the number of character scanned prior to find the value
     */
  
{
    WORD32 i;
    UWORD32 IntNumber;
    STRING *Delimiter;
    STRING Number[10];
    
    i=0;

    Delimiter=(STRING *) ArgString;
    if(Delimiter!=NIL) {
        while(*Delimiter==' ')
            Delimiter++;

        while(*Delimiter>='0' && *Delimiter<='9') {
            Number[i++]=*Delimiter++;
        }
        Number[i]='\0';
      
        /* Convert the string integer to an integral integer */
      
        IntNumber=KLasciiToInt(Number);

        /* Look for numeric base suffixes */
        
        switch(*Delimiter) {
            case 'K':
                IntNumber*=1024;
                break;
            case 'M':
                IntNumber*=(1024*1024);
                break;
        }
        
        /* Kill trailing comma, if any */
        
        while(*Delimiter==' ' || *Delimiter==',') {
            Delimiter++;
            i++;
        }
        
        *Integer=IntNumber;
    }
    
    return(i);
}

PUBLIC WORD32 KLgetStringParameter(STRING *String, INPUT STRING *StringArg)
  
    /*
     * From an argument string, get a string value. Strings are defined
     * as all characters between a '"' pair.
     * 
     * INPUT:
     * StringArg: the argument string.
     * 
     * OUTPUT:
     * String: the resulting string, without '"'
     * Output: the number of character scanned prior to find the value
     */
  
{
    WORD32 Offset;
    WORD32 i;
    STRING *Delimiter;
    
    Offset=0;
    if(StringArg!=NIL) {
        Delimiter=(STRING *) StringArg;
        while(*Delimiter++!='"') {
            Offset++;
        }
    
        i=0;    
        while(*Delimiter!='"') {
            String[i]=*Delimiter;
            Delimiter++;
            Offset++;
            i++;
        }
        
        String[i]='\0';
    }
    
    return(Offset);
}

/* ##############  Here is where the real IOSK stuff begins  ############## */

/* increasing the resolution on the mutexen--e.g. one per session instead of
 * one for all sessions--would increase performance.
 */
Mutex deviceTableMutex;
IOSKDeviceTable deviceTable[MAXCHAINS];

Mutex sessionMutex;
IOSKSession session[MAXSESSIONS];

PUBLIC VOID IOSKbuildDeviceTable(INPUT WORD32 chainNumber, INPUT DBQuery *data)
  
    /*
     * Init a device table for IOSK.
     * 
     * INPUT:
     * ChainNumber: index in the IOSK Device table of the chain to be initted
     * Data: the shape of LM chain (i.e., names of LMs and their order)
     * 
     * RETURN:
     * None
     * 
     */

{
    WORD32 i;
    WORD32 offset;
    UWORD32 value;
    STRING permission[4];
    Symbol LMchainInitStrings[4];

    KLmutexLock(&deviceTableMutex); /* Probably not needed, but.... */
    
    /* We firmly divide data structures relative to HW/SW devices 
     * (and their states) from data structures relative to IOSK
     * connections and states, since the two are logically 
     * different. Here we init data structures about devices, 
     * and we do not bother about their use. 
     * That will be done on OpenSession().                      */
    
    /* Step 1. Get the order of LMs in the given chain */
    
    LMparseInitString(data->LMInit, LMchainInitStrings);

    for(i=0;;i++) {
        printf("<%s>", LMchainInitStrings[i].Name);
        if(*LMchainInitStrings[i+1].Name!='\0') {
            printf("<-->");
        } else {
            break;
        }
    }

    printf("\n");

    /* Step 2. Allocate buffers for chain communication queues */
  
    offset=KLgetNumericParameter(&value, data->IOSKAttributes); 
    printf("Max packets=%d\n", value);
    deviceTable[chainNumber].chainQ.MaxPackets=value;
    offset=KLgetNumericParameter(&value, data->IOSKAttributes+offset);  
    printf("Queue size=%d\n", value);
    deviceTable[chainNumber].chainQ.Maxsize=value;
    
    /* Some terminology here.
     * We store data in queue for LMs in units called packets.
     * Each packet have variable length, and contains in addition
     * to the data itself information about:
     * 
     * - The IOSK session that packet belongs to
     */

    if(KLqueueAlloc(&deviceTable[chainNumber].chainQ)==FALSE) {
        printf("Error allocating queues\n");
    }
    
    /* Step 3. Set chain permissions */

    offset=KLgetStringParameter(permission, data->IOSKAttributes+offset);
    if(KLstringCompare(permission, "RW")==0) {
        deviceTable[chainNumber].permission=Alliance_IOSK_RW;
    } else {
        if(KLstringCompare(permission, "RO")==0) {
            deviceTable[chainNumber].permission=Alliance_IOSK_RO;
        } if(KLstringCompare(permission, "WO")==0) {
            deviceTable[chainNumber].permission=Alliance_IOSK_WO;
        } else {
            deviceTable[chainNumber].permission=Alliance_IOSK_UNDEF;
        }
    }
    printf("Permission:%s\n", permission);

    KLmutexUnlock(&deviceTableMutex);
}

PUBLIC VOID IOSKinit(VOID)
  
    /*
     * Called once at IOSK startup.
     * QUESTION: Should be something standardized
     * in all SKs, though, so named differently (SKInit?).
     * 
     * INPUT:
     * None
     * 
     * OUTPUT:
     * None
     * 
     */
  
{
    BOOLEAN finished=FALSE;
    WORD32 queryResult;
    WORD32 i;
    DBQuery data;

    TRACE("%s\n", "IOSKinit called.");

    KLmutexCreate(&deviceTableMutex);
    KLmutexCreate(&sessionMutex);
    KLmutexCreate(&iMutex);

    /* Mark all IOSK session as free */

    for(i=0;i<MAXSESSIONS;i++) {
        session[i].chainNumber=-1;
    }

    /* Query the minidatabase contained in SystemConfig file for
     * data relevant to IOSK. Loop until all data gathered.        */

    i=0;
    while(finished==FALSE) {
        queryResult=KLgetSystemConfig("", &data);
        switch(queryResult) {
	case -1:
	    printf("Error reading database\n");
	    finished=TRUE;
	    break;
	case 0:
	    printf("No more queries.\n");
	    finished=TRUE;
	    break;
	case 1:
	    printf("Queried:");
	    IOSKbuildDeviceTable(i++, &data);
	    break;
        }	
    } 
    
    /* After all of the above is executed, IOSK knows all HW
     * configuration pertinent to his area. Subsequent 
     * requests made by other SKs will use the device table
     * initted in this way. 
     * This does not forbid to load new device in the system
     * at run-time: simply init IOSK in a known default state.   */
}

Alliance_IOSK_Session IOSKopenSession(CORBA_char* deviceName,
                                      Alliance_IOSK_Permission perm,
                                      Alliance_IOSK_Caller caller)

/* QUESTION: how the IOSK users will identify device names?
 * Since I have introduced the GetSystemDevice() call, with
 * the same structure as a file, and the input data is
 * built by the caller, maybe the same module that calls
 * IOSK and give that data can call FSSK. So, device name
 * may be what follows ":" in the data, now IDE(0) for
 * instance.
 */

/* ANSWER: device names are standard-format init strings.
 */

{
    UWORD32 i;
    UWORD32 ChainNumber;
    Alliance_IOSK_Session Handle;

    TRACE("%s\n", "IOSKopenSession called.");
    KLmutexLock(&sessionMutex);
	
    /* Step 1. Validate IOSK user permissions for that device
     * QUESTION: how CORBA identifies remote callers, and so,
     * in which way I can test matching in IOSK internal list
     * of Allowed Users/Denied Users?                         */
  
    /* Step 2. Get a valid session handle for this session
     * I prefer an KL-wide routine for that.               */
	
    for(i=0;i<MAXSESSIONS;i++) {
	if(session[i].chainNumber<0) { /* available session are marked -1 */
	    Handle=i;
	    break;
	}
    }
	
    if(i==MAXSESSIONS) {
	KLmutexUnlock(&sessionMutex);
	return(NOMOREHANDLES);
    }
  
    /* Step 3. Load LM relative to HW device requested if needed.
     * 
     * Since we are (possibly) dealing with device drivers, it is
     * best that HW-dependant LMs are to be loaded first in a chain,
     * since it is wasteful to discover that requested chain cannot
     * operate at the end of the process due to some low-level
     * problem. so, we scan the LM chain from the tail, looking
     * for unloaded LMs if needed.
     */

    /* QUESTION: in which way IOSK caller will identify the device
     * For me it's best to use the same device name found in 
     * SystemDevices, so IDE(0) for instance.
     */
	
    ChainNumber=0; /* Must be retrieved from SystemDevices */
    session[Handle].chainNumber=ChainNumber;

    KLmutexLock(&deviceTableMutex);

    i=deviceTable[ChainNumber].last;
    for(;i>=0;i--) {
	if(deviceTable[ChainNumber].element[i].stateLM==LMUNLOADED) {
	    /* LMLoad(); */
	}
    }

    KLmutexUnlock(&deviceTableMutex);
    KLmutexUnlock(&sessionMutex);
    
    return(Handle);
}

BOOLEAN IOSKcloseSession(Alliance_IOSK_Session session,
                         Alliance_IOSK_Caller caller)

{
    TRACE("%s\n", "IOSKcloseSession called.");

    KLmutexLock(&sessionMutex);
    
    /* Step 1. Validate IOSK user permissions for that device */
  
    /* Step 2. Flush data current buffered in the queue, if any */
  
    /* Step 3. Nuke all the data relative to the session */

    KLmutexUnlock(&sessionMutex);
    
    return(TRUE);
}

PUBLIC UWORD32 IOSKread(Alliance_IOSK_Session handle,
			Alliance_IOSK_bytes *data,
			UWORD32* length,
			BOOLEAN block,
			Alliance_IOSK_Caller caller)

{
    TRACE("%s\n", "IOSKread called.");
    
    /* Step 1. Validate IOSK user permissions for that device */
  
    /* Step 2. Put packet in the queue with read request */
  
    /* Step 3. Wait for data */
  
    return(0x1);
}


#define QUEUEOVERFLOW 0xFFFF0001L

PUBLIC UWORD32 IOSKwrite(Alliance_IOSK_Session handle,
			 Alliance_IOSK_bytes *data,
			 UWORD32 size,
			 Alliance_IOSK_Caller caller)
  
{
    TRACE("%s\n", "IOSKwrite called.");
    
    /* Step 1. Validate IOSK user permissions for that device */
  
    /* Step 2. Put packet in the Queue that holds data for correct chain */
  
    /* Step 3. Wait ack of write */

    return(0x1);
}

PUBLIC BOOLEAN IOSKreadSeek(Alliance_IOSK_Session handle,
			    Alliance_IOSK_Address where,
			    Alliance_IOSK_SeekDir dir,
			    Alliance_IOSK_Caller caller)
{
    TRACE("%s\n", "IOSKreadSeek called.");
    
    /* move the 'read head' on the chain to a new location, if that is
     * possible with the specified chain. If not, return false.
     */
    
    return TRUE;
}

PUBLIC BOOLEAN IOSKwriteSeek(Alliance_IOSK_Session handle,
			     Alliance_IOSK_Address where,
			     Alliance_IOSK_SeekDir dir,
			     Alliance_IOSK_Caller caller)
{
    TRACE("%s\n", "IOSKwriteSeek called.");
    
    /* move the 'write head' on the chain to a new location, if that is
     * possible with the specified chain. If not, return false.
     */
    
    return TRUE;
}

BOOLEAN IOSKlock(Alliance_IOSK_Session session,
		 Alliance_IOSK_Permission perm,
		 Alliance_IOSK_Caller caller)

{
    TRACE("%s\n", "IOSKlock called.");
    
    KLmutexLock(&sessionMutex);
    
    /* Step 1. Validate IOSK user permissions for that device */
  
    /* Step 2 set state of the session accordingly */
  
    KLmutexUnlock(&sessionMutex);
    
    return(TRUE);
}

/* This stuff is replaced by a CORBA test client

STRING *buffer= "OK, that's right\n";
STRING bufferRead[32];

VOID main()
  
{
    Alliance_IOSK_Session handle;
	UWORD32 rc;

    IOSKInit();
    Handle=IOSKsessionOpen("IDE(0)", 0, 0);

    rc=IOSKwrite(handle, buffer, KLstringLength(buffer));
    rc=IOSKread(handle, bufferRead, 32);
}

*/
