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

#include "types.h"
#include "dtkBase.h"


#define DTK_INVALID_TYPE      ((u_int32_t) 0x057EC941)

dtkBase::dtkBase(u_int32_t in)
{
  type = in;
  magic_number = DTK_INVALID_TYPE;
}

dtkBase::~dtkBase(void)
{
  magic_number = DTK_INVALID_TYPE;
}

void dtkBase::setType(u_int32_t in)
{
  type = in;
  magic_number = DTK_INVALID_TYPE;
}

u_int32_t dtkBase::getType(void) const
{
  return type;
}

int dtkBase::isValid(u_int32_t in) const
{
  if(in == DTK_ANY_TYPE)
    return (type == magic_number);
  else
    return (type == magic_number && type == in);
}

int dtkBase::isInvalid(void) const
{
  return (type != magic_number);
}

void dtkBase::validate(void)
{
  magic_number = type; // now it's valid
}

void dtkBase::invalidate(void)
{
  magic_number = DTK_INVALID_TYPE;
}
