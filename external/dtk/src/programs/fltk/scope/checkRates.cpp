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

static void set_swipe(void)
{
  swipe = 2*((int) (rate*time_step) + line_width + point_radius);
  if(swipe < 4 + line_width + point_radius)
    swipe = 4 + line_width + point_radius;
}

double set_rate(double rate_in)
{
  if(time_step*rate_in > (double) (MIN_WIN_WIDTH/2))
    rate = ((double) MIN_WIN_WIDTH)/(2.0*time_step);
  else if(time_step*rate_in < 0.5)
    rate = 0.5/time_step;
  else
    rate = rate_in;

  set_swipe();

  return rate;
}

double set_time_step(double time_step_in)
{
  time_step_in = ((int) (time_step_in * 100.0))/100.0;
  if(time_step_in < 0.01)
    time_step_in = 0.01;

  if(time_step_in*rate > (double) (MIN_WIN_WIDTH/2))
    {
      time_step = ((double) MIN_WIN_WIDTH)/(2.0*rate);
      if(time_step < 0.01)
	{
	  time_step = 0.01;
	  set_rate(rate);
	}
    }
  else if(time_step_in*rate < 0.5)
    time_step = 0.5/rate;
  else
    time_step = time_step_in;

  set_swipe();

  return time_step;
}

int set_line_width(int line_width_in)
{
  if(line_width_in > MAX_LINE_WIDTH)
    line_width = MAX_LINE_WIDTH;
  else if(line_width_in < MIN_LINE_WIDTH)
    line_width = MIN_LINE_WIDTH;
  else
    line_width = line_width_in;

  set_swipe();

  return line_width;
}

int set_point_radius(int point_radius_in)
{
  if(point_radius_in > MAX_POINT_RADIUS)
    point_radius = MAX_POINT_RADIUS;
  else if(point_radius_in < MIN_POINT_RADIUS)
    point_radius = MIN_POINT_RADIUS;
  else
    point_radius = point_radius_in;

  set_swipe();

  return point_radius;
}

