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


#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
# include <winnt.h> // for LONGLONG
# include <winbase.h>
#else
# include <sys/time.h>
#endif

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_CYGWIN
# include <unistd.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkTime.h"

#ifdef DTK_ARCH_WIN32_VCPP
// from  gettimeofday.cpp
extern int gettimeofday(struct timeval *t, void *p=NULL);
#endif

/* gettimeofday() time zero is January 1, 1970 at 00:00:00 */


dtkTime::dtkTime(struct timeval *time0, int flag) :
  dtkBase(DTKTIME_TYPE)
{
  if(flag)// time0 is that the current time should be now
    {
      if(gettimeofday(&t0, NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTime::dtkTime() failed: "
		     "gettimeofday() failed.\n");
	  invalidate();
	  return;
	}
      
      t0.tv_sec  -= time0->tv_sec;
      // t0.tv_usec will be fine if it's negitive. It's a signed long.
      t0.tv_usec -= time0->tv_usec;
    }
  else  // time0 is the offset
    memcpy(&t0, time0, sizeof(struct timeval));

  validate();
}


dtkTime::dtkTime(double time0, int flag) : dtkBase(DTKTIME_TYPE)
{

  long l = (long) time0;

  if(flag)
    {
      if(gettimeofday(&t0, NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTime::reset() failed: "
		     "gettimeofday() failed.\n");
	  invalidate();
	  return;
	}
      
      t0.tv_sec  -= l;
      // t0.tv_usec will be fine if it's negitive. It's a signed long.
      t0.tv_usec -= ((long)((time0 - l)*1.0e+6));
    }
  else
    {
      t0.tv_sec = l;
      t0.tv_usec = ((long)((time0 - l)*1.0e+6));
    }
  validate();
}

dtkTime::dtkTime(long double time0, int flag) : dtkBase(DTKTIME_TYPE)
{
  long l = (long) time0;

  if(flag)
    {
      if(gettimeofday(&t0, NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTime::reset() failed: "
		     "gettimeofday() failed.\n");
	  invalidate();
	  return;
	}

      t0.tv_sec  -= l;
      // t0.tv_usec will be fine if it's negitive. It's a signed long.
      t0.tv_usec -= ((long)((time0 - l)*1.0e+6));
    }
  else
    {
      t0.tv_sec = l;
      t0.tv_usec = ((long)((time0 - l)*1.0e+6));
    }
  validate();
 }

dtkTime::~dtkTime(void) {}

long double dtkTime::delta(void)
{
  struct timeval tp1;
  long double delta_time;

  if(gettimeofday(&tp1, NULL))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkTime::delta() failed: "
		 "gettimeofday() failed.\n");
      return (long double) 0.0; // error
    }

  delta_time = (tp1.tv_usec - t0.tv_usec)*((long double) 1.0e-6) +
    tp1.tv_sec - t0.tv_sec;

  t0.tv_sec = tp1.tv_sec; /* save the last time */
  t0.tv_usec = tp1.tv_usec;
  return delta_time;
}


// Get the time since dtkTime::dtkTime() or dtkTime::delta()
// was called, which ever was later.

long double dtkTime::get(void)
{
  struct timeval tp1;

  if(gettimeofday(&tp1, NULL))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkTime::get() failed: "
		 "gettimeofday() failed.\n");
      return (long double) 0.0;
    }

  return (long double) (tp1.tv_usec - t0.tv_usec)*((long double)1.0e-6) + 
    tp1.tv_sec - t0.tv_sec;
}


void dtkTime::reset(struct timeval *time0, int flag)
{
  if(flag) // time0 is that the current time should be now
    {
      if(gettimeofday(&t0, NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTime::dtkTime() failed: "
		     "gettimeofday() failed.\n");
	  invalidate();
	  return;
	}

      t0.tv_sec  -= time0->tv_sec;
      // t0.tv_usec will be fine if it's negitive. It's a signed long.
      t0.tv_usec -= time0->tv_usec;
    }
  else // time0 is the offset
    memcpy(&t0, time0, sizeof(struct timeval));
  validate();
}

void dtkTime::reset(double time0, int flag)
{
  long l = (long) time0;

  if(flag)
   {
     if(gettimeofday(&t0, NULL))
       {
	 dtkMsg.add(DTKMSG_ERROR, 1,
		    "dtkTime::reset() failed: "
		    "gettimeofday() failed.\n");
	 invalidate();
	 return;
       }
     
     t0.tv_sec  -= l;
     // t0.tv_usec will be fine if it's negitive. It's a signed long.
     t0.tv_usec -= ((long)((time0 - l)*1.0e+6));
   }
  else
    {
      t0.tv_sec = l;
      t0.tv_usec = ((long)((time0 - l)*1.0e+6));
    }
  validate();
}

void dtkTime::reset(long double time0, int flag)
{
  long l = (long) time0;

  if(flag)
    {
      if(gettimeofday(&t0, NULL))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTime::reset() failed: "
		     "gettimeofday() failed.\n");
	  invalidate();
	  return;
	}

      t0.tv_sec  -= l;
      t0.tv_usec -= ((long)((time0 - l)*1.0e+6));
    }
  else
    {
      t0.tv_sec = l;
      t0.tv_usec = ((long)((time0 - l)*1.0e+6));
    }
  validate();
}
