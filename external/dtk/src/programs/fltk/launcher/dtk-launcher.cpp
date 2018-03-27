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
#include <dtk.h>
#ifdef DTK_ARCH_LINUX
# ifndef __USE_UNIX98
#  define __USE_UNIX98
# endif
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <sys/wait.h>
#endif
#include <stdlib.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Scroll.H>

// from mainWindow.C
extern Fl_Window* make_mainWindow(int argc, char **argv);

// from launchButtons.C
extern int buildLaunchButtons(void);
extern int launch_buttonCount;


// from killButtons.C
extern int updateKillButtons(const char *prepath, const char *path,
			     const char *args, pid_t add_pid);

// Global flags
int running = 1;
int update_launcherButtons = 0;
int update_killButtons = 0;
char *listFile = NULL;


/*
FLTK options are:
 -d[isplay] host:n.n
 -g[eometry] WxH+X+Y
 -t[itle] windowtitle
 -n[ame] classname
 -i[conic]
 -fg color
 -bg color
 -bg2 color
*/

int Usage(void)
{
   fprintf(stdout,"\n"
	   "  Usage: dtk-launcher [-l|--list FILE] [FLTK_OPTIONS]\n"
	   "\n"
	   "   A GUI that launches programs and send signals to the programs\n"
	   " launched.  Kind of like a very crude GUI shell.\n");
   fprintf(stdout,
	   "   By default it looks in the current directory for executables\n"
	   " and sets up a launch button for each executable found and a\n"
	   " change directory button for each directory. When an executable\n" 
	   " is \"launched\" a \"kill\" button is added to the widgets.\n"
	   " When a \"kill\" button is pressed a SIGTERM signal is sent\n"
	   " to the \"launched\" program. The \"kill\" button is removed\n"
	   " when the corresponding program no longer exists.  If you\n"
	   " wish to run dtk-launcher in a different directory try\n"
	   " running: (cd DIR; dtk-launcher).\n");
   fprintf(stdout,
	   "  -l|--list FILE  build launcher buttons from a list of files in\n"
	   "                  FILE. It will change directory to where the\n"
	   "                  file is before running that file.\n"
	   "                  This mode will not build \"cd\" buttons.\n");

 dtkFLTKOptions_print(stdout);
 return 1;
}

int parseOptions(int argc, const char** argv)
{
  for(int i=1;i<argc;)
    {
      char *str;
      if((str = dtk_getOpt("-l","--list",argc,
			   (const char **) argv, &i)))
	{
	  listFile = str;
	}
      else if(!strcmp("-h",argv[1]) || !strcmp("--help",argv[1]))
	{
	  return Usage();
	}
      else
	{
	  fprintf(stdout,
		  "dtk-launcher Error: unknown option argument: %s\n",
		  argv[i]);
	  return Usage();
	}
    }
  return 0;
}

void sigChild_caughter(int sig)
{
  // This lets the child return it's status, so that they do not
  // become defunct processes.
  int status;
  wait(&status);
  update_killButtons = 1;
}

int main(int argc, char** argv)
{
  int fltk_argc;
  char **fltk_argv;
  int other_argc;
  char **other_argv;

  // dtkFLTKOptions_get will pull out FLTK options into its return pointer.
  if(dtkFLTKOptions_get(argc, (const char **) argv,
			&fltk_argc,  &fltk_argv,
			&other_argc, &other_argv)) return 1;
  if(parseOptions(other_argc, (const char **) other_argv)) return 1;

  dtkMsg.setPreMessage("%s%s%s",dtkMsg.color.vil,
		       argv[0], dtkMsg.color.end);

#if 0 /* test getting fltk option args */
  {
    char **c = fltk_argv;
    for(;*c;c++)
      printf("'%s' ",*c);
    printf("'%s'\n%d FLTK args\nLIST FILE=%s\n",
	   *c,fltk_argc, listFile);
    return 0;
  }
#endif
  char *path = getenv("PATH");
  if(!path)
    {
      dtkMsg.add(DTKMSG_ERROR, "Can't get PATH.\n");
      return 1;
    }
  char *Path = (char *) malloc(strlen(path) + strlen("PATH=.:") + 1);
  sprintf(Path,"PATH=.:%s",path);
  if(putenv(Path))
    {
      dtkMsg.add(DTKMSG_ERROR, "putenv(\"%s\") falied\n",Path);
      return 1;
    }

  // I don't know what Fl_Window will do with data from fltk_argv so I
  // will not free fltk_argv.

  Fl_Window* mainWindow = make_mainWindow(fltk_argc, fltk_argv);
  if(!(mainWindow)) return 1;

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
  struct sigaction sa;
  sa.sa_handler = sigChild_caughter;
  sigemptyset( &sa.sa_mask );
  sa.sa_flags = SA_RESTART;
  if( sigaction( SIGCHLD, &sa, NULL ) == -1 )
  {
    perror( "sigaction" );
    exit(1);
  }
#else
  sigset(SIGCHLD, sigChild_caughter);
#endif

  while (Fl::wait() && running)
    {
      if(update_launcherButtons)
	{
	  if(buildLaunchButtons())
	    return 1; // failure
	}
      if(update_killButtons)
	{
	  if(updateKillButtons(NULL, NULL, NULL, (pid_t) 0))
	    return 1; // failure
	}
    }

  return 0;
}
