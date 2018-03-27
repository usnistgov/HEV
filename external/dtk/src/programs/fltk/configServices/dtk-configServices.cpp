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
#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
# include <string.h>
#endif

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <FL/Fl.H>

// from mainWindow.cpp
extern int make_mainWindow(char *file, int argc, char **argv);
extern void activate_launch_button(pid_t pid);


// Global flags
int running = 1;
int exitStatus = 1;

static int print_Dirs(char *fileName)
{
  int count = 0;
  FILE *file = fopen(fileName, "r");
  if(!file)
    {
      dtkMsg.add(DTKMSG_ERROR,1,
		 "failed to open file \"%s\".\n",
		 fileName);
      return 1; // error
    }
#define BUFFSIZE  1000
  char str[BUFFSIZE];
  while(fgets(str,BUFFSIZE,file))
    {
      char *s = str;
      for(;*s == ' ' || *s == '\t';s++);
      if(*s != '#' && *s != '\n' && *s)
	{
	  char *argString[2];
	  argString[1] = const_cast<char*>("");
	  int i =0;
	  for(;i<2 && *s;i++)
	    {
	      for(;*s == ' ' || *s == '\t';s++);
	      argString[i] = s;
	      if(*s == '"')
		{
		  argString[i] = s+1;
		  for(s++;*s && *s != '"';s++);
		}
	      else
		for(s++;*s && *s != ' ' && *s != '\t';s++);
	      if(*s)
		{
		  *s = '\0';
		  s++;
		}
	    }
	  for(;*s && (*s == ' ' || *s == '\t');s++);
	  int installFlag = atoi(argString[0]);
	  if(count) printf(" ");
	  if(installFlag)
	    {
	      printf("%s", argString[1]);
	      count++;
	    }
	}
    }
  fclose(file);
  if(count) printf("\n");
  if(count) return 0; // got one or more
  else return 1; // didn't get any
}


static int Usage(int returnVal)
{
  printf("\n"
	 "  Usage: dtk-configServices FILE [-p] [FLTK_OPTIONS]\n"
	 "\n"
	 "   dtk-configServices is a GUI to configure DTK services.\n"
	 " If the exit status is zero than changes have been made to FILE,\n"
	 " or else no changes have been made.\n"
	 " Each line in LIST_FILE contains the following four strings:\n\n"
	 "  NAME          is the name of the service, like is900,\n"
	 "  DIRECTORY     is the directory where the service source is,\n"
	 "  CONFIG_SCRIPT is the program to run to configure"
	 " the service, and\n"
	 "  HELP_SCRIPT   is the program to run to get "
	 "information about the service.\n\n"
	 "  OPTION\n"
	 "  -p    print the installed directories from FILE to stdout\n\n");
  dtkFLTKOptions_print(stdout);
  return returnVal;
}

static void sigChild_caughter(int sig)
{
  // This lets the child return it's status, so that they do not
  // become defunct processes, and reactivate launch button.
  int status;
  pid_t pid = wait(&status);
  if(pid)
    {
      char procFile[40];
#ifdef DTK_ARCH_LINUX
      snprintf(procFile, 39,"/proc/%d", pid);
#else // IRIX and other
      snprintf(procFile, 39,"/proc/%10.10d", pid);
#endif
      //printf("file=%s line=%d procFile=%s\n",__FILE__,__LINE__, procFile);
      int fd = open(procFile, O_RDONLY);
      if(fd == -1)
	activate_launch_button(pid);
      else
	close(fd);
    }
}

int main(int argc, char** argv)
{
  {
    int i;
    for(i=0;i<argc;i++)
      if(!strcmp("--help", argv[i]))
	return Usage(1);
  }

  dtkMsg.setPreMessage("%s%s%s",dtkMsg.color.vil,
		       argv[0], dtkMsg.color.end);
  int fltk_argc, other_argc;
  char **fltk_argv, **other_argv;

  if(dtkFLTKOptions_get(argc, (const char**) argv,
                        &fltk_argc, &fltk_argv,
                        &other_argc, &other_argv))
    return 1; // error

  if(other_argc == 3 && !strcmp("-p",argv[2]))
    return print_Dirs(other_argv[1]);

  if(other_argc != 2)
  return Usage(1);

  if(make_mainWindow(other_argv[1], fltk_argc, fltk_argv))
    return 1;

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
    ;

  return exitStatus;
}
