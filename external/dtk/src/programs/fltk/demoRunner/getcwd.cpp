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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


// For some unknown reason there is no good way to
// get a string that is the current working directory.

// This returns a pointer to a string that is the
// current working directory or NULL on error.

// Use strdup(Getcwd()) if you need a copy in your own
// memory.

#define STEP     ((size_t) 40)
#define MAXSIZE  ((size_t) 100)*STEP
static size_t size = STEP;
static char *str = NULL;

char *Getcwd()
{
  if(!str)
    str = (char *) malloc(size);

  for(;size < MAXSIZE ;)
    {
      if(getcwd(str,size-1))
	{
	  //printf("getcwd()=\"%s\"\n",str);
	  return str;
	}
      //printf("line=%d file=%s\n",__LINE__,__FILE__);
      size += STEP;
      str = (char *) realloc(str, size);
    }

  // it failed
  free(str);
  str = NULL;
  size = STEP;
  printf("dtk-launcher Error line=%d file=%s: "
	 "getcwd() failed:\n"
	 "error number %d: \"%s\"\n"
	 "i.e.: Can't get the current working directory.\n",
	  __LINE__, __FILE__, errno, strerror(errno));
  return NULL;
}
