/*
 * math.c:  Code to deal with the i387 FPU
 *
 * (C) 1998 Ramon van Handel, The Alliance Operating System Team
 *
 * HISTORY
 * Date      Author      Rev    Notes
 * 01/01/98  ramon       1.0    First release
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
#include "sys/taskstate.h"
#include "ckobjects.h"
#include "sched.h"

/*
 * Okay, let's review some basic coprocessor workings.
 *
 * When we do a context switch, the coprocessor status isn't automatically
 * saved in the old task's TSS.  The coprocessor register stack needs to
 * be saved explicitly using the fnsave command.  Similarly, the new
 * coprocessor context needs to be restored from main memory using the
 * frstor command.
 *
 * In order to do these things, the i386 series provides us with a way of
 * book-keeping.  The i386 sets the TS flag in %cr0 on every context
 * switch.  When the TS flag is set, this signifies that the current
 * coprocessor context might not be consistent with the main processor's
 * context.  Thus, when TS is set, the processor does not send commands
 * to the coprocessor, but instead generates an int $7 (coprocessor not
 * available) on the first coprocessor command to be executed.  It is the
 * job of the interrupt handler for int $7 to save the context of the
 * previous task that used the FPU in the appropiate place, and then to
 * either load the FPU context for the current task (if available) or to
 * initialise the FPU for the task (if it hasn't used the FPU before
 * during its runtime) using fninit.
 *
 * We keep the address of the object structure belonging to the last
 * thread that used the FPU in lastFPUUser.  The function
 * CKi387SetupContext() is hooked to int7 and is responsible for saving
 * the current FPU context in the TSS of the previous FPU user, and set up
 * the context for the current thread.  It uses the flag
 * currtask->taskState.i387.used in order to determine whether to set up a
 * clean context or to restore an old one from the TSS.  When a thread is
 * created, its i387.used is always set to 0, and it is set to 1 after the
 * first time it uses math.
 * We also check if we use the FPU for the first time (or haven't used it
 * for a long time) by checking whether lastFPUUser is NIL.  If so, we
 * clear the FPU exception flags using fnclex (originally I didn't do
 * this, but when I read the linux code I discovered it is so similar to
 * mine that I just put this one in too :))
 */


PUBLIC EVENT *lastFPUUser = NIL;


PUBLIC VOID CKi387SetupContext(VOID)
/*
 * Save the FPU context for previous math-using thread and set up maths
 * for the current thread
 *
 * INPUT:
 *     none
 *
 * RETURNS:
 *     none
 */
{
    /*
     * We start by enabling maths by clearing the TS bit in %cr0,
     * so that we don't get called again until the next context switch.
     * Anyway, this needs to be done before we call any other FPU
     * operations (like fnsave), otherwise we'll trap again and recurse...
     * (made this mistake the first time around.  Ack !)
     */

    asm( "clts" );            /* Enable maths                         */

    /*
     * Save the FPU context for the previous thread that used the
     * FPU in its TSS structure, if any.  If this is the first thread,
     * clear the FPU exception flags.
     */

    if(lastFPUUser)
        asm( "fnsave %0" : "=m" (lastFPUUser->taskState.i387.context) );
    else
        asm( "fnclex" );

    /*
     * Make us the latest FPU user so that our context can be saved
     * properly by the next FPU using thread.
     */

    lastFPUUser = currtask;   /* We are that latest FPU using thread  */

    /*
     * Now we set up the new context.  If this thread has used the FPU
     * before, we load the context from its TSS, and otherwise we set
     * up a clean context using fninit
     */

    if(currtask->taskState.i387.used)
        asm( "frstor %0" : : "m" (currtask->taskState.i387.context) );
    else {
        asm( "fninit" );
        currtask->taskState.i387.used++;
    }
}
