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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
# include <winnt.h> // for LONGLONG
# include <winbase.h>
#else
# include <unistd.h>
# include <sys/time.h>
# include <sys/un.h>
# include <netinet/in.h>
#endif

#ifdef DTK_ARCH_IRIX
   typedef uint64_t u_int64_t;
#endif

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_IRIX || defined DTK_ARCH_CYGWIN
#  include <sys/socket.h>
#endif


#include "types.h"
#include "dtkBase.h"
#include "dtkFilter.h"
#include "filterList.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkRWLock.h"
#include "dtkConditional.h"
#include "dtkSegAddrList.h"
#include "dtkSocket.h"
#include "dtkUDPSocket.h"
#include "dtkSocketList.h"
#include "dtkSharedMem.h"
#include "dtkSharedTime.h"

#ifdef DTK_ARCH_WIN32_VCPP
// from  gettimeofday.cpp
extern int gettimeofday(struct timeval *t, void *p=NULL);
#endif


/* gettimeofday() time zero is January 1, 1970 at 00:00:00 */

dtkSharedTime::dtkSharedTime(const char *sharedMemName, int start_paused)
  : dtkBase(DTKSHAREDTIME_TYPE)
{
  if(gettimeofday(&(st.t0), NULL))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedTime::dtkSharedTime() failed: "
		 "gettimeofday() failed.\n");
      return;
    }

  // delta_t_offset = start = stop = t0
  memcpy(&(delta_t_offset),  &(st.t0), sizeof(struct timeval));
  memcpy(&(st.stop),  &(st.t0), sizeof(struct timeval));
  memcpy(&(st.start),  &(st.t0), sizeof(struct timeval));
  
  st.running = (start_paused)?0:1;

  // this will over write st if it was already set in shared memory
  shm = new dtkSharedMem(sizeof(struct dtkSharedTime_), sharedMemName, &st);
  if(!shm || shm->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR, "dtkSharedTime::dtkSharedTime(\"%s\",%p)"
		 " failed:\n", sharedMemName,shm);
      return;
    }

  validate();
}

// return (long double) -1.0 on error

long double dtkSharedTime::delta(void)
{
  if(isInvalid()) return (long double) -1.0; // error

  if(shm->read(&st)) return (long double) -1.0; // error

  if(!st.running)//   not running
    return (long double) 0.0;

  struct timeval tp1;
  if(gettimeofday(&tp1, NULL))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedTime::delta() failed: "
		 "gettimeofday() failed.\n");
      return (long double) -1.0; // error
    }

  long double delta_time;


  // if(delta_t_offset > st.start)
  if(delta_t_offset.tv_sec > st.start.tv_sec ||
     (delta_t_offset.tv_sec == st.start.tv_sec &&
      delta_t_offset.tv_usec > st.start.tv_usec))
    delta_time = (tp1.tv_usec - delta_t_offset.tv_usec)*
      ((long double) 1.0e-6) +
      tp1.tv_sec - delta_t_offset.tv_sec;
  else // delta_t_offset <= st.start
    delta_time = (tp1.tv_usec - st.start.tv_usec)*
      ((long double) 1.0e-6) +
      tp1.tv_sec - st.start.tv_sec;

  delta_t_offset.tv_sec = tp1.tv_sec; /* save the last time */
  delta_t_offset.tv_usec = tp1.tv_usec;
  return delta_time;
}


// Get the time since dtkSharedTime::dtkSharedTime().

long double dtkSharedTime::get(void)
{
  if(isInvalid()) return (long double) -1.0; // error

  if(shm->read(&st)) return (long double) -1.0; // error

  if(!st.running)//   not running
    return (st.stop.tv_usec - st.t0.tv_usec)*((long double) 1.0e-6) +
      st.stop.tv_sec - st.t0.tv_sec ;

  struct timeval tp1;
  if(gettimeofday(&tp1, NULL))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedTime::delta() failed: "
		 "gettimeofday() failed.\n");
      return (long double) -1.0; // error
    }

  return (tp1.tv_usec - st.t0.tv_usec)*((long double) 1.0e-6) +
    tp1.tv_sec - st.t0.tv_sec ;
}


int dtkSharedTime::reset(long double time0)
{
  if(isInvalid()) return -1; // error

  if(gettimeofday(&(st.t0), NULL))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedTime::reset() failed: "
		 "gettimeofday() failed.\n");
      invalidate();
      return -1;
    }

  memcpy(&(delta_t_offset),  &(st.t0), sizeof(struct timeval));
  memcpy(&(st.stop),  &(st.t0), sizeof(struct timeval));
  memcpy(&(st.start),  &(st.t0), sizeof(struct timeval));

  long l = (long) time0;
  st.t0.tv_sec  -= l;
  st.t0.tv_usec -= ((long)((time0 - l)*1.0e+6));

  if(shm->write(&st)) return -1;

  return 0;
}

int dtkSharedTime::pause(void)
{
  if(isInvalid()) return -1; // error

  if(shm->read(&st))  return -1; // error
  if(st.running)
    {
      if(gettimeofday(&(st.stop), NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkSharedTime::delta() failed: "
		     "gettimeofday() failed.\n");
	  return -1; // error
	}
      st.running = 0;
      if(shm->write(&st))  return -1; // error
    }

  return 0;
}

int dtkSharedTime::resume(void)
{
  if(isInvalid()) return -1; // error

  if(shm->read(&st))  return -1; // error

  if(!st.running)
    {
      if(gettimeofday(&(st.start), NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkSharedTime::delta() failed: "
		     "gettimeofday() failed.\n");
	  return -1; // error
	}
      // Finding the difference between to timevals is a pain in the
      // butt, if the result is another timeval.
      st.t0.tv_usec += st.start.tv_usec - st.stop.tv_usec;
      st.t0.tv_sec += st.start.tv_sec - st.stop.tv_sec;

      st.running = 1;
      if(shm->write(&st)) return -1; // error
    }
  return 0;
}

int dtkSharedTime::isRunning(void)
{
  if(isInvalid()) return -1; // error

  if(shm->read(&st))  return -1; // error

  return st.running;
}

dtkSharedTime::~dtkSharedTime(void)
{
  invalidate();

  if(shm)
    {
      delete shm;
      shm = NULL;
    }
}

