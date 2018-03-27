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

//struct dtkXWindowList_display *displayList;
//struct dtkXWindowList_win *currentWindowList;

dtkXWindowList::dtkXWindowList(const char *name)
  : dtkAugment(name)
{
  setType(DTKXWINDOWLIST_TYPE);
  setDescription("list of X windows display strings and window IDs");
  displayList = NULL;
  validate();
}

dtkXWindowList::~dtkXWindowList(void)
{
  while(displayList)
    {
      struct dtkXWindowList_display *d = displayList;
      displayList = displayList->next;

      struct dtkXWindowList_win *win = d->winList;
      while(win)
	{
	  struct dtkXWindowList_win *w = win;
	  win = win->next;
	  if(w->name)
	    free(w->name);
	  free(w);
	}
      if(d->name)
	free(d->name);
      free(d);
    }
}

// Get a window ID by it's name.  The window name can be set by the
// dtkDisplay that made this list.  Returns 0 is not found.  This
// "named window" can be used to find particular windows of interest
// in your application.
Window dtkXWindowList::
getWindowID(const char *name, char **displayString)
{
  struct dtkXWindowList_display *d = displayList;
  for(; d; d = d->next)
    {
      struct dtkXWindowList_win *w = d->winList;
      for(; w; w = w->next)
	{
	  if(w->name && !strcmp(w->name, name))
	    {
	      *displayString = d->name;
	      return w->winID;
	    }
	}
    }

  return (Window) 0; // not found
}

int dtkXWindowList::
_add(struct dtkXWindowList_display *d, Window winID, const char *name)
{
  struct dtkXWindowList_win *w = (struct dtkXWindowList_win *)
    dtk_malloc(sizeof(struct dtkXWindowList_win));

  if(name) w->name = dtk_strdup(name);
  else w->name = NULL;
  w->winID = winID;
  w->next = d->winList;
  d->winList = w;

  return 0;
}

// returns 0 on success, and -1 on error (displayName/winID or name
// not unique).
int dtkXWindowList::
add(const char *displayName, Window winID, const char *name)
{
  if(winID == (Window) 0) return -1;

  struct dtkXWindowList_display *d = displayList;
  for(; d; d = d->next)
    {
      if((!displayName && !(d->name)) ||
	 ((displayName) && (d->name) && !strcmp(displayName, d->name)))
	{
	  struct dtkXWindowList_win *w = d->winList;
	  for(; w; w = w->next)
	    if(winID == w->winID ||
	       (w->name && name && !strcmp(w->name, name)))
	      return -1; // already present
	  return _add(d, winID, name);
	}
    }
  d = (struct dtkXWindowList_display *) 
    dtk_malloc(sizeof(struct dtkXWindowList_display));
  if(displayName) d->name = dtk_strdup(displayName);
  else d->name = NULL;
  d->next = displayList;
  d->winList = NULL;
  displayList = d;

  return _add(d, winID, name);
}

// Remove a window ID from the list.  returns 0 on success, and -1 if
// not found. displayName and winID, and name are unique.
int dtkXWindowList::remove(const char *displayName, Window winID)
{
  struct dtkXWindowList_display *d = displayList;
  for(; d; d = d->next)
    {
      if((!displayName && !(d->name)) ||
	 ((displayName) && (d->name) && !strcmp(displayName, d->name)))
	{
	  struct dtkXWindowList_win *w = d->winList;
	  struct dtkXWindowList_win *prev = NULL;
	  for(; w; w = w->next)
	    {
	      if(winID == w->winID)
		{
		  if(prev)
		    prev->next = w->next;
		  else
		    d->winList = w->next;
		  if(w->name)
		    free(w->name);
		  free(w);
		  return 0;
		}
	      prev = w;
	    }
	}
    }

  return -1;
}

int dtkXWindowList::remove(const char *name)
{
  if(!name) return -1;

  struct dtkXWindowList_display *d = displayList;
  for(; d; d = d->next)
    {
      struct dtkXWindowList_win *w = d->winList;
      struct dtkXWindowList_win *prev = NULL;
      for(; w; w = w->next)
	{
	  if(w->name && !strcmp(name,w->name))
	    {
	      if(prev)
		prev->next = w->next;
	      else
		d->winList = w->next;
	      if(w->name)
		free(w->name);
	      free(w);
	      return 0;
	    }
	  prev = w;
	}
    }

  return -1;
}
