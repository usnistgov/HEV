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
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <dtk.h>
#ifdef DTK_ARCH_IRIX
#  include <strings.h>
#endif

#ifdef DTK_ARCH_WIN32_VCPP
# define bzero(x,y)  memset((x), 0, (y))
#endif

const char *PROGRAM_NAME = "dtk-writeStrings";

int running = 1;
unsigned int flags = 0;

dtkSharedMem *shm = NULL;

int Usage(void)
{
  printf("\n"
         "  Usage: %s SIZE SHM_FILE\n"
         "\n"
         "   Write to DTK shared memory file, SHM_FILE, from standard input.\n"
         "\n",
         PROGRAM_NAME);
  return 1;
}


void sig_catcher(int sig)
{
  if(shm) delete shm;
#ifndef DTK_ARCH_WIN32_VCPP
  if(sig == SIGPIPE) dtkMsg.add(DTKMSG_INFO, "\nBroken pipe ... exiting.\n");
  else
#endif
  dtkMsg.add(DTKMSG_INFO, "\ncaught signal %d exiting\n",sig);
  exit(0);
}

int main(int argc, char **argv)
{
  if(argc < 3)
    return Usage();

  size_t size = strtoul(argv[1],NULL,10);

  dtkMsg.setPreMessage(PROGRAM_NAME);

  shm = new dtkSharedMem(size,argv[2]);
  if(shm->isInvalid()) { delete shm; return 1; }

  char *buf = (char *) dtk_malloc(size);
  if(!buf) return 1;

  signal(SIGINT, sig_catcher);
  signal(SIGTERM, sig_catcher);
#ifndef DTK_ARCH_WIN32_VCPP
  signal(SIGQUIT, sig_catcher);
  signal(SIGPIPE, sig_catcher);
#endif

  while(1)
    {
      size_t i = 0;
      bzero(buf,size);
      int c = ' ';
      for(;i<size && c != '\n' && c != EOF;i++)
	buf[i] = c = getchar();
      buf[i-1] = '\0';
      if(shm->write(buf)) break;
      if(c == EOF) break;
    }

  if(shm) delete shm;
  return 0;
}
