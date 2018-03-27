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
/* This file was originally written by Eric Tester.  Many
 * modifications have been made by Lance Arsenault.
 */

#include <dtk.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Light_Button.H>

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
# include <string.h>
#endif

#define PROGRAM_NAME  "dtk-gnomonDisplay"

// from mainWindow.C
extern void update_xyzhpr_Output(Fl_Widget *who, void *data);
extern int make_mainWindow(char *sharedMemName,
			   int fltk_argc, char **fltk_argv);
extern int getIndex(void);
extern Fl_Light_Button *connect_Button;
extern Fl_Light_Button *continous_Button;
extern Fl_Window       *mainWindow;

int running = 1;

static float *shmData = NULL;
static size_t shmData_size = 0;
static size_t shm_size = 0;
static dtkSharedMem *shm = NULL;

double scale[6]  = {1.0,1.0,1.0,1.0,1.0,1.0};
double offset[6] = {0.0,0.0,0.0,0.0,0.0,0.0};

void window_callback (Fl_Widget*)
{
	running = 0;
}

int readIn_xyzhpr(double *xyzhpr) // return 6 doubles
{
  if(shm)
    {
      if(shm->read(shmData)) return 1; // error
      int i = getIndex();
      int num = shm_size/sizeof(float) - i;
      if(num > 6) num = 6;
      else if(num < 0) num = 0;
      int j;
      for(j=5;j>=num;j--)
	{
	  xyzhpr[j] = offset[j];
	  //printf("xyzhpr[%d]=%g\n",j,xyzhpr[j]);
	}
      for(j=0;j<num;j++)
	{
	  xyzhpr[j] = shmData[i+j];
	  //printf("xyzhpr[%d]=%g\n",j,xyzhpr[j]);
	}
      return 0; // success
    }
  else
    {
      int j;
      for(j=0;j<6;j++)
	xyzhpr[j] = 0.0;
      return 0;
    }
}

int connectShm(const char *sharedMemName)
{
  static int was_continous = 1;
  if(!sharedMemName || sharedMemName[0] == '\0')
    {
      // disconnecting
      if(shm)
	{
	  delete shm;
	  shm = NULL;
	  was_continous = continous_Button->value();
	}
      continous_Button->value(0);
      connect_Button->value(0);
      return 0;
    }
  if(shm)
    {
      delete shm;
      was_continous = continous_Button->value();
    }
  shm = new dtkSharedMem(sharedMemName, 0);
  if(!shm || shm->isInvalid())
    {
      shm_size = 0;
      connect_Button->value(0);
      continous_Button->value(0);
      fl_message("WARNING: Failed to "
		 "connect to DTK Shared Memory "
		 "\"%s\".\n", sharedMemName);
      return 1; // error
    }
  shm->setAutomaticByteSwapping(sizeof(float));
  shm_size = shm->getSize();
  if(shm_size < sizeof(float))
    {
      connect_Button->value(0);
      continous_Button->value(0);
      dtkMsg.add(DTKMSG_WARN,
		 "DTK sharedMem \"%s\" is "
		 "smaller than a float.\n", sharedMemName);
      fl_message("WARNING: DTK shared memory "
		 "\"%s\" is smaller than a float.\n",
		 sharedMemName);
      delete shm;
      shm = NULL;
      return 1;
    }
  connect_Button->value(1);
  continous_Button->value(was_continous ||
			  continous_Button->value());
  if(shmData_size < shm_size)
    {
      shmData_size = shm_size;
      shmData = (float *) realloc(shmData, shmData_size);
    }
  int num = shmData_size/sizeof(float);
  for(int i=0;i<num;i++)
    shmData[i] = offset[i];

  return 0; // success
}
  

// signal catcher
void catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,
	     "caught signal %d: exiting.\n",sig);
  running = 0;
}

