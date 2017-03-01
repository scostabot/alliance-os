/*
 * syscalls.c:  Access system calls through call gates
 *
 * (C) 1998 Jens Albretsen, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 17/12/98  jens        1.0    First internal release
 * 19/12/98  ramon       1.1    Made calling stubs save stack frame
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

#include <typewrappers.h>
#include "sys/sysdefs.h"
#include "ckobjects.h"

/*
 * I changed (Resource *) to (VOID *) in CKresourceAlloc() to avoid
 * 'passing arg of incompatible pointer type' warnings from EGCS.
 *                                                            -- Ramon
 */

PUBLIC DATA CKcacheObject(ADDR object, DATA signature, ADDR size, DATA flags)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKCACHEOBJECTGATE)
    );
    return result;
}

PUBLIC DATA CKunCacheObject(DESCRIPTOR object)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKUNCACHEOBJECTGATE)
    );
    return result;
}

PUBLIC DATA CKsetThreadState(DESCRIPTOR object,ThreadState state)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSETTHREADSTATEGATE)
    );
    return result;
}

PUBLIC DATA CKthreadTunnel(DESCRIPTOR object,TunnelType type)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKTHREADTUNNELGATE)
    );
    return result;
}

PUBLIC DATA CKresourceAlloc(VOID *resource, DESCRIPTOR kernel, DATA flags)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKRESOURCEALLOCGATE)
    );
    return result;
}

PUBLIC DATA CKresourceFree(RESOURCE resource)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKRESOURCEFREEGATE)
    );
    return result;
}

PUBLIC DATA CKloadMapping(DESCRIPTOR object,ADDR physaddr,ADDR logicaladdr,ADDR length,UWORD32 flags)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKLOADMAPPINGGATE)
    );
    return result;
}

PUBLIC DATA CKunloadMapping(DESCRIPTOR object,ADDR logicaladdr,ADDR length)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKUNLOADMAPPINGGATE)
    );
    return result;
}

PUBLIC DATA CKshMemSetRange(DESCRIPTOR object,ADDR logAddr,ADDR size)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSHMEMSETRANGEGATE)
    );
    return result;
}

PUBLIC DATA CKshMemInvite(DESCRIPTOR object,EVENT *thread)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSHMEMINVITEGATE)
    );
    return result;
}

PUBLIC DATA CKshMemRemap(DESCRIPTOR object,ADDR logAddr,ADDR size)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSHMEMREMAPGATE)
    );
    return result;
}

PUBLIC DATA CKshMemAttach(DESCRIPTOR object,ADDR logAddr,ADDR size)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSHMEMATTACHGATE)
    );
    return result;
}

PUBLIC DATA CKshMemDetach(DESCRIPTOR object)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSHMEMDETACHGATE)
    );
    return result;
}

PUBLIC DATA CKshMemNotify(DESCRIPTOR object)
{
    DATA result;
    asm (
        "pushl %%ebp       \n"   /* Save stack frame       */
        "movl %%esp,%%ebp  \n"   /* Load local stack frame */
        "lcall %1,$0       \n"   /* Call call gate         */
        "movl %%ebp,%%esp  \n"   /* Restore stack frame    */
        "popl %%ebp        \n"
        : "=a"(result)
        : "p"(CKSHMEMNOTIFYGATE)
    );
    return result;
}

