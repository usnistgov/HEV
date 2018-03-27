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
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <FL/Fl.H>

#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# define snprintf _snprintf
#else
# include <strings.h>
#endif

#include "buttons.h"

#define PROGRAM_NAME   "dtk-buttons"
#define DEFAULT_NUMBEROFBUTTONS  8

dtkSharedMem *shm = NULL;
unsigned char *buffer = NULL;
unsigned char *buffer2 = NULL;
int num_bits; // number of bits in shared memory
// num_bits may be more than the number of buttons

int running = 1;
int returnVal = 0;

static void startUpError_bail(void)
{
  if(shm)
    delete shm;
  exit(1);
}


static void catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,"caught signal %d: exiting.\n",sig);
  running = 0;
}

static int Usage(void)
{
  dtkColor c;
  printf("\n"
	 "  Usage: %s [%sFILE_NAME%s] [-h|--help]\\\n"
	 "   [-N|--number NUM_BUTTONS] [-l|-labels LABEL0 LABEL1 ...]\\\n"
	 "   [FLTK_OPTIONS]\n"
	 "\n"
         "   Make some buttons that are connected to DTK\n"
	 " shared memory, or print to stdout if FILE_NAME\n"
	 " is not given.\n"
	 "\n"
	 "   OPTIONS\n"
	 "\n"
	 " %sFILE_NAME%s                  connect to DTK shared memory named FILE_NAME\n"
	 " -h|--help                      print this help\n"
	 " -l|--labels LABEL0 LABEL1 ...  label the buttons with the labels LABEL0 LABEL1 and so on\n"
         "                                -l|-labels must be the last option.\n"
	 " -N|--number NUM_BUTTONS        set up NUM_BUTTONS buttons\n"
	 "\n",
	 PROGRAM_NAME, c.tur,c.end, c.tur,c.end);
  dtkFLTKOptions_print(stdout);
  return 1;
}


static int parse_args(int argc, const char **argv,
		      int *numberOfButtons,
		      char **shm_name, int *labels_index)
{
  int i;

  for(i=1;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-N","--number",argc,argv,&i)))
	{
	  if((*numberOfButtons = atoi(str)) <= 0)
	    return Usage();
	}
      else if((str = dtk_getOpt("-l","--labels",argc,argv,&i)))
	{
	  // printf("got labels index = %d\n",i - 1);
	  *labels_index = i - 1;
	  return 0;
	}
      else if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
        return Usage();
      else
	*shm_name = (char *) argv[i++];
    }
  return 0;
}


int main(int argc, char **argv)
{
  char *shm_name=NULL;
  int numberOfButtons=-1, labels_index = -1;

  int fltk_argc;
  char **fltk_argv;
  int other_argc;
  char **other_argv;
  // dtkFLTKOptions_get will pull out FLTK options into its return pointer.
  if(dtkFLTKOptions_get(argc, (const char **) argv,
                        &fltk_argc,  &fltk_argv,
                        &other_argc, &other_argv)) return 1;

  if(parse_args(other_argc, (const char **) other_argv,
		&numberOfButtons, &shm_name, &labels_index))
    return 1;

  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);
#if 0
  {
    printf("got shm_name=\"%s\" numberOfButtons=%d\n"
	   "lables=",
	   shm_name, numberOfButtons);
    if(labels_index > -1)
      for(int i=labels_index;i<other_argc;i++)
	printf("\"%s\" ",other_argv[i]);
    printf("\n");
  }
#endif

  size_t size;

  if(shm_name)
    {
      shm = new dtkSharedMem(shm_name, 0);

      if(shm->isValid())
	{
	  size = shm->getSize();
	  if(numberOfButtons > (int) (size*8))
	    {
	      dtkMsg.add(DTKMSG_ERROR,
			 "The shared memory \"%s\" is"
			 " size %d which is not large enough for"
			 " %d buttons.\n",
			 shm_name, size, numberOfButtons);
	      startUpError_bail(); // error
	    }

	  if(numberOfButtons == -1)
	    numberOfButtons = size*8;
	}
      else // if(shm->isInvalid())
	{
	  if(numberOfButtons == -1)
	    numberOfButtons = DEFAULT_NUMBEROFBUTTONS;
	  size = numberOfButtons/8 +((numberOfButtons%8)?1:0);
	  shm = new dtkSharedMem(size, shm_name);
	  if(shm->isInvalid()) startUpError_bail(); // error
	}
    }
  else if(numberOfButtons == -1)
    {
      numberOfButtons = DEFAULT_NUMBEROFBUTTONS;
      size = numberOfButtons/8 +((numberOfButtons%8)?1:0);
    }
  else // if(!shm_name && numberOfButtons != -1)
    size = numberOfButtons/8 +((numberOfButtons%8)?1:0);

  if(numberOfButtons*8 != (int) size)
    {
      buffer2 = (unsigned char *) dtk_malloc(size);
      if(!buffer2) startUpError_bail();
    }

  num_bits = size*8; // number of bits in shared memory

  buffer = (unsigned char *) dtk_malloc(size);
  if(!buffer) startUpError_bail();

  struct Button *s = (struct Button *)
    dtk_malloc(sizeof(struct Button)*numberOfButtons);
  if(!s) startUpError_bail(); // error

  int i;
  for(i=0;i<numberOfButtons;i++)
    {
      s[i].start = 0;
      s[i].reset = 0;
      s[i].label = dtk_strdup("-----------------");
      if(!(s[i].label)) startUpError_bail(); // error
      snprintf(s[i].label,16,"-%d-",i);
    }

  if(labels_index > -1)
    {
      i = labels_index-1;
      char *str = dtk_getOpt("-l","--labels",other_argc,(const char **) other_argv,&i);
      if(str)
	snprintf(s[0].label,16,"%s",str);
      int j=1;
      for(; i<other_argc && j<numberOfButtons; i++)
	snprintf(s[j++].label,16,"%s",other_argv[i]);
    }

  if(shm) /*********** get initial values ************/
    {
      shm->read(buffer);
      for(i=0;i<numberOfButtons;i++)
	s[i].start = buffer[i/8] & (01 << i % 8);
    }

  char winLabel[64];
  if(shm_name)
    snprintf(winLabel, (size_t)63, "dtk-buttons: \"%s\"", shm_name);
  else
    snprintf(winLabel, (size_t)63, "dtk-buttons: stdout");

#if(0)
  printf("got shm_name=\"%s\" numberOfButtons=%d\n",
	 shm_name, numberOfButtons);

  for(i=0;i<numberOfButtons;i++)
    printf("button %d: label=\"%s\" start=%3.3d "
	   "reset=%3.3d \n",
	   i, s[i].label, s[i].start, s[i].reset);
#endif

  signal(SIGINT,catcher);
  signal(SIGTERM,catcher);

  if(make_window(numberOfButtons, size, s, (const char *) winLabel, fltk_argc, fltk_argv))
    running = 0;

  while (running && Fl::wait());

  if(shm)
    delete shm;
  return returnVal;
}
