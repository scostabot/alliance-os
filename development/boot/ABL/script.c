/*
 * script.c:  Interpreter for the ABL configuration script
 *
 * (C) 1999 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author  Rev   Notes
 * 20/07/99  ramon   1.0   Initial draft (doesn't compile yet)
 * 23/07/99  ramon   1.1   Now it works :)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Alliance header files */
#include <typewrappers.h>
#include "sys/sysdefs.h"
#include "decl.h"


/* Declarations */
LOCAL DATA ABLloadCK(STRING *fname);
LOCAL VOID ABLloadKernel(STRING *fname);
LOCAL VOID ABLloadLM(STRING *fname);
LOCAL DATA ABLlink(VOID);


/* External variables */
extern UADDR CK[];        /* CK entry point address                  */


/* Global variables */
LOCAL DATA line = 1;      /* The line in the script we're processing */


PUBLIC DATA ABLdoScript(STRING *script, ADDR size)
/*
 * Parse the ABL configuration script by loading all the
 * appropiate ELF binaries into memory, dynlink initial LMs,
 * and generate a boot configuration structure to pass on to
 * the CK.
 * This is the parser proper;  the actual loading etc. is
 * done by the functions below.
 *
 * INPUT:
 *     script:  Location in memory of the configuration script to parse
 *     size:    Size of the configuration script
 *
 * RETURNS:
 *     Location of boot configuration structure in extern binf;
 */
{
    STRING *loc      = script;
    STRING *fname    = script;
    DATA   ckloaded  = 0;

    /* First:  some handy definitions */
    #define searchend(x)                                 \
        loc--;                                           \
        while(++loc < (script+size) && (x));             \
        if(loc >= (script+size)) break
    #define searchfor(x)                                 \
        while(loc < (script+size) && (loc++)[0] != x);   \
        if(loc >= (script+size)) break
    #define stripwhitespace()                            \
        searchend((loc[0] == '\n' && line++) || loc[0] == ' ' || loc[0] == '\t')
    #define skipname()                                   \
        searchend(loc[0] != '\n' && loc[0] != ' ' && loc[0] != '\t')


    /* Start the configuration structure with no modules loaded */
    binf->modules = NIL;

    /* The main parsing loop */
    while(loc < (script+size)) {
        /* Skip leading whitespace */
        stripwhitespace();

        /* Check whether rest of line is a comment, and if so, skip it */
        if(loc[0] == '#') {
            searchfor('\n');
            line++;
            continue;
        }

        if(loc[0] == 'c') {           /* CK ELF binary location         */
            if(ckloaded) {            /* We can only have one CK !!!    */
                print("ABL:  Cannot load multiple CKs on line %d, skipping line\n", line);
                searchfor('\n');
                line++;
                continue;
            }
            print("script:  Loading CK\n");
            searchfor('=');           /* Search for start of parameters */
            stripwhitespace();        /* Strip the unused whitespace    */
            fname = loc;              /* Record start of filename       */
            skipname();               /* Find end of filename           */
            (loc++)[0] = '\0';        /* Mark end of filename string    */
            ckloaded =                /* Mark that we've loaded the CK  */
                ABLloadCK(fname);     /* Actually load the CK           */
                                      /**********************************/
        } else if(loc[0] == 'k') {    /* EK ELF binary location         */
            if(!ckloaded) {           /* Need to load CK first (for MM) */
                print("ABL:  Can't load kernel before CK on line %d, skipping line\n", line);
                searchfor('\n');
                line++;
                continue;
            }
            print("script:  Loading kernel\n");
            searchfor('=');           /* Search for start of parameters */
            stripwhitespace();        /* Strip the unused whitespace    */
            fname = loc;              /* Record start of filename       */
            skipname();               /* Find end of filename           */
            (loc++)[0] = '\0';        /* Mark end of filename string    */
            ABLloadKernel(fname);     /* Actually load the kernel       */
                                      /**********************************/
        } else if(loc[0] == 'l') {    /* ELF link library (LM) location */
            print("script:  Loading library\n");
            searchfor('=');           /* Search for start of parameters */
            stripwhitespace();        /* Strip the unused whitespace    */
            fname = loc;              /* Record start of filename       */
            skipname();               /* Find end of filename           */
            (loc++)[0] = '\0';        /* Mark end of filename string    */
            ABLloadLM(fname);         /* Actually load the dynlib/LM    */
                                      /**********************************/
        } else if(loc[0] == 'r') {    /* Root filesystem specification  */
            searchfor('=');           /* Search for start of parameters */
            stripwhitespace();        /* Strip the unused whitespace    */
            fname = loc;              /* Record start of filename       */
            skipname();               /* Find end of filename           */
            (loc++)[0] = '\0';        /* Mark end of filename string    */
            set_device(fname);        /* Set the root device name       */
            if(!errnum)               /* If nothing went wrong with it, */
                open_device();        /* try to mount the partition     */
            if(errnum)                /* Did it work ?                  */
                print("ABL:  Unable to mount root partition %s\n", fname);
            saved_partition = current_partition;  /* Mark mounted part. */
            saved_drive     = current_drive;      /* the default one    */
                                      /**********************************/
        } else if(loc[0] == '\0') {   /* End of file                    */
            break;                    /* We're done with the script !   */
                                      /**********************************/
        } else {                      /* Unrecognised command           */
            print("ABL:  Unrecognised command in script on line %d, skipping line\n", line);
            searchfor('\n');
            line++;
            continue;
        }
    }

    /* Now look for errors */
    if(!ckloaded) {
        print("ABL:  CK was not loaded.  You need a CK to boot !!!\n");
        return 0;
    }

    /* Do dynamic linking */
    if(ABLlink()) {
        print("ABL:  Linking failed:  unresolved references\n");
        return 0;
    }

    return 1;   /* Sucess */
}


LOCAL DATA ABLloadCK(STRING *fname)
{
    UADDR tempMemTop = extMemTop;

    if(tempMemTop > 0x150000) {
        print("ABL:  CK memory already allocated on line %d\n", line);
        return 0;
    }

    tempMemTop = ABLloadELF(fname, 0x150000, CK);
    if(!CK[0]) {
        print("ABL:  Failed loading CK on line %d\n", line);
        return 0;
    }

    extMemTop = tempMemTop;

    return 1;  /* Success */
}


LOCAL VOID ABLloadKernel(STRING *fname)
{
    UADDR tempMemTop = extMemTop;
    bootmodule *bmod = (bootmodule *) lowMemTop;

    /* First align the extended memory top to page boundary */
    if(tempMemTop & 0xfff)
        tempMemTop = (tempMemTop & ~0xfff) + 0x1000;

    bmod->physstart = tempMemTop;

    tempMemTop = ABLloadELF(fname, tempMemTop, &bmod->entry);
    if(!bmod->entry) {
        print("ABL:  Failed loading kernel on line %d\n", line);
        return;
    }

    bmod->size = tempMemTop - bmod->physstart;
    bmod->next = binf->modules;
    binf->modules = bmod;

    lowMemTop += sizeof(bootmodule);
    extMemTop  = tempMemTop;

    return;
}


LOCAL VOID ABLloadLM(STRING *fname)
{
    return;
}


LOCAL DATA ABLlink(VOID)
{
    return 0;
}
