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
#include <GL/glx.h>
#include <GL/gl.h>

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
#include "dtkBasicDisplay.h"
#include "utils.h"

#include "dtkXWindowList.h"
#include "dtkXEvent.h"
#include "dtkGLXSimpleDisplay.h"

#define X 0
#define Y 0
#define W 400
#define H 400


void dtkGLXSimpleDisplay::clean_up(void)
{
  if(windowList)
    {
      if(manager->check(windowList))
	manager->remove(windowList);
      delete windowList;
      windowList = NULL;
    }
  if(win)
    {
      if(dpy) XDestroyWindow(dpy, win);
      win = 0;
    }
  if(cmap)
    {
      if(dpy) XFreeColormap(dpy, cmap);
      cmap = 0;
    }
  if(cx)
    {
      if(dpy) glXDestroyContext(dpy, cx);
      cx = NULL;
    }
  if(vi)
    {
      XFree(vi);
      vi = NULL;
    }
  if(dpy)
    {
      XCloseDisplay(dpy);
      dpy = NULL;
    }
  invalidate();
}


dtkGLXSimpleDisplay::~dtkGLXSimpleDisplay(void)
{
  clean_up();
}

dtkGLXSimpleDisplay::
dtkGLXSimpleDisplay(dtkManager *manager, const char *name,
		    int x, int y, int w, int h,
		    const char *title)
  : dtkBasicDisplay(manager, name)
{
  init(name, x, y, w, h, title);
}

dtkGLXSimpleDisplay::
dtkGLXSimpleDisplay(dtkManager *manager, const char *name,
		  int x, int y, const char *title)
  : dtkBasicDisplay(manager, name)
{
  init(name, x, y, W, H, title);
}

dtkGLXSimpleDisplay::
dtkGLXSimpleDisplay(dtkManager *manager_in, const char *name,
		    const char *title)
  : dtkBasicDisplay(manager, name)
{
  init(name, X, Y, W, H, title);
}


void dtkGLXSimpleDisplay::init(const char *name, 
			       int x, int y, int w,
			       int h, const char *title)
{
  // initialize data
  dpy = NULL;
  vi = NULL;
  doubleBuffer = True;
  cx = NULL;
  cmap = 0;
  win = 0;

  //needRedraw = False;
  //recalcModelView = True;
  isMapped = 1;

  refresh_count = 0;

  int attribList[] =
  {
    GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE,
    1, GLX_BLUE_SIZE, 1,
    GLX_DOUBLEBUFFER, None
  };

  setDescription("a simple one X window GL display");

  unsigned long windowAttributes_mask =
    CWBackPixel|CWBorderPixel|CWColormap|CWEventMask;

  XSetWindowAttributes swa;
  {
    swa.background_pixmap=0;
    swa.background_pixel=0;
    //swa.border_pixmap=0;
    //swa.border_pixel=0;
    //swa.bit_gravity=0;

    //swa.win_gravity=0;
    //swa.backing_store=0;
    //swa.backing_planes=0;
    //swa.backing_pixel=0;
    //swa.save_under=0;

    swa.event_mask=XEVENT_MASK;
    //swa.do_not_propagate_mask=0;
    //swa.override_redirect=0;
    //swa.colormap=cmap; // set below
    //swa.cursor=0;
  }

  // Open X display connection
  dpy = XOpenDisplay(NULL);
  if(!dpy)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "XOpenDisplay(NULL) failed:\n");
      goto XSimpleDisplay_error;
    }

  if(!glXQueryExtension(dpy, NULL, NULL))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "glXQueryExtension() failed:\n");
      goto XSimpleDisplay_error;
    }

  vi = glXChooseVisual(dpy, DefaultScreen(dpy), attribList);
  if(!vi) // may not be double-buffer-able
    {
      attribList[7] = None;
      doubleBuffer = False;
      vi = glXChooseVisual(dpy, DefaultScreen(dpy), attribList);
      if(!vi)
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "glXChooseVisual() failed: no "
		     "RGB visual with depth buffer:\n");
	  goto XSimpleDisplay_error;
	}
    }

  if(vi->c_class != TrueColor)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "glXChooseVisual() failed: can't"
		 " get a TrueColor visual\n");
      goto XSimpleDisplay_error;
    }

  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen),
			 vi->visual, AllocNone);
  if(!cmap)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "XCreateColormap() failed:\n");
      goto XSimpleDisplay_error;
    }
  swa.colormap=cmap;

  win = XCreateWindow(dpy, RootWindow(dpy, vi->screen),
		      x, y, w, h, 0, vi->depth, InputOutput,
		      vi->visual, windowAttributes_mask, &swa);
  if(!win)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "XCreateWindow() failed:\n");
      goto XSimpleDisplay_error;
    }

  {
    XSizeHints sizehints;
    sizehints.x = x;
    sizehints.y = y;
    sizehints.width  = w;
    sizehints.height = h;
    sizehints.flags = USSize | USPosition;
    XSetNormalHints(dpy, win, &sizehints);
    XSetStandardProperties(dpy, win, title, title,
			   None, NULL, 0, &sizehints);
  }

  cx = glXCreateContext(dpy, vi,
			/* No sharing of display lists */ NULL,
			/* Direct rendering if possible */ True);
  if(!cx)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "glXCreateContext() failed:\n");
      goto XSimpleDisplay_error;
    }

  glXMakeCurrent(dpy, win, cx);
  XMapWindow(dpy, win);


  // Configure the OpenGL context for rendering.
  /* Enable depth buffering. */
  glEnable(GL_DEPTH_TEST);
  /* Set up a default projection transformation. */
  reshapeWin(w, h);


  dtkMsg.add(DTKMSG_DEBUG,
	     "dtkGLXSimpleDisplay::dtkGLXSimpleDisplay() "
	     "setup visual with%s double buffering.\n",
	     (doubleBuffer)?"out":"");

  windowList = new dtkXWindowList;
  windowList->add(NULL, win);
  if(manager->add(windowList))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkGLXSimpleDisplay::"
		 "dtkGLXSimpleDisplay() failed:\n");
      clean_up();
      return;
    }

  // dtkDisplay objects start out invalid.
  validate();

  return;  // success

 XSimpleDisplay_error:
  dtkMsg.append("in dtkGLXSimpleDisplay::dtkGLXSimpleDisplay"
		"(\"%s\",%d,%d,%d,%d,\"%s\"):\nwith "
		"env variable DISPLAY=\"%s\"",
		name,x,y, w,h,title, getenv("DISPLAY"));
  clean_up();
}

