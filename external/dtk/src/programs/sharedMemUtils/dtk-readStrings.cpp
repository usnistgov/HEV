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
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

const char *PROGRAM_NAME = "dtk-readStrings";

int running = 1;
unsigned int flags = 0;

#define COUNT 01
#define QUEUE 02
#define TIME  04

int Usage(void)
{
  printf("\n"
         "  Usage: %s SHM_FILE [-c|--count] [-q|--queue] [-t|--time-stamp]|[-h|--help]\n"
         "\n"
         "   Read from DTK shared memory than print (ascii) data to standard out.\n"
	 " SHM_FILE is the name the DTK shared memory segment to read from.\n"
         "\n",
         PROGRAM_NAME);
  return 1;
}

int parse_options(int argc, char **argv, char **name)
{
  int i;
  for(i=1;i<argc;i++)
    {
      if(!strcmp(argv[i],"--help"))
	return Usage();
      else if(!strcmp(argv[i],"--count"))
	flags |= COUNT;
      else if(!strcmp(argv[i],"--queue"))
	flags |= QUEUE;
      else if(!strcmp(argv[i],"--time-stamp"))
	flags |= TIME;
      else if(argv[i][0] == '-' && argv[i][1] != '-')
	{
	  // parse options grouped like: -qt = -q -t .
	  char *s = &(argv[i][1]);
	  for(;*s;s++)
	    {
	      switch (*s)
		{
		case 'c':
		  flags |= COUNT;
		  break;
		case 'q':
		  flags |= QUEUE;
		  break;
		case 't':
		  flags |= TIME;
		  break;
		case 'h':
		default:
		  return Usage();
		  break;
		}
	    }
	}
      else
	{
	  *name = argv[i];
	}
    }

  if(!(*name))
    return Usage();

  return 0;
}

void sig_catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO, "caught signal %d ... exiting\n",sig);
  running = 0;
}

int main(int argc, char **argv)
{
  char *name = NULL;

  if(parse_options(argc,argv, &name))
    return 1;

  dtkMsg.setPreMessage(PROGRAM_NAME);

  dtkSharedMem shm(name);
  if(shm.isInvalid()) return 1;

  size_t size = shm.getSize();
  char *buf = (char *) dtk_malloc(size+1);
  if(!buf) return 1;
  buf[size] = '\0';

  if(flags & QUEUE)
    if(shm.queue()) return 1;

  if(flags & COUNT)
    if(shm.writeCount()) return 1;

  if(flags & TIME)
    if(shm.timeStamp()) return 1;

  signal(SIGINT, sig_catcher);
  signal(SIGTERM, sig_catcher);

  while(running)
    {
      int i=2;

      if(flags & QUEUE)
	{
	  if((i = shm.qread(buf)) < 0) return 1;
	}
      else
	if(shm.read(buf)) return 1;

      if(i > 0)
	{
	  // strip ending '\n' off if there is one.
	  size_t len = strlen(buf);
	  len = (len > size) ? size:len;
	  char *s = buf;
	  for(;*s && *s != '\n';s++);
	  *s = '\0';

	  if(flags & COUNT)
	    printf("%lld ",shm.getWriteCount());
	  if(flags & TIME)
	    printf("%.21Lg ", shm.getTimeStamp());

	  printf("%s\n",buf);
	}

      if(!(flags & QUEUE))
	usleep(200000);
      else if(i < 2) // IS QUEUED
	usleep(10000);  
    }
  return 0;
}
