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
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dtk.h>
#include <limits.h>

#if defined(DTK_ARCH_IRIX) || defined(DTK_ARCH_DARWIN) || defined(DTK_ARCH_CYGWIN)
# define ULONG_MAX ((unsigned long) -1)
#endif

#define PROGRAM_NAME  "dtk-loadFilter"

int Usage(void)
{
  printf("  Usage: "PROGRAM_NAME" SHM_FILE [-n|--name NAME] [-p|--pid PID] FILTER_FILE [FILTER_ARGS]\n"
	     "\n"
	 "   Load the DSO r/w filter from the file FILTER_FILE into dtkSharedMem objects associated with the\n"
	 " shared memory file SHM_FILE.\n"
	 "\n"
	 "  -n|--name NAME  name the filter NAME. Each loaded filter in a given associated with a given shared\n"
	 "                  memory file must have a unique name.  The default name will be set to FILTER_FILE.\n"
	 "  -p|--pid PID    load the filter into the dtkSharedMem objects in the process with process ID PID\n"
	 "\n");
      return 1; // error
}


int main(int argc, char **argv)
{
  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);

  int i;
  for(i=1;i<argc;i++)
    if(!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help"))
      return Usage();

  if(argc < 3)
    return Usage();

  char *dso_file = NULL;
  char **dso_argv = NULL;
  int dso_argc = 0;
  char *filter_name = NULL;
  pid_t pid = DTK_ALL_PID;

  for(i=2;i<argc;)
    {
      char *opt = dtk_getOpt("-n","--name",argc,(const char **)argv,&i);
      if(opt)
	filter_name = opt;
      else if((opt = dtk_getOpt("-p","--pid",argc,(const char **)argv,&i)))
	{
	  pid = (pid_t) strtoul(opt, NULL, 10);
	  if(pid == (pid_t) ULONG_MAX)
	    {
	      printf("bad \"-p|--pid PID\" option.\n");
	      return Usage();
	    }
	}
      else
	break;
    }

  if(i<argc)
    dso_file = argv[i++];
  else
    return Usage();

  if(i<argc)
    {
      dso_argv = &(argv[i]);
      dso_argc = argc-i;
    }

  dtkSharedMem shm(argv[1]);
  if(shm.isInvalid()) return 1;

  // This will force the user to give the filter a name if this has
  // been loaded already.
  if(!filter_name) filter_name = dso_file;

  return - shm.loadFilter(dso_file, dso_argc, (const char **) dso_argv, pid, filter_name);
}
