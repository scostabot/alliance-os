/*
 * demo.h:  The demo tasks' stacks etc.
 *
 * (C) 1998 Jens Albretsen, Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 07/11/98  ramon/jens  1.0    First full internal release
 * 16/11/98  ramon       1.1    Updated to use generic ckobjects.h
 * 31/01/99  haggai      1.2    Keyboard mapping
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

#ifndef __DEMO_H
#define __DEMO_H

#include <typewrappers.h>
#include "ckobjects.h"

PUBLIC DATA print(CONST STRING *formatString, ...);
PUBLIC VOID setTextAttr(DATA attr);

extern UADDR _start[];  /* start of kernel */
extern UADDR _end[];    /* end of kernel   */

extern ThreadObject TSTASK1;
extern BYTE  TSTACK1[];
extern BYTE  TKSTACK1[];
extern VOID tstask1(VOID);

extern ThreadObject TSTASK2;
extern BYTE  TSTACK2[];
extern BYTE  TKSTACK2[];
extern VOID tstask2(VOID);

extern ThreadObject TSTASK3;
extern BYTE  TSTACK3[];
extern BYTE  TKSTACK3[];
extern VOID tstask3(VOID);

extern ThreadObject TSTASK4;
extern BYTE  TSTACK4[];
extern BYTE  TKSTACK4[];
extern VOID tstask4(VOID);

extern ThreadObject RTTASK1;
extern BYTE  RSTACK1[];
extern BYTE  RKSTACK1[];
extern VOID rttask1(VOID);

extern ThreadObject RTTASK2;
extern BYTE  RSTACK2[];
extern BYTE  RKSTACK2[];
extern VOID rttask2(VOID);

extern ThreadObject RTTASK3;
extern BYTE  RSTACK3[];
extern BYTE  RKSTACK3[];
extern VOID rttask3(VOID);

extern ThreadObject FPTASK1;
extern BYTE  FSTACK1[];
extern BYTE  FKSTACK1[];
extern VOID fptask1(VOID);

extern ThreadObject FPTASK2;
extern BYTE  FSTACK2[];
extern BYTE  FKSTACK2[];
extern VOID fptask2(VOID);

extern ThreadObject DSTASK;
extern BYTE DSSTACK[];
extern BYTE DSKSTACK[];
extern VOID demoshell(void);

extern VOID AKsigHandlStub(VOID);


/*
 * This is the keyboard mapping for the DemoKernel keyboard driver
 *
 * Usage: scan2ascii[0][] - shift off, NUMLOCK on
 *        scan2ascii[1][] - SHIFT on,  numlock off
 */

#define SPECIALMASK  0x10000

#define CTRL         0x10000
#define LSHFT        0x10001
#define RSHFT        0x10002
#define ALT          0x10003
#define CAPS         0x10004

#define F1           0x10101
#define F2           0x10102
#define F3           0x10103
#define F4           0x10104
#define F5           0x10105
#define F6           0x10106
#define F7           0x10107
#define F8           0x10108
#define F9           0x10109
#define F10          0x10110
#define F11          0x10111
#define F12          0x10112

#define PAUSE        0x10005
#define SCRL         0x10006
#define HOME         0x10007
#define UP           0x10008
#define PGUP         0x10009
#define LEFT         0x10010
#define FIVE         0x10011
#define RIGHT        0x10012
#define END          0x10013
#define DOWN         0x10014
#define PGDN         0x10015
#define INS          0x10016
#define DEL          0x10017


#ifdef __DEMOKERNEL_C

UWORD32 scan2ascii[2][256] = {
    {   0,    27,   '1',  '2',   '3',  '4',  '5',   '6',  '7',   '8',  /*00*/
      '9',   '0',   '-',  '=',     8,    9,  'q',   'w',  'e',   'r',  /*10*/
      't',   'y',   'u',  'i',   'o',  'p',  '[',   ']',   13,  CTRL,  /*20*/
      'a',   's',   'd',  'f',   'g',  'h',  'j',   'k',  'l',   ';',  /*30*/
      '\'',  '`', LSHFT, '\\',   'z',  'x',  'c',   'v',  'b',   'n',  /*40*/
      'm',   ',',   '.',  '/', RSHFT,  '*',  ALT,   ' ', CAPS,    F1,  /*50*/
      F2,     F3,    F4,   F5,    F6,   F7,   F8,    F9,  F10, PAUSE,  /*60*/
      SCRL, HOME,    UP, PGUP,   '-', LEFT, FIVE, RIGHT,  '+',   END,  /*70*/
      DOWN, PGDN,   INS,  DEL,     0,    0,    0,   F11,  F12,         /*80*/
    },

    {    0,   27,   '!',  '@',   '#',  '$',  '%',   '^',  '&',   '*',  /*00*/
       '(',  ')',   '_',  '+',     8,    9,  'Q',   'W',  'E',   'R',  /*10*/
       'T',  'Y',   'U',  'I',   'O',  'P',  '{',   '}',   13,  CTRL,  /*20*/
       'A',  'S',   'D',  'F',   'G',  'H',  'J',   'K',  'L',   ':',  /*30*/
      '\"',  '~', LSHFT,  '|',   'Z',  'X',  'C',   'V',  'B',   'N',  /*40*/
       'M',  '<',   '>',  '?', RSHFT,  '*',  ALT,   ' ', CAPS,    F1,  /*50*/
        F2,   F3,    F4,   F5,    F6,   F7,   F8,    F9,  F10, PAUSE,  /*60*/
      SCRL,  '7',   '8',  '9',   '-',  '4',  '5',   '6',  '+',   '1',  /*70*/
       '2',  '3',   '0',  '.',     0,    0,    0,   F11,  F12,         /*80*/
    }
};

#endif /* __DEMOKERNEL_C */

#endif /* __DEMO_H */
