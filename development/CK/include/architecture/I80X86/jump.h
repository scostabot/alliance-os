/*
 * jump.h:  External jump stub declarations and loading routine
 *
 * NOTE:  This header file was autogenerated using the generatejumps script
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 26/12/98  ramon       1.0    First release
 * 07/05/99  ramon       1.1    Changed IRQ-range stubs
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

#ifndef __SYS_JUMP_H
#define __SYS_JUMP_H

/* Header autogenerated on Fri May  7 22:47:06 1999 */

extern VOID __forward_local_0x20(VOID);  /* Jump stub for interrupt 0x20 */
extern VOID __forward_local_0x21(VOID);  /* Jump stub for interrupt 0x21 */
extern VOID __forward_local_0x22(VOID);  /* Jump stub for interrupt 0x22 */
extern VOID __forward_local_0x23(VOID);  /* Jump stub for interrupt 0x23 */
extern VOID __forward_local_0x24(VOID);  /* Jump stub for interrupt 0x24 */
extern VOID __forward_local_0x25(VOID);  /* Jump stub for interrupt 0x25 */
extern VOID __forward_local_0x26(VOID);  /* Jump stub for interrupt 0x26 */
extern VOID __forward_local_0x27(VOID);  /* Jump stub for interrupt 0x27 */
extern VOID __forward_local_0x28(VOID);  /* Jump stub for interrupt 0x28 */
extern VOID __forward_local_0x29(VOID);  /* Jump stub for interrupt 0x29 */
extern VOID __forward_local_0x2a(VOID);  /* Jump stub for interrupt 0x2a */
extern VOID __forward_local_0x2b(VOID);  /* Jump stub for interrupt 0x2b */
extern VOID __forward_local_0x2c(VOID);  /* Jump stub for interrupt 0x2c */
extern VOID __forward_local_0x2d(VOID);  /* Jump stub for interrupt 0x2d */
extern VOID __forward_local_0x2e(VOID);  /* Jump stub for interrupt 0x2e */
extern VOID __forward_local_0x2f(VOID);  /* Jump stub for interrupt 0x2f */
extern VOID __forward_local_0x30(VOID);  /* Jump stub for interrupt 0x30 */
extern VOID __forward_local_0x31(VOID);  /* Jump stub for interrupt 0x31 */
extern VOID __forward_local_0x32(VOID);  /* Jump stub for interrupt 0x32 */
extern VOID __forward_local_0x33(VOID);  /* Jump stub for interrupt 0x33 */
extern VOID __forward_local_0x34(VOID);  /* Jump stub for interrupt 0x34 */
extern VOID __forward_local_0x35(VOID);  /* Jump stub for interrupt 0x35 */
extern VOID __forward_local_0x36(VOID);  /* Jump stub for interrupt 0x36 */
extern VOID __forward_local_0x37(VOID);  /* Jump stub for interrupt 0x37 */
extern VOID __forward_local_0x38(VOID);  /* Jump stub for interrupt 0x38 */
extern VOID __forward_local_0x39(VOID);  /* Jump stub for interrupt 0x39 */
extern VOID __forward_local_0x3a(VOID);  /* Jump stub for interrupt 0x3a */
extern VOID __forward_local_0x3b(VOID);  /* Jump stub for interrupt 0x3b */
extern VOID __forward_local_0x3c(VOID);  /* Jump stub for interrupt 0x3c */
extern VOID __forward_local_0x3d(VOID);  /* Jump stub for interrupt 0x3d */
extern VOID __forward_local_0x3e(VOID);  /* Jump stub for interrupt 0x3e */
extern VOID __forward_local_0x3f(VOID);  /* Jump stub for interrupt 0x3f */
extern VOID __forward_local_0x40(VOID);  /* Jump stub for interrupt 0x40 */
extern VOID __forward_local_0x41(VOID);  /* Jump stub for interrupt 0x41 */
extern VOID __forward_local_0x42(VOID);  /* Jump stub for interrupt 0x42 */
extern VOID __forward_local_0x43(VOID);  /* Jump stub for interrupt 0x43 */
extern VOID __forward_local_0x44(VOID);  /* Jump stub for interrupt 0x44 */
extern VOID __forward_local_0x45(VOID);  /* Jump stub for interrupt 0x45 */
extern VOID __forward_local_0x46(VOID);  /* Jump stub for interrupt 0x46 */
extern VOID __forward_local_0x47(VOID);  /* Jump stub for interrupt 0x47 */
extern VOID __forward_local_0x48(VOID);  /* Jump stub for interrupt 0x48 */
extern VOID __forward_local_0x49(VOID);  /* Jump stub for interrupt 0x49 */
extern VOID __forward_local_0x4a(VOID);  /* Jump stub for interrupt 0x4a */
extern VOID __forward_local_0x4b(VOID);  /* Jump stub for interrupt 0x4b */
extern VOID __forward_local_0x4c(VOID);  /* Jump stub for interrupt 0x4c */
extern VOID __forward_local_0x4d(VOID);  /* Jump stub for interrupt 0x4d */
extern VOID __forward_local_0x4e(VOID);  /* Jump stub for interrupt 0x4e */
extern VOID __forward_local_0x4f(VOID);  /* Jump stub for interrupt 0x4f */
extern VOID __forward_local_0x50(VOID);  /* Jump stub for interrupt 0x50 */
extern VOID __forward_local_0x51(VOID);  /* Jump stub for interrupt 0x51 */
extern VOID __forward_local_0x52(VOID);  /* Jump stub for interrupt 0x52 */
extern VOID __forward_local_0x53(VOID);  /* Jump stub for interrupt 0x53 */
extern VOID __forward_local_0x54(VOID);  /* Jump stub for interrupt 0x54 */
extern VOID __forward_local_0x55(VOID);  /* Jump stub for interrupt 0x55 */
extern VOID __forward_local_0x56(VOID);  /* Jump stub for interrupt 0x56 */
extern VOID __forward_local_0x57(VOID);  /* Jump stub for interrupt 0x57 */
extern VOID __forward_local_0x58(VOID);  /* Jump stub for interrupt 0x58 */
extern VOID __forward_local_0x59(VOID);  /* Jump stub for interrupt 0x59 */
extern VOID __forward_local_0x5a(VOID);  /* Jump stub for interrupt 0x5a */
extern VOID __forward_local_0x5b(VOID);  /* Jump stub for interrupt 0x5b */
extern VOID __forward_local_0x5c(VOID);  /* Jump stub for interrupt 0x5c */
extern VOID __forward_local_0x5d(VOID);  /* Jump stub for interrupt 0x5d */
extern VOID __forward_local_0x5e(VOID);  /* Jump stub for interrupt 0x5e */
extern VOID __forward_local_0x5f(VOID);  /* Jump stub for interrupt 0x5f */
extern VOID __forward_local_0x60(VOID);  /* Jump stub for interrupt 0x60 */
extern VOID __forward_local_0x61(VOID);  /* Jump stub for interrupt 0x61 */
extern VOID __forward_local_0x62(VOID);  /* Jump stub for interrupt 0x62 */
extern VOID __forward_local_0x63(VOID);  /* Jump stub for interrupt 0x63 */
extern VOID __forward_local_0x64(VOID);  /* Jump stub for interrupt 0x64 */
extern VOID __forward_local_0x65(VOID);  /* Jump stub for interrupt 0x65 */
extern VOID __forward_local_0x66(VOID);  /* Jump stub for interrupt 0x66 */
extern VOID __forward_local_0x67(VOID);  /* Jump stub for interrupt 0x67 */

