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
 *
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>

#include <errno.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "types.h"
#include "dtkRWLock.h"
#include "dtkConditional.h"


#ifdef _DTK_USE_SYSTEM_V

#include <sys/ipc.h>
#include <sys/sem.h>

#if defined DTK_ARCH_LINUX || defined DTK_ARCH_CYGWIN
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


#define SEM_ARRAY_SIZE     6
#define NUM_WAIT_SEMS     (SEM_ARRAY_SIZE -2)
#define CURRENT_WAIT_SEM  (SEM_ARRAY_SIZE -2)
#define EXCL_SEM          (SEM_ARRAY_SIZE -1)


#if(0) // debug printing thingy
static void _print(struct dtkConditional *l, int line)
{
  printf("\n------------------\n"
	 "DEBUG PRINT line=%d dtkConditional system "
	 "V semaphore id=%d key=%d\n",
	 line,
	 l->id, l->key);

  unsigned short val[SEM_ARRAY_SIZE];
  union semun sem_union;

  sem_union.array = val;
  errno = 0;

  if(semctl((int) l->id, 0, GETALL, sem_union) == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_signal(): key = %d "
		 "semctl(%d, 0, GETALL, sem_union) failed.\n",
		 l->key, l->id);
    }
  {
    int i=0;
    printf("semvals= ");
    for(i=0;i<SEM_ARRAY_SIZE;i++)
      printf("%d ",val[i]);
    printf("\n-----------------------\n");
  }
}
#endif // debug printing thingy

/* This uses system V semaphores to make a conditional.  It is not a
 * prefect algorithm.  It will fail in some very unlikely cases.  In
 * the future this code will be replaced by a better method.
 *
 * The wait semaphore changes to a different one after each time a
 * dtkConditional_signal() is called.  It's hoped that this will keep
 * processes that have called dtkConditional_wait() from being blocked
 * by being slow in responding to the release of the wait semaphore,
 * and getting blocked in the next wait-signal cycle.  It's the same
 * problem you get when a tree falls (semaphore releases) in the
 * forest and not only do you not observe it, but they also set the
 * tree back (semaphore blocks) up by the time you look.  For this to
 * be fool proof there needs to be an infinite number of wait
 * semaphores (trees), so that it never cycles back to the same wait
 * semaphore (tree).  Tests have shown that this works okay with one
 * wait semaphore, but the cost of this addition ansurance is very
 * small.
 */


int dtkConditional_create(struct dtkConditional *l, int mode)
{
  unsigned short i[SEM_ARRAY_SIZE] =
    {
      (unsigned short) 1,  /* wait lock set */
      (unsigned short) 0,  /* wait lock unset */
      (unsigned short) 0,  /* wait lock unset */
      (unsigned short) 0,  /* wait lock unset */
      (unsigned short) 0,  /* current wait lock semnum (a way to share
			      a number) */
      (unsigned short) 1   /* exclusion lock */
    };
  union semun sem_union;
  sem_union.array = i;
  key_t key_stop = key + NUMBER_OF_TRIES_FOR_SEM;

  // find an unused sys V semaphore key
  for(l->id = -1;l->id < 0 && key < key_stop;)
    l->id = semget( ((key_t) (key+=1)), SEM_ARRAY_SIZE,
		    mode|IPC_CREAT|IPC_EXCL);

  l->key= (int32_t) key;

  if(l->id  == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_create(): semget(KEY,%d,"
		 "(0%o|IPC_CREAT|IPC_EXCL)) failed for\nKEY=%d to %d."
		 "You may have reached the system limit in the number of\n"
		 "system V semaphores allowed.\n",
		 SEM_ARRAY_SIZE, mode,
		 key_stop-NUMBER_OF_TRIES_FOR_SEM,
		 key_stop-1);
      l->magic_number = 0;
      return -1;
    }

  errno = 0; // clear the system error number from all the semget() calls

  if(semctl((int) l->id, 0, SETALL, sem_union) == -1)
    {
      // remove the broken semaphore
      union semun arg;
      arg.val = 0;
      semctl(l->id, 0, IPC_RMID, arg);

      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_create(): key = %d "
		 "semctl(%d,0,SETALL,sem_union) failed.\n",
		 l->key,l->id);
      l->magic_number = 0;
      return -1;
    }

  //printf("key=%d\n",key);

  l->magic_number = DTKCONDITIONAL_TYPE;
  return 0; // success
}


