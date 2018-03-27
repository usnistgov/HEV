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
#include "types.h"
#include "dtkAugment.h"
#include "dtkRecord.h"
#include "dtkInput.h"
#include "dtkDequeuer.h"


dtkDequeuer::dtkDequeuer(dtkRecord *record_in):
  dtkBase(DTKDEQUEUER_TYPE)
{
  queue_count = 0;
  queue_add_count = 0;
  change_count = 0;
  next_event = NULL;
  record = record_in;

  if(!record || record->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR, "dtkDequeuer::dtkDequeuer() failed:"
	     " passed invalid dtkRecord object.\n");
      record = NULL;
      return;
    }

  sync_with_record();
  validate();
}


dtkDequeuer::~dtkDequeuer(void)
{
  queue_count = 0;
  queue_add_count = 0;
  change_count = 0;
  next_event = NULL;
  record = NULL;
  invalidate();
}


void dtkDequeuer::sync_with_record(void)
{
  if(queue_add_count != record->queue_add_count)
    {
      int count_diff = record->queue_add_count - queue_add_count;

      if(!(count_diff + queue_count >= record->queue_length))
	{
	  queue_count += count_diff;
	  queue_add_count = record->queue_add_count;
	}
      else // queue buffer over run
	{
	  next_event = record->current_event->next;
	  queue_count = record->queue_length - 1;
	  queue_add_count = record->queue_add_count;
	}
    }

  if(change_count != record->change_count)
    {
      queue_count = record->queue_count;
      next_event = record->next_event;
      change_count = record->change_count;
      queue_add_count = record->queue_add_count;
    }
}

// Get the next event in the queue, and dequeue the queue. Returns the
// number of events in the queue before this call.  The pointers
// returned should not be written to by the user.
int dtkDequeuer::getNextEvent(struct dtkRecord_event **event_return,
				dtkInput **input_return)
{
  // first sync with the dtkRecord
  if(queue_add_count != record->queue_add_count ||
     change_count != record->change_count)
    sync_with_record();

  if(queue_count)
    {
      if(event_return)
	*event_return = next_event;
      if(input_return)
	*input_return = next_event->obj;
      next_event = next_event->next;
      int return_val = queue_count;
      queue_count--;
      return return_val;
    }
  else
    {
      if(event_return)
	*event_return = NULL;
      if(input_return)
	*input_return = NULL;
      return 0;
    }
}

// This one the same as the above but skips events that are not from
// the dtkInput input.  input is a pointer to the device who's data
// your looking form.  Returns NULL is no events is found.
struct dtkRecord_event *dtkDequeuer::getNextEvent(dtkInput *input)
{
  // first sync with the dtkRecord
  if(queue_add_count != record->queue_add_count ||
     change_count != record->change_count)
    sync_with_record();

  while(queue_count && input != next_event->obj)
    {
      next_event = next_event->next;
      queue_count--;
    }

  if(queue_count) // input == next_event->obj
    {
      struct dtkRecord_event *return_event = next_event;
      next_event = next_event->next;
      queue_count--;
      return return_event;
    }
  else
    return NULL;
}

// Gets the dtkRecord_event from the last thing dequeued in this
// object.
struct dtkRecord_event *dtkDequeuer::getEvent(void)
{
  return next_event;
}


// gets the dtkInput that caused the event from the last thing
// dequeued in this object.
dtkInput *dtkDequeuer::getInput(void)
{
  return next_event->obj;
}