extern VOID __forward_local_0x78(VOID);  /* Jump stub for interrupt 0x78 */
extern VOID __forward_local_0x79(VOID);  /* Jump stub for interrupt 0x79 */
extern VOID __forward_local_0x7a(VOID);  /* Jump stub for interrupt 0x7a */
extern VOID __forward_local_0x7b(VOID);  /* Jump stub for interrupt 0x7b */
extern VOID __forward_local_0x7c(VOID);  /* Jump stub for interrupt 0x7c */
extern VOID __forward_local_0x7d(VOID);  /* Jump stub for interrupt 0x7d */
extern VOID __forward_local_0x7e(VOID);  /* Jump stub for interrupt 0x7e */
extern VOID __forward_local_0x7f(VOID);  /* Jump stub for interrupt 0x7f */
extern VOID __forward_local_0x80(VOID);  /* Jump stub for interrupt 0x80 */
extern VOID __forward_local_0x81(VOID);  /* Jump stub for interrupt 0x81 */
extern VOID __forward_local_0x82(VOID);  /* Jump stub for interrupt 0x82 */
extern VOID __forward_local_0x83(VOID);  /* Jump stub for interrupt 0x83 */
extern VOID __forward_local_0x84(VOID);  /* Jump stub for interrupt 0x84 */
extern VOID __forward_local_0x85(VOID);  /* Jump stub for interrupt 0x85 */
extern VOID __forward_local_0x86(VOID);  /* Jump stub for interrupt 0x86 */
extern VOID __forward_local_0x87(VOID);  /* Jump stub for interrupt 0x87 */
extern VOID __forward_local_0x88(VOID);  /* Jump stub for interrupt 0x88 */
extern VOID __forward_local_0x89(VOID);  /* Jump stub for interrupt 0x89 */
extern VOID __forward_local_0x8a(VOID);  /* Jump stub for interrupt 0x8a */
extern VOID __forward_local_0x8b(VOID);  /* Jump stub for interrupt 0x8b */
extern VOID __forward_local_0x8c(VOID);  /* Jump stub for interrupt 0x8c */
extern VOID __forward_local_0x8d(VOID);  /* Jump stub for interrupt 0x8d */
extern VOID __forward_local_0x8e(VOID);  /* Jump stub for interrupt 0x8e */
extern VOID __forward_local_0x8f(VOID);  /* Jump stub for interrupt 0x8f */
extern VOID __forward_local_0x90(VOID);  /* Jump stub for interrupt 0x90 */
extern VOID __forward_local_0x91(VOID);  /* Jump stub for interrupt 0x91 */
extern VOID __forward_local_0x92(VOID);  /* Jump stub for interrupt 0x92 */
extern VOID __forward_local_0x93(VOID);  /* Jump stub for interrupt 0x93 */
extern VOID __forward_local_0x94(VOID);  /* Jump stub for interrupt 0x94 */
extern VOID __forward_local_0x95(VOID);  /* Jump stub for interrupt 0x95 */
extern VOID __forward_local_0x96(VOID);  /* Jump stub for interrupt 0x96 */
extern VOID __forward_local_0x97(VOID);  /* Jump stub for interrupt 0x97 */
extern VOID __forward_local_0x98(VOID);  /* Jump stub for interrupt 0x98 */
extern VOID __forward_local_0x99(VOID);  /* Jump stub for interrupt 0x99 */
extern VOID __forward_local_0x9a(VOID);  /* Jump stub for interrupt 0x9a */
extern VOID __forward_local_0x9b(VOID);  /* Jump stub for interrupt 0x9b */
extern VOID __forward_local_0x9c(VOID);  /* Jump stub for interrupt 0x9c */
extern VOID __forward_local_0x9d(VOID);  /* Jump stub for interrupt 0x9d */
extern VOID __forward_local_0x9e(VOID);  /* Jump stub for interrupt 0x9e */
extern VOID __forward_local_0x9f(VOID);  /* Jump stub for interrupt 0x9f */
extern VOID __forward_local_0xa0(VOID);  /* Jump stub for interrupt 0xa0 */
extern VOID __forward_local_0xa1(VOID);  /* Jump stub for interrupt 0xa1 */
extern VOID __forward_local_0xa2(VOID);  /* Jump stub for interrupt 0xa2 */
extern VOID __forward_local_0xa3(VOID);  /* Jump stub for interrupt 0xa3 */
extern VOID __forward_local_0xa4(VOID);  /* Jump stub for interrupt 0xa4 */
extern VOID __forward_local_0xa5(VOID);  /* Jump stub for interrupt 0xa5 */
extern VOID __forward_local_0xa6(VOID);  /* Jump stub for interrupt 0xa6 */
extern VOID __forward_local_0xa7(VOID);  /* Jump stub for interrupt 0xa7 */
extern VOID __forward_local_0xa8(VOID);  /* Jump stub for interrupt 0xa8 */
extern VOID __forward_local_0xa9(VOID);  /* Jump stub for interrupt 0xa9 */
extern VOID __forward_local_0xaa(VOID);  /* Jump stub for interrupt 0xaa */
extern VOID __forward_local_0xab(VOID);  /* Jump stub for interrupt 0xab */
extern VOID __forward_local_0xac(VOID);  /* Jump stub for interrupt 0xac */
extern VOID __forward_local_0xad(VOID);  /* Jump stub for interrupt 0xad */
extern VOID __forward_local_0xae(VOID);  /* Jump stub for interrupt 0xae */
extern VOID __forward_local_0xaf(VOID);  /* Jump stub for interrupt 0xaf */
extern VOID __forward_local_0xb0(VOID);  /* Jump stub for interrupt 0xb0 */
extern VOID __forward_local_0xb1(VOID);  /* Jump stub for interrupt 0xb1 */
extern VOID __forward_local_0xb2(VOID);  /* Jump stub for interrupt 0xb2 */
extern VOID __forward_local_0xb3(VOID);  /* Jump stub for interrupt 0xb3 */
extern VOID __forward_local_0xb4(VOID);  /* Jump stub for interrupt 0xb4 */
extern VOID __forward_local_0xb5(VOID);  /* Jump stub for interrupt 0xb5 */
extern VOID __forward_local_0xb6(VOID);  /* Jump stub for interrupt 0xb6 */
extern VOID __forward_local_0xb7(VOID);  /* Jump stub for interrupt 0xb7 */
extern VOID __forward_local_0xb8(VOID);  /* Jump stub for interrupt 0xb8 */
extern VOID __forward_local_0xb9(VOID);  /* Jump stub for interrupt 0xb9 */
extern VOID __forward_local_0xba(VOID);  /* Jump stub for interrupt 0xba */
extern VOID __forward_local_0xbb(VOID);  /* Jump stub for interrupt 0xbb */
extern VOID __forward_local_0xbc(VOID);  /* Jump stub for interrupt 0xbc */
extern VOID __forward_local_0xbd(VOID);  /* Jump stub for interrupt 0xbd */
extern VOID __forward_local_0xbe(VOID);  /* Jump stub for interrupt 0xbe */
extern VOID __forward_local_0xbf(VOID);  /* Jump stub for interrupt 0xbf */
extern VOID __forward_local_0xc0(VOID);  /* Jump stub for interrupt 0xc0 */
extern VOID __forward_local_0xc1(VOID);  /* Jump stub for interrupt 0xc1 */
extern VOID __forward_local_0xc2(VOID);  /* Jump stub for interrupt 0xc2 */
extern VOID __forward_local_0xc3(VOID);  /* Jump stub for interrupt 0xc3 */
extern VOID __forward_local_0xc4(VOID);  /* Jump stub for interrupt 0xc4 */
extern VOID __forward_local_0xc5(VOID);  /* Jump stub for interrupt 0xc5 */
extern VOID __forward_local_0xc6(VOID);  /* Jump stub for interrupt 0xc6 */
extern VOID __forward_local_0xc7(VOID);  /* Jump stub for interrupt 0xc7 */
extern VOID __forward_local_0xc8(VOID);  /* Jump stub for interrupt 0xc8 */
extern VOID __forward_local_0xc9(VOID);  /* Jump stub for interrupt 0xc9 */
extern VOID __forward_local_0xca(VOID);  /* Jump stub for interrupt 0xca */
extern VOID __forward_local_0xcb(VOID);  /* Jump stub for interrupt 0xcb */
extern VOID __forward_local_0xcc(VOID);  /* Jump stub for interrupt 0xcc */
extern VOID __forward_local_0xcd(VOID);  /* Jump stub for interrupt 0xcd */
extern VOID __forward_local_0xce(VOID);  /* Jump stub for interrupt 0xce */
extern VOID __forward_local_0xcf(VOID);  /* Jump stub for interrupt 0xcf */
extern VOID __forward_local_0xd0(VOID);  /* Jump stub for interrupt 0xd0 */
extern VOID __forward_local_0xd1(VOID);  /* Jump stub for interrupt 0xd1 */
extern VOID __forward_local_0xd2(VOID);  /* Jump stub for interrupt 0xd2 */
extern VOID __forward_local_0xd3(VOID);  /* Jump stub for interrupt 0xd3 */
extern VOID __forward_local_0xd4(VOID);  /* Jump stub for interrupt 0xd4 */
extern VOID __forward_local_0xd5(VOID);  /* Jump stub for interrupt 0xd5 */
extern VOID __forward_local_0xd6(VOID);  /* Jump stub for interrupt 0xd6 */
extern VOID __forward_local_0xd7(VOID);  /* Jump stub for interrupt 0xd7 */
extern VOID __forward_local_0xd8(VOID);  /* Jump stub for interrupt 0xd8 */
extern VOID __forward_local_0xd9(VOID);  /* Jump stub for interrupt 0xd9 */
extern VOID __forward_local_0xda(VOID);  /* Jump stub for interrupt 0xda */
extern VOID __forward_local_0xdb(VOID);  /* Jump stub for interrupt 0xdb */
extern VOID __forward_local_0xdc(VOID);  /* Jump stub for interrupt 0xdc */
extern VOID __forward_local_0xdd(VOID);  /* Jump stub for interrupt 0xdd */
extern VOID __forward_local_0xde(VOID);  /* Jump stub for interrupt 0xde */
extern VOID __forward_local_0xdf(VOID);  /* Jump stub for interrupt 0xdf */
extern VOID __forward_local_0xe0(VOID);  /* Jump stub for interrupt 0xe0 */
extern VOID __forward_local_0xe1(VOID);  /* Jump stub for interrupt 0xe1 */
extern VOID __forward_local_0xe2(VOID);  /* Jump stub for interrupt 0xe2 */
extern VOID __forward_local_0xe3(VOID);  /* Jump stub for interrupt 0xe3 */
extern VOID __forward_local_0xe4(VOID);  /* Jump stub for interrupt 0xe4 */
extern VOID __forward_local_0xe5(VOID);  /* Jump stub for interrupt 0xe5 */
extern VOID __forward_local_0xe6(VOID);  /* Jump stub for interrupt 0xe6 */
extern VOID __forward_local_0xe7(VOID);  /* Jump stub for interrupt 0xe7 */
extern VOID __forward_local_0xe8(VOID);  /* Jump stub for interrupt 0xe8 */
extern VOID __forward_local_0xe9(VOID);  /* Jump stub for interrupt 0xe9 */
extern VOID __forward_local_0xea(VOID);  /* Jump stub for interrupt 0xea */
extern VOID __forward_local_0xeb(VOID);  /* Jump stub for interrupt 0xeb */
extern VOID __forward_local_0xec(VOID);  /* Jump stub for interrupt 0xec */
extern VOID __forward_local_0xed(VOID);  /* Jump stub for interrupt 0xed */
extern VOID __forward_local_0xee(VOID);  /* Jump stub for interrupt 0xee */
extern VOID __forward_local_0xef(VOID);  /* Jump stub for interrupt 0xef */
extern VOID __forward_local_0xf0(VOID);  /* Jump stub for interrupt 0xf0 */
extern VOID __forward_local_0xf1(VOID);  /* Jump stub for interrupt 0xf1 */
extern VOID __forward_local_0xf2(VOID);  /* Jump stub for interrupt 0xf2 */
extern VOID __forward_local_0xf3(VOID);  /* Jump stub for interrupt 0xf3 */
extern VOID __forward_local_0xf4(VOID);  /* Jump stub for interrupt 0xf4 */
extern VOID __forward_local_0xf5(VOID);  /* Jump stub for interrupt 0xf5 */
extern VOID __forward_local_0xf6(VOID);  /* Jump stub for interrupt 0xf6 */
extern VOID __forward_local_0xf7(VOID);  /* Jump stub for interrupt 0xf7 */
extern VOID __forward_local_0xf8(VOID);  /* Jump stub for interrupt 0xf8 */
extern VOID __forward_local_0xf9(VOID);  /* Jump stub for interrupt 0xf9 */
extern VOID __forward_local_0xfa(VOID);  /* Jump stub for interrupt 0xfa */
extern VOID __forward_local_0xfb(VOID);  /* Jump stub for interrupt 0xfb */
extern VOID __forward_local_0xfc(VOID);  /* Jump stub for interrupt 0xfc */
extern VOID __forward_local_0xfd(VOID);  /* Jump stub for interrupt 0xfd */
extern VOID __forward_local_0xfe(VOID);  /* Jump stub for interrupt 0xfe */
extern VOID __forward_local_0xff(VOID);  /* Jump stub for interrupt 0xff */

