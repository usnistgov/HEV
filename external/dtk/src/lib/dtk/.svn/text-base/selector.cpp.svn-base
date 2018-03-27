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
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkAugment.h"
#include "dtkRecord.h"
#include "dtkInput.h"
#include "dtkInSelector.h"

dtkInSelector::dtkInSelector(dtkRecord *rec, int number_in,
				 const char *name):
  dtkInput(rec, sizeof(int32_t)*number_in, name)
{
  if(isInvalid()) // dtkInput::dtkInput() should validate()
    {
      dtkMsg.add(DTKMSG_ERROR,"dtkInSelector::dtkInSelector(%p,"
		 "%d,\"%s\") failed:\n",
		 rec, _number, name);
      return;
    }
  setType(DTKINVALUATOR_TYPE);
  // need to re-validate after calling dtkBase::setType()
  validate();
  _number = number_in;
}

dtkInSelector::~dtkInSelector(void) {}

int dtkInSelector::number(void) const
{
  return _number;
}



int32_t *dtkInSelector::read(const struct dtkRecord_event *rec)
{
  return (int32_t *) readRaw(rec);
}

int dtkInSelector::read(int32_t *x_out, const struct dtkRecord_event *rec)
{
  int32_t *x = (int32_t *) readRaw(rec);
  if(x)
    {
      memcpy(x_out,x,sizeof(int32_t)*_number);
      return 0;
    }
  else
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkInSelector::read(%p,%p) failed.\n",
		 x_out,rec);
      return -1;
    }
}

int32_t dtkInSelector::read(int index, const struct dtkRecord_event *rec)
{
  int32_t *x = (int32_t *) readRaw(rec);
  if(x && index < _number)
    {
      return x[index];
    }
  else
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkInSelector::read(%d,%p) failed.\n",
		 index, rec);
      return 0;
    }
}

int dtkInSelector::write(const int32_t *x)
{
  return writeRaw(x);
}


