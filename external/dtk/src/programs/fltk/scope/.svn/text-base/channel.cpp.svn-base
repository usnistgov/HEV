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
#include "scope.h"

Channel::Channel(int canvasY, int canvasHeight, int shm_index_in,
		 TYPE min_in, TYPE max_in,
		 // line and point colors
		 uchar r_l, uchar g_l, uchar b_l,
		 uchar r_p, uchar g_p, uchar b_p)
{
  shm_index = shm_index_in;

  // vertical (Y) scale and shift.
  setScale(canvasY, canvasHeight, min_in, max_in);

  // line and point colors
  r_line = r_l;
  g_line = g_l;
  b_line = b_l;
  r_point = r_p;
  g_point = g_p;
  b_point = b_p;

  isPlotted = 1;
};


void Channel::setScale(Canvas *canvas, TYPE min, TYPE max)
{
  setScale(canvas->y(), canvas->h(), min, max);
}

void Channel::setScale(int canvasY, int canvasHeight,
		       TYPE min_in, TYPE max_in)
{
  min = min_in;
  max = max_in;
  setScale(canvasY, canvasHeight);
}

void Channel::setScale(int canvasY, int canvasHeight)
{
  scale =  - (canvasHeight - 2*line_width - 1) / (max - min);
  shift = - scale * max + line_width + canvasY;
}

void Channel::setPointColor(uchar r, uchar g, uchar b)
{
  r_point = r;
  g_point = g;
  b_point = b;  
}


void Channel::setLineColor(uchar r, uchar g, uchar b)
{
  r_line = r;
  g_line = g;
  b_line = b;
}
  
void Channel::getPointColor(uchar &r, uchar &g, uchar &b)
{
  r = r_point;
  g = g_point;
  b = b_point;
}

void Channel::getLineColor(uchar &r, uchar &g, uchar &b)
{
  r = r_line;
  g = g_line;
  b = b_line;
}
