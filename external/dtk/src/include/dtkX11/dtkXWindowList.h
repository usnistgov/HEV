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
/****** dtkXWindowList.h ******/

// These display strings should be the same for the same Xserver
// display.  X11R5 (6?) does not provide a direct way to query a
// unique display string.  For example the display string name on host
// foo with a Xclient on foo can be localhost:0.0 or foo:0.0 or :0.0
// (no hostname).  Xlib does not provide a direct way to get a unique
// hostname assocated with a connection.  One could open an X
// connection and plant an ID (by using an X property) in the Xserver
// and use that to see if you have connected to the same Xserver
// display, but I'm not going to go there.

/* ... therefore display strings that are associated with connections
 * to the same display number and screen should have the same display
 * string.  NULL is a good display string to use.
 */


struct dtkXWindowList_display
{
  char *name; // display connection string
  struct dtkXWindowList_win *winList;
  struct dtkXWindowList_display *next;
};

// Each display connection can have many windows.
struct dtkXWindowList_win
{
  char *name; // user given name
  Window winID; // X window ID
  struct dtkXWindowList_win *next;
};


class dtkXWindowList: public dtkAugment
{
 public:

  dtkXWindowList(const char *name=NULL);
  virtual ~dtkXWindowList(void);

  // Do not write to the list returned.
  inline struct dtkXWindowList_display *getDisplayList(void)
    { return displayList; }

  // Get a window ID by it's name.  The window name can be set by the
  // dtkDisplay that made this list.  Returns 0 is not found.  This
  // "named window" can be used to find particular windows of interest
  // in your application.  *displayString is a return string, if it is
  // non-NULL.  Don't mess with the displayString memory returned.
  Window getWindowID(const char *name, char **displayString=NULL);

  // returns 0 on success, and -1 on error. displayName and winID, and
  // name are unique.
  int add(const char *displayName, Window winID, const char *name=NULL);

  // Remove a window ID from the list.  returns 0 on success, and -1 if
  // not found. displayName and winID, and name are unique.
  int remove(const char *displayName, Window winID);
  int remove(const char *name);

 private:

  int _add(struct dtkXWindowList_display *d,
	   Window winID, const char *name);

  struct dtkXWindowList_display *displayList;
};
