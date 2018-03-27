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
 */

#include "_config.h"
#include "config.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <sys/time.h>
#endif
#include <stdio.h>
#include <errno.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <GL/glx.h>
#endif
#include <GL/gl.h>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Button.H>

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

#include "dtkFLTKSimpleDisplay.h"

// Main window defaults
#define X 0
#define Y 0
#define W 400
#define H 400


static void quit_callback(Fl_Widget *w, void *m)
{
  dtkManager *manager = static_cast<dtkManager *>(m);
  manager->state &= ~DTK_ISRUNNING;
}


dtkFLTKSimpleDisplay::~dtkFLTKSimpleDisplay(void)
{
}

dtkFLTKSimpleDisplay::
dtkFLTKSimpleDisplay(dtkManager *m, const char *name,
		    int x, int y, int w, int h,
		    const char *title)
  : dtkBasicDisplay(m, name), Fl_Gl_Window(2, 25, w-4, h-25)
{
  init(name, x, y, w, h, title);
}

dtkFLTKSimpleDisplay::
dtkFLTKSimpleDisplay(dtkManager *m, const char *name,
		  int x, int y, const char *title)
  : dtkBasicDisplay(m, name), Fl_Gl_Window(2, 25, W-4, H-25)
{
  init(name, x, y, W, H, title);
}

dtkFLTKSimpleDisplay::
dtkFLTKSimpleDisplay(dtkManager *m, const char *name,
		    const char *title)
  : dtkBasicDisplay(m, name), Fl_Gl_Window(2, 25, W-4, H-25)
{
  init(name, X, Y, W, H, title);
}

void dtkFLTKSimpleDisplay::draw(void)
{
  struct dtkBasicDisplay_drawList *m, *l = drawList;
  for(;l;l=m)
    {
      m = l->next;
      if(l->draw())
	dtkBasicDisplay::remove(l->draw);
    }
}

void dtkFLTKSimpleDisplay::init(const char *name, 
			       int x, int y, int w,
			       int h, const char *title)
{
  setDescription("FLTK simple OpenGL display");

  fl_window = new Fl_Window(x, y, w, h, title);
  fl_window->add(this);

  fl_window->size_range(65, 60, 1280, 1024, 1, 1, 0);

  fl_window->box(FL_ENGRAVED_BOX);
  Fl_Group::current()->resizable(this);

  Fl_Button *quitButton =
    new Fl_Button(0, 0,  56, 25, "quit");
  quitButton->callback(quit_callback, manager);

  validate();
}

int dtkFLTKSimpleDisplay::config(void)
{
  reshapeWin(w(), h());
  fl_window->end();
  fl_window->show();
  return REMOVE_CALLBACK;
}

int dtkFLTKSimpleDisplay::sync(void)
{
  // Need to fix this to a real sync-like thing.
  Fl::wait(0.1);
  return CONTINUE;
}


int dtkFLTKSimpleDisplay::frame(void)
{
  return REMOVE_CALLBACK; // I'll change this later.
}

void dtkFLTKSimpleDisplay::reshapeWin(int width, int height)
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
