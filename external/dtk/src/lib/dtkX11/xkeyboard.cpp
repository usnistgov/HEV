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
#include "_config.h"
#include "config.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkAugment.h"
#include "dtkRecord.h"
#include "dtkInput.h"
#include "dtkInButton.h"
#include "dtkInValuator.h"
#include "dtkInLocator.h"
#include "dtkDisplay.h"
#include "dtkManager.h"

#include "dtkXWindowList.h"
#include "dtkXEvent.h"
#include "dtkInXKeyboard.h"

// the data in the record for this keyboard event is of the form:
// char keys[32*numDisplays]   at begining
// Window winID                at winIDOffset
// int pressed                 at pressedOffset
// int display_number          at display_numberOffset
// KeyCode key                 at KeyCodeOffset

size_t dtkInXKeyboard::getEventSize(Display **dis)
{
  for(numDisplays=0; dis[numDisplays]; numDisplays++);

  // first is  char keys[32*numDisplays]
  winIDOffset = 32*numDisplays;
  pressedOffset = sizeof(Window) + winIDOffset;
  display_numberOffset = sizeof(int) + pressedOffset;
  KeyCodeOffset = sizeof(int) + display_numberOffset;

  return KeyCodeOffset + sizeof(KeyCode);
}


KeyCode dtkInXKeyboard::getKeyCode(KeySym sym, int display_number)
{
  if(xdisplay[display_number])
    return XKeysymToKeycode(xdisplay[display_number], sym);
  else
    return (KeyCode) 0; // error
}


dtkInXKeyboard::
dtkInXKeyboard(dtkRecord *record,
	       // dis is NULL terminated
	       Display **dis,
	       dtkManager *manager_in,
	       const char *name):
  dtkInput(record, event_size = getEventSize(dis), name)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkInXKeyboard::dtkInXKeyboard() failed\n");
      return;
    }
  invalidate();

  if(numDisplays == 0)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkInXKeyboard::dtkInXKeyboard() failed:"
		 "bad argument.\n");
      return;
    }

  setType(DTKINXKEYBOARD_TYPE);
  manager = manager_in;

  if(manager && manager->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkInXKeyboard::dtkInXKeyboard() failed:"
		 "bad argument: invalid dtkManager object.\n");
      return;
    }

  xdisplay = NULL;
  write_buffer = NULL;
  xkc_esc = NULL;

  int i,j;

  for(i=0; i<numDisplays; i++)
    for(j=0; j<i; j++)
      if(dis[i] == dis[j])
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkInXKeyboard::dtkInXKeyboard() failed:"
		     " two X displays \"%s\" and \"%s\".\n"
		     "They should not be the same.\n",
		     DisplayString(dis[i]),
		     DisplayString(dis[j]));
	  // bail with no displays setup
	  numDisplays = 0;
	  clean_up();
	  return;
	}

  write_buffer = (unsigned char *) malloc(event_size);

  xkc_esc = (KeyCode *) malloc(sizeof(KeyCode)*numDisplays);

  setDescription("a dtkInXKeyboard object for X keyboard input");

  xdisplay = (Display **) malloc(sizeof(Display *)*numDisplays);
  for(i=0;i<numDisplays;i++)
    xdisplay[i] = dis[i];

  validate();

  for(i=0;i<numDisplays;i++)
    {
      xkc_esc[i] = getKeyCode(XK_Escape, i);
      if(!xkc_esc[i])
	{
	  dtkMsg.add(DTKMSG_NOTICE,
		     "dtkInXKeyboard::dtkInXKeyboard():"
		     " failed to get the x key code for"
		     " X key sym XK_Escape"
		     " on display %p \"%s\"\n.",
		     dis[i], DisplayString(dis[i]));
	}
    }  
}

void dtkInXKeyboard::escapeUnsetsISRUNNING(dtkManager *manager_in)
{
  // This is the only reason that we need manager.
  manager = manager_in;

  if(manager && manager->isInvalid())
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkInXKeyboard::setEscapeSet() failed:"
		 "bad argument: invalid dtk object at address %p.\n",
		 manager);
      manager = NULL;
    }
}


