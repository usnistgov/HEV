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
 *
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <time.h>
# include <process.h>
# define LOCK_TIMEOUT  INFINITE
#endif

#include <errno.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "types.h"
#include "dtkRWLock.h"


#ifdef _DTK_USE_SYSTEM_V

#include <sys/ipc.h>
#include <sys/sem.h>

#if defined (DTK_ARCH_LINUX) || defined (DTK_ARCH_CYGWIN)
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
  int val;                    /* value for SETVAL */
  struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
  unsigned short int *array;  /* array for GETALL, SETALL */
  struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif /* #if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED) */
#endif /* #ifdef DTK_ARCH_LINUX */


// Linux has a 128 semaphore limit
#define NUMBER_OF_TRIES_FOR_SEM  ((key_t) 1000)

static key_t key = 0; // The last key used to create

#if(0)
static void dtkRWLock_print(struct dtkRWLock *l)
{
  printf("dtkRWLock id=%d key=%d\n",l->id, l->key);
}
#endif

int dtkRWLock_create(struct dtkRWLock *l, int mode)
{
  unsigned short i[3] = { (unsigned short) 0, (unsigned short) 0,
			  (unsigned short) 1};
  union semun sem_union;
  sem_union.array = i;
  key_t key_stop = key + NUMBER_OF_TRIES_FOR_SEM;

  // find an unused sys V semaphore key
  for(l->id = -1;l->id < 0 && key < key_stop;)
    l->id = semget( ((key_t) (key+=1)),3,mode|IPC_CREAT|IPC_EXCL);

  l->key= (int32_t) key;

  if(l->id  == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_create(): semget(KEY,3,"
		 "(0%o|IPC_CREAT|IPC_EXCL)) failed for\nKEY=%d to %d."
		 "You may have reached the system limit in the number of\n"
		 "system V semaphores allowed.\n",
		 mode,
		 key_stop-NUMBER_OF_TRIES_FOR_SEM,
		 key_stop-1);
      l->magic_number = 0;
      return -1;
    }

  errno = 0; // clear the system error number from all the semget() calls

  if(semctl((int) l->id,0,SETALL,sem_union) == -1)
    {
      // remove the broken semaphore
      union semun arg;
      arg.val = 0;
      semctl(l->id,0,IPC_RMID,arg);

      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_create(): key = %d "
		 "semctl(%d,0,SETVAL,1) failed.\n",
		 l->key,l->id);
      l->magic_number = 0;
      return -1;
    }

  //printf("key=%d\n",key);

  l->magic_number = DTKRWLOCK_TYPE;
  return 0; // success
}


int dtkRWLock_destroy(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_destroy() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  union semun arg;
  arg.val = 0;

  if(semctl(l->id,0,IPC_RMID,arg) == -1)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkRWLock_destroy(): semctl(id"
		 "=%d,0,IPC_RMID,arg) failed.\n",
		 l->id);
      return -1;
    }
  l->magic_number = 0;

  return 0; // success
}

// when just connecting to an existing r/w lock
int dtkRWLock_connect(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_connect() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  /*********** Check if semaphore exists first ****************/

  errno = 0;
  int id = semget((key_t) l->key,3,0666|IPC_CREAT|IPC_EXCL);

  if(-1 == id && errno == EEXIST) // it exists so connect
    {
      errno = 0;
      if(semget((key_t) l->key,3,0) !=  (int) l->id)
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkRWLock_connect() failed: semget"
		     "(key=%d,3,0) failed.\n",
		     l->key);
	  return -1;
	}
      return 0;
    }

  if(id != -1)
    {
      // remove the semaphore
      union semun arg;
      arg.val = 0;
      semctl(id,0,IPC_RMID,arg);
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_connect() failed:\nsemget"
		 "(key=0x%x=%d,3,0666|IPC_CREAT|IPC_EXCL) returned %d\n"
		 "shows that the semaphore does not exist, "
		 "so you can't connect to it.\n",
		 l->key,l->key, id);
    }
  else
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "dtkRWLock_connect() failed:\nsemget"
	       "(key=0x%x=%d,3,0666|IPC_CREAT|IPC_EXCL) returned %d\n"
	       "Can't tell if the semaphore exists or not.\n",
	       l->key,l->key, id);

  return -1; 
}


