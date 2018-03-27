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
 *
 */

#include "_config.h"
#include "config.h"

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef DTK_ARCH_WIN32_VCPP
//# include "../../config/dtkConfigure.h"
#endif

#include "privateConstants.h"
#include "dtkConfigure.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"


int dtk_printEnv(FILE *file, int is_color)
{
  if(!file) file = stdout;
  // Sets up some escape color sequences or not based on the value of
  // environment variable DTK_COLOR and whether file is a tty.
  dtkColor color(file, is_color);
  dtkConfigure config;

  {
    /* here is a list of all usable DTK env variables */
    char *dtk_env[] =
    {
      const_cast<char*>("DTK_COLOR"),
      const_cast<char*>("DTK_CONNECTION"),
      const_cast<char*>("DTK_FILTER_PATH"),
      const_cast<char*>("DTK_PORT"),
      const_cast<char*>("DTK_ROOT"),
      const_cast<char*>("DTK_SHAREDMEM_DIR"),
      const_cast<char*>("DTK_SERVICE_PATH"),
      const_cast<char*>("DTK_SERVICE_CONFIG_PATH"),
      const_cast<char*>("DTK_CALIBRATION_PATH"),
      const_cast<char*>("DTK_CALIBRATION_CONFIG_PATH"),
      const_cast<char*>("DTK_SPEW"),
      const_cast<char*>("DTK_SPEW_FILE"),
      NULL
    };
    char **str = dtk_env;
    
    fprintf(file,"\n  --------- your current DTK environment variables ---------\n\n");
    for(;*str;str++)
      fprintf(file,"   %s%s%s=%s\n",color.tur,
	      *str,color.end,getenv(*str));
  }

  fprintf(file,"\n"
	  "  --------- about DTK environment variables --------\n"
	  "\n"
	  " DTK environment variables, many times, will over-ride DTK user source coded\n"
	  " behaviors.  This in effect can give users more control than the programmers.\n"
	  " If you have built and installed DTK, you typically will not need to set any\n"
	  " environment variables to use any part of DTK. "
	  " If you did not build and\n"
	  " install DTK with the same default %sDTK_ROOT%s"
	  " as that which is compiled into\n"
	  " it, you will have to set %sDTK_ROOT%s to"
	  " the top installation directory"
	  " of DTK.\n"
	  " See `man env'. \n"
	  "\n",
	  color.tur,color.end, color.tur, color.end);
  
  fprintf(file,"\n"
	  " %senv variable%s        %saffected part(s) of DTK%s\n"
	  " **************    ***************************************************\n"
	  "\n",color.tur,color.end, color.vil,color.end);

  fprintf(file," %sDTK_COLOR%s         "
	  "%sclasses dtkMessage and dtkColor and in-turn many DTK programs%s\n"
	  "   Setting DTK_COLOR=ON will make the color esc strings, in the dtk spew,\n"
	  "   non-null. Setting DTK_COLOR=OFF will make all the color esc strings in the\n"
	  "   dtk spewage be null.  Setting DTK_COLOR=AUTO will set the esc strings to\n"
	  "   non-null values if isatty() of the dtkMessage (or dtkColor) output file is\n"
	  "   true otherwise the esc strings will all be set to null.  The default DTK color\n"
	  "   setting is %s\n"
	  "\n",color.tur,color.end, color.vil,color.end,
	  (DEFAULT_DTKCOLOR_SWITCH==DTKCOLOR_AUTO)?"AUTO":
	  ((DEFAULT_DTKCOLOR_SWITCH==DTKCOLOR_ON)?"TRUE":"FALSE"));

  fprintf(file," %sDTK_CONNECTION%s    %sclass dtkClient and the programs that use dtkClient%s\n"
	  "   When set, DTK_CONNECTION sets the socket address and/or socket port that the\n"
	  "   objects of class dtkClient uses to initially communicate with a DTK server.  The\n"
	  "   address and the port may be specified with address and port seperated by a colon (:).\n"
	  "   A colon must precede the port.\n"
	  "   Examples: DTK_CONNECTION=:2343 DTK_CONNECTION=foo.com:2343 DTK_CONNECTION=\"foo.com\".\n"
	  "   The default connection address is %s and the\n"
	  "   default connection port is %s.\n"
	  "\n",color.tur,color.end, color.vil,color.end,
	  DTK_DEFAULT_SERVER_ADDRESS, DTK_DEFAULT_SERVER_PORT);

  fprintf(file," %sDTK_FILTER_PATH%s  %sclass dtkSharedMem and the programs that use DTK\n"
	  "   DTK shared memory%s\n"
	  "   When set, DTK_FILTER_PATH sets search path for DTK shared memory read/write filter\n"
	  "   DSO (dynamic shared object) files. The default value for the shared memory filter\n"
	  "   path is: .%c%s\n"
	  "   with DTK_ROOT%s%s\n"
	  "\n", color.tur,color.end, color.vil,color.end,
	  PATH_SEP,
	  config.getString(dtkConfigure::FILTER_DSO_DIR),
	  getenv("DTK_ROOT")?"=":"",
	  getenv("DTK_ROOT")?getenv("DTK_ROOT"):" not set.");

  fprintf(file," %sDTK_ROOT%s          %sclass dtkConfigure and in-turn many parts of DTK%s\n"
	  "   When set, DTK_ROOT can be set to help the DTK library function then it is not located\n"
	  "   in the complied (default) installation directory.  This is because some running DTK\n"
	  "   programs must use others files, not just shared object libraries like many programs.\n"
	  "   Setting DTK_ROOT will change the value many default parmeters such as the default\n"
	  "   for DTK service DSO file search path, the default DTK shared memory read/write filter\n"
	  "   DSO search path, and the default DTK augment DSO search path.\n"
	  "\n",
	  color.tur,color.end, color.vil,color.end);

  fprintf(file," %sDTK_PORT%s          %sthe DTK server program, `dtk-server'%s\n"
	  "   When set, DTK_PORT can be set to tell a DTK server what network socket port to\n"
	  "   bind to.  Setting DTK_PORT=NONE will cause the DTK server not to load network\n"
	  "   services.  The default DTK server network socket port is %s\n"
	  "\n",
	  color.tur,color.end, color.vil,color.end, DTK_DEFAULT_SERVER_PORT);

  fprintf(file," %sDTK_SHAREDMEM_DIR%s %sclass dtkSharedMem and in-turn many DTK programs%s\n"
	  "   When set, DTK_SHAREDMEM_DIR is the DTK shared memory directory, where DTK shared\n"
	  "   memory files will be when the full path of the shared memory files is not given.\n"
	  "   If DTK_SHAREDMEM_DIR is not set than the DTK shared memory directory will be\n"
	  "   %s.\n"
	  "\n",
	  color.tur,color.end, color.vil,color.end, DTK_DEFAULT_SHAREDMEM_DIR);

  fprintf(file," %sDTK_SERVICE_PATH%s  %sthe DTK server program, `dtk-server'%s\n"
	  "   When set, DTK_SERVICE_PATH is the search path that the DTK server uses to find DTK\n"
	  "   service DSOs.  The default DTK service search\n"
	  "   path is: .%c%s\n"
	  "   with DTK_ROOT%s%s\n"
	  "\n", color.tur,color.end, color.vil,color.end,
	  PATH_SEP,
	  config.getString(dtkConfigure::SERVICE_DSO_DIR),
	  getenv("DTK_ROOT")?"=":"",
	  getenv("DTK_ROOT")?getenv("DTK_ROOT"):" not set.");

  fprintf(file," %sDTK_SERVICE_CONFIG_PATH%s  %sthe DTK server program, `dtk-server'%s\n"
	  "   When set, DTK_SERVICE_CONFIG_PATH is the search path that the DTK server uses\n"
      "   to find DTK service configuration files.  The default DTK service configuration\n"
      "   search path is: .%c%s\n"
	  "   with DTK_ROOT%s%s\n"
	  "\n", color.tur,color.end, color.vil,color.end,
	  PATH_SEP,
	  config.getString(dtkConfigure::SERVICE_DSO_DIR),
	  getenv("DTK_ROOT")?"=":"",
	  getenv("DTK_ROOT")?getenv("DTK_ROOT"):" not set.");

  fprintf(file," %sDTK_CALIBRATION_PATH%s  %sthe DTK server program, `dtk-server'%s\n"
	  "   When set, DTK_CALIBRATION_PATH is the search path that the DTK server uses\n"
      "   to find DTK calibration DSOs.  The default DTK calibration search\n"
	  "   path is: .%c%s\n"
	  "   with DTK_ROOT%s%s\n"
	  "\n", color.tur,color.end, color.vil,color.end,
	  PATH_SEP,
	  config.getString(dtkConfigure::SERVICE_DSO_DIR),
	  getenv("DTK_ROOT")?"=":"",
	  getenv("DTK_ROOT")?getenv("DTK_ROOT"):" not set.");

  fprintf(file," %sDTK_CALIBRATION_CONFIG_PATH%s  %sthe DTK server program, `dtk-server'%s\n"
	  "   When set, DTK_CALIBRATION_CONFIG_PATH is the search path that the DTK server uses\n"
      "   to find DTK calibration configuration files.  The default DTK calibration\n"
      "    configuration search path is: .%c%s\n"
	  "   with DTK_ROOT%s%s\n"
	  "\n", color.tur,color.end, color.vil,color.end,
	  PATH_SEP,
	  config.getString(dtkConfigure::SERVICE_DSO_DIR),
	  getenv("DTK_ROOT")?"=":"",
	  getenv("DTK_ROOT")?getenv("DTK_ROOT"):" not set.");

  char *level;
  if(DTKMSG_DEFAULT_LEVEL==DTKMSG_NONE) level=const_cast<char*>("NONE");
  else if(DTKMSG_DEFAULT_LEVEL==DTKMSG_FATAL) level=const_cast<char*>("FATAL");
  else if(DTKMSG_DEFAULT_LEVEL==DTKMSG_ERROR) level=const_cast<char*>("ERROR");
  else if(DTKMSG_DEFAULT_LEVEL==DTKMSG_WARN) level=const_cast<char*>("WARNING");
  else if(DTKMSG_DEFAULT_LEVEL==DTKMSG_NOTICE) level=const_cast<char*>("NOTICE");
  else if(DTKMSG_DEFAULT_LEVEL==DTKMSG_INFO) level=const_cast<char*>("INFO");
  else if(DTKMSG_DEFAULT_LEVEL==DTKMSG_DEBUG) level=const_cast<char*>("DEBUG");

  fprintf(file," %sDTK_SPEW%s          %sclass dtkMessage and in-turn most DTK programs%s\n"
	  "   The DTK_SPEW environment variables will set the"
	  " action of, the DTK message class,\n   dtkMessage, when"
	  " an message condition occurs.  Valid"
	  " values are FATAL, ERROR, WARNING,\n"
	  "   NOTICE, INFO, and DEBUG.\n"
	  "   The default DTK spew level is %s\n"
	  "\n",color.tur,color.end, color.vil,color.end, level);

  fprintf(file," %sDTK_SPEW_FILE%s     %sclass dtkMessage and in-turn most DTK programs%s\n"
	  "   Setting DTK_SPEW_FILE=1 will set the dtkMessage spew to standard out.\n"
	  "   Setting DTK_SPEW_FILE=2 will set the dtkMessage spew to standard error.\n"
	  "   The default DTK spew file is %s\n"
	  "\n",color.tur,color.end, color.vil,color.end, (DTKMSG_DEFAULT_FILE==stdout)?"1":"2");

  fprintf(file," Note:  DTK does not use an environment variable for the path to find augment DSO\n"
	  "   files.  It is thought that this would interfer with higher level API's that use\n"
	  "   DTK, and build of the dtkAugment object loader.\n\n");

  return 0;
}
