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

#include <signal.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#else
# include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <sys/wait.h>
#endif
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

#ifdef DTK_ARCH_WIN32_VCPP
# define open     _open
# define close    _close
#endif

// from mainWindow.C
extern Fl_Window* make_mainWindow(int argc, char **argv);
extern Fl_Scroll *launch_scroll;
extern Fl_Button* killButton;
extern Fl_Box* killLabel;

// from launchButtons.C
extern int buildLaunchButtons(void);
extern pid_t launch_pid;
extern char *launch_procfile;
extern char *killLabelString;
extern char *killButtonString;

// Global flags
int running = 1;
int update_launcherButtons = 0;

char *listFile = NULL;

int switchLaunchToKill = 0;
int switchKillToLaunch = 0;


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

static char *fltk_args2[] =
{
  const_cast<char*>("-d"), const_cast<char*>("-g"), const_cast<char*>("-t"), const_cast<char*>("-n"), const_cast<char*>("-f"), const_cast<char*>("-b"), NULL
};

static char *fltk_args1[] =
{
  const_cast<char*>("-i"), NULL
};

static void Usage(void)
{
   printf("\n"
	  "  Usage: dtk-demoRunner [-l|--list FILE] [FLTK_OPTIONS]\n"
	  "\n"
	  "   A GUI that launches one program at a time.\n");
   printf("   By default it looks in the current directory for executables\n"
	  " and sets up a launch button for each executable found and a\n"
	  " change directory button for each directory. When an executable\n" 
	  " is \"launched\" a \"Kill\" button created. When a \"Kill\" button\n"
	  " is pressed a SIGTERM signal is sent to the \"launched\" program.\n"
	  " The \"Kill\" button is removed when the corresponding program\n"
	  " process no longer exists.  If you wish to run dtk-demoRunner in a\n"
	  " different directory try running: (cd DIR; dtk-demoRunner).  See\n"
	  " also `dtk-launcher --help'.\n\n");

   printf("   OPTIONS\n\n"
	  "  -l|--list FILE  build launcher buttons from a list of files in\n"
	  "                  FILE. It will change directory to where the\n"
	  "                  file is before running that file.\n"
	  "                  This mode will not build \"cd\" buttons.\n\n");
   printf("   FLTK_OPTIONS\n\n"
	  "   -d[isplay] host:n.n\n"
	  "   -g[eometry] WxH+X+Y\n"
	  "   -t[itle] windowtitle\n"
	  "   -n[ame] classname\n"
	  "   -i[conic]\n"
	  "   -fg color\n"
	  "   -bg color\n"
	  "   -bg2 color\n"
	  "\n");

}

// parseOptions() returns the char **argv options for FLTK to parse.
char **parseOptions(int argc, const char** argv,
		    int *fltk_argc)
{
  *fltk_argc = 1;
  char **fltk_argv = (char **)
    malloc(sizeof(char *)*2);
  fltk_argv[0] = (char *) argv[0];

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
	  Usage();
	  return NULL;
	}
      else // pull out FLTK options
	{
	  int got_one = 0;
	  char **c;
	  for(c = fltk_args2;*c && (argv[i+1]);c++)
	    {
	      if(!strncmp(argv[i], *c, strlen(*c)))
		{
		  (*fltk_argc) += 2;
		  fltk_argv = (char **)
		    realloc(fltk_argv,
			    sizeof(char *)*((*fltk_argc)+1));
		  fltk_argv[(*fltk_argc)-2] = (char *) argv[i];
		  fltk_argv[(*fltk_argc)-1] = (char *) argv[i+1];
		  i += 2;
		  got_one++;
		  break;
		}
	    }

	  for(c = fltk_args1;!got_one && *c;c++)
	    {
	      if(!strncmp(argv[i], *c, strlen(*c)))
		{
		  (*fltk_argc)++;
		  fltk_argv = (char **)
		    realloc(fltk_argv,
			    sizeof(char *)*((*fltk_argc)+1));
		  fltk_argv[(*fltk_argc)-1] = (char *) argv[i];
		  i++;
		  got_one++;
		  break;
		}
	    }
	  if(!got_one)
	    {
	      fprintf(stderr,
		      "dtk-demoRunner Error: unknown option argument: %s\n",
		      argv[i]);
	      Usage();
	      return NULL;
	    }
	}
    }
  fltk_argv[(*fltk_argc)] = NULL;
  return fltk_argv; // success
}

void sigChild_caughter(int sig)
{
  // This lets the child return it's status, so that they do not
  // become defunct processes.
  int status;
  wait(&status);
  if(launch_pid)
    {
      int fd = open(launch_procfile, O_RDONLY);
      if(fd == -1)
	{
	  launch_pid = (pid_t) 0;
	  switchKillToLaunch = 1;
	}
      else
	close(fd);
    }
}

int main(int argc, char** argv)
{
  int fltk_argc;
  // parseOptions will pull out FLTK options into its return pointer.
  char **fltk_argv = parseOptions(argc, (const char **) argv, &fltk_argc);
  if(!(fltk_argv)) return 1;

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
      if(update_launcherButtons &&
	 buildLaunchButtons())
	return 1; // failure

      if((switchLaunchToKill))
	{
	  //printf("line=%d file=%s\n",__LINE__,__FILE__);
	  launch_scroll->hide();
	  killButton->label(killButtonString);
	  killButton->show();
	  killLabel->label(killLabelString);
	  killLabel->show();
	  switchLaunchToKill = 0;
	}
      else if((switchKillToLaunch))
	{
	  //printf("line=%d file=%s\n",__LINE__,__FILE__);
	  killButton->hide();
	  killLabel->hide();
	  launch_scroll->show();
	  switchKillToLaunch = 0;
	}
    }

  return 0;
}
