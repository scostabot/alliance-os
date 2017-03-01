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
 * 09/12/98 djarb     2      comment out KLmutexIsLocked from the POSIX
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 * This program is distributed but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 */

#ifdef REAL_ALLIANCE
#define REAL_SET
#undef REAL_ALLIANCE
#endif

#include <typewrappers.h>
#include <klibs.h>

#ifdef REAL_ALLIANCE

/* These are the mutex implementation for use with the Alliance CK
 */

PUBLIC DATA KLmutexCreate(Mutex* mutex)
/*
 * Fills in a mutal exclusion structure.
 *
 * INPUT:
 *	none
 *
 * RETURNS:
 *	mutex: The filled in mutex structure
 *	KLmutexCreate() != 0: return value is a failure code
 *	KLmutexCreate() == 0: success
 */
{

}

PUBLIC DATA KLmutexLock(Mutex* mutex)
/*
 * If the mutex is locked, waits until the mutex becomes free, the proceedes
 * Locks the mutex, so that any other thread attempting to lock it will be
 * blocked, until this thread is done.
 *
 * INPUT:
 *	mutex: The mutex to lock.
 *
 * RETURNS:
 *   	mutex: the locked mutex.
 *	KLmutexLock() != 0: return value is a failure code
 *	KLmutexLock() == 0: success
 */
{

}

PUBLIC DATA KLmutexUnlock(Mutex* mutex)
/*
 * Unlocks the mutex, so that other threads may proceed.
 *
 * INPUT:
 *	mutex: The mutex to unlock.
 *
 * RETURNS:
 *   	mutex: the unlocked mutex.
 *	KLmutexUnlock() != 0: return value is a failure code
 *	KLmutexUnlock() == 0: success
 */
{

}

PUBLIC BOOLEAN KLmutexIsLocked(INPUT Mutex* mutex)
/*
 * Checks the state of a mutex.
 *
 * INPUT:
 *	mutex: The mutex to check
 *
 * RETURNS:
 *	KLmutexIsLocked() == TRUE: The mutex is locked.
 *	KLmutexIsLocked() == FALSE: The mutex is unlocked.
 */
{

}

#else  /* not REAL_ALLIANCE */

/* These are a mapping of the Alliance mutex API onto POSIX threads.
 * Naturally, they are somewhat clumsy, since POSIX and Alliance are different.
 */

#include <pthread.h>

PUBLIC DATA KLmutexCreate(Mutex* mutex)
/*
 * Fills in a mutal exclusion structure.
 *
 * INPUT:
 *	none
 *
 * RETURNS:
 *	mutex: The filled in mutex structure
 *	KLmutexCreate() != 0: return value is a failure code
 *	KLmutexCreate() == 0: success
 */
{
    pthread_mutex_init(mutex, NULL);
    return 0;
    /* This function returns a value only for consistency's sake. */
}

PUBLIC DATA KLmutexLock(Mutex* mutex)
/*
 * If the mutex is locked, waits until the mutex becomes free, the proceedes
 * Locks the mutex, so that any other thread attempting to lock it will be
 * blocked, until this thread is done.
 *
 * INPUT:
 *	mutex: The mutex to lock.
 *
 * RETURNS:
 *   	mutex: the locked mutex.
 *	KLmutexLock() != 0: return value is a failure code
 *	KLmutexLock() == 0: success
 */
{
    return pthread_mutex_lock(mutex);
}

PUBLIC DATA KLmutexUnlock(Mutex* mutex)
/*
 * Unlocks the mutex, so that other threads may proceed.
 *
 * INPUT:
 *	mutex: The mutex to unlock.
 *
 * RETURNS:
 *   	mutex: the unlocked mutex.
 *	KLmutexUnlock() != 0: return value is a failure code
 *	KLmutexUnlock() == 0: success
 */
{
    return pthread_mutex_unlock(mutex);
}

PUBLIC BOOLEAN KLmutexIsLocked(INPUT Mutex* mutex)
/*
 * Checks the state of a mutex.
 *
 * INPUT:
 *	mutex: The mutex to check
 *
 * RETURNS:
 *	KLmutexIsLocked() == TRUE: The mutex is locked.
 *	KLmutexIsLocked() == FALSE: The mutex is unlocked.
 */
{
    /* This complains about losing the const, and it's absolutely right.
     * The native version really will have this as a const operation, but
     * this is a kludge.
     */
    /* Besides which, pthread_mutex_trylock seems to be missing from Linux
    if(pthread_mutex_trylock(mutex)) {
	return TRUE;
    }

    pthread_mutex_unlock(mutex);
     */
    return FALSE;
}

#endif

#ifdef REAL_SET
#define REAL_ALLIANCE
#undef REAL_SET
#endif