int dtkConditional_destroy(struct dtkConditional *l)
{
  if(l->magic_number != DTKCONDITIONAL_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkConditional_destroy() failed:\n"
		 "Bad magic number. Invalid struct dtkConditional.\n");
      return -1;
    }

  union semun arg;
  arg.val = 0;

  if(semctl(l->id, 0, IPC_RMID, arg) == -1)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkConditional_destroy(): semctl(id"
		 "=%d, 0, IPC_RMID, arg) failed.\n",
		 l->id);
      return -1;
    }
  l->magic_number = 0;

  return 0; // success
}

// when just connecting to an existing conditional
int dtkConditional_connect(struct dtkConditional *l)
{
  if(l->magic_number != DTKCONDITIONAL_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkConditional_connect() failed:\n"
		 "Bad magic number. Invalid struct dtkConditional.\n");
      return -1;
    }

  /*********** Check if semaphore exists first ****************/

  errno = 0;
  int id = semget((key_t) l->key, SEM_ARRAY_SIZE, 0666|IPC_CREAT|IPC_EXCL);

  if(-1 == id && errno == EEXIST) // it exists so connect
    {
      errno = 0;
      if(semget((key_t) l->key, SEM_ARRAY_SIZE, 0) !=  (int) l->id)
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkConditional_connect() failed: semget"
		     "(key=%d, %d, 0) failed.\n",
		     l->key, SEM_ARRAY_SIZE);
	  return -1;
	}
      return 0;
    }

  if(id != -1)
    {
      // remove the semaphore
      union semun arg;
      arg.val = 0;
      semctl(id, 0, IPC_RMID, arg);
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_connect() failed:\nsemget"
		 "(key=0x%x=%d, %d, 0666|IPC_CREAT|IPC_EXCL) returned %d\n"
		 "shows that the semaphore does not exist, "
		 "so you can't connect to it.\n",
		 l->key,l->key, SEM_ARRAY_SIZE, id);
    }
  else
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "dtkConditional_connect() failed:\nsemget"
	       "(key=0x%x=%d, %d, 0666|IPC_CREAT|IPC_EXCL) returned %d\n"
	       "Can't tell if the semaphore exists or not.\n",
	       l->key,l->key, SEM_ARRAY_SIZE, id);

  return -1; 
}

// The wait process (thread) has to do more stuff than the signaler.
int dtkConditional_wait(struct dtkConditional *l)
{
  if(l->magic_number != DTKCONDITIONAL_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkConditional_wait() failed:\n"
		 "Bad magic number. Invalid struct dtkConditional.\n");
      return -1;
    }

  struct sembuf sop[1];

  sop[0].sem_num = EXCL_SEM;  /* get exclusion lock */
  sop[0].sem_op  = -1;
  sop[0].sem_flg = SEM_UNDO;

  errno = 0;
  if(semop((int)l->id, sop, (size_t) 1))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkConditional_wait(): semop(%d, sop, 1) failed: line=%d.\n",
		 l->id,__LINE__);
      return -1;
    }

  unsigned short val[SEM_ARRAY_SIZE];
  union semun sem_union;

  sem_union.array = val;
  errno = 0;

  if(semctl((int) l->id, 0, GETALL, sem_union) == -1)
    {
      // remove the broken semaphore
      union semun arg;
      arg.val = 0;
      semctl(l->id, 0, IPC_RMID, arg);

      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_signal(): key = %d "
		 "semctl(%d, 0, GETVAL, sem_union) failed.\n",
		 l->key,l->id);
      l->magic_number = 0;
      return -1;
    }

  int wait_semnum = val[CURRENT_WAIT_SEM];
  sem_union.val = 1;

  if(semctl((int) l->id, wait_semnum, SETVAL, sem_union) == -1)
    {
      // remove the broken semaphore
      union semun arg;
      arg.val = 0;
      semctl(l->id, 0, IPC_RMID, arg);

      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_signal(): key = %d "
		 "semctl(%d, %d, SETVAL, sem_union) failed.\n",
		 l->key, l->id, wait_semnum);
      l->magic_number = 0;
      return -1;
    }

  sop[0].sem_num = EXCL_SEM; /* release exclusion lock */
  sop[0].sem_op  = 1;
  sop[0].sem_flg = SEM_UNDO;

  if(semop((int)l->id, sop, (size_t) 1))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_wait(): semop(%d,sop,1) "
		 "failed: file=%s line=%d.\n",
		 l->id, __FILE__, __LINE__);
      return -1;
    }

  // sem_union.val is the index of the current wait semaphore.
  sop[0].sem_num = wait_semnum; /* wait blocker */
  sop[0].sem_op  = 0;
  sop[0].sem_flg = 0;

  if(semop((int)l->id, sop, (size_t) 1))
    {
      if(errno == EINTR) // Interrupted function call
	dtkMsg.add(DTKMSG_INFO, 1,
		 "dtkConditional_wait(): semop(%d, sop, 1) "
		 "failed: file=%s line=%d.\n",
		 l->id, __FILE__, __LINE__);
      else
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkConditional_wait(): semop(%d, sop, 1) "
		     "failed: file=%s line=%d.\n",
		     l->id, __FILE__, __LINE__);
	  return -1;
	}
    }

  return 0; // success
}


