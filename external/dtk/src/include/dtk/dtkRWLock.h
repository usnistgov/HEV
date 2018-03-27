/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */


// "_config.h" will define _DTK_IRIX_32_WITH_64 or not.  _config.h
// must be included before this.

/* System V (5) semphores semop() uses a system call, so it's slow but
 * the POSIX rwlock (and other like) data structures are not the same
 * size for 32bit and 64bit IRIX programs and since this data is
 * required to be in shared memory that will not let both 32bit and
 * 64bit IRIX programs use the same DTK shared memory.
 *
 * define _DTK_USE_SYSTEM_V to use use system V (5) semaphores to make
 * the rwlocks and conditionals that will make DTK shared memory work
 * between 32bit and 64bit IRIX programs.  This will cost you speed.
 *
 * define _DTK_USE_POSIX_SYNC to make the rwlocks and conditionals
 * from the POSIX (Pthreads) rwlocks and conditionals.  On IRIX and
 * GNU/Linux this will be fast, compared to using system V (5)
 * semaphores.
 *
 * Linux kernel version 2.4.x does not support inter-process POSIX
 * rwlocks and conditionals.  Linux kernel version 2.5.x and 2.6.x
 * does optionally support inter-process POSIX rwlocks and
 * conditionals.  NPTL-enabled glibc is also needed for this
 * inter-process POSIX rwlocks and conditionals.
 *
 * It's good to avoid using the _DTK_USE_SYSTEM_V option.
 */

/********************************************************************
 ****************       2 chooses on UNIXES          ****************
 ********************************************************************
 *  there are two chooses _DTK_USE_SYSTEM_V or _DTK_USE_POSIX_SYNC  *
 ********************************************************************/

#if defined _DTK_USE_SYSTEM_V && defined _DTK_USE_POSIX_SYNC
#  error "both _DTK_USE_SYSTEM_V and _DTK_USE_POSIX_SYNC are defined"
#endif


/********************************************************************
 ****************          DATA STRUCTURES           ****************
 ********************************************************************/


#ifdef _DTK_USE_SYSTEM_V

struct dtkRWLock
{
  u_int32_t magic_number;
  int32_t id;
  int32_t key;
};

#endif /* #ifdef _DTK_SYSTEM_V */


#ifdef _DTK_USE_POSIX_SYNC

#include <pthread.h>

struct dtkRWLock
{
  u_int32_t magic_number;
  pthread_rwlock_t lock;
};

#endif /* #ifdef _DTK_USE_POSIX_SYNC  */


#ifdef DTK_ARCH_WIN32_VCPP

// We use windoz Semaphore Objects.  They are just exclusive locks and
// not read/write locks.  It sounded resource expensive to build a
// read/write lock from two or three windoz Semaphore Objects.

struct dtkRWLock
{
	u_int32_t magic_number;
	char name[64];

	// Each process has a different handle.
	HANDLE handle; // handle is not shared.
};


#endif /* #ifdef DTK_ARCH_WIN32_VCPP */



/********************************************************************
 ****************        dtkRWLock  INTERFACES       ****************
 ********************************************************************/


#define DTK_RWLOCK_DEFAULT_MODE ((int) 0666)


/*! \file
 *
 * \brief a shared memory based read/write lock wrapper.
 *
 * dtkRWLock is a class of functions that provide shared memory based
 * (or not) read write locks that is simular to pthreads read write
 * locks.  On IRIX it's just a wrapper on the pthreads read write
 * locks (fast) and on GNU/Linux (and IRIX with 32bit and 64bit shared
 * memory) it's a wrapper on system V semaphores (slow).  This will
 * change in the future as NPTL-enabled glibc becomes common.
 *
 * dtkRWLock is used by the dtkSharedMem class to keep processes from
 * reading and writing to the same memory at the same time, which
 * could cause the data in shared memory to be in an unwanted state.
 */

/*! \fn int dtkRWLock_create(struct dtkRWLock *rwlock, int mode)
 * \brief Create a dtkRWLock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock that is in shared
 * memory.
 *
 * \param mode is the access
 * mode of the read write lock. For the case when 
 * dtkRWLock is a wrapper on pthreads read write locks the
 * access mode is ignored.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_create(struct dtkRWLock *rwlock,
			                int mode=DTK_RWLOCK_DEFAULT_MODE);

/*! \fn int dtkRWLock_destroy(struct dtkRWLock *rwlock);
 * \brief Destroy a dtkRWLock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock
 * that is in shared memory. This will make the read/write
 * lock unsuable by all processes and remove resources associated
 * with it.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_destroy(struct dtkRWLock *rwlock);

// when just connecting to an existing r/w lock
/*! \fn int dtkRWLock_connect(struct dtkRWLock *rwlock)
 * \brief Connect to a dtkRWLock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock that is in shared
 * memory and has been initialized by some process using
 * dtkRWLock_create().  This will make the read/write lock uable by
 * the connecting process.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_connect(struct dtkRWLock *rwlock);

/*! \fn int dtkRWLock_rlock(struct dtkRWLock *rwlock);
 * \brief Set a read lock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock that is in shared
 * memory and has been initialized.  If another process has called
 * dtkRWLock_wlock() without dtkRWLock_wunlock() yet, the current
 * process (or thread) will block until dtkRWLock_wunlock() is called
 * by another process (or thread).
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_rlock(struct dtkRWLock *rwlock);

/*! \fn int dtkRWLock_runlock(struct dtkRWLock *rwlock);
 * \brief Unset a read lock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock that is in shared
 * memory and has been initialized This should be called at some point
 * after dtkRWLock_rlock() has been called to release the read lock.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_runlock(struct dtkRWLock *rwlock);

/*! \fn int dtkRWLock_wlock(struct dtkRWLock *rwlock);
 * \brief Set a write lock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock that is in shared
 * memory and has been initialized.  If another process has called
 * dtkRWLock_wlock() (or dtkRWLock_rlock()) without
 * <b>dtkRWLock_wunlock</b> (or dtkRWLock_runlock()) yet, the current
 * process (or thread) will block until dtkRWLock_wunlock() (or
 * dtkRWLock_runlock()) is called by another process (or thread).
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_wlock(struct dtkRWLock *rwlock);

/*! \fn int dtkRWLock_wunlock(struct dtkRWLock *rwlock);
 * \brief Unset a write lock.
 *
 * \param rwlock is a pointer to a struct dtkRWLock that is in shared
 * memory and has been initialized This should be called at some point
 * after dtkRWLock_wlock() has been called to release the write lock.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkRWLock_wunlock(struct dtkRWLock *rwlock);