int dtkGLXSimpleDisplay::sync(void)
{
  if(isMapped)
    {
      if(doubleBuffer)
	{
	  // This does not work correctly (yet) on the two GNU/Linux
	  // boxes that I tested it on.
	  glXSwapBuffers(dpy, win);
	}
      else
	glFlush();
    }
  else
    usleep(100000);

  return CONTINUE;
}


int dtkGLXSimpleDisplay::frame(void)
{
  // Check for X events
  while(XPending(dpy))
    {
      XEvent event;
      XNextEvent(dpy, &event);
      switch(event.type)
	{
	  //case MappingNotify:
	case ConfigureNotify:
	  //dtkMsg.add(DTKMSG_DEBUG, "dtkGLXSimpleDisplay::frame()"
	  //     " got a ConfigureNotify X event.\n");
	  reshapeWin(event.xconfigure.width,
		     event.xconfigure.height);
	  break;
	case UnmapNotify:
	  dtkMsg.add(DTKMSG_DEBUG, "dtkGLXSimpleDisplay::frame()"
		     " got a UnmapNotify X event.\n");
	  isMapped = 0;
	  break;
	case MapNotify:
	  dtkMsg.add(DTKMSG_DEBUG, "dtkGLXSimpleDisplay::frame()"
		     " got a MapNotify X event.\n");
	  isMapped = 1;
	  break;
	}
    }

  if(isMapped)
    {
      struct dtkBasicDisplay_drawList *m, *l = drawList;
      for(;l;l=m)
	{
	  m = l->next;
	  if(l->draw())
	    remove(l->draw);
	}
    }
  return CONTINUE;
}


void dtkGLXSimpleDisplay::reshapeWin(int width, int height)
{
  GLfloat h = (GLfloat) height / (GLfloat) width;
  glViewport(0, 0, (GLint) width, (GLint) height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -40.0);  
}


//void dtkGLXSimpleDisplay::setSxSySzXYZHPRModelPosition(GLfloat *x){}
//void dtkGLXSimpleDisplay::setSXYZHPRModelPosition(GLfloat *x){}
//void dtkGLXSimpleDisplay::setXYZHPRModelPosition(GLfloat *x){}
