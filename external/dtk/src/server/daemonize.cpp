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

#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
#  include <pwd.h>
#  include <unistd.h>
#endif

#include "fileSignal.h"

char *sigFile;

void signalDaemonStarted(int timeout, int success)
{
  if(success)
    sendFileSignal(sigFile, "OK", timeout);
  else
    sendFileSignal(sigFile, "FAIL", timeout);
}
  

int daemonize(int timeout)
{
  sigFile = dtk_tempname("/tmp/dtk-server_sigFile");
  if(!sigFile) return 1; // error

  pid_t pid = fork();

  if(pid < 0) // error
    return dtkMsg.add(DTKMSG_FATAL, 1, 1,
		      "fork() failed.\n");
  else if (pid != 0) // I'm the parent.
    {
      char *s = waitForFileSignal(sigFile, timeout);
      if(s && !strcmp("OK",s))
	exit(0); // success return 0 to status
      else
	exit(1); // error return 1 to status
    }

  // I'm the child

  if(setsid() < 0)
    {
      dtkMsg.add(DTKMSG_FATAL, 1,
                 "setsid() failed.\n");
      sendFileSignal(sigFile, "FAIL", timeout);
      return 1; // error
    }

  pid = fork();
  if(pid < 0) // error
    {
      dtkMsg.add(DTKMSG_FATAL, 1,
                 "fork() failed.\n");
      sendFileSignal(sigFile, "FAIL", timeout);
      return 1;
    }
  else if (pid != 0) // I'm the next generation parent.
    exit(0);

  // I'm the next generation child.

  close(STDIN_FILENO);

  // Close STDOUT and STDERR under most conditions.
  // If stderr (or stdout) of `dtk-server --daemon' is redirected to
  // a file or piped, then the file stderr (or stdout) will not be
  // closed.

  struct stat stat_data;
  if((fstat(STDOUT_FILENO, &stat_data) == 0 &&
      !( S_ISFIFO(stat_data.st_mode) || S_ISREG(stat_data.st_mode) )) ||
     isatty(STDOUT_FILENO))
    {
      close(STDOUT_FILENO);
      if(dtkMsg.file() == stdout)
        // turn off the spew from dtkMsg
	putenv(const_cast<char*>("DTK_SPEW=NONE"));
    }
  if((fstat(STDERR_FILENO, &stat_data) == 0 &&
      !( S_ISFIFO(stat_data.st_mode) || S_ISREG(stat_data.st_mode) )) ||
     isatty(STDERR_FILENO))
    {
      close(STDERR_FILENO);
      if(dtkMsg.file() == stderr)
        // turn off the spew from dtkMsg
        putenv(const_cast<char*>("DTK_SPEW=NONE"));
    }

  // It will no longer find DSO services in the current directory
  // since that will be "/".
  chdir("/");
  umask(0);

  // If user is "root".
  if(geteuid() == (uid_t) 0)
    {
      // Try to run as user "nobody".
      struct passwd *pass;
      pass = getpwnam("nobody");
      if(!pass || setgid(pass->pw_gid) || setuid(pass->pw_uid))
	{
	  sendFileSignal(sigFile, "FAIL", timeout);
	  dtkMsg.add(DTKMSG_DEBUG,
		     "running pid %ld failed to "
		     "run as user \"nobody\".\n",
		     getpid());
	  return 1;
	}

      dtkMsg.add(DTKMSG_DEBUG,
		 "running pid %ld as user \"nobody\".\n",
		 getpid());
    }

  return 0;
}
