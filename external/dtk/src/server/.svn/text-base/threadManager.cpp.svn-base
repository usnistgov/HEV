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

#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <process.h>
#else
# include <pthread.h>
#endif
#include <sys/types.h>
#include <signal.h>

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"
#include "threadManager.h"


extern void (*signal_intr(int signo, void (*func)(int)))(int);
extern void signal_catcher(int sig);

#ifdef DTK_ARCH_WIN32_VCPP
  DWORD WINAPI
#else
  void *
#endif
  thread_routine(void *threadList_in)
{
  Condition *isServing = &(((ThreadList *) threadList_in)->isServing);
  dtkService *service = ((ThreadList *) threadList_in)->service;

  dtkMsg.add(DTKMSG_INFO, "started thread=%ld PID=%d "
	     "with service named \"%s%s%s\".\n",
	     ((ThreadList *) threadList_in)->thread, getpid(),
	     dtkMsg.color.red, serviceManager.getName(service),
	     dtkMsg.color.end);

  ((ThreadList *) threadList_in)->pid = getpid();

  // signal that I got my PID
  //isServing->signal();

  int service_rtVal = DTKSERVICE_CONTINUE;

   while(isRunning.get() && isServing->get())
     {
       service_rtVal = service->serve();

       if(service_rtVal != DTKSERVICE_CONTINUE)
	 break; // error or unload
     }

  // if joining do to this service instruction.
  if(isServing->get() && isRunning.get())
    {
      // Let the main thread know that I'm joining (quiting).
      isServing->set(0);
      // wake up the main thread.
      isRunning.signal();
    }

  dtkMsg.add(DTKMSG_INFO, "   thread=%ld PID=%d "
	     "with service named \"%s%s%s\" joining\n",
	     ((ThreadList *) threadList_in)->thread, getpid(),
	     dtkMsg.color.red, serviceManager.getName(service),
	     dtkMsg.color.end);

  return 
#ifdef DTK_ARCH_WIN32_VCPP
	  (DWORD)
#else
	  (void *)
#endif
	        service_rtVal;
}


ThreadManager::ThreadManager(void)
{
  list = NULL;
}


ThreadManager::~ThreadManager(void)
{
  while(list)
    {
      ThreadList *l = list;
      list = list->next;
      delete l;
    }  
}


int ThreadManager::add(dtkService *service)
{
  ThreadList *l = new ThreadList;
  l->service = service;

#ifdef DTK_ARCH_WIN32_VCPP
	if(!(l->thread = CreateThread(NULL, 0, thread_routine,
	                             (LPVOID) l, NULL, NULL)))
    {
      dtkMsg.add(DTKMSG_WARN, "CreateThread() failed for service"
		 " named \"%s\".",
		 serviceManager.getName((dtkService *) service));
      delete l;
      return -1;
    }
#else
   if(pthread_create(&(l->thread), NULL, thread_routine, (void *) l))
    {
      dtkMsg.add(DTKMSG_WARN, "pthread_create() failed for service"
		 " named \"%s\".",
		 serviceManager.getName((dtkService *) service));
      delete l;
      return -1;
    }
#endif

  // wait for the thread to get it's PID
  //l->isServing.wait();

  /**** add to the list ***/
  l->next = list;
  list = l;
  return 0;
}


void ThreadManager::remove(dtkService *service)
{
  ThreadList *prev = NULL, *l = list;
  for(;l;l = l->next)
    {
      if(l->service == service)
	{
	  /* stop the thread */
	  l->isServing.set(0);
printf("file=%s line=%d removing service thread named "
		       "\"%s\".\n", __FILE__, __LINE__,
		     serviceManager.getName((dtkService *) service));
#ifdef DTK_ARCH_WIN32_VCPP

      // Windoz sucks.  Just in case you didn't know.
      // There is no nice general way to stop the blocking read
      // of the thread that I'm trying to stop.
      // See is the Thread will return by it's self.
      service->stop();
	  // The service has time to comply.
      if(WAIT_TIMEOUT == 
		  WaitForSingleObject(l->thread, 2000 /* millisecs */))
	  {
	     if(!TerminateThread(l->thread, 1))
         dtkMsg.add(DTKMSG_WARN, "Failed to terminal thread "
		 "service named \"%s\".\n"
		 "TerminateThread(%d,1) failed: GetLastError()=%d.\n",
		       serviceManager.getName((dtkService *) service),
			   l->thread, GetLastError());
	  }

#else
	  // On UNIX it's easy, pthread_kill() interupts the read().
	  pthread_kill(l->thread, SIGINT);

	  /* wait for thread to return */
	  int rt = DTKSERVICE_CONTINUE;
	  int i = pthread_join(l->thread, (void **) &rt);

	  if(i)
	    dtkMsg.add(DTKMSG_WARN, "service thread named "
		       "\"%s\" failed to join:\n"
		       "pthread_join()=%d failed.\n",
		     serviceManager.getName((dtkService *) service),
		     i);

	  else
	    //if(rt != DTKSERVICE_CONTINUE)
	    dtkMsg.add(DTKMSG_INFO, "service thread named "
		       "\"%s\" returned %d.\n",
		       serviceManager.getName((dtkService *) service),
		       rt);
#endif

	  /* remove from the list */

	  if(prev)
	    prev->next = l->next;
	  else
	    list = l->next;
	  delete l;
	  break;
	}
      prev = l;
    }
}

/******* ONLY THE MASTER SERVER THREAD CALLS THIS *******/
// returns a pointer to the first thread waiting to be joined.
// returns NULL if none are found.
dtkService *ThreadManager::getJoiningThread(void)
{
  if(pthread_self() != mainThread)
    return NULL;

  ThreadList *l = list;
  for(;l;l = l->next)
    {
      if(!l->isServing.get())
	return l->service;
    }
  return NULL;
}

#ifdef DTK_ARCH_WIN32_VCPP
dtkService *ThreadManager::getService(pthread_t thread)
{
  if(pthread_self() == mainThread)
    return NULL;

  ThreadList *l = list;
  for(;l;l = l->next)
    {
      if(thread == l->thread)
	return l->service;
    }
  return NULL;
}
#endif

ThreadManager threadManager;


