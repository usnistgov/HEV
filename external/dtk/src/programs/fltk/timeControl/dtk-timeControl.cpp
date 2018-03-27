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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>

#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#define PROGRAM_NAME      "dtk-timeControl"
#define DEFAULT_SHM_NAME  "time"


// from file timeControl.C
extern int make_window(int argc, char **argv, char *winlabel);
extern Fl_Output *time_output;
extern Fl_Output *timeSec_output;
extern Fl_Button *pause_resume_Button;
extern int isPauseButton;

int running = 1;
dtkSharedTime *sharedTime;

static void catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,"caught signal %d: exiting.\n",sig);
  running = 0;
}

static int Usage(void)
{
  printf("\n"
	 "  Usage: %s [FILE_NAME] [-p|--pause] [FLTK_OPTIONS]\n",PROGRAM_NAME);
  printf("\n"
	 "   %s controls a dtkSharedTime object using a GUI.\n"
	 " dtkSharedTime shares a time offset between processes, and keeps\n"
	 " a time state such as \"running\" or \"paused\", so that you\n"
	 " may freeze time and things like that in your distributed\n"
	 " simulations. The default FILE_NAME is \"%s\"\n\n",
	 PROGRAM_NAME,DEFAULT_SHM_NAME);

  printf("   OPTIONS\n\n"
	 "  -p|--pause  start with time paused if this program creates the\n"
	 "              dtkSharedTime\n\n");

  dtkFLTKOptions_print(stdout);
  return 1;
}


int parseOptions(int argc, char **argv, char **name, int *pause)
{
  for(int i=1;i<argc;)
    {
      if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
	return Usage();
      else if(!strcmp("-p",argv[i]) || !strcmp("--pause",argv[i]))
	{
	  *pause = 1;
	  i++;
	}
      else if(!(*name))
	*name = argv[i++];
      else
	{
	  printf("\nbad option: %s\n",argv[i]);
	  return Usage();
	}
    }
  return 0;
}


int main(int argc, char **argv)
{
  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);

  int fltk_argc, other_argc;
  char **fltk_argv, **other_argv;

  if(dtkFLTKOptions_get(argc, (const char**) argv,
			&fltk_argc, &fltk_argv,
			&other_argc, &other_argv))
    return 1; // error

  char *name = NULL;
  int pause = 0;

  if(parseOptions(other_argc, other_argv, &name, &pause)) return 1;

  if(!name) name =  const_cast<char*>(DEFAULT_SHM_NAME);

  dtkSharedTime t(name, pause);
  if(t.isInvalid()) return 1;
  sharedTime = &t;

  free(other_argv);

  if(make_window(fltk_argc, fltk_argv, name)) return 1;

  signal(SIGINT,catcher);
  signal(SIGTERM,catcher);


  while(running)
    {
      long double sec = t.get();
      char str1[] = "                                              ";
      sprintf(&str1[strlen(str1)-12],"%.1Lf",sec);
      char str[] = "   0:00:00.0";
      int min = (int) (sec/60);
      sec -= min*60;
      int hour = min/60;
      min -= hour*60;
      str[11] = 060 + (int)((sec - ((int) sec))*10.0);
      str[9] =  060 + ((int) sec) - ((int)(sec/10.0))*10;
      str[8] =  060 + ((int) sec/10);
      str[6] = 060 + min - (min/10)*10;
      str[5] = 060 + min/10;
      if(hour > 0)
	{
	  str[3] = 060 + hour - (hour/10)*10;
	  if(hour > 9)
	    {
	      str[2] = 060 + hour/10 - (hour/100)*10;
	      if(hour > 99)
		{
		  str[1] = 060 + hour/100 - (hour/1000)*10;
		  if(hour > 999)
		    str[0] = 060 + hour/1000 - (hour/10000)*10;
		}
	    }
	}

      time_output->value(str);
      timeSec_output->value(&str1[strlen(str1)-12]);

      int i = t.isRunning();

      //printf("pid=%d isRunning=%d isPauseButton=%d\n",
      //     getpid(), i, isPauseButton);

      if((i == 1 && !isPauseButton) ||
	 (i == 0 && isPauseButton))
	{
	  // I call hide() and show() to force the pause_resume_Button
	  // Fl_Button to change its color and text.  I believe this
	  // to be a bug in FLTK.
	  pause_resume_Button->hide();
	  pause_resume_Button->do_callback(pause_resume_Button,
					   (void *) 1);
	  pause_resume_Button->show();
	}

      Fl::wait(0.1);
    }
  return 0;
}
