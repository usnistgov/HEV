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
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#ifndef DTK_ARCH_WIN32_VCPP
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"

#define MAX (511)
#define TRY  6
#define OFFSET 1002278312UL

char *dtk_tempname(const char *pre)
{
  char str[MAX+1];
  struct stat buf;
  struct timeval t;
  int i;

  for(i=0;i<TRY;i++)
    {
      if(gettimeofday(&t
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
		      ,NULL
#endif
		      ))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk_tempname() failed: gettimeofday() failed\n");
	  return NULL;
	}
      errno = 0;
      snprintf(str,MAX,"%s_%ld.%6.6ld",pre,t.tv_sec-OFFSET,t.tv_usec);
      if(stat((const char *) str, &buf) && errno == ENOENT)
	{
	  errno =0;
	  char *s = strdup(str); // uses malloc()
	  if(!s)
	    {
	      dtkMsg.add(DTKMSG_ERROR, 1,
			 "dtk_tempname() failed: strdup() failed\n");
	    }
	  return s;
	}
    }
  errno = 0;
  dtkMsg.add(DTKMSG_ERROR,
	     "dtk_tempname(\"%s\") failed "
	     "to find a non-existent filename that begins with %s.\n",
	     pre, pre);
  return NULL;
}
#else /* #ifndef DTK_ARCH_WIN32_VCPP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"

char *dtk_tempname(const char *pre)
{ 
   char *dir = _strdup(pre);

   char *prefix = &dir[strlen(dir)];
   while(prefix != dir && *prefix != '\\') prefix--;
   if(*prefix == '\\')
   {
	   char *s = prefix;
	   prefix++;
	   s--;
       while(s != dir && *s == '\\') s--;
	   *s = '\0';
   }
   else
      prefix = "";

   char *str = _tempnam(dir, prefix);

   if(!str)
      dtkMsg.add(DTKMSG_ERROR,
	         "dtk_tempname(\"%s\") failed:\n"
		     "_tempname(\"%s\",\"%s\") failed.\n",
			 pre, dir, prefix);

   free(dir);
   return str;
}

#endif
