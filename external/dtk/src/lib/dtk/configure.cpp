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
#include "_config.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DTK_ARCH_WIN32_VCPP
//# include "../../config/dtkConfigure.h"
# define strdup _strdup
#endif

#include "privateConstants.h"
#include "dtkConfigure.h"

static int init(char **string)
{
  int gotRootEnv;
  int i;
  for(i=0;i<dtkConfigure::NUM_STRINGS;i++)
    string[i] = NULL;
   
  string[dtkConfigure::ROOT_DIR] = getenv("DTK_ROOT_DIR");
   
  if(string[dtkConfigure::ROOT_DIR])
    {
      string[dtkConfigure::ROOT_DIR] = strdup(string[dtkConfigure::ROOT_DIR]);
      gotRootEnv = 1;
    }
  else
    {
      string[dtkConfigure::ROOT_DIR] = strdup(DTK_DEFAULT_ROOT_DIR);
      gotRootEnv = 0;
    }
  { // remove extra directory slashes
     size_t i = strlen(string[dtkConfigure::ROOT_DIR]) - 1;
     for(;i>(size_t)0 && string[dtkConfigure::ROOT_DIR][i] == DIRCHAR;i--)
       string[dtkConfigure::ROOT_DIR][i] = '\0';
  }
  return gotRootEnv;
}
                                                              
dtkConfigure::dtkConfigure(void)
{
  gotRootEnv = init(string);
}

dtkConfigure::~dtkConfigure(void)
{
  int i;
  for(i=0;i<NUM_STRINGS;i++)
    if(string[i])
      {
	free(string[i]);
	string[i] = NULL;
      }
}

