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
#include <stdlib.h>
#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <malloc.h>
#endif
#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
# include <string.h>
#endif

#include "scope.h"

#define HEIGHT  200


static void quit_callback(Fl_Widget *w, void *data)
{
  running = 0;
}

char *MainWin::getLabel(dtkSharedMem **shm)
{
  label = dtk_strdup(PROGRAM_NAME);
  for(;*shm;shm++)
    {
      size_t len = strlen(label);
      label = (char *)
	dtk_realloc(label,  len +
		    strlen((*shm)->getShortName()) + 4);
      sprintf(&(label[strlen(label)]),
	      " \"%s\"", (*shm)->getShortName());
    }

  return label;
}

MainWin::~MainWin(void)
{
  if(label)
    {
      free(label);
      label = NULL;
    }
}

MainWin::MainWin(dtkSharedMem **shm, int argc, char **argv):
  Fl_Window(500, HEIGHT, getLabel(shm))
{
  Fl_Pack *p = new Fl_Pack(0,0, 500, MENU_HEIGHT);
  p->type(FL_HORIZONTAL);
  {
    Fl_Group *g = new Fl_Group(0,0, 500, MENU_HEIGHT);
    {
      Fl_Button *o;
      o = new Fl_Button(0,   0,  170, MENU_HEIGHT, "Quit");
      o->callback(quit_callback, NULL);
      o = new Fl_Button(170,   0, 330, MENU_HEIGHT, "Controls");
      controlWin_args.mainWin = this;
      controlWin_args.shm = shm;
      o->callback(controls_callback, (void *) &controlWin_args);
    }
    g->end();
    p->resizable(g);
  }
  p->end();
  canvas = new Canvas(shm, 0, MENU_HEIGHT, 500, HEIGHT-MENU_HEIGHT);
  resizable(canvas);
  
  size_range(MIN_WIN_WIDTH, 100);
  color(FL_BLACK);
  end();

  show(argc, argv);
}