int dtkConditional_signal(struct dtkConditional *l)
{  
  if(l->magic_number != DTKCONDITIONAL_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkConditional_signal() failed:\n"
		 "Bad magic number. Invalid struct dtkConditional.\n");
      return -1;
    }

  struct sembuf sop[2];

  //printf("file=%s line=%d\n", __FILE__, __LINE__);

  sop[0].sem_num = EXCL_SEM;  /* get exclusion lock */
  sop[0].sem_op  = -1;
  sop[0].sem_flg = SEM_UNDO;

  errno =0;

  if(semop((int)l->id, sop, (size_t) 1))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkConditional_signal(): semop(%d, sop, 1) failed.\n",
		 l->id);
      return -1;
    }

  unsigned short val[SEM_ARRAY_SIZE];
  union semun sem_union;
  sem_union.array = val;

  errno = 0;

  if(semctl((int) l->id, 0, GETALL, sem_union) == -1)
    {
      // remove the broken semaphore
      union semun arg;
      arg.val = 0;
      semctl(l->id, 0, IPC_RMID, arg);

      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_signal(): key = %d "
		 "semctl(%d, 0, GETALL, sem_union) failed.\n",
		 l->key,l->id);
      l->magic_number = 0;
      return -1;
    }

  int current_wait_sem = val[CURRENT_WAIT_SEM];
  if(val[current_wait_sem])
    {
      sem_union.val = (current_wait_sem + 1) % NUM_WAIT_SEMS;

      if(semctl((int) l->id, CURRENT_WAIT_SEM, SETVAL, sem_union) == -1)
	{
	  // remove the broken semaphore
	  union semun arg;
	  arg.val = 0;
	  semctl(l->id, 0, IPC_RMID, arg);
	  
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkConditional_signal(): key = %d "
		     "semctl(%d, %d, SETVAL, sem_union) failed.\n",
		     l->key, l->id, CURRENT_WAIT_SEM);
	  l->magic_number = 0;
	  return -1;
	}
    }

  unsigned int nsops;

  sop[0].sem_num = EXCL_SEM; /* release exclusion lock */
  sop[0].sem_op  = 1;
  sop[0].sem_flg = SEM_UNDO;

  if(val[current_wait_sem] == (unsigned short) 1)
    {
      nsops = 2;
      sop[1].sem_num = val[CURRENT_WAIT_SEM];  /* release wait blocker  */
      sop[1].sem_op  = -1;
      sop[1].sem_flg = 0;
    }
  else //  val[val[CURRENT_WAIT_SEM]] should be zero
    {
      nsops = 1;
      // no need to signal release wait
    }

  if(semop((int)l->id, sop, nsops))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkConditional_signal(): semop(%d, sop, %d) failed.\n",
		 l->id, nsops);
      return -1;
    }

  return 0; // success
}


