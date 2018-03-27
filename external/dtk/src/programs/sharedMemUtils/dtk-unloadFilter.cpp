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
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <dtk.h>

#define PROGRAM_NAME  "dtk-unloadFilter"

int Usage(void)
{
  printf("  Usage: "PROGRAM_NAME" SHM_FILE FILTER_NAME\n"
	 "\n"
	 "   Unload the DSO r/w filter named FILTER_NAME from the dtkSharedMem objects associated with the\n"
	 " shared memory file SHM_FILE.\n"
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

  dtkSharedMem shm(argv[1]);
  if(shm.isInvalid()) return 1;

  return - shm.unloadFilter(argv[2]);
}