extern VOID __spurious_int(VOID);        /* Spurious interrupt stub      */

LOCAL inline VOID CKinitLocalForwards(VOID)
/*
 * Initialise the IDT with the local interrupt forwarding stubs
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    CKsetVector(__forward_local_0x20, 0x20, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x21, 0x21, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x22, 0x22, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x23, 0x23, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x24, 0x24, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x25, 0x25, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x26, 0x26, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x27, 0x27, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x28, 0x28, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x29, 0x29, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x2a, 0x2a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x2b, 0x2b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x2c, 0x2c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x2d, 0x2d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x2e, 0x2e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x2f, 0x2f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x30, 0x30, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x31, 0x31, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x32, 0x32, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x33, 0x33, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x34, 0x34, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x35, 0x35, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x36, 0x36, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x37, 0x37, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x38, 0x38, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x39, 0x39, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x3a, 0x3a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x3b, 0x3b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x3c, 0x3c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x3d, 0x3d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x3e, 0x3e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x3f, 0x3f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x40, 0x40, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x41, 0x41, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x42, 0x42, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x43, 0x43, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x44, 0x44, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x45, 0x45, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x46, 0x46, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x47, 0x47, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x48, 0x48, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x49, 0x49, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x4a, 0x4a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x4b, 0x4b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x4c, 0x4c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x4d, 0x4d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x4e, 0x4e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x4f, 0x4f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x50, 0x50, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x51, 0x51, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x52, 0x52, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x53, 0x53, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x54, 0x54, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x55, 0x55, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x56, 0x56, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x57, 0x57, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x58, 0x58, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x59, 0x59, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x5a, 0x5a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x5b, 0x5b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x5c, 0x5c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x5d, 0x5d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x5e, 0x5e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x5f, 0x5f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x60, 0x60, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x61, 0x61, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x62, 0x62, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x63, 0x63, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x64, 0x64, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x65, 0x65, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x66, 0x66, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x67, 0x67, (D_PRESENT+D_TRAP+D_DPL3));

    CKsetVector(__spurious_int, 0x68, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x69, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x6a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x6b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x6c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x6d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x6e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x6f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x70, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x71, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x72, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x73, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x74, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x75, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x76, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__spurious_int, 0x77, (D_PRESENT+D_TRAP+D_DPL3));

    CKsetVector(__forward_local_0x78, 0x78, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x79, 0x79, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x7a, 0x7a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x7b, 0x7b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x7c, 0x7c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x7d, 0x7d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x7e, 0x7e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x7f, 0x7f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x80, 0x80, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x81, 0x81, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x82, 0x82, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x83, 0x83, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x84, 0x84, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x85, 0x85, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x86, 0x86, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x87, 0x87, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x88, 0x88, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x89, 0x89, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x8a, 0x8a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x8b, 0x8b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x8c, 0x8c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x8d, 0x8d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x8e, 0x8e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x8f, 0x8f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x90, 0x90, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x91, 0x91, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x92, 0x92, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x93, 0x93, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x94, 0x94, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x95, 0x95, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x96, 0x96, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x97, 0x97, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x98, 0x98, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x99, 0x99, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x9a, 0x9a, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x9b, 0x9b, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x9c, 0x9c, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x9d, 0x9d, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x9e, 0x9e, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0x9f, 0x9f, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa0, 0xa0, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa1, 0xa1, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa2, 0xa2, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa3, 0xa3, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa4, 0xa4, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa5, 0xa5, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa6, 0xa6, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa7, 0xa7, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa8, 0xa8, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xa9, 0xa9, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xaa, 0xaa, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xab, 0xab, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xac, 0xac, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xad, 0xad, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xae, 0xae, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xaf, 0xaf, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb0, 0xb0, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb1, 0xb1, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb2, 0xb2, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb3, 0xb3, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb4, 0xb4, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb5, 0xb5, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb6, 0xb6, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb7, 0xb7, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb8, 0xb8, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xb9, 0xb9, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xba, 0xba, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xbb, 0xbb, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xbc, 0xbc, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xbd, 0xbd, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xbe, 0xbe, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xbf, 0xbf, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc0, 0xc0, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc1, 0xc1, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc2, 0xc2, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc3, 0xc3, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc4, 0xc4, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc5, 0xc5, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc6, 0xc6, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc7, 0xc7, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc8, 0xc8, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xc9, 0xc9, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xca, 0xca, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xcb, 0xcb, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xcc, 0xcc, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xcd, 0xcd, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xce, 0xce, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xcf, 0xcf, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd0, 0xd0, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd1, 0xd1, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd2, 0xd2, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd3, 0xd3, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd4, 0xd4, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd5, 0xd5, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd6, 0xd6, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd7, 0xd7, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd8, 0xd8, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xd9, 0xd9, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xda, 0xda, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xdb, 0xdb, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xdc, 0xdc, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xdd, 0xdd, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xde, 0xde, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xdf, 0xdf, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe0, 0xe0, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe1, 0xe1, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe2, 0xe2, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe3, 0xe3, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe4, 0xe4, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe5, 0xe5, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe6, 0xe6, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe7, 0xe7, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe8, 0xe8, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xe9, 0xe9, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xea, 0xea, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xeb, 0xeb, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xec, 0xec, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xed, 0xed, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xee, 0xee, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xef, 0xef, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf0, 0xf0, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf1, 0xf1, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf2, 0xf2, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf3, 0xf3, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf4, 0xf4, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf5, 0xf5, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf6, 0xf6, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf7, 0xf7, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf8, 0xf8, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xf9, 0xf9, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xfa, 0xfa, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xfb, 0xfb, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xfc, 0xfc, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xfd, 0xfd, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xfe, 0xfe, (D_PRESENT+D_TRAP+D_DPL3));
    CKsetVector(__forward_local_0xff, 0xff, (D_PRESENT+D_TRAP+D_DPL3));
}

/* End of autogenerated data */

#endif
