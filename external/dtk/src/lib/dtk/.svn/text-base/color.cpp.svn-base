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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "privateConstants.h"

#ifdef DTK_ARCH_WIN32_VCPP
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include "dtkConfigure.h"
#include "dtkColor.h"

#define DEFAULT_FILE  stdout

// Most tty terminals display the following escape sequences as
// different colors.  "\033" is the escape character.

#define END     "\033[0m"

#define BLD     "\033[5m"
#define UND     "\033[4m"
#define REV     "\033[7m"

#define RED     "\033[31m"
#define GRN     "\033[32m"
#define YEL     "\033[33m"
#define BLU     "\033[34m"
#define VIL     "\033[35m"
#define TUR     "\033[36m"

#define RRED     "\033[41m"
#define RGRN     "\033[42m"
#define RYEL     "\033[43m"
#define RBLU     "\033[44m"
#define RVIL     "\033[45m"
#define RTUR     "\033[46m"


static int check_if_on(FILE *file, int Default)
{
  char *env = getenv("DTK_COLOR");
  if(env)
    {
      if(!strncasecmp(env, "true", (size_t) 1) ||
	 !strcasecmp(env, "on") ||
	 !strcmp(env, "1"))
	return 1;
      else if(!strncasecmp(env, "false", (size_t) 1) ||
	      !strncasecmp(env, "off", (size_t) 2) ||
	      !strcmp(env, "0"))
	return 0;
      else // auto case is the default
	if(file && isatty(fileno(file)))
	  return 1;
      else
	return 0;
    }
  if(Default == DTKCOLOR_AUTO)
    {
      if(file && isatty(fileno(file)))
	return 1;
      return 0;
    }
  return Default;
}

void dtkColor::make_color_chars(FILE *file, int color_on)
{
  
  if(check_if_on(file, color_on))
    {
      (void) strcpy(end,END);
      (void) strcpy(bld,BLD);
      (void) strcpy(und,UND);
      (void) strcpy(rev,REV);
  
      (void) strcpy(red,RED);
      (void) strcpy(grn,GRN);
      (void) strcpy(yel,YEL);
      (void) strcpy(blu,BLU);
      (void) strcpy(vil,VIL);
      (void) strcpy(tur,TUR);
      
      (void) strcpy(rred,RRED);
      (void) strcpy(rgrn,RGRN);
      (void) strcpy(ryel,RYEL);
      (void) strcpy(rblu,RBLU);
      (void) strcpy(rvil,RVIL);
      (void) strcpy(rtur,RTUR);
    }
  else
    *end = *bld = *und = *rev
      = *red = *grn = *yel
      = *blu = *vil = *tur
      = *rred = *rgrn = *ryel
      = *rblu = *rvil = *rtur
      = '\0';
}

void dtkColor::reset(FILE *file, int TorF)
{
  if(!file) file = DEFAULT_FILE;
  make_color_chars(file, TorF);
}

dtkColor::dtkColor(FILE *file, int TorF)
{
  if(!file) file = DEFAULT_FILE;
  make_color_chars(file, TorF);
}

// Just need a virtual destructor so that others my inherit this
// class if they like.
dtkColor::~dtkColor(void) {}

int dtkColor::isOn(void)
{
  if(*end)
    return 1;
  else
    return 0;
}