void dtkInXKeyboard::clean_up(void)
{
  if(write_buffer)
    {
      free(write_buffer);
      write_buffer = NULL;
    }

  if(xkc_esc)
    {
      free(xkc_esc);
      xkc_esc =NULL;
    }

  if(xdisplay)
    {
      free(xdisplay);
      xdisplay = NULL;
    }

  if(manager)
    manager = NULL;

  invalidate();
}


dtkInXKeyboard::~dtkInXKeyboard(void)
{
  clean_up();
}


// returns KeyCode, winID and display_number
// returns 1 if key is pressed
// returns 0 if key is released
// Poll the from the current state if event = NULL.
int dtkInXKeyboard::read(KeyCode *key_return,
			  const struct dtkRecord_event *event,
			  int *display_number, Window *winID)
{
  unsigned char *rec_data = (unsigned char *) readRaw(event);

  memcpy(key_return, &rec_data[KeyCodeOffset], sizeof(KeyCode));
  if(winID)
    memcpy(winID, &rec_data[winIDOffset], sizeof(Window));
  if(display_number)
    memcpy(display_number, &rec_data[display_numberOffset], sizeof(int));

  int pressed;
  memcpy(&pressed, &rec_data[pressedOffset], sizeof(int));

  return pressed;
}


#define POLL_KEY(K,X) ((int)((X)[(K)/8] & (01 << ((K)%8))))

// returns winID and display_number to the pointer arguments
// returns 1 if key is pressed
// returns 0 if key is released
// Poll the from the current state if event = NULL.
// If display_number is non-NULL and *display_number is a valid value
// than the key will be queried from the display that corresponds to
// *display_number, else if display_number is nonNULL it will be set
// to the display number from the event.
int dtkInXKeyboard::read(KeyCode key, const struct dtkRecord_event *event,
			 Window *winID, int *display_number)
{
  unsigned char *rec_data = (unsigned char *) readRaw(event);

  if(winID)
    memcpy(winID, &rec_data[winIDOffset], sizeof(Window));

  int displayNumber;

  if(display_number &&  numDisplays > (*display_number) && 
     (*display_number) > -1)
    displayNumber = *display_number;
  else
    {
      memcpy(&displayNumber,&(rec_data[display_numberOffset]),sizeof(int));
      if(display_number)
	*display_number = displayNumber;
    }

  return (POLL_KEY(key, rec_data+displayNumber*32))?1:0;
}

int dtkInXKeyboard::getNumberOfDisplays(void)
{
  return numDisplays;
}

/******* write() is not for the dtk user *********/
// write keys[32] from XQueryKeymap
// and KeyCode from an XEvent.
// pressed = 1 for XEvent key press.
// pressed = 0 for XEvent key release.
// keys points to 32 bytes for each X server.
int dtkInXKeyboard::write(const char *keys, KeyCode key, int pressed,
			   Window winID, int display_number)
{
  // I need to put all this data into one continuous piece of memory.

  // first stuff all the data into write_buffer
  memcpy(write_buffer, keys, 32*numDisplays);
  KeyCode k = key;
  memcpy(&write_buffer[KeyCodeOffset], &k, sizeof(KeyCode));
  int i = pressed;
  memcpy(&write_buffer[pressedOffset], &i, sizeof(int));

  Window w = winID;
  memcpy(&write_buffer[winIDOffset], &w, sizeof(Window));

  i = (display_number < numDisplays && display_number >= 0)?
    display_number: 0;
  memcpy(&write_buffer[display_numberOffset], &i, sizeof(int));

  // optional check for escape key to set dtkManager::state isRunning bit
  if(manager && pressed && key == xkc_esc[i])
    manager->state &= ~(DTK_ISRUNNING);

  // now stuff write_buffer into the event buffer
  return writeRaw(write_buffer);
}


  
