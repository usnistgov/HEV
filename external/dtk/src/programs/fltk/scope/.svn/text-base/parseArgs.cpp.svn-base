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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "scope.h"


static dtkSharedMem **cleanupSharedMem(dtkSharedMem **shm)
{
  dtkSharedMem **s = shm;
  for(;*shm;shm++)
    delete *shm;
  if(s)
    free(s);
  return NULL;
}


// Returns a NULL terminated array.
dtkSharedMem **parseArgs(int argc, char **argv)
{
  if(argc < 2) return Usage();
  int i;

  // parse cases that exit.
  for(i=1;i<argc;i++)
    if(!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h"))
      return Usage();

  int num_shm = 0;
  dtkSharedMem **shm = NULL;

  for(i=1;i<argc;)
    {
      if((dtk_getOpt("-s","--scale",argc,(const char  **) argv,&i)))
	{
	   if(i + 2 >= argc)
	    {
	      printf("Bad option: -s|--scale\n\n");
	      cleanupSharedMem(shm);
	      return Usage();
	    }
	  else
	    i += 3; // skip the INDX MIN MAX arg values
	}
      else if(!strcmp("-p",argv[i]) || !strcmp("--poll",argv[i]))
	{
	  queue_read = 0;
	  i++;
	}
      else
	{
	  shm = (dtkSharedMem **)
	    dtk_realloc(shm, sizeof(dtkSharedMem *)*(num_shm+2));
	  shm[num_shm] = new dtkSharedMem(argv[i]);
	  shm[num_shm+1] = NULL;

	  if(shm[num_shm]->isInvalid())
	    return cleanupSharedMem(shm);

	  if(shm[num_shm]->getSize()/sizeof(TYPE) < (size_t) 1)
	    {
	      dtkMsg.add(DTKMSG_ERROR, "shared memory \"%s\" is "
			 "smaller than a %s.\n",
			 shm[num_shm]->getName(),
			 TYPE_STRING);
	      return cleanupSharedMem(shm);
	    }
	  if(shm[num_shm]->timeStamp())
	    return cleanupSharedMem(shm);
	  shm[num_shm]->setAutomaticByteSwapping(sizeof(TYPE));

	  num_shm++;
	  i++;
	}
    }

  if(queue_read)
    {
      for(i=0;i<num_shm;i++)
	{
	  shm[i]->stopFilters();
	  if(shm[i]->queue())
	    return cleanupSharedMem(shm);
	}
    }

  return shm;
}


// This will work for a TYPE == double or float, but not long double.
int parse_scaleOptions(MainWin *mainWin, int argc, char **argv)
{
  int i;
  for(i=1;i<argc;)
    {
      char *shmfilename;
      if((shmfilename = dtk_getOpt("-s","--scale", argc, 
				   (const char  **) argv,&i)))
	{
	  int index = atoi(argv[i++]);
	  if(index < 0 || index > 1000000)
	    {
	      dtkMsg.add(DTKMSG_ERROR,"Bad option: -s|--scale\n\n");
	      Usage();
	      return 1;
	    }
	  errno = 0;
	  TYPE min = (TYPE) strtod(argv[i++], NULL);
	  if(errno == ERANGE)
	    {
	      dtkMsg.add(DTKMSG_ERROR,"Bad option: -s|--scale\n\n");
	      Usage();
	      return 1;
	    }
	  errno = 0;
	  TYPE max = (TYPE) strtod(argv[i++], NULL);
	  if(errno == ERANGE)
	    {
	      dtkMsg.add(DTKMSG_ERROR,"Bad option: -s|--scale\n\n");
	      Usage();
	      return 1;
	    }
	  if(max == min)
	    {
	      dtkMsg.add(DTKMSG_ERROR,"Bad option: -s|--scale  MIN=MAX\n\n");
	      Usage();
	      return 1;
	    }
	  shmfilename = dtkSharedMem_getFullPathName(shmfilename);

	  dtkMsg.add(DTKMSG_DEBUG, "scale: shared mem file %s%s%s[%s%d%s] min=%s%g%s max=%s%g%s.\n",
		     dtkMsg.color.tur, shmfilename, dtkMsg.color.end,
		     dtkMsg.color.yel, index, dtkMsg.color.end,
		     dtkMsg.color.yel, min, dtkMsg.color.end,
		     dtkMsg.color.yel, max, dtkMsg.color.end);

	  Plot **plot = mainWin->canvas->plot;
	  for(;*plot;plot++)
	    {
	      if(!strcmp(shmfilename,(*plot)->shm->getName()))
		{
		  if(index >= (*plot)->num_floats)
		    {
		      dtkMsg.add(DTKMSG_ERROR, "DTK shared memory file %s%s%s "
				 "(size = %dbytes = %.1g %ss)\n"
				 " is not large "
				 "enough for %s array index=%d.\n",
				 dtkMsg.color.tur, shmfilename, dtkMsg.color.end,
				 (*plot)->shm->getSize(),
				 ((float)(*plot)->shm->getSize())/((float) sizeof(TYPE)),
				 TYPE_STRING, TYPE_STRING, index);
		      return 1;
		    }
		  if(index < (*plot)->num_channels)
		    {
		      (*plot)->channel[index]->setScale(mainWin->canvas, min, max);
		    }
		  // else // figure out how to add channels or some way to
		  // deal with this.

		  break;
		}
	    }
	  if(!(*plot))
	    {
	      dtkMsg.add(DTKMSG_ERROR, "not connected to DTK shared memory file %s%s%s\n"
			 " Bad option: -s|--scale\n\n",
			 dtkMsg.color.tur, shmfilename, dtkMsg.color.end);
	      Usage();
	      return 1;
	    }
	}
      else
	i++;
    }

  return 0;
}
