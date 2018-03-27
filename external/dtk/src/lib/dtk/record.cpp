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

#ifdef DTK_ARCH_IRIX
# include <strings.h> /* for function bzero() */
#endif

#ifdef DTK_ARCH_WIN32_VCPP
# include <memory.h>
#endif

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
#include "utils.h"


#define MAX_LENGTH ((size_t) 1023)
#define CHUNCK_SIZE ((size_t) 8) // UNIX: sizeof(long long) == 8
#define _SIZE_PLUS(x)  ( ((size_t)(x)) +  ((((size_t)(x)) % CHUNCK_SIZE)? \
           CHUNCK_SIZE - (((size_t)(x)) % CHUNCK_SIZE): (size_t) 0))

dtkRecord::dtkRecord(size_t lenght_in) :
  dtkBase(DTKRECORD_TYPE)
{
  change_count = 0;
  queue_count = 0;
  event_size = 0;
  queue_add_count = 0;
  current_event = next_event = NULL;
  queue_length = (lenght_in > 1)?lenght_in:DTKRECORD_DEFAULT_LENGTH;
  if((size_t) queue_length > MAX_LENGTH)
    queue_length = MAX_LENGTH;
  event_buffer = NULL;
  account = NULL;

  list = (struct dtkRecord_event *)
    dtk_malloc(sizeof(struct dtkRecord_event)*queue_length);
  // initialize the event list
  int i;
  for(i=0;i<queue_length-1;i++)
    {
      list[i].next = &(list[i+1]);
      list[i].obj  = NULL;
      list[i].buffer = NULL;
    }
  list[i].next = &(list[0]);
  list[i].obj  = NULL;
  list[i].buffer = NULL;
  validate(); // From dtkBase
}


dtkRecord::~dtkRecord(void)
{
  clean_up();
}


void dtkRecord::clean_up(void)
{
  change_count = 0;
  queue_count = 0;
  queue_add_count = 0;
  event_size = 0;
  queue_length = 0;

  if(list)
    {
      free(list);
      next_event = current_event = list = NULL;
    }
  if(event_buffer)
    {
      free(event_buffer);
      event_buffer = NULL;
    }
  if(account)
    {
      struct dtkRecord_account *a = account;
      for(;a;a = account)
	{
	  // Let dtk delete the dtkInput object.
	  account = a->next;
	  free(a);
	}
    }
  invalidate();
}


// debug print
void dtkRecord::print(FILE *file)
{
  file = (file) ? ((FILE *) file) : stdout;

  if(!file) return;

  fprintf(file,"############ debug dtkRecord::print ###################\n"
	  "change_count=%d queue_length=%d "
	  "event_size=%ld event_buffer=0x%lx\n",
	  change_count, queue_length, (unsigned long) event_size,
	  (unsigned long) event_buffer);
  fprintf(file,"list=0x%lx current_event=0x%lx next_event=0x%lx "
	  "queue_count=%d queue_add_count=%d\n",
	  (unsigned long) list, (unsigned long) current_event,
	  (unsigned long) next_event, queue_count, queue_add_count);
  
  fprintf(file,"------------ events list --starting at next_event\n");

  struct dtkRecord_event *l= next_event;
  do
    {
      fprintf(file,"Event at=0x%lx from Obj=0x%lx next=0x%lx buffer=0x%lx\n",
	      (unsigned long) l, (unsigned long) l->obj,
	      (unsigned long) l->next, (unsigned long) l->buffer);
      l = l->next;
    }
  while(l != next_event);


  fprintf(file,"------------ dtkRecord ACCOUNTS ----0x%lx--------\n",
	  (unsigned long) account);
  struct dtkRecord_account *a = account;
  for(;a;a = a->next)
    fprintf(file,"ACCOUNT at=0x%lx forObj=0x%lx size=%ld offset=%ld"
	    " next=0x%lx\n",
	    (unsigned long) a,(unsigned long) a->obj,
	    (unsigned long)a->size,
	    (unsigned long) a->offset,(unsigned long) a->next);
   fprintf(file,"\n");
}


