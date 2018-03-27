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
// Just a small X event wrapper class that does not block to get X
// events on multipule X servers and windows.  This tries not to hide
// any of the under lying X-ness.


//XChangeKeyboardControl
//XGetKeyboardControl
//XAutoRepeatOn
//XAutoRepeatOff

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
#include "utils.h"

#include "dtkXWindowList.h"
#include "dtkXEvent.h"

void dtkXEvent::clean_up(void)
{
  if(xdisplay)
    {
      int i = 0;
      for(;i<numDisplays;i++)
	{
	  if(xdisplay[i])
	    {
	      if(save_auto_repeat[i] == AutoRepeatModeOn)
		XAutoRepeatOn(xdisplay[i]);
	      XCloseDisplay(xdisplay[i]);
	    }
	}
      free(xdisplay);
      free(windowIDs);
      free(width);
      free(height);
      free(save_auto_repeat);
      free(isMapped);
    }

  lastDisplay = NULL;
  invalidate();
}


// Get the X Display pointer from the last event.
Display *dtkXEvent::getDisplay(void)
{
  return lastDisplay;
}


dtkXEvent::dtkXEvent(dtkXWindowList *windowList, long xmask):
  dtkBase(DTKXEVENT_TYPE)
{
  /*********** initialize data *****************************/

  // per X display connection
  xdisplay = NULL;
  lastDisplay = NULL;
  save_auto_repeat = NULL;

  key_grabing = 0; // key_grabing flag 0 or 1 = no or yes

  // per window
  isMapped = NULL;

  xevent_mask = xmask;

  numWindows = 0;
  numDisplays = 0;

  /*********************************************************/

  {
    // get the number of displays and windows
    struct dtkXWindowList_display *ds = windowList->getDisplayList();
    for(;ds ; ds = ds->next)
      {
	numDisplays++;
	struct dtkXWindowList_win *w = ds->winList;
	for(;w ; w = w->next)
	  numWindows++;
      }
  }

  xdisplay = (Display **) dtk_malloc(sizeof(Display *)*numDisplays);
  save_auto_repeat = (int *) dtk_malloc(sizeof(int)*numDisplays);
  isMapped = (int *) dtk_malloc(sizeof(int)*numWindows);
  windowIDs = (Window *) dtk_malloc(sizeof(Window)*numWindows);
  width = (int *) dtk_malloc(sizeof(int)*numWindows);
  height = (int *) dtk_malloc(sizeof(int)*numWindows);

  int i=0; // display index
  // Initialize some private data values, so we have a way to clean up
  // if constructor fails.
  for(i=0;i<numDisplays;i++)
    xdisplay[i] = NULL;

  int j=0; // window index
  i = 0; // display index

  struct dtkXWindowList_display *ds = windowList->getDisplayList();
  if(!ds)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkXEvent::dtkXEvent() failed:"
		 " failed to find a display.\n");
      clean_up();
      return;
    }
      
  for(;ds ; ds = ds->next)
    {
      xdisplay[i] = XOpenDisplay(ds->name);
      if(!xdisplay[i])
	{
	  
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkXEvent::dtkXEvent() failed:"
		     " failed to get X connection to display \"%s\"\n",
		     ds->name);
	  clean_up();
	  return;
	}
      
      XKeyboardState kbs;
      XGetKeyboardControl(xdisplay[i], &kbs);
      save_auto_repeat[i] = kbs.global_auto_repeat;

      struct dtkXWindowList_win *w = ds->winList;
      for(; w; w = w->next)
	{
	  long xeventmask = xmask;
	  XWindowAttributes wattr;
	  XGetWindowAttributes(xdisplay[i], w->winID, &wattr);
	  // X will not let you get ButtonPress events if there is
	  // something else getting button press events.
	  if(wattr.all_event_masks & ButtonPressMask)
	    xeventmask &= ~ButtonPressMask;
	  xeventmask &= ~SubstructureRedirectMask;
	  XSelectInput(xdisplay[i], w->winID, xeventmask);
	  isMapped[j] = 1;
	  width[j] = wattr.width;
	  height[j] = wattr.height;
	  windowIDs[j] = w->winID;
	  j++; // window index
	}
      i++; // display index
    }
  lastDisplay = xdisplay[0];

  validate();
}

// This will not wait for an event.
int dtkXEvent::getNextEvent(XEvent *xe)
{
  if(isInvalid()) return 0;
  int i;
  for(i=0;i<numDisplays;i++)
    {
      if(XCheckMaskEvent(xdisplay[i], xevent_mask, xe))
	{
	  lastDisplay = xdisplay[i];
	  switch(xe->type)
	    {
	    case KeyPress:
	      {
		//printf("line=%d file=%s keycode =%d\n",
		//__LINE__,__FILE__, ((XKeyEvent *)xe)->keycode);
		if(key_grabing)
		  XGrabKey(lastDisplay, xe->xkey.keycode,
			   AnyModifier, xe->xany.window, True,
			 GrabModeAsync, GrabModeAsync);
		break;
	      }

	    case LeaveNotify:
	      if(save_auto_repeat[i] == AutoRepeatModeOn)
		XAutoRepeatOn(lastDisplay);
	    break;

	    case EnterNotify:
	      if(save_auto_repeat[i] == AutoRepeatModeOn)
		XAutoRepeatOff(lastDisplay);
	      break;
	    }

	  // find the window index
	  int j =0;
	  for(;j<numWindows;j++)
	    if(windowIDs[j] == xe->xany.window)
	      break;

	  if(j == numWindows)
	    return 1;

	  last_window_index = j;
	  
	  switch(xe->type)
	    {
	    case ConfigureNotify:
	      {
		width[j] = xe->xconfigure.width;
		height[j] = xe->xconfigure.height;
	      }
	      break;
	      
	      // This will happen when a windows is deiconified.
	    case MapNotify:
	      isMapped[j] = 1;
	      break;
	      
	      // This will happen when a windows is iconified.
	    case UnmapNotify:
	      isMapped[j] = 0;
	      break;
	    }

	  return 1;
	}
    }

  return 0;
}
    

dtkXEvent::~dtkXEvent(void)
{
  clean_up();
}

void dtkXEvent::setKeyGrabing(void)
{
  key_grabing = 1;
}


int dtkXEvent::isWindowMapped()
{
  for(int j=0;j<numWindows;j++)
    {
      if(isMapped[j])
	return 1;
    }
  return 0;
}
