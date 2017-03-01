/*
 * callgates.h: call gate definitions
 *
 * (C) 1998 Jens Albretsen, the Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 11/12/98  jens        1.0    First full internal release
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

#ifndef __SYS_STUBS_CALLGATES_H
#define __SYS_STUBS_CALLGATES_H

#include <typewrappers.h>
#include "ckobjects.h"
 
#define SETUPCALLGATE(DSC,ADDR,NOARGS) \
    GDT[DSC].gate.offset_low = ((UWORD32)ADDR) & 0xffff; \
    GDT[DSC].gate.offset_high = ((UWORD32)ADDR) >> 16; \
    GDT[DSC].gate.access+=(NOARGS+2);

PUBLIC VOID CKcacheObjectGateCode();
PUBLIC VOID CKunCacheObjectGateCode();
PUBLIC VOID CKsetThreadStateGateCode();
PUBLIC VOID CKresourceAllocGateCode();
PUBLIC VOID CKresourceFreeGateCode();
PUBLIC VOID CKloadMappingGateCode();
PUBLIC VOID CKunloadMappingGateCode();
PUBLIC VOID CKshMemSetRangeGateCode();
PUBLIC VOID CKshMemInviteGateCode();
PUBLIC VOID CKshMemRemapGateCode();
PUBLIC VOID CKshMemAttachGateCode();
PUBLIC VOID CKshMemDetachGateCode();
PUBLIC VOID CKshMemNotifyGateCode();
PUBLIC VOID CKthreadTunnelGateCode();

#endif
