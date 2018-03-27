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

#define MAX_NUM_CHANNELS  ((int) 6)

#define MIN_ ((TYPE) -1.0)
#define MAX_ ((TYPE)  1.0)

#ifdef DTK_ARCH_WIN32_VCPP
// I don't really need for this to be the minimum sized long double
// just as long as it's not commonly not the number of seconds of
// the current time.
# define LDBL_MIN ((long double) -1.13004e300)
#endif

//LDBL_MIN

static uchar cl[MAX_NUM_CHANNELS*3] =
// line colors    point color = (255 - line_color)%256
{
  255, 0, 0,     // 0
  0, 255, 0,     // 1
  0, 0, 255,     // 2
  0, 255, 255,   // 3
  255, 0, 255,   // 4
  255, 255, 0    // 5
};

Plot::Plot(dtkSharedMem *shm_in, int X, int Y, int width, int height)
{
  shm = shm_in;

  num_floats = (int) shm->getSize()/sizeof(TYPE);
  if(num_floats > MAX_NUM_CHANNELS)
    num_channels = MAX_NUM_CHANNELS;
  else
    num_channels = num_floats;

  channel = (Channel **) dtk_malloc(sizeof(Channel *)*(num_channels+1));
  int i;
  for(i=0; i<num_channels; i++)
    {
      channel[i] = new
	Channel(Y, height, i,
		MIN_, MAX_,
		cl[3*i], cl[3*i+1], cl[3*i+2],
		cl[3*i], cl[3*i+1], cl[3*i+2]);
    }
  channel[num_channels] = NULL;
  buffer = (TYPE *) dtk_malloc(shm->getSize());
  buffer_length = shm->getSize()/sizeof(TYPE);
};

Plot::~Plot(void)
{
  if(channel)
    {
      for(Channel **c = channel;*c;c++)
	    delete *c;
      free(channel);
      channel = NULL;
    }

  if(buffer)
    {
      free(buffer);
      buffer = NULL;
    }
}


int Plot::setScale(int X, int Y, int width, int height,
		    long double t_)
{
  canvasX = X;
  canvasY = Y;
  canvasWidth = width;
  canvasHeight = height;

  offset_time = 0.0;
  t = LDBL_MIN;

  if(shm->flush() < 0) return 1; // error
  if(shm->read(buffer)) return 1; // error

  t = shm->getTimeStamp();
  offset_time = t - (t_ - time_step/2);

  int i;
  for(i=0;i<num_channels; i++)
    {
      channel[i]->setScale(canvasY, canvasHeight);
      channel[i]->setY(buffer);
    }
  return 0;
}


// This is to get data for the next frame rendered.
int Plot::plotFrame(long double t1, long double t2,
		    int x1, int x2)
{
  int i = 1;
  int got_point = 0;

  // If I found a point that was between t1 and t2 from a frame
  // before.
  if(t >= t1 && t < t2)
    {
      // draw a line and than a point
      got_point = 1;
      int X = (int) (x1 + (x2-x1)*(t-t1)/(t2-t1));

      int j;
      for(j=0; j<num_channels; j++)
	{
	  Channel *c = channel[j];
	  if(c->isPlotted)
	    {
	      c->lineColor();
	      if(x < X)
		fl_line(x, c->getY(), X, c->getY());
	      else if(x > X + 5) // wrapped condition
		{
		  fl_line(x, c->getY(), canvasWidth-1, c->getY());
		  fl_line(x1, c->getY(), X, c->getY());
		}
	      
	      c->setY(buffer);
	      c->pointColor();
	      fl_line(X-point_radius+1, c->getY()-point_radius+1,
		      X+point_radius, c->getY()+point_radius);
	      fl_line(X+point_radius, c->getY()-point_radius,
		      X-point_radius+1, c->getY()+point_radius-1);
	    }
	  else
	    c->setY(buffer);
	}
      x = X;
    }

  while((i && t < t2) || t > t2 + time_step)
    {
      if(queue_read)
	{
	  i = shm->qread(buffer);
	  if(i==0) break; // got none
	  else if(i<0) return 1; // error
	}
      else // polling read
	{
	  if(shm->read(buffer)) return 1; // error
	  i = 0;
	}

      t = shm->getTimeStamp() - offset_time;

      if(t < t1 - time_step
	 || t > t2 + time_step)
	offset_time = shm->getTimeStamp() - (t1 - time_step/2);

      if(t > t2) { got_point = 0; break; }// got future point

      // draw a line and than a point
      got_point = 1;
      int X = (int) (x1 + (x2-x1)*(t-t1)/(t2-t1));

      int j;
      for(j=0; j<num_channels; j++)
	{
	  Channel *c = channel[j];
	  if(c->isPlotted)
	    {
	      c->lineColor();
	      if(x < X)
		fl_line(x, c->getY(), X, c->getY());
	      else if(x > X + 5) // wrapped condition
		{
		  fl_line(x, c->getY(), canvasWidth-1, c->getY());
		  fl_line(x1, c->getY(), X, c->getY());
		}

	      c->setY(buffer);
	      c->pointColor();
	      fl_line(X-point_radius+1, c->getY()-point_radius+1,
		      X+point_radius, c->getY()+point_radius);
	      fl_line(X+point_radius, c->getY()-point_radius,
		      X-point_radius+1, c->getY()+point_radius-1);
	    }
	  else
	    c->setY(buffer);
	}
      x = X;
    }

  if(!got_point) // haven't plotted anything this frame yet.
    {
      if(t < t1) // I've got no points in this frame yet,
	// but maybe I'll find them in the next call, or not.
	{
	  for(i=0;i<num_channels; i++)
	    {
	      Channel *c = channel[i];
	      if(c->isPlotted)
		{
		  c->lineColor();
		  if(x < x1)
		    fl_line(x, c->getY(), x1, c->getY());
		  else if(x > x1 + 5) // wrap condition
		    {
		      fl_line(x, c->getY(), canvasWidth-1, c->getY());
		      fl_line(0, c->getY(), x1, c->getY());
		    }
		}
	    }
	  x = x1;
	}
      else if(t > t2) // got future point that will be plotted in
	// a future call.
	{
	  for(i=0;i<num_channels; i++)
	    {
	      Channel *c = channel[i];
	      if(c->isPlotted)
		{
		  c->lineColor();
		  if(x < x2)
		    fl_line(x, c->getY(), x2, c->getY());
		  else if(x > x2 + 5) // wrap condition
		    {
		      fl_line(x, c->getY(), canvasWidth-1, c->getY());
		      fl_line(0, c->getY(), x2, c->getY());
		    }
		}
	    }
	  x = x2;
	}
    }

  return 0;
}

