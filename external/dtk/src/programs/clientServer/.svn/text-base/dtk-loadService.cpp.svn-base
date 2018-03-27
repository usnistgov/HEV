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

#define PROGRAM_NAME  "dtk-loadService"

int Usage(void)
{
  dtkColor c(stdout);
  printf("\n Usage: %s%s%s [%s[%sDSO_FILE%s [%sNAME%s [%sARG%s]]%s]%s |\\\n"
         " [%s--config%s|%s-c%s CONFIGFILE%s]%s] | [%s--help%s|%s-h%s]\n"
	 "\n"
	 "  %s%s%s loads the DTK DSO service file, %sDSO_FILE%s,\n"
	 "into a running DTK server.  %sDSO_FILE%s is the file name of the\n"
	 "DSO file.  %sNAME%s is the name of the DTK service.  The default\n"
	 "DTK service name is the base name of the DSO file without a\n"
	 "\".os\" suffix.  %sARG%s is an optional argument string that is\n"
	 "passed to the DTK service constructor.  The environment\n"
	 "variable %sDTK_SERVICE_PATH%s may be set to find the DSO file. The\n"
	 "environment variable %sDTK_CONNECTION%s may be set to the servers'\n"
	 "address and port (examples: foo.com:1234, :1234, :dtk). The default\n"
	 "DTK connection address and port is %s%s%s\n"
	 "\n",
	 c.yel,PROGRAM_NAME,c.end,
	 c.yel, c.grn, c.end, c.grn, c.end, c.grn, c.end, c.yel, c.end,
     c.grn, c.end, c.grn, c.tur, c.yel, c.end, c.grn, c.end, c.grn, c.end,
	 c.yel,PROGRAM_NAME,c.end,
	 c.grn, c.end, c.grn, c.end, c.grn, c.end, c.grn, c.end,
	 c.tur, c.end, c.tur, c.end,
	 c.grn, DTK_DEFAULT_SERVER_CONNECTION, c.end);
  return 1;
}


int main(int argc, char **argv)
{
  if(argc < 2 || !strcmp("--help",argv[1]) || !strcmp("-h",argv[1]))
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

  if( !strcmp( "--config", argv[1] ) || !strcmp( "-c", argv[1] ) )
    return - client.loadConfig( argv[2] );

  char *arg = NULL;
  if(argc > 3)
    arg = argv[3];

  // argv[2] may be NULL
  return - client.loadService(argv[1],argv[2],arg);
}
