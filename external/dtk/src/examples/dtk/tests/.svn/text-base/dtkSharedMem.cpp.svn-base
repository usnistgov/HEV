/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
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
#include <sys/types.h>
#include <signal.h>
#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
#  include <process.h>
#else
#  include <unistd.h>
#endif

int running = 1;

void catcher(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE,"caught signal %d exiting ...\n");
  running = 0;
}


int main(int argc, char **argv)
{
  if(argc < 2)
    return dtkMsg.add(DTKMSG_ERROR,0,1,"Usage: %s SEG_NAME\n",
		      argv[0]);

  float x[2] = { 0.0f, 0.0f };

  dtkSharedMem shm(sizeof(float)*2, argv[1], x);
  if(shm.isInvalid()) return 1;

  signal(SIGINT, catcher);
  signal(SIGTERM, catcher);
#ifndef DTK_ARCH_WIN32_VCPP
  signal(SIGQUIT, catcher);
#endif

  while(running)
    {
      if(shm.read(x)) return 1;
      x[0] += 1.0f;
      x[1] += 2.0f;
      if(shm.write(x)) return 1;
      printf("pid=%d f=%g %g\n", getpid(), x[0], x[1]);
      sleep(1);
    }

  return 0;
}
