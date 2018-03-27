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
/****** dtkXEvent.h ******/
/* Very limited and light X events wrapper that works with multiple X
 * display connections. */

// You'll need to use XKeysymToKeycode in your keyboard code.

#define DTKXEVENT_EVENTMASK   (ButtonPressMask| ButtonReleaseMask|\
                                 KeyPressMask| KeyReleaseMask |\
                                 PointerMotionMask | LeaveWindowMask |\
                                 EnterWindowMask | StructureNotifyMask|\
                                 ExposureMask)

class dtkXEvent : public dtkBase
{
public:

  // This will make a copy of the dtkXWindowList that is passed in.
  dtkXEvent(dtkXWindowList *windowList,
	    long xevent_mask=DTKXEVENT_EVENTMASK);

  virtual ~dtkXEvent(void);
 
  // returns 1 if there is an XEvent else returns 0
  int getNextEvent(XEvent *xe); // non-blocking

  // Only one X client may do XGrabKey() per X display, there may be
  // problems with this.
  void setKeyGrabing(void);
 
  // Get the X Display pointer from the last event.
  Display *getDisplay(void);

  // Is any window mapped.
  int isWindowMapped();

  inline int getWindowWidth(void) { return width[last_window_index]; }
  inline int getWindowHeight(void) { return height[last_window_index]; }

private:

  int numWindows;
  int numDisplays;

  int last_window_index;

  // array of X display connections
  Display **xdisplay;
  Display *lastDisplay;
  int *save_auto_repeat;
  long xevent_mask;

  int key_grabing; // key_grabing flag 0 or 1 = no or yes

  Window *windowIDs;
  // per window
  int *isMapped;
  int *width, *height;

  void clean_up(void);

};
