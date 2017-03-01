/*
 * Loadable Module for parallel port handling
 *
 * This LM handle the interface to parallel port chip on a IBM pc.
 *
 * HISTORY
 * Date     Author    Rev    Notes
 * 24/03/99 scosta    1.0    First draft
 * 27/04/99 scosta    1.1    Small fixes in LMprobe
 * 04/05/99 scosta    1.2    More compliant to LM generic spec 5
 * 05/10/00 scosta    1.3    Fixed bug in definitions of bitmasks
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#include <klibs.h> 	/* This LM belong to an SK environment */
#include <lm.h>			/* Generic LM definitions */
#include <iosklm.h>		/* Definitions specific to IOSK LMs */
/*#include <ioport.h> 	 For use outport[b,w] and inport[b,w] */

#include <sys/perm.h>   /* Temporary, to be removed */

/* The algorithm used to detect parallel ports, specifications and
 * useful informations in general about parallel ports handling
 * is derived from "Interfacing the IBM PC Parallel Printer Port"
 * document rel. 0.96, by Zhahai Stewart, <zstewart@hisys.com>.    */

/*
 *  LM local defines and typedefs.
 */

#define INIT	 0x04 /* \                             */
#define SELECT   0x08 /* | Control out register bits   */
#define STROBE   0x01 /* /                             */

#define BUSY	 0x80 /* \                          */
#define PAPEROUT 0x20 /* | Status register BUSY bit */
#define ERROR    0x08 /* /                          */   

enum status { absent, present };

typedef struct {
    UWORD16 number;      /* I/O port number */
    enum status state;   /* Flag: <present|absent> parellel port at
                            the above number                        */
} PortDef;

/*
 *  LM Global Data.
 */

STRING lmName[]="SPP Standard Parallel Port driver";

/* LM description structure */
PUBLIC LMDescription lmDescription=
{
	lmName,       /* Descriptive name for this LM */
	0,
	LMIOSK1,
	{ 1, 1, 1 },
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	TRUE,
	(BYTE *) NIL 
};

/* Allowed I/O port numbers for parallel ports.
 * The order of elements in the table determine the actual
 * checking order.
 * At LM startup we don't assume a default configuration, so
 * all ports are flagged absent.                               */

LOCAL PortDef allowedPrinterPort[] = { 
	{ 0x3BC, absent },
	{ 0x378, absent },
	{ 0x278, absent }
};

/* Here are stored the port addresses of really present ports */
	
LOCAL UWORD16 existingPrinterPort[2];	
LOCAL UWORD16 totalNumOfPorts; /* Total number of probed parallel ports */
LOCAL STRING probeString[66];  /* The LM probe string */
LOCAL UBYTE deviceStatus;      /* Local copy of Status Register */

/* 
 * Standard LM entry points.
 */
  
PUBLIC BOOLEAN LMinit(VOID)

{
    /* Nothing particular must be done for this LM driver */
  
    return(TRUE);
}

PUBLIC STRING *LMprobe(VOID)
  
/*
 * Probe parallel port presence and address
 * 
 * INPUT:
 * None
 * 
 * RETURNS:
 * the parallel port configuration probed as a string.
 * See "LM for IOSK specification document" for details.
 */

{
    WORD16 i;
    UWORD16 testCode;
    UWORD16 result;
    STRING stringIndex[6];

		/* Yes, I'm lazy: I should use CKresourceAlloc. I'll do
		 * that next release.                                   */

    if(ioperm(0x200, 0x1F0, 0xFF)<0) {
        return((STRING *) 0);
    }  

    /* We don't analize BIOS area for printer ports (0x0:0x408
     * up to 0x0:0x410) but we do totally on our own, writing and
     * subsequently reading standard I/O port numbers allocatable
     * to parallel ports.
     * To see if a parallel port chip is connected to one of these
     * I/O ports we simply check if what we have written is equal
     * to what we read from the same port.
     * If so, we update the counter of total available ports and
     * in any case we go to the next I/O port number in the list
     * of allowed ones.                                           */

    testCode=0xAA;
    totalNumOfPorts=0;

    for(i=0;i<sizeof(allowedPrinterPort)/sizeof(PortDef);i++) {
        outb(allowedPrinterPort[i].number, testCode);
        result=inpb(allowedPrinterPort[i].number);
        if(result==testCode) {
            /* We have found an NS compatible chip, mark it
             * found for later use.                         */

            allowedPrinterPort[i].state=present;
	        existingPrinterPort[totalNumOfPorts]=allowedPrinterPort[i].number;
            totalNumOfPorts++;
        } else {
            /* Nothing found. Mark it non-existant */

            allowedPrinterPort[i].state=absent;
        }
    }
    
    /* After we have collected hardware configuration, build
       the probe string with harvested data.                  */

    for(i=0;i<totalNumOfPorts;i++) {
        KLstringAppend(probeString, "LMparallel(");
        KLstringFormat(stringIndex, "%d)=WO", i);
        KLstringAppend(probeString, stringIndex);
        KLstringAppend(probeString, ";");
    }

    return(probeString);
}  

BOOLEAN LMwrite(UBYTE *buffer, UWORD32 size, UWORD32 channel)

/*
 * Write the contents of buffer for size bytes in port channel.
 * 
 * INPUT:
 * buffer:  the pointer to a buffer containing data to write
 * size:    for how many bytes to read from buffer
 * channel: the parallel port pointed by the probe string definition
 * 
 * RETURNS:
 * TRUE if all bytes are written in the device, FALSE in all other cases.
 */
  
{
    UWORD16 printerPort;
    UWORD32 i;
	UBYTE *ptr=buffer;
	
	/* Reject trivial case */
	
	if(size==0) {
	    return(FALSE);
	}
	
	/* Output data to parallel port at the specified port */
	
	printerPort=existingPrinterPort[channel];     /* This is the base addr */
    for(i=0;i<size;i++) {                         /* Send 1 byte at a time */
	    outb(printerPort, *ptr++);            /* Output data */
	    while(!(inpb(printerPort+1) & BUSY));  /* Wait until device ready */
		outb(printerPort+2, INIT|SELECT|STROBE);
		outb(printerPort+2, INIT|SELECT);     /* Pulse Strobe on the line */
		(VOID) inportb(printerPort+1);			  /* Force chip state change */
		deviceStatus=inpb(printerPort+1);      /* What's happened? */
		if(deviceStatus & (PAPEROUT|ERROR)) {     /* An error is occurred? */
		    return(FALSE);                        /* Yes, exit aborting */
		}
	}
	
	return(TRUE);
}

/*
 * TODO:
 * take away ioperm()
 * add LMerror
 */
