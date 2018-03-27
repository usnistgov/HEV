/*
 * The DIVERSE Toolkit
 * Copyright (C) 1996 - 2004  Lance Arsenault
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

#ifdef DTK_ARCH_LINUX
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
#endif
#include <sys/types.h>
#ifdef DTK_ARCH_WIN32_VCPP
#else
# include <sys/time.h>
# include <unistd.h>
#endif
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkRealtime.h"

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
// This is a kluge that needs fixing.
#  define SIG_HOLD (void(*)(int))3
#endif

static void Alarm_handler(int sig) 
{
  //printf("called Alarm_handler with sig=%d\n",sig);
}

static void (*old_handler)(int) = NULL;

static int realtime_delete(const char *caller)
{
  int err=0;
  
  struct itimerval it;

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
  signal(SIGALRM, SIG_IGN);
#else
  sigignore(SIGALRM);
#endif 

  it.it_interval.tv_sec = 1;
  it.it_interval.tv_usec = 1;
  it.it_value.tv_usec = 0;
  it.it_value.tv_sec = 0;

  if(setitimer(ITIMER_REAL, &it, NULL))
    {
      err = -1;
      dtkMsg.add(DTKMSG_WARN, 1,
		 "%s failed: setitimer() failed.\n",
		 caller);
    }
#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN) 
    if(old_handler == Alarm_handler && SIG_ERR == signal(SIGALRM, SIG_DFL))
#else
    if(old_handler == Alarm_handler && SIG_ERR == sigset(SIGALRM, SIG_DFL))
#endif
 
    {
      err = -1;
      dtkMsg.add(DTKMSG_WARN, 1,
		 "%s failed. resetting SIGALRM handler. "
		 "sigset(SIGALRM,SIG_DFL) failed.\n",
		 caller);
    }

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
  else if(old_handler && SIG_ERR == signal(SIGALRM, old_handler))
#else
  else if(old_handler && SIG_ERR == sigset(SIGALRM, old_handler))
#endif
    {
      err = -1;
      dtkMsg.add(DTKMSG_WARN, 1,
		 "%s failed. resetting SIGALRM handler. "
		 "sigset(SIGALRM,) failed.\n",
		 caller);
    }
  old_handler = NULL;
  return err;
}

int dtkRealtime_init(double sec_in)
{
  int sec  = (int) sec_in;
  int usec = (int)(((double)(sec_in-(double)sec))*1000000.0);
  return dtkRealtime_init(sec, usec);
}

int dtkRealtime_init(int sec, int usec)
{
  if(sec < 0 || usec < 0 || (sec == 0 && usec == 0))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRealtime_init(%d,%d) failed.\n"
		 "The arguements (%d,%d) are not within"
		 " the valid range.\n", sec,usec,
		 sec,usec);
      return -1;
    }

  if(old_handler && realtime_delete("dtkRealtime_init()"))
    return -1;

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
   if(SIG_ERR == (old_handler = signal(SIGALRM, SIG_HOLD)))
#else
   if(SIG_ERR == (old_handler = sigset(SIGALRM, SIG_HOLD)))
#endif
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRealtime_init() failed: sigset(SIGALRM, SIG_HOLD)"
		 " failed");
      return -1;
    }

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
  if(SIG_ERR == signal(SIGALRM, Alarm_handler))
#else
  if(SIG_ERR == sigset(SIGALRM, Alarm_handler))
#endif
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRealtime_init() failed: sigset(SIGALRM,)"
		 " failed");
      (void) realtime_delete("dtkRealtime_init()");
      return -1;
    }

  struct itimerval it;
  it.it_interval.tv_sec = (long) sec;
  it.it_interval.tv_usec = (long) usec;
  it.it_value.tv_sec = (long) sec;
  it.it_value.tv_usec = (long) usec;

  if(setitimer(ITIMER_REAL, &it, NULL))
    {      
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRealtime_init() failed: setitimer(ITIMER_REAL,,NULL) failed");
      (void) realtime_delete("dtkRealtime_init()");
      return -1;
    }

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
  if(SIG_ERR == signal(SIGALRM, SIG_HOLD))
#else
  if(SIG_ERR == sigset(SIGALRM, SIG_HOLD))
#endif
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRealtime_wait() failed: sigset(SIGALRM, SIG_HOLD)"
		 " failed");
      return -1;
    }

  return 0;
}


int dtkRealtime_wait(void)
{
  sigpause(SIGALRM);

#if defined (DTK_ARCH_DARWIN) || defined (DTK_ARCH_CYGWIN)
  if(SIG_ERR == signal(SIGALRM, SIG_HOLD))
#else
  if(SIG_ERR == sigset(SIGALRM, SIG_HOLD))
#endif
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRealtime_wait() failed: sigset(SIGALRM, SIG_HOLD)"
		 " failed");
      return -1;
    }

  return 0;
}

int dtkRealtime_close(void)
{
  return realtime_delete("dtkRealtime_close()");
}
