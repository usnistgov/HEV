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
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#ifndef TYPE
# define TYPE  float
# define TYPE_STRING  "float"
# define PROGRAM_NAME   "dtk-noiseFloats"
#else /* TYPE = double */
# define TYPE_STRING  "double"
# define PROGRAM_NAME   "dtk-writeDoubles"
#endif

static int running = 1;

static int Usage(void)
{
  printf("\n"
	 " Usage: %s SHM_FILE [-h|--help] \n"
	 "     [-n|--number NUM]\n\n"
	 "  Write small random numbers to the DTK shared memory file named SHM_FILE.\n"
	 " This is a very very simple program. The idea is to "
	 "edit the source\n"
	 " (if you can find: \"%s\") and make it do what you want.\n"
	 "\n"
	 "   OPTIONS\n"
	 "\n"
	 " -h|--help         print this help and than exit\n"
	 " -n|--number NUM   the size of the shared memory is NUM %ss\n\n",
	 PROGRAM_NAME, __FILE__, TYPE_STRING);
  return 1;
}

static void caught(int sig)
{ 
  dtkMsg.add(DTKMSG_INFO, "caught signal %d ... Exiting\n",sig);
  running=0;
}

static int do_args(int argc, const char **argv, char **name,
			    int *num_floats)
{
  for(int i=1;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-n","--number",argc,argv,&i)))
        *num_floats=atoi(str);
      else if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help"))
        return Usage();
      else
        *name = (char *) argv[i++];
    }
  if(!(*name)) return Usage();

  return 0;
}


int main(int argc, char **argv)
{
  if(argc < 2) return Usage();

  int num_floats=0;
  char *name=NULL;

  if(do_args(argc, (const char **) argv, &name,
	     &num_floats)) return 1;

#if(0)
  {
    printf("got name=%s  num_floats=%d\n",
	   name, num_floats);
    return 0;
  }
#endif

  dtkMsg.setPreMessage(PROGRAM_NAME);

  dtkSharedMem *shm = new dtkSharedMem(name, 0);

  if(shm->isValid())
    num_floats = shm->getSize()/sizeof(TYPE);
  else if(num_floats == 0)
    {
      Usage();
      dtkMsg.add(DTKMSG_ERROR, "Try the --number option.\n");
      delete shm;
      return 1;
    }
  else
    {
      delete shm;
      shm = new dtkSharedMem(sizeof(TYPE)*num_floats, name);
      if(shm->isInvalid()) { delete shm; return 1; }
    }

  TYPE *val = (TYPE *) dtk_malloc((num_floats+1)*sizeof(TYPE));
  if(!val) { delete shm; return 1; }

  int i;

  signal(SIGINT,caught);
#ifndef DTK_ARCH_WIN32_VCPP
  signal(SIGQUIT,caught);
#endif
  signal(SIGTERM,caught);

  //printf("line=%d file=%s\n",__LINE__,__FILE__);

  dtkTime time(0.0, 1);

  while(running)
    {
      for(i=0;i<(num_floats+1);i++)
        val[i] = (rand()%1000)/100000.0;

      TYPE t = (TYPE) time.get();
      /*
      for(i=0;i<num_floats;i++)
	val[i] = (TYPE) sin(0.3*t + i*6.28/num_floats);
      for(i=3;i<num_floats;i++)
	val[i] *= (TYPE) 180.0;
    */
      if(shm->write(val)) running = 0;
      usleep(30000);
    }

  delete shm;

  return 0;
}
