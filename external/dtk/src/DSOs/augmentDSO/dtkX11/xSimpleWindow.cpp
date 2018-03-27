/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
// dtk DSO plug-in for making one X window.  Can be handy for testing
// things.

#include <stdlib.h>
#include <dtk.h>
#include <dtkX11.h>
#include <dtk/dtkDSO_loader.h>

class XSimpleWindow : public dtkAugment
{
public:

  XSimpleWindow(dtkManager *manager);
  ~XSimpleWindow(void);

private:

  dtkManager *manager;
  Display *display;
  Window win; 
  dtkXWindowList *windowList;

  void clean_up(void);
};


XSimpleWindow::XSimpleWindow(dtkManager *manager_in):
  manager(manager_in)
{
  display = NULL;
  win = 0;
  windowList = 0;

  dtkAugment *a = NULL;
  dtkXWindowList *winList = NULL;

  setDescription("Opens a simple top level X window");
  
  display = XOpenDisplay(NULL);

  if(!display)
  {
    dtkMsg.add(DTKMSG_ERROR,"XOpenDisplay() failed: ");
    goto XSimpleWindow_error;
  }
 
  win = XCreateSimpleWindow(display, RootWindow(display, DefaultScreen(display)),
			    20, 20, 400, 400, 0, 0, CopyFromParent);

  if(!win)
  {
    dtkMsg.add(DTKMSG_ERROR,
	       "XCreateSimpleWindow() failed: ");
    goto XSimpleWindow_error;
  }

  XMapWindow(display, win);
  XFlush(display);

  do 
    if((windowList = dynamic_cast<dtkXWindowList *>
	(a = manager->getNext(a))))
      break;
  while(a);


  if(!winList)
  {
    winList = windowList = new dtkXWindowList;
    if(manager->add(winList))
      {
	goto XSimpleWindow_error;
      }
  }

  winList->add(NULL, win);

  // dtkDisplay objects start out invalid.
  validate();

  return;  // success


  XSimpleWindow_error:

  clean_up();
  dtkMsg.append("XSimpleWindow::XSimpleWindow() failed:\n"
		"with environment variable DISPLAY=\"%s\"",
		getenv("DISPLAY"));
}


XSimpleWindow::~XSimpleWindow(void)
{
  clean_up();
}

void XSimpleWindow::clean_up(void)
{
  if(windowList)
    {
      if(manager)
	manager->remove(windowList);
      delete windowList;
      windowList = NULL;
    }

  if(win)
    {
      if(display) XDestroyWindow(display, win);
      win = 0;
    }

  if(display)
    {
      XCloseDisplay(display);
      display = NULL;
    }
}

/************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO plug-in files are required to declare these
 * two functions.  These functions are called by the loading program
 * to get your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
  return new XSimpleWindow(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
