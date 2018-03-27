/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2004  Lance Arsenault
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <dtk.h>

pthread_rwlock_t *rwlock;
pthread_rwlockattr_t *att;

static int init_rwlock(pthread_rwlock_t *lock)
{
  // get shared memory
  int fd = open ("pthread_rwlock_shm", O_RDWR|O_CREAT, 0666);
  if(fd < 0)
    {
      perror("open(\"foo\") failed\n");
      return -1;
    }

  int alloc_size = getpagesize();


  pthread_rwlockattr_t *att = (pthread_rwlockattr_t *)
    mmap(NULL, alloc_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if((void *) att == MAP_FAILED)
    {
      perror("mmap() failed\n");
      return -1;
    }


  if(ftruncate(fd, alloc_size))
    {
      perror("ftruncate() failed\n");
      return -1;
    }

  errno = 0;
  if(pthread_rwlockattr_init(att))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "pthread_rwlockattr_init(att) failed.\n"
		 "Can't initialize pthread_rwlock.\n");
      return -1; // error
    }
  dtkMsg.add(DTKMSG_NOTICE, "initialized pthread_rwlock\n");

  if(pthread_rwlockattr_setpshared(att, PTHREAD_PROCESS_SHARED))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "pthread_rwlockattr_setpshared(att, "
		 "PTHREAD_PROCESS_SHARED) failed.\n"
		 "Can't initialize pthread_rwlock.\n");
      return -1; // error
    }
  dtkMsg.add(DTKMSG_NOTICE,
	     "pthread_rwlockattr_setpshared(att, "
	     "PTHREAD_PROCESS_SHARED) succeeded\n");

  if(pthread_rwlock_init(lock, att))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "pthread_rwlock_init(lock, att) failed.\n"
		 "Can't initialize pthread_rwlock.\n");
      return -1; // error
    }
  dtkMsg.add(DTKMSG_NOTICE, "pthread_rwlock_init() succeeded\n");

  int i;
  pthread_rwlockattr_getpshared(att, &i);
  if(i == PTHREAD_PROCESS_SHARED)
    printf("PTHREAD_PROCESS_SHARED is set\n");
  return 0;
}

void Exit(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE, "caught signal %d, exiting ...\n", sig);
  pthread_rwlock_unlock(rwlock);
  exit(0);
}

void Pause(unsigned int t)
{
  for(unsigned int i=0;i<t;i++)
    {
      printf("%d ",i);
      fflush(stdout);
      sleep(1);
    }
}


int main(int argc, char **argv)
{
  if(argc == 1)
    if(init_rwlock(rwlock)) return 1;

  printf("\n");

  signal(SIGINT, Exit);
  signal(SIGTERM, Exit);
  signal(SIGQUIT, Exit);

  //// now the two processes duel for the lock
  for(int i=0;;i++)
    {
      if(argc == 1)
	{
	  printf("\rlocking(%d  5 seconds) ",i);
	  fflush(stdout);
	  pthread_rwlock_wrlock(rwlock);
	  Pause((unsigned int) 7);
	}
      else
	{
	  printf("\rlocking(%d  6 seconds) ",i);
	  fflush(stdout);
	  pthread_rwlock_rdlock(rwlock);
	  Pause((unsigned int) 6);
	}
      pthread_rwlock_unlock(rwlock);
      printf("done locking(%d) ",i);
      fflush(stdout);
      sleep(1);
    }
  return 0;
}
