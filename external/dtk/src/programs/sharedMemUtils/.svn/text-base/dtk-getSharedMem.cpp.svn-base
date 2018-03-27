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
#include <stdlib.h>
#include <dtk.h>

#define PROGRAM_NAME  "dtk-getSharedMem"

int Usage(void)
{
  dtkColor c(stdout);

  printf("\n"
	 "Usage: "PROGRAM_NAME" %sSIZE%s %sSHM_FILE%s | [--help|-h]\n"
	 "\n"
	 "  "PROGRAM_NAME" gets a DTK shared memory of size %sSIZE%s.\n"
	 " %sSHM_FILE%s will be the name of the DTK shared memory file.\n"
	 " The environment variable %sDTK_SHAREDMEM_DIR%s can be used to select\n"
	 " the shared memory directory the that %sSHM_FILE%s may be in when the\n"
	 " full path is not given.  The default shared memory directory,\n"
	 " %s%s%s, will be used if %sDTK_SHAREDMEM_DIR%s is not set.",
	 c.vil,c.end, c.vil,c.end, c.vil,c.end, c.vil,c.end,
	 c.tur,c.end,
	 c.vil,c.end,
	 c.grn,DTK_DEFAULT_SHAREDMEM_DIR,c.end,
	 c.tur,c.end);

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


int main(int argc, char **argv)
{
  if(argc < 3)
    return Usage();

  for(int i=1;i<argc;i++)
    if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
      return Usage();

  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil, 
		       PROGRAM_NAME, dtkMsg.color.end);

  size_t size = strtoul(argv[1],NULL,10);

  dtkSharedMem shm(size, argv[2]);

  if(shm.isValid()) return 0; // success
  return 1; // error
}
