/*
 * callgates.S:  call gate stubs for system calls
 *
 * (C) 1998 Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 11/12/98  jens        1.0    First full internal release
 * 17/12/98  ramon       1.1    Cleanup;  converted to .S file
 * 18/12/98  ramon       1.2    Adjusted to do correct intersegment return
 * 20/01/99  ramon       1.3    Complete revamp / added signals
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

#include "sys/asm.h"

/*
 * The goal of the stubs in this file is twofold:
 *
 * (1) Set up the processor in such a state after a call gate has been
 *     been invoked, that the appropiate CK function can be run, call that
 *     function, and return control to the caller.
 * (2) Make sure that CK system calls don't get interrupted by pending
 *     signals (by setting currtask->interruptable,) and process any
 *     signals that were invoked while the system call was running after
 *     the system call was finished.
 */

#define GATESTUB(symbol,kstub,parms)                                     \
.globl symbol                  ;  /* Define stub as a global symbol   */ \
.align 16,0x90                 ;  /* Align for speed                  */ \
symbol:                        ;  /* System call entry point          */ \
    pushl %ds                  ;  /* Save caller data segment         */ \
    pushl %ss                  ;  /* Switch to kernel data segment    */ \
    popl %ds                   ;  /* %ss is always valid (ss0 in TSS) */ \
    pushl %edx                 ;  /* Save the user registers          */ \
    pushl %ecx                 ;  /* Not %eax, it'll contain retcode  */ \
    pushl %ebp                 ;  /* Save the user base pointer       */ \
    pushl %edi                 ;  /* Save the user destination index  */ \
    movl CURRTASK,%ebp         ;  /* Get the currently running thread */ \
    movl USERTASK(%ebp),%edi   ;  /* Get the thread user-level struct */ \
    incl INTRPTBL(%edi)        ;  /* Mark us unfit to receive signals */ \
    cld                        ;  /* GCC likes this                   */ \
    call kstub                 ;  /* Call stub (skip stack ret block) */ \
    decl INTRPTBL(%edi)        ;  /* Okay, those signals can come now */ \
    movl SIGNALS(%ebp),%ebp    ;  /* Do we have any signals pending ? */ \
    orl %ebp,%ebp              ;  /* Hmmm, lessee now...              */ \
    jnz 2f                     ;  /* Yup, let's go handle them        */ \
1:  popl %edi                  ;  /* Restore the user dest index      */ \
    popl %ebp                  ;  /* Restore the user base pointer    */ \
    popl %ecx                  ;  /* Restore the user registers       */ \
    popl %edx                  ;  /* No %eax, it contains return code */ \
    popl %ds                   ;  /* Restore caller data segment      */ \
    lret $(4*parms+8)          ;  /* Return from call gate            */ \
2:  movl SIGPEND(%ebp),%ebp    ;  /* Get the pending signal           */ \
    testl $SFBUSY,%ebp         ;  /* Is it being handled ?            */ \
    jnz 1b                     ;  /* Yep, we can return safely        */ \
    popl %edi                  ;  /* Restore the user dest index      */ \
    popl %ebp                  ;  /* Restore the user base pointer    */ \
    popl %ecx                  ;  /* Restore the user registers       */ \
    popl %edx                  ;  /* No %eax, it contains return code */ \
    pushl (4*parms+0x18)(%esp) ;  /* Some stack rearrangement magic   */ \
    pushl (4*parms+0x18)(%esp) ;  /* CKsignal() expects to be called  */ \
    pushfl                     ;  /* from an interrupt, so we need to */ \
    pushl 0x14(%esp)           ;  /* rearrange the stack in such a    */ \
    pushl 0x14(%esp)           ;  /* way that it looks like we're     */ \
    pushl 0x14(%esp)           ;  /* coming from an interrupt.        */ \
    SAVEREGS                   ;  /* Save the user registers          */ \
    call CKsignal              ;  /* Handle the pending signal        */


.text

GATESTUB(CKcacheObjectGateCode,CKcacheObjectKernelStub,4)
GATESTUB(CKunCacheObjectGateCode,CKunCacheObjectKernelStub,1)
GATESTUB(CKsetThreadStateGateCode,CKsetThreadStateKernelStub,2)
GATESTUB(CKresourceAllocGateCode,CKresourceAllocKernelStub,3)
GATESTUB(CKresourceFreeGateCode,CKresourceFreeKernelStub,1)
GATESTUB(CKloadMappingGateCode,CKloadMappingKernelStub,5)
GATESTUB(CKunloadMappingGateCode,CKunloadMappingKernelStub,3)
GATESTUB(CKshMemSetRangeGateCode,CKshMemSetRangeKernelStub,3)
GATESTUB(CKshMemInviteGateCode,CKshMemInviteKernelStub,2)
GATESTUB(CKshMemRemapGateCode,CKshMemRemapKernelStub,3)
GATESTUB(CKshMemAttachGateCode,CKshMemAttachKernelStub,3)
GATESTUB(CKshMemDetachGateCode,CKshMemDetachKernelStub,1)
GATESTUB(CKshMemNotifyGateCode,CKshMemNotifyKernelStub,1)
GATESTUB(CKthreadTunnelGateCode,CKthreadTunnelKernelStub,2)
