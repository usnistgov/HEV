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
/* This file was originally written by Eric Tester.  Many
 * modifications have been made by Lance Arsenault.
 */
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Return_Button.H>

// declares static char *eric
#include  "mugshot.h"


static Fl_Window *aboutWindow = NULL;
static void make_aboutWindow(void);

void aboutCallback(Fl_Widget *who, void *data)
{
  if(!aboutWindow)
    make_aboutWindow();
  else
    aboutWindow->show();
}

// call at exit time
static void aboutCleanup(void)
{
  if(aboutWindow)
    {
      delete aboutWindow;
      aboutWindow = NULL;
    }
}

static void aboutOkCallback(Fl_Widget *who, void *data)
{
  aboutWindow->hide();
}

static void make_aboutWindow(void)
{
  aboutWindow = new Fl_Window(450, 215, "About dtk-Gnomon Graphical Display");
  {
    Fl_Box* o = new Fl_Box(20, 60, 85, 134);
    Fl_Pixmap *pixmap_mugshot = new Fl_Pixmap(eric);
    pixmap_mugshot->label(o);
  }
  
  {
    Fl_Box* o = new Fl_Box(150, 170, 120, 25, "Written by: Eric Tester");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->align(FL_ALIGN_INSIDE);
  }
  {
    Fl_Box* o = new Fl_Box(5, 5, 440, 205);
    o->box(FL_EMBOSSED_FRAME);
    o->labeltype(FL_NO_LABEL);
  }
  {
    Fl_Box* o = new Fl_Box(80, 15, 275, 35, "DTK Gnomon Graphical Display");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->labelfont(2);
    o->labelsize(20);
    o->labelcolor(4);
  }
  {
    Fl_Return_Button* o = new Fl_Return_Button(310, 170, 110, 25, "OK");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback((Fl_Callback*) aboutOkCallback);
  }
  {
    Fl_Box* o = new Fl_Box(110, 40, 320, 150,
			   "The DTK Gnomon Graphical Display visually interperets "
			   "data in DTK shared memory as x, y, z coordinates and "
			   "heading (H), pitch (P), roll (R), in degrees."
			   "\n\nIt requires that the DTK shared memory "
			   "preexist and be at least 1 float in size.\n\n");
    o->align(132|FL_ALIGN_INSIDE);
  }
  aboutWindow->end();
  aboutWindow->show();
  atexit(aboutCleanup);
}