int dtkRWLock_rlock(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_rlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  struct sembuf sop[2];

  sop[0].sem_num = 0;  /*** writers blocking readers. rest val = 0 ***/
  sop[0].sem_op  = 0;
  sop[0].sem_flg = SEM_UNDO;

  sop[1].sem_num = 1;  /*** readers blocking writers. rest val = 0 ***/
  sop[1].sem_op  = 1;
  sop[1].sem_flg = SEM_UNDO;

  if(semop((int) l->id,sop,(size_t) 2))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_rlock(): semop(%d,sop,2) failed.\n",
		 l->id);
      return -1;
    }

  return 0; // success
}


 int dtkRWLock_runlock(struct dtkRWLock *l)
{  
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_runlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  struct sembuf sop[1];

  sop[0].sem_num = 1;  /*** readers unblocking writers. rest val = 0 ***/
  sop[0].sem_op  = -1;
  sop[0].sem_flg = SEM_UNDO;

  if(semop((int)l->id,sop,(size_t) 1))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_runlock(): semop(%d,sop,1) failed.\n",
		 l->id);
      return -1;
    }

  return 0; // success
}


int dtkRWLock_wlock(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_wlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  struct sembuf sop[2];

  sop[0].sem_num = 0;  /*** writers blocking readers. rest val = 0 ***/
  sop[0].sem_op  = 1;
  sop[0].sem_flg = SEM_UNDO;

  sop[1].sem_num = 2;  /*** writers blocking writers. rest val = 1 ***/
  sop[1].sem_op  = -1;
  sop[1].sem_flg = SEM_UNDO;

  if(semop((int)l->id,sop,(size_t) 2))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_wlock(): semop(%d,sop,2) failed.\n",
		 l->id);
      return -1;
    }

  sop[0].sem_num = 1;  /*** readers blocking writers. rest val = 0 ***/
  sop[0].sem_op  = 0;
  sop[0].sem_flg = SEM_UNDO;

  if(semop((int)l->id,sop,(size_t) 1))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_wlock(): semop(%d,sop,1) failed.\n",
		 l->id);
      return -1;
    }
  return 0; // success
}


int dtkRWLock_wunlock(struct dtkRWLock *l)
{  
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_wunlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  struct sembuf sop[2];

  sop[0].sem_num = 0;  /*** writers unblocking readers. rest val = 0 ***/
  sop[0].sem_op  = -1;
  sop[0].sem_flg = SEM_UNDO;

  sop[1].sem_num = 2; /**** writers unblocking writers. rest val = 1 ***/
  sop[1].sem_op  = 1;
  sop[1].sem_flg = SEM_UNDO;

  if(semop((int)l->id,sop,(size_t) 2))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRWLock_wunlock(): semop(%d,sop,2) failed.\n",
		 l->id);
      return -1;
    }

  return 0; // success
}

#endif /* #ifdef _DTK_SYSTEM_V */



#ifdef _DTK_USE_POSIX_SYNC

/**************************************************************************/
/************************* using PThreads rwlock **************************/
/**************************************************************************/

int dtkRWLock_create(struct dtkRWLock *l, int mode)
{
  l->magic_number = DTKRWLOCK_TYPE;

  pthread_rwlockattr_t att;
  if( pthread_rwlockattr_init(&att) ||
      pthread_rwlockattr_setpshared(&att, PTHREAD_PROCESS_SHARED) ||
      pthread_rwlock_init(&(l->lock), &att)
      )
    {
      l->magic_number = 0;
      dtkMsg.add(DTKMSG_ERROR, 1,
                 "dtkRWLock_create(() failed.\n"
                 "Can't initialize pthread_rwlock.\n");
      return -1; // error
    }

  return 0; // success
}


int dtkRWLock_create(struct dtkRWLock *l)
{
  return dtkRWLock_create(l,DTK_RWLOCK_DEFAULT_MODE);
}


int dtkRWLock_destroy(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR, 
		 "dtkRWLock_destroy() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  int errNum;
  if((errNum = pthread_rwlock_destroy(&(l->lock))))
    {
      if(errNum == EBUSY)
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkRWLock_destroy() failed:\n"
		   "The read-write lock is currently "
		   "held by a thread.\n");
      else
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkRWLock_destroy() failed:\n"
		   "pthread_rwlock_destroy() failed.\n");
      return -1;
    }

  l->magic_number = 0;

  return 0; // success
}


// when just connecting to an existing r/w lock
int dtkRWLock_connect(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_connect() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }
  // nothing to do to connnect for pthread_rwlock

  return 0; 
}


int dtkRWLock_rlock(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_rlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  pthread_rwlock_rdlock(&(l->lock));

  return 0; // success
}


int dtkRWLock_runlock(struct dtkRWLock *l)
{  
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_runlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  pthread_rwlock_unlock(&(l->lock));

  return 0; // success
}


int dtkRWLock_wlock(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_wlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  pthread_rwlock_wrlock(&(l->lock));

  return 0; // success
}


int dtkRWLock_wunlock(struct dtkRWLock *l)
{  
  if(l->magic_number != DTKRWLOCK_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRWLock_wunlock() failed:\n"
		 "Bad magic number. Invalid struct dtkRWLock.\n");
      return -1;
    }

  pthread_rwlock_unlock(&(l->lock));

  return 0; // success
}

