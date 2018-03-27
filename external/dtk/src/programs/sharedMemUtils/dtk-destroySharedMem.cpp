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
#include <stdio.h>
#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#define PROGRAM_NAME  "dtk-destroySharedMem"

static int Usage(void)
{
  dtkColor c(stdout);
  printf("\n"
	 " Usage: "PROGRAM_NAME" [%sSHM_FILE ...%s] [-r|--recursive] | [-h|--help]\n"
	 "\n"
	 "   Destroy a DTK shared memory file.  %sSHM_FILE ...%s is the DTK shared memory\n"
	 " file names. The environment variable %sDTK_SHAREDMEM_DIR%s can be used to\n"
	 " select the shared memory directory the that %sSHM_FILE%s may be in when the\n"
	 " full path is not given.  The default shared memory directory,\n"
	 " %s%s%s, will be used if %sDTK_SHAREDMEM_DIR%s is not set.\n",
	 c.vil,c.end, c.vil,c.end, c.tur,c.end,
	 c.vil,c.end, c.grn,DTK_DEFAULT_SHAREDMEM_DIR,c.end,
	 c.tur,c.end);

  printf("\n"
	 "   -h|--help        print this help and exit\n"
	 "   -r|--recursive   destroy all shared memory files in a directory\n");

  if(getenv("DTK_SHAREDMEM_DIR"))
    printf("\n"
	   "   You currently have %sDTK_SHAREDMEM_DIR%s set to \"%s%s%s\".\n"
	   "\n",
	   c.tur,c.end, c.grn,getenv("DTK_SHAREDMEM_DIR"),c.end);
  else
    printf("\n"
	   "   %sDTK_SHAREDMEM_DIR%s is currently not set.\n"
	   "\n",
	   c.tur,c.end);

  return 1;
}

int parse_args(int argc, char **argv, int *flags, int *num_files)
{
  if(argc < 2) return Usage();

  int i=1;
  for(;argv[i];i++)
    {
      if(!strcmp("-h",argv[i]) ||!strcmp("--help",argv[i]))
	return Usage();
      if(!strcmp("-r",argv[i]) ||!strcmp("--recursive",argv[i]))
        *flags |= DTK_RECURSE;
      else
	(*num_files)++;
    }
  return 0;
}


int main(int argc, char **argv)
{
  int flags = 0, num_files = 0;
  if(parse_args(argc, argv, &flags, &num_files)) return 1;

  int return_val = 0;
  dtkMsg.setPreMessage(PROGRAM_NAME);

  if(num_files)
    {
      for(argv++;*argv;argv++)
	if(strcmp("-h",*argv) && strcmp("--help",*argv)
	   && strcmp("-r",*argv) && strcmp("--recursive",*argv))
	  return_val -= dtkSharedMem_destroy(*argv, flags);
    }
  else if(flags & DTK_RECURSE)
    return_val -= dtkSharedMem_destroy(NULL, flags);

  return return_val;
}
