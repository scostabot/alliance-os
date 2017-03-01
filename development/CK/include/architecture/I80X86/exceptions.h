/*
 * exceptions.h:  External stub declarations and loading function
 *
 * (C) 1998 Ramon van Handel, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 26/12/98  ramon       1.0    First release
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

#ifndef __SYS_EXCEPTIONS_H
#define __SYS_EXCEPTIONS_H

extern VOID __int_0x00(VOID);
extern VOID __int_0x02(VOID);
extern VOID __int_0x03(VOID);
extern VOID __int_0x04(VOID);
extern VOID __int_0x05(VOID);
extern VOID __int_0x06(VOID);
extern VOID __int_0x08(VOID);
extern VOID __int_0x09(VOID);
extern VOID __int_0x0a(VOID);
extern VOID __int_0x0b(VOID);
extern VOID __int_0x0c(VOID);
extern VOID __int_0x0d(VOID);
extern VOID __int_0x0e(VOID);
extern VOID __int_0x0f(VOID);
extern VOID __int_0x10(VOID);
extern VOID __int_0x11(VOID);
extern VOID __int_0x12(VOID);
extern VOID __int_0x13(VOID);
extern VOID __int_0x14(VOID);
extern VOID __int_0x15(VOID);
extern VOID __int_0x16(VOID);
extern VOID __int_0x17(VOID);
extern VOID __int_0x18(VOID);
extern VOID __int_0x19(VOID);
extern VOID __int_0x1a(VOID);
extern VOID __int_0x1b(VOID);
extern VOID __int_0x1c(VOID);
extern VOID __int_0x1d(VOID);
extern VOID __int_0x1e(VOID);
extern VOID __int_0x1f(VOID);

extern VOID CKsaveFPUContext(VOID);

LOCAL inline VOID CKinitExceptionStubs(VOID)
{
    CKsetVector(__int_0x00, 0x00, D_PRESENT+D_INT);
    CKsetVector(__int_0x02, 0x02, D_PRESENT+D_INT);
    CKsetVector(__int_0x03, 0x03, D_PRESENT+D_INT+D_DPL3);
    CKsetVector(__int_0x04, 0x04, D_PRESENT+D_INT+D_DPL3);
    CKsetVector(__int_0x05, 0x05, D_PRESENT+D_INT+D_DPL3);
    CKsetVector(__int_0x06, 0x06, D_PRESENT+D_INT);
    CKsetVector(__int_0x08, 0x08, D_PRESENT+D_INT);
    CKsetVector(__int_0x09, 0x09, D_PRESENT+D_INT);
    CKsetVector(__int_0x0a, 0x0a, D_PRESENT+D_INT);
    CKsetVector(__int_0x0b, 0x0b, D_PRESENT+D_INT);
    CKsetVector(__int_0x0c, 0x0c, D_PRESENT+D_INT);
    CKsetVector(__int_0x0d, 0x0d, D_PRESENT+D_INT);
    CKsetVector(__int_0x0e, 0x0e, D_PRESENT+D_INT);
    CKsetVector(__int_0x0f, 0x0f, D_PRESENT+D_INT);
    CKsetVector(__int_0x10, 0x10, D_PRESENT+D_INT);
    CKsetVector(__int_0x11, 0x11, D_PRESENT+D_INT);
    CKsetVector(__int_0x12, 0x12, D_PRESENT+D_INT);
    CKsetVector(__int_0x13, 0x13, D_PRESENT+D_INT);
    CKsetVector(__int_0x14, 0x14, D_PRESENT+D_INT);
    CKsetVector(__int_0x15, 0x15, D_PRESENT+D_INT);
    CKsetVector(__int_0x16, 0x16, D_PRESENT+D_INT);
    CKsetVector(__int_0x17, 0x17, D_PRESENT+D_INT);
    CKsetVector(__int_0x18, 0x18, D_PRESENT+D_INT);
    CKsetVector(__int_0x19, 0x19, D_PRESENT+D_INT);
    CKsetVector(__int_0x1a, 0x1a, D_PRESENT+D_INT);
    CKsetVector(__int_0x1b, 0x1b, D_PRESENT+D_INT);
    CKsetVector(__int_0x1c, 0x1c, D_PRESENT+D_INT);
    CKsetVector(__int_0x1d, 0x1d, D_PRESENT+D_INT);
    CKsetVector(__int_0x1e, 0x1e, D_PRESENT+D_INT);
    CKsetVector(__int_0x1f, 0x1f, D_PRESENT+D_INT);

    CKsetVector(CKsaveFPUContext, 0x07, D_PRESENT+D_INT);
}

#endif
