/*
 * Alliance Kernel Library Threads/Mutexes
 * 
 * Unlike most of the AKL, the Threads/Mutexes section does not use code
 * from glibc. This is because threading in the CK system and threading in
 * a POSIX system are totally different. This library does provide a
 * secondary implementation that sits on top of posix threads, mapping the
 * Alliance API to the POSIX API, in order to ease parallel development of
 * the CK and the KLs.
 * 
 * HISTORY
 * Date     Author    Rev    Notes
 * 20/11/98 djarb     1      First draft
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

/* Right now, we want to override the global setting */
#ifdef REAL_ALLIANCE
#define REAL_SET
#undef REAL_ALLIANCE
#endif

#include <typewrappers.h>
#include <klibs.h>

#ifdef REAL_ALLIANCE

/* These are the thread implementation for use with the Alliance CK
 */

PUBLIC DATA KLthreadCreate(Thread* thread,
			   INPUT DATA (*threadMain) (VOID*),
			   VOID* mainParam)
/*
 * Create a new thread, running the code pointed to by threadMain.
 *
 * INPUT:
 *	threadMain: The address, in the current thread's address space, of
 *                  the entry point for the new thread.
 *
 * RETURNS:
 *	thread: A fully filled in thread structure which is needed for certain
 *	        of the thread manipulations.
 *	KLthreadCreate() != 0: return value is a failure code
 *	KLthreadCreate() == 0: success.
 */
{

}

PUBLIC DATA KLthreadJoin(INPUT Thread* thread, DATA* threadReturn)
/*
 * Stops this thread's execution untill such time as thread has terminated.
 *
 * INPUT:
 *	thread: The thread to which the current thread should join.
 *
 * RETURNS:
 *	threadReturn: threadReturn contains the return value of thread
 *	KLthreadJoin() != 0: return value is a failure code
 *	KLthreadJoin() == 0: success.
 */
{

}

PUBLIC VOID KLthreadExit(INPUT DATA returnValue)
/*
 * Signals that the current thread has completed its exeuction run.
 *
 * INPUT:
 *	returnValue: The return value of the thread. (Sooo redundant ;)
 *
 * RETURNS:
 *    	none
 */
{

}

PUBLIC DATA KLthreadSleep(INPUT DATA microseconds)
/*
 * Temporarily stops execution of the current thread. The resolution may not
 * be in microseconds on some systems. This isn't a POSIX call.
 *
 * INPUT:
 *	microseconds: If elapsed time since sleeping > than this value, the
 *	              thread will be awakened.
 *
 * RETURNS:
 *	KLthreadSleep() != 0: return value is a failure code
 *	KLthreadSleep() == 0: success.
 */
{

}

PUBLIC DATA KLthreadYield()
/*
 * If there is another thread wanting CPU time, give up the CPU.
 *
 * INPUT:
 *	none
 *
 * RETURNS:
 *    	none
 */
{

}

#else  /* not REAL_ALLIANCE */

/* These are a mapping of the Alliance thread API onto POSIX threads.
 * Naturally, they are somewhat clumsy, since POSIX and Alliance are different.
 */

#include <pthread.h>
#include <sched.h>
#include <unistd.h>

PUBLIC DATA KLthreadCreate(Thread* thread,
			   /* INPUT */ DATA (*threadMain) (VOID*),
			   VOID* mainParam)
/*
 * Create a new thread, running the code pointed to by threadMain.
 *
 * INPUT:
 *	threadMain: The address, in the current thread's address space, of
 *                  the entry point for the new thread.
 *
 * RETURNS:
 *	thread: A fully filled in thread structure which is needed for certain
 *	        of the thread manipulations.
 *	KLthreadCreate() != 0: return value is a failure code
 *	KLthreadCreate() == 0: success.
 */
{
    /* That's a fun cast. */
    VOID* (*castMain)(VOID*) = (VOID* (*)(VOID*)) threadMain;

    /* We'll probably want to translate this return into an Alliance failure
     * code, so that error checks actually work.
     */
    return pthread_create(thread, NULL, castMain, mainParam);
}

PUBLIC DATA KLthreadJoin(INPUT Thread* thread, DATA* threadReturn)
/*
 * Stops this thread's execution untill such time as thread has terminated.
 *
 * INPUT:
 *	thread: The thread to which the current thread should join.
 *
 * RETURNS:
 *	threadReturn: threadReturn contains the return value of thread
 *	KLthreadJoin() != 0: return value is a failure code
 *	KLthreadJoin() == 0: success.
 */
{
    return pthread_join(*thread, (VOID**) threadReturn);
}

PUBLIC VOID KLthreadExit(INPUT DATA returnValue)
/*
 * Signals that the current thread has completed its exeuction run.
 *
 * INPUT:
 *	returnValue: The return value of the thread. (Sooo redundant ;)
 *
 * RETURNS:
 *    	none
 */
{
    pthread_exit((VOID*)returnValue);
}

PUBLIC DATA KLthreadSleep(INPUT DATA microseconds)
/*
 * Temporarily stops execution of the current thread. The resolution may not
 * be in microseconds on some systems. This isn't a POSIX call.
 *
 * INPUT:
 *	microseconds: If elapsed time since sleeping > than this value, the
 *	              thread will be awakened.
 *
 * RETURNS:
 *	KLthreadSleep() != 0: return value is a failure code
 *	KLthreadSleep() == 0: success.
 */
{
    usleep(microseconds);
    return 0;
}

PUBLIC DATA KLthreadYield()
/*
 * If there is another thread wanting CPU time, give up the CPU.
 *
 * INPUT:
 *	none
 *
 * RETURNS:
 *    	none
 */
{
    return sched_yield();
}

#endif

#ifdef REAL_SET
#define REAL_ALLIANCE
#undef REAL_SET
#endif

