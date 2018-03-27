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
// This is a slow and cheap way for processes that are run as
// different users to signal each other.  signal(2) will not do this
// in general.  Both processes must know the filename.  It's like
// signaling by using touch files.  The reason for doing this is that
// it is a way that works consistantly in which user "nobody" is
// signaling user "root".  Most signal methods will not allow this on
// all OSs.  filename should not be a NFS mounted file.  Use
// /tmp/somefile.  The filename will be removed.  Use a different
// filename each time, to insure that it works.  Use dtkTempname(const
// char *pre) which uses malloc().  tempnam() is buggy on IRIX.

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include <dtk.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <io.h>
#else
# include <unistd.h>
#endif

#include "fileSignal.h"

#define LONG_TIME  ((long double) 300000.0)

// seconds is the timeout time.  seconds=0 waits forever.  returns 0
// on success.  returns non-zero otherwise.  It's a good idea to make
// sure that filename does not exist before calling sendSignal().
// This blocks until the other process responds with a call to
// waitForFileSignal().

int sendFileSignal(const char *filename, int seconds)
{
  return sendFileSignal(filename, NULL, seconds);
}

int sendFileSignal(const char *filename, const char *message_in, int seconds)
{
  char *message = (message_in)?((char *) message_in):((char *) "hi");
  size_t message_size = strlen(message) + 1;
  if(message_size > MESSAGE_SIZE)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "sendSignal() from file=%s failed:\n"
		 "The message is to long.\n",
		 __FILE__, filename);
      return -1; // error
    }

  mode_t old_mask = umask((mode_t) 00);

  FILE *file = fopen(filename,"w+");
  if(!file)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "sendSignal() from file=%s failed:\n"
		 "fopen(\"%s\", \"w+\") failed.\n",
		 __FILE__, filename);
      umask(old_mask);
      return -1; // error
    }

  size_t i = fwrite((const void *) message, (size_t) 1,
		    message_size, file);
  fflush(file);

  if(i != message_size)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "sendSignal() from file=%s failed:\n"
		 "fwrite() to file \"%s\" failed.\n",
		 __FILE__, filename);
      fclose(file);
      unlink(filename);
      umask(old_mask);
      return -1; // error
    }

  dtkTime t(0.0, 1);
  long double t_stop = seconds;

  if(seconds == 0) t_stop = LONG_TIME;

  while(t.get() <  t_stop)
    {
      struct stat buff;
      if(stat(filename, &buff) < 0 || buff.st_size < (off_t) message_size + 3)
	usleep(10000);
      else
	break;
    }

  char response[4] = { '\0', '\0', '\0', '\0' };
  i = fread(response, (size_t) 1, (size_t) 3, file);
  fclose(file);
  unlink(filename);

  if(strcmp("ok",response))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "sendSignal() from file=%s failed:\n"
		 "failed to read correct response.\n",
		 __FILE__, filename);
      umask(old_mask);
      return -1;
    }

  return 0; // success
}


// wait for signal.  seconds is the timeout time.  seconds=0 waits a
// long time.  returns non-NULL on success.  returns NULL on timeout.
// The pointer returned points to a static local string.
char *waitForFileSignal(const char *filename, int seconds)
{
  dtkTime t(0.0,1);
  long double t_stop = seconds;

  if(seconds == 0) t_stop = LONG_TIME;

  while(t.get() <  t_stop)
    {
      struct stat buff;

      if(stat(filename, &buff) < 0 || buff.st_size < (off_t) 1)
	usleep(10000);
      else // I see a file
	{
	  FILE *file = fopen(filename, "r+");
	  if(!file)
	    {
	      dtkMsg.add(DTKMSG_WARN, 1,
			 "waitForSignal() from file=%s failed:\n"
			 "fopen(\"%s\",) failed.\n",
			 __FILE__, filename);
	      return NULL; // error
	    }

	  static char message[MESSAGE_SIZE];

	  size_t i = fread(message, (size_t) 1, MESSAGE_SIZE, file);
	  if(i < ((size_t) 1))
	    {
	      dtkMsg.add(DTKMSG_WARN, 1,
			 "waitForSignal() from file=%s failed:\n"
			 "fread() to file \"%s\" failed.\n",
			 __FILE__, filename);
	      return NULL; // error
	    }

	  message[MESSAGE_SIZE-1] = '\0';
	  // now write signal back
	  i = fwrite((const void *) "ok", (size_t) 1, (size_t) 3, file);
	  fclose(file);
	  if(i != ((size_t) 3))
	    {
	      dtkMsg.add(DTKMSG_WARN, 1,
			 "waitForSignal() from file=%s failed:\n"
			 "fwrite() to file \"%s\" failed.\n",
			 __FILE__, filename);
	      return NULL; // error
	    }
	  return message; // success
	}
      }

  dtkMsg.add(DTKMSG_WARN,
	     "waitForSignal(\"%s\",,) from file=%s failed:\n"
	     "Timed out waiting to read message in %2.2Lg seconds.\n",
	      filename, __FILE__, t_stop);
  return NULL; // timeout
}
