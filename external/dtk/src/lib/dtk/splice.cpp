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

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"


// dtk_splice() builds a data structure like
// { "str0", "str1", "str2", ... }
// The data (...) must be NULL terminated.
// returns length of data written on success.
// returns 0 on failure.

size_t dtk_splice(size_t length, char *buff, ...)
{
  char *limit = buff + length;
  va_list ap;
  va_start(ap, buff);
  char *str = va_arg(ap, char *);
  size_t len = 0;

  while(str && (buff + (length = strlen(str) + 1)) < limit)
    {
      strcpy(buff, str);
      buff += length;
      len += length;
      str = va_arg(ap, char *);
    }
  va_end(ap);

  if(str)
    return (size_t) 0; // error buffer not big enough
  else
    return len;  // success
}
