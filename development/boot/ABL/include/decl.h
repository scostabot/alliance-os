/*
 * decl.h:  ABL declarations
 *
 * (C) 1999 Rudy Gingles, Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 01/12/99  rudy        1.0    First release
 * 04/02/99  ramon       1.1    Added misc declarations
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

#ifndef __DECL_H
#define __DECL_H

#include <typewrappers.h>
#include "sys/sysdefs.h"

PUBLIC VOID ABLsetVector(VOID *handler, UBYTE interrupt, UWORD16 control_major);
PUBLIC VOID ABLdisableIRQ(UDATA irq);
PUBLIC VOID ABLenableIRQ(UDATA irq);
PUBLIC VOID ABLinitIntr(VOID);

PUBLIC DATA print(CONST STRING *formatString, ...);
PUBLIC VOID ABLclrScr(VOID);
PUBLIC VOID ABLsetTextAttr(DATA attr);
PUBLIC VOID ABLinitCons(VOID);
PUBLIC VOID ABLinitMem(VOID);
PUBLIC VOID ABLsetupMem(VOID);
PUBLIC DATA ABLinitMP(UADDR mem_lower);

PUBLIC ADDR ABLloadELF(STRING *file, ADDR memoffs, ADDR *entry);
PUBLIC DATA ABLdoScript(STRING *script, ADDR size);

extern UADDR lowMemTop, extMemTop, totalMem;
extern bootinfo *binf;

/*********************************************************************/

/* Delay stuff */

#define MILLISEC 10           /* Take a 10ms fixed timer rate   */
#define FREQ (1000/MILLISEC)  /* Timer freqency for MILLISEC    */

extern UDATA delayCount;

PUBLIC UDATA ABLcalDelayLoop(VOID);
PUBLIC VOID __delay(UDATA loops);

#define udelay(x) __delay((x/(MILLISEC*1000))*delayCount)
#define mdelay(x) __delay((x/MILLISEC)*delayCount)

/*********************************************************************/

/* From the shared boot library */

extern struct multiboot_info *mbinf;
extern int errnum;

void __print_error(const char *fl, const long ln);
#define print_error() __print_error(__FILE__,__LINE__) 

/* these are the current file position and maximum file position */
extern int filepos;
extern int filemax;

/* These are the currently active and the previously mounted partitions */
extern unsigned long current_drive;
extern unsigned long current_partition;
extern unsigned long saved_drive;
extern unsigned long saved_partition;

int rawread(int drive, int sector, int byte_offset, int byte_len, int addr);
int devread(int sector, int byte_offset, int byte_len, int addr);
int open(char *filename);
int read(int addr, int len);  /* If "length" is -1, read rest of file     */
int dir(char *dirname);       /* List directory, printing all completions */
char *set_device(char *device);  /* get standard device from string       */
int open_device(void);           /* mount standard device                 */

#endif