// prints out options for command line
int Usage(int returnVal)
{
  printf("\n"
	 "Usage: %s [FILE_NAME]|[-s|--scale x y z h p r]|\n"
	 "   [-o|--offset x y z h p r]|[-h|--help]\n\n"
	 "  The dtk-Gnomon Graphical Display visually interperets\n"
	 "data in DTK shared memory as x, y, z coordinates and\n"
	 "heading (H), pitch (P), roll (R) by displaying a moving\n"
	 "Gnomon.  If FILE_NAME is given it connects to DTK\n"
	 "shared memory file FILE_NAME at startup.\n"
	 "The offset is added to the data after it is scaled. scale\n"
	 "and offset only effect the graphical display. For more\n"
	 "information run it.  \n\n",
	 PROGRAM_NAME);
  return returnVal;
}

// does stuff with the command line arguments
static int do_args(int argc, char **argv, char **name)
{
  for(int i=1;i<argc;)
    {
      if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
	{
	  return Usage(1);
	}
      else if (!strcmp("-s",argv[i]) || !strcmp("--scale",argv[i]))
	{
	  i++;

	  if(argc-i >= 6)
	    {
	      errno = 0;
	      for(int j=0;j<6;j++)
		{
		  scale[j] = strtod(argv[i++],NULL);
		  if(errno || scale[j] == 0.0)
		    {
		      dtkMsg.add(DTKMSG_ERROR,1,
				 "bad scale value from \"%s\".\n",
				 argv[i-1]);
		      return Usage(1);
		    }
		}
	    }
	  else
	    {
	      dtkMsg.add(DTKMSG_ERROR,
			 "bad option \"%s\".\n",
			 argv[i-1]);
	      return Usage(1);
	    }
	}
      else if (!strcmp("-o",argv[i]) || !strcmp("--offset",argv[i]))
	{
	  i++;

	  if(argc-i >= 6)
	    {
	      errno = 0;
	      for(int j=0;j<6;j++)
		{
		  offset[j] = strtod(argv[i++],NULL);
		  if(errno)
		    {
		      dtkMsg.add(DTKMSG_ERROR,1,
				 "bad offset value from \"%s\".\n",
				 argv[i-1]);
		      return Usage(1);
		    }
		}
	    }
	  else
	    {
	      dtkMsg.add(DTKMSG_ERROR,
			 "bad option \"%s\".\n",
			 argv[i-1]);
	      return Usage(1);
	    }
	}
      else
	{
	  *name = argv[i++];
	}
    }

#if 0
  printf("scale= %g %g %g %g %g %g\n",
	 scale[0],scale[1],scale[2],
	 scale[3],scale[4],scale[5]);

  printf("offset= %g %g %g %g %g %g\n",
	 offset[0],offset[1],offset[2],
	 offset[3],offset[4],offset[5]);
#endif

  return 0;
}

int main (int argc, char** argv)
{
  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);

  int fltk_argc;
  char **fltk_argv;
  int other_argc;
  char **other_argv;
  // dtkFLTKOptions_get will pull out FLTK options into its return pointer.
  if(dtkFLTKOptions_get(argc, (const char **) argv,
                        &fltk_argc,  &fltk_argv,
                        &other_argc, &other_argv)) return 1;

  char *sharedMemName = NULL;
  if(do_args(other_argc, other_argv, &sharedMemName)) return 1;

  shmData = (float *) dtk_malloc(shmData_size = 6*sizeof(float));

  if(make_mainWindow(sharedMemName, fltk_argc, fltk_argv))
    return Usage(1);

	mainWindow->callback( window_callback );

  signal(SIGINT, catcher);
  signal(SIGTERM, catcher);  

#ifdef DTK_ARCH_WIN32_VCPP
  while(running && Fl::wait())
    {
      if(continous_Button->value() &&
	 mainWindow->visible_r())
	{
//	  while (Fl::wait(0));
	  update_xyzhpr_Output(NULL, NULL);
	}
#else
  while(running)
    {
      if(continous_Button->value() &&
	 mainWindow->visible_r())
	{
	  while (Fl::wait(0));
	  update_xyzhpr_Output(NULL, NULL);

#ifdef DTK_ARCH_LINUX /* It does not seem to sync to the refresh rate on
              *  GNU/Linux, so I'll sleep a little to stop CPU
              *  spinning. */
	  usleep(50000);
#endif
	}
      else
	{
	  while(running &&
		(
		 (Fl::wait() && !continous_Button->value())
		 ||
		 !mainWindow->visible_r()
		 )
		)
	    ;
	}
#endif
    }
  return 0;
}
  