#endif /* #ifdef _DTK_USE_POSIX_SYNC */


#ifdef DTK_ARCH_WIN32_VCPP


/**************************************************************************/
/************************* Windows rwlock **************************/
/**************************************************************************/

// The struct dtkRWLock *rwlock is not required
// to be in shared memory for this code.

int dtkRWLock_create(struct dtkRWLock *l, int mode)
{
	// printf("MAX_PATH =%d\n",MAX_PATH);
    l->handle = NULL;

	int i=0;
	for(;i<5 && !(l->handle);i++) // 5 tries
	{
		// We need a unique Semaphore Object name.
		// This is not fool proof but it's close enough.
		srand(time(NULL) * _getpid());
		sprintf(l->name, "dtkRWLock_%6.6d.%6.6d",
					rand(), rand());
		srand(time(NULL));
		sprintf(&(l->name[strlen(l->name)]),
				    ".%6.6d.%6.6d.%6.6d",
					rand(), rand(), rand());

        l->handle = CreateSemaphore(NULL,
							  1L, // initial count
							  1L, // maximum count
							  l->name);

        SetLastError(0);
	    if(l->handle && GetLastError() == ERROR_ALREADY_EXISTS)
		{
		    CloseHandle(l->handle);
		    l->handle = NULL;
		}
	}

	if((l->handle))
	{
      l->magic_number = DTKRWLOCK_TYPE;
	  return 0;
	}
	else
	  return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_create() failed: \n"
		  "CreateSemaphore() failed:"
		  "Windoz GetLastError()=%d\n",
          GetLastError());
}


int dtkRWLock_destroy(struct dtkRWLock *l)
{
  if(l->magic_number != DTKRWLOCK_TYPE)
     return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_destroy() failed:\n"
		  "bad magic number.\n");

    l->magic_number = DTKBASE_TYPE;
	CloseHandle(l->handle);
	return 0;
}

// This is close to being the same as dtkRWLock_create()
// but the name must be set.  If no process is using
// the Semaphore(s) they with not exist, so we need to
// recreate them here.

int dtkRWLock_connect(struct dtkRWLock *l)
{
    if(l->magic_number != DTKRWLOCK_TYPE)
     return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_connect() failed:\n"
		  "bad magic number.\n");

    l->handle = CreateSemaphore(NULL,
							  1L, // initial count
							  1L, // maximum count
							  l->name);

	if(!(l->handle))
        return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_connect() failed: \n"
		  "CreateSemaphore() failed:"
		  "Windoz GetLastError()=%d\n",
          GetLastError());

	return 0;
}


int dtkRWLock_rlock(struct dtkRWLock *l)
{
   if(l->magic_number != DTKRWLOCK_TYPE)
       return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_rlock() failed:\n"
		  "bad magic number.\n");

    DWORD i = WaitForSingleObject(l->handle,LOCK_TIMEOUT);

	if(i == WAIT_ABANDONED || i == WAIT_FAILED)
	   return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_rlock() failed:\n"
		  "ReleaseSemaphore() failed:"
		  "Windoz GetLastError()=%d\n",
          GetLastError());

	return 0;
}


int dtkRWLock_wlock(struct dtkRWLock *l)
{
   if(l->magic_number != DTKRWLOCK_TYPE)
       return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_wlock() failed:\n"
		  "bad magic number.\n");

    DWORD i = WaitForSingleObject(l->handle, LOCK_TIMEOUT);

	if(i == WAIT_ABANDONED || i == WAIT_FAILED)
	   return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_wlock() failed:\n"
		  "ReleaseSemaphore() failed:"
		  "Windoz GetLastError()=%d\n",
          GetLastError());

	return 0;
}


int dtkRWLock_runlock(struct dtkRWLock *l)
{
   if(l->magic_number != DTKRWLOCK_TYPE)
       return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_runlock() failed:\n"
		  "bad magic number.\n");

   if(! ReleaseSemaphore(l->handle, 1L, NULL))
       return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_runlock() failed:\n"
		  "ReleaseSemaphore() failed:"
		  "Windoz GetLastError()=%d\n",
          GetLastError());

	return 0;
}


int dtkRWLock_wunlock(struct dtkRWLock *l)
{
   if(l->magic_number != DTKRWLOCK_TYPE)
       return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_wunlock() failed:\n"
		  "bad magic number.\n");

   if(! ReleaseSemaphore(l->handle, 1L, NULL))
       return dtkMsg.add(DTKMSG_WARN, 0, -1,
	      "dtkRWLock_wunlock() failed:\n"
		  "ReleaseSemaphore() failed:"
		  "Windoz GetLastError()=%d\n",
          GetLastError());

	return 0;
}

#endif /* #ifdef DTK_ARCH_WIN32_VCPP */
