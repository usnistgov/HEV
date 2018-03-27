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
#include "scope.h"

#ifdef DTK_ARCH_WIN32_VCPP
# include <windows.h>
#else
typedef long long LONGLONG;
#endif

Canvas::Canvas(dtkSharedMem **shm, int x_, int y_,
	       int width, int height)
  : Fl_Widget(x_, y_, width, height),
    // set time to the current system time
    Time(0.0, 1)
{
  int numPlots = 0;
  plot = (Plot **) dtk_malloc(sizeof(Plot *));
  plot[numPlots] = NULL;
  dtkSharedMem **s;
  for(s=shm;*s;s++)
    {
      plot[numPlots] = new Plot(*s, x_, y_, width, height);
      numPlots++;
      plot = (Plot **) dtk_realloc(plot, sizeof(Plot *)*(numPlots+1));
      plot[numPlots] = NULL;
    }

  t1 = Time.get() - time_step;
  x1 = x() + (int) (((LONGLONG) (rate * t1)) % ((LONGLONG)(w())));

  do_rescale = 1;

  old_w = w();
  old_h = h();
}


Canvas::~Canvas(void)
{
  if(plot)
    {
      for(Plot **p = plot;*p;p++)
	    delete *p;
      free(plot);
      plot = NULL;
    }
}

void Canvas::draw(long double t1, long double t2,
		  int X1, int X2)
{
  for(Plot **p = plot;*p;p++)
    {
      if((*p)->plotFrame(t1, t2, X1, X2))
	running = 0; // error
    }
}

void Canvas::draw(void)
{
  if(do_rescale)
    {
      do_rescale = 0;
      t1 = Time.get();
      for(Plot **p = plot;*p;p++)
	if((*p)->setScale(x(), y(), w(), h(), t1))
	  {
	    running = 0; // error
	    return;
	  }

      t1 = Time.get();
      x1 = x() + (int) (((LONGLONG) (rate * t1)) % ((LONGLONG)(w())));
      fl_color(FL_BLACK);
      fl_rectf(x(), y(), w(), h());
      return;
    }

  if(old_h != h() || old_w != w())
    {
      do_rescale = 1;
      old_w = w();
      old_h = h();
      return;
    }
  fl_line_style(0, line_width);

  long double t2 = Time.get();
  int x2 = x() + (int) (((LONGLONG) (rate*t2)) % ((LONGLONG)(w())));
  int x3 = x() + (int)
    (((LONGLONG) (x1+point_radius+line_width+1-x())% ((LONGLONG)(w()))));
  int x4 = x() + (int)
    (((LONGLONG) (x1+point_radius+line_width+1-x()+swipe)% ((LONGLONG)(w()))));

  /* draw the black swipe from x3 to x4 and the gray line at x4 */
  if(x4 > x3)
    {
      // wipe in black pixels ahead from x3 up to x4.
      fl_color(FL_BLACK);
      fl_rectf(x3, y(), x4 - x3, h());
    }
  else // x3 > x4 // It wrapped to the other side.
    {
      fl_color(FL_BLACK);
      fl_rectf(x3, y(), w() - x3, h());
      fl_rectf(x(), y(), x4 - x(), h());
    }

  fl_line_style(0, 1);
  fl_color(FL_GRAY);
  fl_line(x4, y(), x4, y() + h() - 1);
  fl_line_style(0, line_width);

  /** draw the lines and plots from x1 to x2 **/
  if(x2 > x1)
    {
      draw(t1, t2, x1, x2);
    }
  else if(x2 < x1 && w() - x1 + x2 - x() > 0)
    {
      // It wrapped to the other side.
      long double t_ = t1 + (t2 - t1)*(w() - x1)/(w() - x1 + x2 - x());

      if(x1 < x()+w()-1)
	draw(t1, t_, x1, x()+w());
      else
	t_ = t1;

      if(x2 > x())
	draw(t_, t2, x(), x2);
      else
	t2 = t_;
    }
  else // We haven't move enough pixels to plot anything.
    // Don't save the last time and x-position.
    {
      fl_line_style(0, 1);
      return;
    }

  t1 = t2; // save last time.
  x1 = x2; // save last x-position.

  fl_line_style(0, 1);
}