#endif /* #ifdef _DTK_SYSTEM_V */



#ifdef _DTK_USE_POSIX_SYNC

/**************************************************************************/
/*****************    using PThreads conditionals    **********************/
/**************************************************************************/


int dtkConditional_create(struct dtkConditional *l, int mode)
{
  l->magic_number = DTKCONDITIONAL_TYPE;

  pthread_mutexattr_t mutex_attr;
  pthread_condattr_t condattr;

  if(
     pthread_mutexattr_init(&mutex_attr) ||
     pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) ||
     pthread_mutex_init(&(l->mutex), &mutex_attr) ||

     pthread_condattr_init(&condattr) ||
     pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED) ||
     pthread_cond_init(&(l->conditional), &condattr)
     )
    {
      l->magic_number = 0;
      dtkMsg.add(DTKMSG_ERROR, 1,
                 "dtkConditional_create(() failed.\n"
                 "Can't initialize pthread conditional and mutex.\n");
      return -1; // error
    }

  return 0; // success
}


int dtkConditional_create(struct dtkConditional *l)
{
  return dtkConditional_create(l,DTK_COND_DEFAULT_MODE);
}


int dtkConditional_destroy(struct dtkConditional *l)
{
  if(l->magic_number != DTKCONDITIONAL_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR, 
		 "dtkConditional_destroy() failed:\n"
		 "Bad magic number. Invalid struct dtkConditional.\n");
      return -1;
    }

  int errNum;
  if((errNum = pthread_cond_destroy(&(l->conditional))))
    {
      if(errNum == EBUSY)
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkConditional_destroy() failed:\n"
		   "The lock is currently held.\n");
      else
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkConditional_destroy() failed:\n"
		   "pthread_cond_destroy() failed.\n");
      return -1;
    }
  if((errNum = pthread_mutex_destroy(&(l->mutex))))
    {
      if(errNum == EBUSY)
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkConditional_destroy() failed:\n"
		   "The lock is currently held.\n");
      else
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkConditional_destroy() failed:\n"
		   "pthread_mutex_destroy() failed "
		   "and returned %d.\n", errNum);
      return -1;
    }

  l->magic_number = 0;

  return 0; // success
}


// when just connecting to an existing r/w lock
int dtkConditional_connect(struct dtkConditional *l)
{
  if(l->magic_number != DTKCONDITIONAL_TYPE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkConditional_connect() failed:\n"
		 "Bad magic number. Invalid struct dtkConditional.\n");
      return -1;
    }
  // nothing to do to connnect for pthread_cond

  return 0; 
}

int dtkConditional_wait(struct dtkConditional *l)
{
  pthread_mutex_lock(&(l->mutex));
  pthread_cond_wait(&(l->conditional), &(l->mutex));
  pthread_mutex_unlock(&(l->mutex));

  return 0; // success
}


int dtkConditional_signal(struct dtkConditional *l)
{
  pthread_mutex_lock(&(l->mutex));
  pthread_cond_broadcast(&(l->conditional));
  pthread_mutex_unlock(&(l->mutex));

  return 0; // success
}

#endif /* #ifdef _DTK_USE_POSIX_SYNC */


#ifdef DTK_ARCH_WIN32_VCPP

#if(0) // debug printing thingy
static void _print(struct dtkConditional *l, int line)
{
}
#endif // debug printing thingy

int dtkConditional_create(struct dtkConditional *l, int mode)
{
  return 0; // success
}


int dtkConditional_destroy(struct dtkConditional *l)
{
  return 0; // success
}

// when just connecting to an existing conditional
int dtkConditional_connect(struct dtkConditional *l)
{
  return -1;
}

// The wait process (thread) has to do more stuff than the signaler.
int dtkConditional_wait(struct dtkConditional *l)
{
  return 0; // success
}


int dtkConditional_signal(struct dtkConditional *l)
{  
  return 0; // success
}


#endif /* #ifdef DTK_ARCH_WIN32_VCPP */
