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
#include <dtk.h>

#define PROGRAM_NAME  "dtk-connectRemoteSharedMem"

int Usage(void)
{
  dtkColor c(stdout);

  printf("  Usage:\n"
	 "   %s%s%s %sSHM_FILE%s %sADDRESS%s[:%sPORT%s] [%sREMOTE_FILE%s]\n",
	 c.yel,PROGRAM_NAME,c.end,
	 c.grn, c.end, c.grn, c.end, c.grn, c.end, c.grn, c.end);

  printf("  OR\n"
	 "   %s%s%s %sSHM_FILE%s [%sADDRESS%s]:%sPORT%s [%sREMOTE_FILE%s]\n",
	 c.yel,PROGRAM_NAME,c.end,
	 c.grn, c.end, c.grn, c.end, c.grn, c.end, c.grn, c.end);

  printf("  OR\n"
	 "   %s%s%s [%s--help%s|%s-h%s]\n",
	 c.yel,PROGRAM_NAME,c.end,
	 c.grn, c.end, c.grn, c.end);

  printf("\n"
	 "  %s%s%s connects the DTK shared memory, named %sSHM_FILE%s, to the\n"
	 " shared meory served to the DTK server at %sADDRESS%s:%sPORT%s.  %sREMOTE_FILE%s \n"
	 " may be used to connect shared memory with different shared memory file names.\n"
	 "\n"
	 " The environment variable %sDTK_CONNECTION%s may be set to the client's server\n"
	 " address and port (examples: foo.com:1234, foo.com, :1234, :dtk). The default DTK\n"
	 " connection address and port is %s%s%s\n"
	 "\n",
	 c.yel,PROGRAM_NAME,c.end,
	 c.grn, c.end, c.grn, c.end, c.grn, c.end, c.grn, c.end, c.grn, c.end,

	 c.grn, DTK_DEFAULT_SERVER_CONNECTION, c.end);

  return 1;
}


int main(int argc, char **argv)
{
  if(argc < 3 || !strcmp("--help",argv[1]) || !strcmp("-h",argv[1]))
    return Usage();

  dtkMsg.setPreMessage("%s%s%s",dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);
  dtkClient client;
  if(client.isInvalid())
    {
      dtkMsg.append("   Do you have a %sdtk-server%s running?\n",
		 dtkMsg.color.vil, dtkMsg.color.end);
      return 1;
    }


  // argv[3] may be NULL
  return - client.connectRemoteSharedMem(argv[1],argv[2],argv[3]);
}
