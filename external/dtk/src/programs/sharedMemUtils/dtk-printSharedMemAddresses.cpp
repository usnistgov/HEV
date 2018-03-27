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
#include <stdlib.h>
#include <dtk.h>

#define PROGRAM_NAME  "dtk-printSharedMemAddresses"

int Usage(void)
{
  dtkColor c(stdout);

  printf("\n"
	 "Usage: "PROGRAM_NAME" %sSHM_FILE ...%s | [--help|-h]\n"
	 "\n"
	 "  "PROGRAM_NAME" prints, to standard output, the shared\n"
	 " memory write addresses for the DTK shared memory files %sSHM_FILE ...%s.\n"
	 "   The environment variable %sDTK_SHAREDMEM_DIR%s can be used to\n"
	 " select the shared memory directory the that %sSHM_FILE%s may be in\n"
	 " when the full path is not given.  The default shared memory directory,\n"
	 " %s%s%s, will be used if %sDTK_SHAREDMEM_DIR%s is not set.",
	 c.vil,c.end, c.vil,c.end,
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
  if(argc < 2)
    return Usage();

  int i;
  for(i=1;i<argc;i++)
    if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
      return Usage();

  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil, 
		       PROGRAM_NAME, dtkMsg.color.end);
  int return_val = 0;

  for(i=1;i<argc;i++)
    {
      dtkSharedMem shm(argv[i]);
      if(shm.isInvalid()) return_val++;
      else
	{
	  dtkSegAddrList *l = shm.getSegAddrList();
	  if(l && l->start())
	    {
	      printf("Write Addresses for shared memory %s%s%s\n",
		     dtkMsg.color.tur, shm.getName(), dtkMsg.color.end);
	      printf("%s_______address_______%s  %s__remote_file_name__%s\n",
		     dtkMsg.color.grn, dtkMsg.color.end,
		     dtkMsg.color.tur, dtkMsg.color.end);
	      
	      char *nam;
	      char *addr = l->getNextAddressPort(&nam);
	      for(;addr;addr = l->getNextAddressPort(&nam))
		printf("%s%s%s %s%s%s\n",
		       dtkMsg.color.grn, addr, dtkMsg.color.end,
		       dtkMsg.color.tur, nam, dtkMsg.color.end);
	    }
	  else
	    printf("No write addresses for shared memory %s%s%s\n",
		   dtkMsg.color.tur, shm.getName(), dtkMsg.color.end);
	}
    }

  return return_val; // error
}
