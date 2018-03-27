/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 */
#include <stdio.h>
#include <dtk.h>

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"

#define PROGRAM_NAME "dtk-server"


int Usage(void)
{
  dtkColor c(dtkMsg.file());
  FILE *file = dtkMsg.file();
  fprintf(file,
	  "\n Usage: %s%s%s [%s[%sSERVICE %s[%s--name%s|%s-n%s NAME%s]"
	  "&[%s--argument%s|%s-a%s ARG%s]"
	  " %s...]%s|\\\n",

	  c.tur, PROGRAM_NAME, c.end,
	  c.yel, c.tur,
	  c.end, c.grn,
	  c.end, c.grn,
	  c.tur, c.end,

	  c.grn, c.end,
	  c.grn, c.tur, c.end,

	  c.yel, c.end);

  fprintf(file,
      "  %s[%s--config%s|%s-c%s CONFIGFILE%s]%s]\\\n",
      c.yel, c.grn,
      c.end, c.grn,
      c.tur, c.yel,
      c.end);

  fprintf(file,  
	  "  &[%s--daemon%s|%s-d%s]"
	  "&[%s--verbose%s|%s-v%s]"
	  "&[%s--timeout%s|%s-t%s %sSEC%s]"
	  "\\\n",

	  c.grn, c.end,
	  c.grn, c.end,

	  c.grn, c.end,
	  c.grn, c.end,
	  c.tur, c.end,

	  c.grn, c.end,
	  c.grn, c.end);

  fprintf(file,
	  "  |[%s--about%s|%s-A%s]|[%s--env%s|%s-e%s]|"
	  "[%s--help%s|%s-h%s]"
	  "|[%s--version%s|%s-V%s]\n\n",

	  
	  c.grn, c.end,
	  c.grn, c.end,

	  c.grn, c.end,
	  c.grn, c.end,

	  c.grn, c.end,
	  c.grn, c.end,

	  c.grn, c.end,
	  c.grn, c.end);


  fprintf(file,  
	  "   Run the DIVERSE Toolkit (DTK) server\n"
	  "\n"
	  " Optional arguments %sSERVICE%s -- cause %s%s%s"
	  " to load DTK\n"
	  " dynamic shared object file %sSERVICE%s[%s.so%s]"
	  " when the DTK server\n"
	  " starts. The default service name is the basename"
	  " of %sSERVICE%s\n"
	  " without the \".so\" suffix.\n"
	  "  When the server successfully starts up as a daemon it will\n"
	  " return status 0 to the parent process. It will return non-zero\n"
	  " status if it's unsuccessful at starting as a daemon.\n",

	  c.tur, c.end,
	  c.tur, PROGRAM_NAME, c.end,

          c.tur, c.end,
	  c.tur, c.end,
	  c.tur, c.end);

  fprintf(file,"\n  OPTIONS\n\n");

  fprintf(file,
	  " %s--about%s|%s-A%s        print "
	  "information about this installation and then exit\n",
	  c.grn, c.end,c.grn, c.end);

  fprintf(file,
	  " %s--argument%s|%s-a%s ARG%s pass "
	  "the argument string %sARG%s "
	  "to the service constructor\n",
	  c.grn, c.end, c.grn,
	  c.tur, c.end,
	  c.tur, c.end);

  fprintf(file,
      " %s--config%s|%s-c%s       load services through a configuration file."
      "                           This option is mutually exclusive from specifying"
      "                           a service.\n",
      c.grn, c.end, c.grn, c.end );

  fprintf(file,
	  " %s--daemon%s|%s-d%s       run as a daemon\n",
	  c.grn, c.end,
	  c.grn, c.end);

  fprintf(file,
	  " %s--env%s|%s-e%s          print info"
	  " related to environment variables when exit\n",
	  c.grn, c.end,
	  c.grn, c.end);

  fprintf(file,
	  " %s--help%s|%s-h%s         print this help and then exit\n",
	  c.grn, c.end,c.grn, c.end);

  fprintf(file,
	  " %s--name%s|%s-n%s NAME%s    set the "
	  "dtk service name to %sNAME%s\n",
	  c.grn, c.end, c.grn,
	  c.tur, c.end,
	  c.tur, c.end);

  fprintf(file,
	  " %s--timeout%s|%s-t%s SEC%s  set the start up time out to %sSEC%s seconds  This timeout\n"
	  "                   is the time that the server will return a status to the forground\n"
	  "                   process.  The default timeout is %d seconds.  If the server fails to\n"
	  "                   finish starting in this time a failure exit status of 1 is returned.\n"
	  "                   This feature is in case users have DTK services that are slow to start.\n",
	  c.grn, c.end, c.grn,
	  c.tur, c.end,
	  c.tur, c.end,
	  DEFAULT_STARTUP_TIMEOUT);

  fprintf(file,
	  " %s--verbose%s|%s-v%s      print more "
	  "information when running\n",
	  c.grn, c.end,c.grn, c.end);

  fprintf(file,
	  " %s--version%s|%s-V%s      print the "
	  "version number and then exit\n",
	  c.grn, c.end,c.grn, c.end);
  
  fprintf(file,
	  "\n"
	  "   To run more than one DTK server you will need to set environment variable\n"
	  " %sDTK_PORT%s for one of the servers.  Setting %sDTK_PORT%s will set the socket\n"
	  " port of the DTK server.  Setting %sDTK_PORT%s=%sNONE%s will cause the DTK server not to\n"
	  " load network services.  Setting %sDTK_SHAREDMEM_DIR%s will set the DTK shared memory\n"
	  " directory that the server looks for in for getting DTK shared memory files.  The default\n"
	  " DTK shared memory directory is %s"DTK_DEFAULT_SHAREDMEM_DIR"%s.\n"
	  "\n"
	  "   See `dtk-server --env' for a list of"
	  " these and other environment variables\n"
	  " that effect DTK and the DTK server.\n"
	  "\n",
	  c.tur, c.end,
	  c.tur, c.end,
	  c.tur, c.end,
	  c.grn, c.end,
	  c.tur, c.end,
	  c.yel, c.end);

  return 1;
}

