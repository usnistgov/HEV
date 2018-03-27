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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"


Condition::Condition(void)
{
  val = 1;
#ifdef MULTI_THREADED
# ifdef DTK_ARCH_WIN32_VCPP
	semaphore = CreateSemaphore(NULL,
							  1L, // initial count
							  1L, // maximum count
							  NULL);

	if(!semaphore)
		dtkMsg.add(DTKMSG_WARN,
		           "CreateSemaphore() failed with error number %d\n",
				   GetLastError());
	else
	    WaitForSingleObject(semaphore, INFINITE);

 
# else
  pthread_cond_init(&waitCondition, NULL);
  pthread_mutex_init(&waitMutex, NULL);
# endif
#endif
}


#ifdef DTK_ARCH_WIN32_VCPP
Condition::~Condition(void)
{
	if(semaphore)
	{
		CloseHandle(semaphore);
		semaphore = NULL;
	}
}
#endif

void Condition::set(int truth)
{
  mutex.lock();
  val = truth;
  mutex.unlock();
}

int Condition::get(void)
{
  mutex.lock();
  int i = val;
  mutex.unlock();
  return i;
}

#ifdef MULTI_THREADED
void Condition::signal()
{
# ifdef DTK_ARCH_WIN32_VCPP
  LONG count;
  ReleaseSemaphore(semaphore, 1L, &count);
  if(count > 0L)
    WaitForSingleObject(semaphore, 0);
# else
  pthread_mutex_lock(&waitMutex);
  pthread_cond_signal(&waitCondition);
  pthread_mutex_unlock(&waitMutex);
# endif
}


void Condition::wait(unsigned int msecs)
{
# ifdef DTK_ARCH_WIN32_VCPP
  WaitForSingleObject(semaphore, INFINITE);
  ReleaseSemaphore(semaphore, 1L, NULL);
# else
  pthread_mutex_lock(&waitMutex);

#if 0
  struct timespec ts;
  ts.tv_sec = msecs/1000;
  ts.tv_nsec = 1000*(msecs%1000);
  pthread_cond_timedwait(&waitCondition, &waitMutex, &ts);
#else
  pthread_cond_wait(&waitCondition, &waitMutex);
#endif

  pthread_mutex_unlock(&waitMutex);
#endif
}
#endif
