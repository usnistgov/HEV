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

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkFLTKOptions.h"

/*
FLTK options are:
 -d[isplay] host:n.n
 -g[eometry] WxH+X+Y
 -t[itle] windowtitle
 -n[ame] classname
 -i[conic]
 -fg color
 -bg color
 -bg2 color
*/

// optional arguments that have two strings
static const char *fltk_args2[] =
{
  "-d", "-g", "-t", "-n", "-f", "-b", NULL
};

// optional arguments that have one strings
static const char *fltk_args1[] =
{
  "-i", NULL
};

// dtk-getFLTKOptions() returns malloc()ed memory that is pointers to
// the strings that are passed in const char** argv.  You may use
// free() on the returned values *fltk_argv and *other_argv.  argv[0]
// is included in both returned argument arrays.  If the any handles
// or pointers are NULL than they are ignored.

// returns 0 on success
// returns -1 if malloc() fails.

int dtkFLTKOptions_get(int argc, const char** argv,
		       int *fltk_argc,  char ***fltk_argv,
		       int *other_argc, char ***other_argv)
{
  if((other_argc))
    *other_argc = 0;
  if((fltk_argc))
    *fltk_argc = 0;
  if(other_argv)
    *other_argv = NULL;
  if(fltk_argv)
    *fltk_argv = NULL;

  if(argc<1) // this should not happen
    return 0;

  //printf("file=%s line=%d\n",__FILE__, __LINE__);

  if(other_argv)
    {
      *other_argv = (char **) malloc(sizeof(char *)*(argc+1));
      if(!(*other_argv))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk-getFLTKOptions() failed:"
		     " malloc(%d) failed.\n",
		     sizeof(char *)*(argc+1));
	  return -1;
	}
      (*other_argv)[0] = (char *) argv[0];
      (*other_argv)[1] = NULL;
    }
  if(other_argc)
    *other_argc = 1;

  if(fltk_argv)
    {
      *fltk_argv = (char **) malloc(sizeof(char *)*(argc+1));
      if(!(*fltk_argv))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk-getFLTKOptions() failed:"
		     " malloc(%d) failed.\n",
		     sizeof(char *)*(argc+1));
	  return -1;
	}
      (*fltk_argv)[0] = (char *) argv[0];
      (*fltk_argv)[1] = NULL;
    }
  if(fltk_argc)
    *fltk_argc = 1;

  int Fltk_argc = 1;
  int Other_argc = 1;

  int i = 1;
  while(i<argc)
    {
      int notGotOne = 1;
      char **str = (char **) fltk_args1;
      for(;*str;str++)
	if(!strncmp(*str,argv[i],strlen(*str)))
	  {
	    notGotOne = 0;
	    if(fltk_argv)
	      (*fltk_argv)[Fltk_argc] = (char *) argv[i];
	    i++;
	    Fltk_argc++;
	    if(fltk_argv)
	      (*fltk_argv)[Fltk_argc] = NULL;
	    break;
	  }
      str = (char **) fltk_args2;
      for(;*str && notGotOne && i<argc-1;str++)
	if(!strncmp(*str,argv[i],strlen(*str)))
	  {
	    notGotOne = 0;
	    if(fltk_argv)
		(*fltk_argv)[Fltk_argc] = (char *) argv[i];
	    i++;
	    Fltk_argc++;
	    if(fltk_argv)
		(*fltk_argv)[Fltk_argc] = (char *) argv[i];
	    i++;
	    Fltk_argc++;
	    if(fltk_argv)
	      (*fltk_argv)[Fltk_argc] = NULL;
	    break;
	  }
      if((notGotOne))
	{
	  if(other_argv)
	    (*other_argv)[Other_argc] = (char *) argv[i];
	  i++;
	  Other_argc++;
	  if(other_argv)
	    (*other_argv)[Other_argc] = NULL;
	}
    }

  if(fltk_argc)
    (*fltk_argc) = Fltk_argc;

  if(other_argc)
    (*other_argc) = Other_argc;

  return 0;
}


void dtkFLTKOptions_print(FILE *file)
{
  if(!file) file = stdout;

  fprintf(file,
	  "   FLTK_OPTIONS\n\n"
	  "  -d[isplay] host:n.n\n"
	  "  -g[eometry] WxH+X+Y\n"
	  "  -t[itle] windowtitle\n"
	  "  -n[ame] classname\n"
	  "  -i[conic]\n"
	  "  -fg color\n"
	  "  -bg color\n"
	  "  -bg2 color\n\n");

}


#ifdef TEST_DTK_FLTK_OPTIONS

// CC -DTEST_DTK_FLTK_OPTIONS `dtk-config --cflags` -o test _FLTKOptions.C `dtk-config --libs`

int main(int argc, char **argv)
{
  char **fltk_argv, **other_argv;
  int fltk_argc, other_argc;

  if(dtkFLTKOptions_get(argc, (const char**) argv,
			&fltk_argc, &fltk_argv,
			&other_argc, &other_argv))
    return 1; // error

  dtkFLTKOptions_print();


  printf(" fltk_argv[%d] = {",fltk_argc+1);
  char **s = fltk_argv;
  for(;*s;s++)
    printf(" \"%s\", ", *s);
  printf(" NULL };\n");

  printf("other_argv[%d] = {",other_argc+1);
  s = other_argv;
  for(;*s;s++)
    printf(" \"%s\", ", *s);
  printf(" NULL };\n");

  return 0;
}

#endif
