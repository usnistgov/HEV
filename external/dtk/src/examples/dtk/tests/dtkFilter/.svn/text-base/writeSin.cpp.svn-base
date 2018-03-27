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

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <dtk.h>

#define PERIOD  5.0f

int isRunning = 1;

void catcher(int sig)
{
  dtkMsg.add(DTKMSG_NOTICE,
	     "caught signal %d ... exiting.\n",
	     sig);
  isRunning = 0;
}


int main(int argc, char **argv)
{
  if(argc < 2)
    {
      printf("Usage: %s FILE0 [FILE1 ...]\n"
	     "  Writes a sine wave to the shared memory files listed.  The shared\n"
	     " memory file must be of size of one float.\n\n",
	     argv[0]);
      return 1;
    }

  dtkMsg.setPreMessage("%s%s%s",dtkMsg.color.tur, argv[0],dtkMsg.color.end);
  dtkSharedMem **shm = (dtkSharedMem **) dtk_malloc((argc-1)*sizeof(dtkSharedMem *));
  int i;
  for(i=0;i<argc-1;i++)
    shm[i] = NULL;
  for(i=0;i<argc-1;i++)
    {
      shm[i] = new dtkSharedMem(sizeof(float), argv[i+1]);
      if(shm[i]->isInvalid()) isRunning = 0;
    }

  if(isRunning)
    {
      signal(SIGINT,catcher);
      signal(SIGTERM,catcher);

      dtkRealtime_init(0.5);
      dtkTime time(0.0,1);
      
      while(isRunning)
	{
	  float x = (float) time.get();
	  x = sinf(x*2.0f*M_PI/PERIOD);
	  for(i=0;i<argc-1;i++)
	    if(shm[i]->write(&x))
	      {
		isRunning = 0;
		break; // error
	      }
	  dtkRealtime_wait();
	}

      dtkRealtime_close();
    }

    for(i=0;i<argc-1;i++)
      if(shm[i]) delete shm[i];

    return 0;
}