char *dtkConfigure::getString(DTK_STRING i)
{
  /* This is to fix an IRIX n32 bug.  There is a globel dtkConfigure dtkConfig
   * object which does not get its constructor called on IRIX.  So we initialize
   * the object data here.  But on GNU/Linux this will do nothing because the
   * constructor is called.
   */
  if(!(string[0])) gotRootEnv = init(string);


  if(i<0 || i>=NUM_STRINGS)
    return NULL;

  // If the string has been looked up before than use the results from
  // before.
  if(string[i])
    return string[i];

  switch(i)
    {
    case ROOT_DIR:
      return string[ROOT_DIR];
      break;
    case LINK_LIBS:
      {
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_CYGWIN
	if(gotRootEnv)
	  {
	    const char *format =
	      "-L%s/lib -ldtk -Wl,-rpath -Wl,%s/lib %s";
	    string[i] = (char *) malloc(2*strlen(string[ROOT_DIR]) + strlen(format) + strlen(DTK_OTHER_LDFLAGS));
	    sprintf(string[i], format, string[ROOT_DIR], string[ROOT_DIR], DTK_OTHER_LDFLAGS);
	  }
	else
	  {
	    const char *format =
	      "-L%s -ldtk -Wl,-rpath -Wl,%s %s";
	    string[i] = (char *) malloc(2*strlen(DTK_LIB_DIR) + strlen(format) + strlen(DTK_OTHER_LDFLAGS));
	    sprintf(string[i], format, DTK_LIB_DIR, DTK_LIB_DIR, DTK_OTHER_LDFLAGS);
	  }
#endif
#ifdef DTK_ARCH_DARWIN
	if(gotRootEnv)
	  {
	    const char *format =
	      "-L%s/lib -ldtk -Wl,-rpath -Wl,%s/lib %s";
	    string[i] = (char *) malloc(2*strlen(string[ROOT_DIR]) + strlen(format) + strlen(DTK_OTHER_LDFLAGS));
	    sprintf(string[i], format, string[ROOT_DIR], string[ROOT_DIR], DTK_OTHER_LDFLAGS);
	  }
	else
	  {
	    const char *format =
	      "-L%s -ldtk";
	    string[i] = (char *) malloc(2*strlen(DTK_LIB_DIR));
	    sprintf(string[i], format, DTK_LIB_DIR);
	  }
#endif
#ifdef DTK_ARCH_IRIX
        const char *pthreadLib =
#  ifndef _DTK_IRIX_32_WITH_64
	  " -lpthread";
#  else
	  "";
#  endif

	  if(gotRootEnv)
	    {
	      const char *format = "-L%s/lib -ldtk -Wl,-rpath -Wl,%s/lib%s -lm";
	      string[i] = (char *) malloc(2*strlen(string[ROOT_DIR]) + strlen(pthreadLib) + strlen(format));
	      sprintf(string[i], format, string[ROOT_DIR], string[ROOT_DIR], pthreadLib);
	    }
	  else
	    {
	      const char *format = "-L%s -ldtk -Wl,-rpath -Wl,%s%s -lm";
	      string[i] = (char *) malloc(2*strlen(DTK_LIB_DIR) + strlen(pthreadLib) + strlen(format));
	      sprintf(string[i], format, DTK_LIB_DIR, DTK_LIB_DIR, pthreadLib);
	    }
#endif
      }
      break;
    case LINK_LIBSX:
      {
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_CYGWIN
	if(gotRootEnv)
	  {
	    const char *format =
	      "-L%s/lib -ldtk -ldtkX11 -Wl,-rpath -Wl,%s/lib %s %s";
	    string[i] = (char *) malloc(2*strlen(string[ROOT_DIR]) + strlen(format) + strlen(DTK_X11_LDFLAGS) + strlen(DTK_OTHER_LDFLAGS));
	    sprintf(string[i], format, string[ROOT_DIR], string[ROOT_DIR], DTK_X11_LDFLAGS, DTK_OTHER_LDFLAGS);
	  }
	else
	  {
	    const char *format =
	      "-L%s -ldtk -ldtkX11 -Wl,-rpath -Wl,%s %s %s";
	    string[i] = (char *) malloc(2*strlen(DTK_LIB_DIR) + strlen(format) + strlen(DTK_X11_LDFLAGS) + strlen(DTK_OTHER_LDFLAGS));
	    sprintf(string[i], format, DTK_LIB_DIR, DTK_LIB_DIR, DTK_X11_LDFLAGS, DTK_OTHER_LDFLAGS);
	  }
#endif
#ifdef DTK_ARCH_IRIX
        const char *pthreadLib =
#  ifndef _DTK_IRIX_32_WITH_64
	  " -lpthread";
#  else
	  "";
#  endif

	  if(gotRootEnv)
	    {
	      const char *format = "-L%s/lib -ldtk -ldtkX11 -Wl,-rpath -Wl,%s/lib%s -lm";
	      string[i] = (char *) malloc(2*strlen(string[ROOT_DIR]) + strlen(pthreadLib) + strlen(format));
	      sprintf(string[i], format, string[ROOT_DIR], string[ROOT_DIR], pthreadLib);
	    }
	  else
	    {
	      const char *format = "-L%s -ldtk -ldtkX11 -Wl,-rpath -Wl,%s%s -lm";
	      string[i] = (char *) malloc(2*strlen(DTK_LIB_DIR) + strlen(pthreadLib) + strlen(format));
	      sprintf(string[i], format, DTK_LIB_DIR, DTK_LIB_DIR, pthreadLib);
	    }
#endif
      }
      break;
    case INCLUDE_FLAGS:
      {
	if(gotRootEnv)
	  {
	    const char *format = "-I%s/include";
	    string[i] = (char *) malloc(strlen(string[ROOT_DIR]) + strlen(format));
	    sprintf(string[i], format, string[ROOT_DIR]);
	  }
	else
	  {
	    const char *format = "-I%s";
	    string[i] = (char *) malloc(strlen(DTK_INCLUDE_DIR) + strlen(format));
	    sprintf(string[i], format, DTK_INCLUDE_DIR);
	  }
      }
      break;
    case CXXFLAGS:
      {
	const char *format = "%s";
	string[i] = (char *) malloc(strlen(format) + strlen(DTK_CXXFLAGS));
	sprintf(string[i], format, DTK_CXXFLAGS);
      }
      break;
    case AUGMENT_DSO_DIR:
      {
	if(gotRootEnv)
	  {
	    const char *format = "%s/etc/dtk/augmentDSO";
	    string[i] = (char *) malloc(strlen(string[ROOT_DIR]) + strlen(format));
	    sprintf(string[i], format, string[ROOT_DIR]);
	  }
	else
	  {
	    const char *format = "%s";
	    string[i] = (char *) malloc(strlen(DTK_AUGMENT_DIR) + strlen(format));
	    sprintf(string[i], format, DTK_AUGMENT_DIR);
	  }
      }
      break;
    case SERVICE_DSO_DIR:
      {
	if(gotRootEnv)
	  {
	    const char *format = "%s/etc/dtk/serviceDSO";
	    string[i] = (char *) malloc(strlen(string[ROOT_DIR]) + strlen(format));
	    sprintf(string[i], format, string[ROOT_DIR]);
	  }
	else
	  {
	    const char *format = "%s";
	    string[i] = (char *) malloc(strlen(DTK_SERVICE_DIR) + strlen(format));
	    sprintf(string[i], format, DTK_SERVICE_DIR);
	  }
      }
      break;
    case FILTER_DSO_DIR:
      {
	if(gotRootEnv)
	  {
	    const char *format = "%s/etc/dtk/filterDSO";
	    string[i] = (char *) malloc(strlen(string[ROOT_DIR]) + strlen(format));
	    sprintf(string[i], format, string[ROOT_DIR]);
	  }
	else
	  {
	    const char *format = "%s";
	    string[i] = (char *) malloc(strlen(DTK_FILTER_DIR) + strlen(format));
	    sprintf(string[i], format, DTK_FILTER_DIR);
	  }
      }
      break;
    case LIB_DIR:
      {
	if(gotRootEnv)
	  {
	    const char *format = "%s/lib";
	    string[i] = (char *) malloc(strlen(string[ROOT_DIR]) + strlen(format));
	    sprintf(string[i], format, string[ROOT_DIR]);
	  }
	else
	  {
	    const char *format = "%s";
	    string[i] = (char *) malloc(strlen(DTK_LIB_DIR) + strlen(format));
	    sprintf(string[i], format, DTK_LIB_DIR);
	  }
      }
      break;
    case SERVER:
      {
	if(gotRootEnv)
	  {
	    const char *format = "%s/bin/dtk-server";
	    string[i] = (char *) malloc(strlen(string[ROOT_DIR]) + strlen(format));
	    sprintf(string[i], format, string[ROOT_DIR]);
	  }
	else
	  {
	    const char *format = "%s/dtk-server";
	    string[i] = (char *) malloc(strlen(DTK_BIN_DIR) + strlen(format));
	    sprintf(string[i], format, DTK_BIN_DIR);
	  }
      }
      break;

      // This should not happen.
    default:
      string[i] = NULL;
    }
  return string[i];
}


/* this is a globel configuration object */

dtkConfigure dtkConfig;
