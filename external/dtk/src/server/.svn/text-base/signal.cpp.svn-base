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
#include <signal.h>
#include <stdio.h>

#ifdef MULTI_THREADED
# include <pthread.h>
#endif

#include <dtk.h>

#include "if_windows.h"

#ifdef DTK_ARCH_WIN32_VCPP
# include <process.h>
#endif

#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"
#include "threadManager.h"


void (*signal_intr(int signo, void (*func)(int)))(int)
{
#ifdef DTK_ARCH_WIN32_VCPP
	return signal(signo, func);
#else
  struct sigaction act;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
# ifdef SunOS
  act.sa_flags |= SA_INTERRUPT;
# endif
  sigaction(signo, &act, NULL);
  return NULL;
#endif
}


#ifdef MULTI_THREADED

/*****************************************************/
/******* multi-threaded version signal_catcher *******/
/*****************************************************/

void signal_catcher(int sig)
{
#ifdef DTK_ARCH_WIN32_VCPP
  if(mainThread == pthread_self())
#else
  if(pthread_equal(mainThread, pthread_self()))
#endif
    {

#ifdef DTK_ARCH_WIN32_VCPP
	  // I interpet this at just a service unloading.
	  // The signal is needed to stop the thread from
	  // blocking on the read.
	  if(sig == SIGTERM)
	  {
        dtkMsg.add(DTKMSG_INFO,
		   "    main thread=%ld "
		   "pid=%ld caught signal %d\n",
		   mainThread, getpid(), sig);
		   return;
	  }
#endif
      // Tell the main thread that is should not be running.
      isRunning.set(0);
      // Tell the main thread to wake up so it can clean up to exit.
      isRunning.signal();

      dtkMsg.add(DTKMSG_INFO,
		 "    main thread=%ld "
		 "pid=%ld caught signal %d. cleaning up ...\n",
		 mainThread,
		 getpid(), sig);
    }
  else
    {
      dtkMsg.add(DTKMSG_DEBUG,
		 "non-main thread=%ld pid=%ld caught signal %d.\n",
		 pthread_self(), getpid(), sig);
      // Tell the main thread to wake up so it can clean up to exit.
      isRunning.signal();
#ifdef DTK_ARCH_WIN32_VCPP
	  // This does not work. The signals do not interupt a blocking
	  // read function.
	  if(!isRunning.get() && sig == SIGTERM)
	  {
	  // I interpet this at just a service unloading.
	  // The signal is needed to stop the thread from
	  // blocking on the read.
		  dtkService *s =
			  threadManager.getService(GetCurrentThread());
		  if(s)
		    CancelIo(s->fd);
	  }
#endif
      //pthread_detach(pthread_self());
      //pthread_exit(0);
    }
}

#else /* #ifdef MULTI_THREADED */

/*****************************************************/
/******** non-threaded version signal_catcher ********/
/*****************************************************/

void signal_catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,
	     "pid=%ld caught signal %d. cleaning up ...\n",
	     getpid(), sig);
  isRunning.set(0);
}

#endif
