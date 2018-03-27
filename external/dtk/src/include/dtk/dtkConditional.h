/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2004  Lance Arsenault
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

// See dtkRWLock.h for more comments.



#ifdef _DTK_USE_POSIX_SYNC

struct dtkConditional
{
  u_int32_t magic_number;
  pthread_mutex_t mutex;
  pthread_cond_t conditional;
};

#endif /* #ifdef_DTK_USE_POSIX_SYNC  */

#ifdef _DTK_USE_SYSTEM_V

struct dtkConditional
{
  u_int32_t magic_number;
  int32_t id;
  int32_t key;
};

#endif


#ifdef DTK_ARCH_WIN32_VCPP

struct dtkConditional
{
  u_int32_t magic_number;
  char name[64];

  // Each process has a different handle.
  HANDLE handle; // handle is not shared.
};

#endif /* #ifdef DTK_ARCH_WIN32_VCPP */



#define DTK_COND_DEFAULT_MODE ((int) 0666)


/*! \file
 *
 * \brief shared memory based signal thingy
 *
 * dtkConditional is a class of functions that provide shared memory
 * based (or not) conditional lock/signal thingy.  On IRIX it's just a
 * wrapper on the pthreads mutex and conditional and on GNU/Linux (and
 * IRIX with 32bit and 64bit shared memory) it's made with system V
 * semaphores.
 *
 * In most DTK applications the utility of dtkConditional is
 * internally used by the dtkSharedMem class, so that knowledge of
 * dtkConditional is not required.
 */

/*! \fn int dtkConditional_create(struct dtkConditional *cond, int mode)
 * \brief Create a dtkConditional.
 *
 * \param cond is a pointer to a struct dtkConditional that is in
 * shared memory.
 *
 * \param mode is the access mode of the read write lock. For the case
 * when dtkConditional is a wrapper on pthreads read write locks the
 * access mode is ignored.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkConditional_create(struct dtkConditional *cond,
			                int mode=DTK_COND_DEFAULT_MODE);

/*! \fn int dtkConditional_destroy(struct dtkConditional *cond);
 * \brief Destroy a dtkConditional.
 *
 * \param cond is a pointer to a struct dtkConditional
 * that is in shared memory. This will make the read/write
 * lock unsuable by all processes and remove resources associated
 * with it.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkConditional_destroy(struct dtkConditional *cond);

// when just connecting to an existing conditional
/*! \fn int dtkConditional_connect(struct dtkConditional *cond)
 * \brief Connect to a dtkConditional.
 *
 * \param cond is a pointer to a struct dtkConditional that is in shared
 * memory and has been initialized by some process using
 * dtkConditional_create().
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkConditional_connect(struct dtkConditional *cond);

/*! \fn int dtkConditional_wait(struct dtkConditional *cond);
 * \brief wait for the signal
 *
 * This call will block until dtkConditional_signal() is called by
 * another process.
 *
 * \param cond is a pointer to a struct dtkConditional that is in shared
 * memory and has been initialized.
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkConditional_wait(struct dtkConditional *cond);

/*! \fn int dtkConditional_signal(struct dtkConditional *cond);
 * \brief signal the waiting processes
 *
 * Signal the waiting processes (or threads) to continue.
 *
 * \param cond is a pointer to a struct dtkConditional that is in shared
 * memory and has been initialized 
 *
 * \return 0 on success, and -1 on error.
 */
extern DTKAPI int dtkConditional_signal(struct dtkConditional *cond);
