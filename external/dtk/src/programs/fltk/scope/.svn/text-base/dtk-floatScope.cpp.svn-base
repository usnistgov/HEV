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
/************** classes in this code *******************
 *
 *  name             description
 * -------      ---------------------------------------
 *
 * MainWin       main FLTK window
 *
 * Canvas        scope drawing window, manages an array
 *                of Plots.
 *
 * Plot          Manages a dtkSharedMem and an array of
 *               Channels. There will be one channel per
 *               TYPE (float or double) in the dtkSharedMem.
 *
 * Channel       Has the min, max, scale and colors
 *               for each line (channel) drawn.
 *
 * ControlsWin   The main controls window.  It manages
 *               PlotWidget's.
 *
 * PlotWidget    Control widget for a plot.
 *
 * ChannelWidget Control widget for a channel.
 *
 *******************************************************/

#include <signal.h>
#include <stdlib.h>
#include "scope.h"

Canvas *canvas;
int running = 1;
int queue_read = 1;

// look at checkRates.cpp if you change these defaults.
double time_step = .02;  // render frame period
double rate      = 50.0; // sweep rate in pixels/second
int swipe = 6; // swipe = 2*((int) (time_step*rate));


int line_width = 1;
int point_radius = 2; // point width = 1 + 2*point_radius
// point_radius can't be larger than swipe/2


// (time_step * rate) must be greater than 5 and less than the window
// width in pixels.

static void catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,
	     "caught signal %d: exiting.\n",sig);
  running = 0;
}

int main(int argc, char **argv)
{
  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
                       PROGRAM_NAME, dtkMsg.color.end);

  int fltk_argc;
  int other_argc;
  char **other_argv;
  char **fltk_argv;

   if(dtkFLTKOptions_get(argc, (const char **) argv,
			 &fltk_argc, &fltk_argv,
			 &other_argc, &other_argv)) return 1; // error

  dtkSharedMem **shm = parseArgs(other_argc, other_argv);
  if(!shm) return 1; // error

  MainWin mainWin(shm, fltk_argc, fltk_argv);
  canvas = mainWin.canvas;

  if(parse_scaleOptions(&mainWin, argc, argv)) running = 0; // error

  signal(SIGINT,catcher);
  signal(SIGTERM,catcher);

  //fl_line_style(0, line_width);
  Fl::wait(time_step);

  while(running)
    {
      if(mainWin.visible_r())
	{
	  canvas->redraw();
	  Fl::wait(time_step);
	}
      else // main window is not mapped.
	Fl::wait();
    }

  for(;*shm;shm++)
    delete *shm;

  return 0;
}
