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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkAugment.h"
#include "dtkColor.h"
#include "utils.h"

#define MAX__STRING  ((size_t) 512)

dtkAugment::dtkAugment(const char *name_in) : dtkBase(DTKAUGMENT_TYPE)
{
  static int count = 0;
  description = dtk_strdup("generic dtkAugment object");
  if(name_in == NULL  || name_in[0] == '\0')
    {
      char str[32];
      sprintf(str,"Augmentation%d",count);
      name = dtk_strdup(str);
    }
  else
    name = dtk_strdup(name_in);
  count++;
}

dtkAugment::~dtkAugment(void)
{
  if(description)
    {
      free(description);
      description = NULL;
    }
  if(name)
    {
      free(name);
      name = NULL;
    }
  invalidate(); // from dtkBase: set magic_number to invalid
}

void dtkAugment::vsetDescription(const char *format, va_list args)
{
  char string[MAX__STRING];
  vsnprintf (string, MAX__STRING-1, format, args);

  if(description)
    free(description);
  description = dtk_strdup(string);
}


void dtkAugment::setDescription(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vsetDescription(format, args);
  va_end(args);
}


// returns a pointer to private description string use
// setDescription() to change this string. Do not mess with this
// memory.
char *dtkAugment::getDescription(void) const
{
  return description;
}


// returns a pointer to private name string. Do not mess with this memory.
char *dtkAugment::getName(void) const
{
  return name;
}


int dtkAugment::preConfig(void) { return REMOVE_CALLBACK; }

int dtkAugment::postConfig(void) { return REMOVE_CALLBACK; }

int dtkAugment::preFrame(void) { return REMOVE_CALLBACK; }

int dtkAugment::postFrame(void) { return REMOVE_CALLBACK; }
