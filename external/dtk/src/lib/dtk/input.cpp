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


dtkInput::dtkInput(dtkRecord *record_in, size_t size_in,
		       const char *name) :
  dtkAugment(name)
{
  setType(DTKINPUT_TYPE);
  if(!record_in || record_in->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkInput::dtkInput(%p,%ld) failed:\n"
		 "%p points to an invalid dtkRecord.\n",
		 record_in,size_in,record_in);
      return;
    }

  if(!size_in)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkInput::dtkInput(%p,%ld) failed:\n"
		 "%ld is an invalid queue entree size.\n",
		 record_in,size_in,size_in);
      return;
    }
  _isQueued = 0;
  record = NULL;
  size = 0;
  change_count = 0;
  offset = 0;
  validate(); // validate so that it can be added

  if(record_in->add(this, size_in))
    {
      clean_up();
       // record->add() will spew more details if true
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkInput::dtkInput(%p,%ld) failed:\n",
		 record_in,size_in);
      return;
    }

  record = record_in;
  size = size_in;
  queue_length = record->queue_length;
  sync_account_with_record();
}


// If dtkRecord and dtkInput codes are consistant this method call
// will not fail.

int dtkInput::sync_account_with_record(void)
{
  struct dtkRecord_account *a;
  for(a = record->account;a;a = a->next)
    if(a->obj == this)
      {
	change_count = record->change_count;
	offset = a->offset;
	break;
      }
  if(a)
    return 0;
  else
    return -1;
}


// Over write this objects entree in the last (newest) event, and if
// this is queued this will cause a new event record to be started and
// the event record before will be the new top event of the queue
// where write()s after this call will be written.
int dtkInput::writeRaw(const void *buffer)
{
  if(record->change_count != change_count)
    {
      if(isValid())
	{
	  if(sync_account_with_record())
	    {
	      dtkMsg.add(DTKMSG_WARN,
			 "dtkInput::writeRaw() failed: object %p not"
			 " found in dtkRecord.\n", this);
	      return -1;
	    }
	}
      else
	{
	  dtkMsg.add(DTKMSG_WARN,
		     "dtkInput::writeRaw() failed: "
		     "object %p is invalid.\n",
		     this);
	  return -1;
	}
    }

  memcpy(&(record->current_event->buffer[offset]),buffer,size);

  if(_isQueued)
    {
      record->current_event->obj = this;
      // make the current_event and the current_event->next the same.
      memcpy(record->current_event->next->buffer,
	     record->current_event->buffer,record->event_size);
      record->current_event = record->current_event->next;
      record->queue_count++;
      record->queue_add_count++;
      // Check if the queue is over flowing.
      if(record->current_event == record->next_event)
	{
	  record->next_event = record->next_event->next;
	  record->queue_count--;
	}
    }

  return 0;
}

// Returns a pointer to the last written event entree for this input
// object.

void *dtkInput::readRaw(const struct dtkRecord_event *event)
{
  if(record->change_count != change_count)
    sync_account_with_record();

  if(event)
    return (void *) &(event->buffer[offset]);
  else
    return (void *) &(record->current_event->buffer[offset]);
}

void dtkInput::clean_up(void)
{
  if(record)
    {  
      dtkRecord *r = record;
      record = NULL;
      r->remove(this);
    }

  _isQueued = 0;
  size = 0;
  change_count = 0;
  offset = 0;
}


dtkInput::~dtkInput(void)
{
  clean_up();
}

// set it to be a queued device or not
void dtkInput::queue(int truth)
{
  if(truth)
    _isQueued = 1;
  else
    _isQueued = 0;
}

int dtkInput::isQueued()
{
  return _isQueued;
}
