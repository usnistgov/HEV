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
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t mutex;
pthread_attr_t attr;

void *thread_routine(void *threadList_in)
{
  printf("\nslave thread running pid=%d.\n",getpid());

  // pthread_mutex_lock(&mutex);

  printf("slave thread returning.\n");
  //pthread_mutex_unlock(&mutex);

  return NULL;
}

int main(void)
{
  pthread_atfork(NULL, NULL, NULL);
#if 1
  printf("parent pid = %d\n",getpid());
  pid_t child_pid = fork();
  if(child_pid != (pid_t) 0) // not child
    return 0;
  printf("\npid = %d\n",getpid());
#endif

  pthread_atfork(NULL, NULL, NULL);
#if 1
  if(setsid() == -1)
    {
      perror("setsid() failed.\n");
      return 1;
    }
  printf("\npid = %d after setsid()\n",getpid());
#endif
  pthread_atfork(NULL, NULL, NULL);
#if 1
  child_pid = fork();
  if(child_pid != (pid_t) 0) // not child
    return 0;
  printf("\npid = %d\n",getpid());
#endif


  //pthread_mutex_init(&mutex, NULL);
  //pthread_mutex_lock(&mutex);

  pthread_attr_init(&attr);
  
  pthread_t thread;
  void *l = NULL;

  if(pthread_create(&thread, &attr, thread_routine, (void *) l))
    {
      printf("pthread_create() failed\n");
      return 1;
    }
  printf("master thread running file=%s line=%d\n", __FILE__,__LINE__);

  //pthread_mutex_unlock(&mutex);
  if(pthread_join(thread, NULL))
    {
      printf("pthread_join() failed\n");
      return 1;
    }

  printf("master thread returning.\n");

  return 0; 
}

  
