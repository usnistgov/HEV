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
#include "dtkInButton.h"


struct dtkButton_event
{
  u_int32_t state; // 0 or 1
  int button; // 0, 1, 2, ... 31
};

dtkInButton::dtkInButton(dtkRecord *rec, int number_in,
			 const char *name):
  dtkInput(rec, sizeof(struct dtkButton_event), name)
{
  _number = number_in;
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,"dtkInButton::dtkInButton(%p, %d, \"%s\") "
		 "failed:\n",rec,_number,name);
      return;
    }

  setType(DTKINBUTTON_TYPE);
  // need to revalidate after calling dtkBase::setType()
  validate();

  if(_number > 8*((int) sizeof(u_int32_t)) || _number < 0)
    {
      invalidate();
      dtkMsg.add(DTKMSG_ERROR,"dtkInButton::"
		 "dtkInButton(0x%lx,%d,\"%s\") failed:"
		 " you can't have %d buttons.\n",rec,_number,name,_number);
      return;
    }
}

dtkInButton::~dtkInButton(void) {}

u_int32_t dtkInButton::read(const struct dtkRecord_event *rec)
{
  struct dtkButton_event *bevent =
    (struct dtkButton_event *) readRaw(rec);

  if(bevent)
    {
      return bevent->state;
    }
  else
    {
      dtkMsg.add(DTKMSG_WARN, "dtkInButton::read(0x%lx) failed.\n",
	     rec);
      return (u_int32_t) -1;
    }
}

// return the state of the button within the event (0 or 1)
int dtkInButton::read(int *button_num, const struct dtkRecord_event *rec)
{
  struct dtkButton_event *bevent =
    (struct dtkButton_event *) readRaw(rec);

  if(bevent)
    {
      int button = bevent->button;
      if(button_num)
	*button_num = button;

      return (bevent->state & (0x01 << button))?1:0;
    }
  else
    {
      dtkMsg.add(DTKMSG_WARN, "dtkInButton::read(0x%lx, %p) failed.\n",
	     (unsigned long) button_num, rec);
      return -1;
    }
}


int dtkInButton::read(int button, const struct dtkRecord_event *rec)
{
  struct dtkButton_event *bevent =
    (struct dtkButton_event *) readRaw(rec);
  if(bevent)
    {
      if(button < 8*((int) sizeof(u_int32_t)) && button >= 0)
	return (bevent->state & (0x01 << button))?1:0;
    }

  // else
  dtkMsg.add(DTKMSG_WARN, "dtkInButton::read(%d, 0x%lx) failed.\n",
	     button, rec);
  return -1;
}


int dtkInButton::write(int button_num, int val)
{
  if(button_num < 8*((int) sizeof(u_int32_t)) && button_num >= 0)
    {
      struct dtkButton_event *prev_event =
	(struct dtkButton_event *) readRaw();

      struct dtkButton_event new_event;
      new_event.button = button_num;
      new_event.state = prev_event->state;

      if(val) // set the bit for button_num
	new_event.state |= (0x01 << button_num);
      else
	new_event.state &= ~(0x01 << button_num);

      return writeRaw(&new_event);
    }
  else
    {
      dtkMsg.add(DTKMSG_WARN, "dtkInButton::write(%d, %d) failed:"
		 " bad button number (%d).\n",
	     button_num, val, button_num);
      return -1;
    }
}


// Use this if the button is not queued.

int dtkInButton::write(u_int32_t state)
{
  struct dtkButton_event b = { state, 0 };

  return writeRaw(&b);
}