// This one the same as the above but skips events that are not from
// the dtkInput input.  input is a pointer to the device who's data
// your looking form.  Returns NULL is no events is found.
struct dtkRecord_event *dtkRecord::getNextEvent(dtkInput *input)
{
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


// Get the next event in the queue, and dequeue the queue. Returns the
// number of events in the queue before this call.  The pointers
// returned should not be written to by the user.
int dtkRecord::getNextEvent(struct dtkRecord_event **event_return,
			      dtkInput **input_return)
{
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


// Gets the dtkRecord_event from the last thing dequeued in this
// object.
struct dtkRecord_event *dtkRecord::getEvent(void)
{
  return next_event;
}

// gets the dtkInput that caused the event from the last thing
// dequeued in this object.
dtkInput *dtkRecord::getInput(void)
{
  return next_event->obj;
}

// Get the current event which is not queued yet, but will be
// soon. This does not dequeue the queue. Returns the number of events
// in the queue before this call (which is the same as after). The
// pointers returned should not be written to by the user.
struct dtkRecord_event *dtkRecord::poll(void)
{
  return current_event;
}


// add an account with client object *input
int dtkRecord::add(dtkInput *input, size_t entree_size)
{
  if(isInvalid()) return -1;

  if(!input || input->isInvalid() || !entree_size)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkRecord::add(0x%lx, size=%ld) failed:"
		 " the dtkInput object at address "
		 "0x%lx is invalid or the requested queue record "
		 "entree size=0.\n",
		 (unsigned long) input, entree_size,
		 (unsigned long) input);
      return -1;
    }

  struct dtkRecord_account *a;

  for(a = account;a;a = a->next)
    if(a->obj == input)
      {
	dtkMsg.add(DTKMSG_WARN,
		   "dtkRecord::add(0x%lx) failed:"
		   " the object at address 0x%lx already "
		   "has an account.\n",
		   (unsigned long) input, (unsigned long) input);
	return -1;
      }

  struct dtkRecord_account *new_account = (struct dtkRecord_account *)
    dtk_malloc(sizeof(struct dtkRecord_account));

  new_account->obj = input;
  new_account->size = entree_size;
  new_account->next = NULL;
  new_account->offset = (size_t) -1; // starts out with invalid offset

  // append the new account to the end of the account list
  if(!account)
    account = new_account;
  else
    {
      for(a = account;a->next;a = a->next);
      a->next = new_account;
    }

  reset_event_buffer_size();

  return 0;
}


// reset_event_buffer_size() is called after accounts are added or
// removed.

void dtkRecord::reset_event_buffer_size(void)
{

  if(!account)// if there are no accounts
    {
      // flush and reset events
      next_event = current_event;
      queue_count = 0;
      event_size = 0;
      queue_add_count = 0;
      if(event_buffer)
	    free(event_buffer);
      event_buffer = NULL;
      return;
    }

  // recompute the new event record size by adding the size of each
  // event entree for each account
  size_t new_event_size = 0;
  struct dtkRecord_account *a;
  for(a = account;a;a = a->next)
    new_event_size += _SIZE_PLUS(a->size);

  // get memory for the new event buffer
  unsigned char *new_event_buffer =
    (unsigned char *) dtk_malloc(new_event_size*queue_length);
#ifdef DTK_ARCH_WIN32_VCPP
  memset(new_event_buffer, 0, new_event_size*queue_length);
#else
  bzero(new_event_buffer, new_event_size*queue_length);
#endif

  {
    // copy the current input data to the new buffer
    // at the start of the new buffer.
    size_t place_marker = 0;
    for(a = account;a;a = a->next)
      {
	if(a->offset != (size_t) -1)
	  memcpy(&new_event_buffer[place_marker],
		 &(current_event->buffer[a->offset]),a->size);
	a->offset = place_marker;
	place_marker += _SIZE_PLUS(a->size);
      }
  }

  // setup pointers to the event list buffers
  int i;
  for(i=0;i<queue_length;i++)
    {
      list[i].buffer = &(new_event_buffer[new_event_size*i]);
    }

  // flush and reset events. Set the current_event to the
  // first in the list
  next_event = current_event = list;
  queue_count = 0;
  queue_add_count = 0;

  if(event_buffer)
    free(event_buffer);
  event_buffer = new_event_buffer;
  event_size = new_event_size;
  change_count++; // Note that the structures have changed.
}


int dtkRecord::remove(struct dtkRecord_account *a)
{
  struct dtkRecord_account *prev = account;
  for(struct dtkRecord_account *i = account;i;i = i->next)
    {
      if(i == a)
	{
	  if(i == account) // first one
	    account = i->next;
	  else
	    prev->next = i->next;
	  free(i);

	  reset_event_buffer_size();
	  return 0;
	}
      prev = i;
    }

  dtkMsg.add(DTKMSG_WARN, "dtkRecord::remove(0x%lx) failed:"
	     " failed to find account with address 0x%lx.\n",
	     (unsigned long) a,(unsigned long) a);
  return -1;
}

// remove account.  This is automatically called by
// dtkInput::~dtkInput().  Removes a dtkInput's account.
int dtkRecord::remove(dtkInput *input)
{
  for(struct dtkRecord_account *a = account;a;a = a->next)
    if(a->obj == input)
      return remove(a);

  dtkMsg.add(DTKMSG_WARN, "dtkRecord::remove(0x%lx) failed:"
	     " failed to find account for object with address 0x%lx.\n",
	     (unsigned long) input, (unsigned long) input);
  return -1;
}

