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
#include <FL/Fl_Simple_Counter.H>
#include "scope.h"
#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
# include <string.h>
#endif


static ControlsWin *controlsWin = NULL;
static Fl_Value_Input *rateValueInput, *time_stepValueInput;

static void close_callback(Fl_Widget *w, void *data)
{
  controlsWin->hide();
}

static void set_line_width_cb(Fl_Widget *w, void *data)
{
  Fl_Simple_Counter *counter = static_cast<Fl_Simple_Counter *>(w);
  counter->value((double) set_line_width((int) counter->value()));
}

static void set_point_radius_cb(Fl_Widget *w, void *data)
{
  Fl_Simple_Counter *counter = static_cast<Fl_Simple_Counter *>(w);
  counter->value((double) set_point_radius((int) counter->value()));
}

static void pix_per_sec_callback(Fl_Widget *w, void *data)
{
  double rate_in = rateValueInput->value();
  rateValueInput->value(set_rate(rate_in));
  time_stepValueInput->value(time_step);

  canvas->rescale();
}

static void time_step_callback(Fl_Widget *w, void *data)
{
  double time_step_in = time_stepValueInput->value();
  time_stepValueInput->value(set_time_step(time_step_in));
  rateValueInput->value(rate);
  canvas->rescale();
}

void controls_callback(Fl_Widget *w, void *data)
{
  if(controlsWin)
    controlsWin->show();
  else
    {
      struct ControlWin_args *args = (struct ControlWin_args *) data;
      controlsWin = new ControlsWin(args->mainWin, args->shm);
    }
}

char *ControlsWin::getLabel(dtkSharedMem **shm)
{
  label = (char *)
    dtk_malloc(strlen(PROGRAM_NAME) + strlen(" controls: ") + 1);
  sprintf(label,"%s%s", PROGRAM_NAME, " controls: ");

  for(;*shm;shm++)
    {
      size_t len = strlen(label);
      label = (char *)
	dtk_realloc(label, len +
		    strlen((*shm)->getShortName()) + 4);
      sprintf(&(label[strlen(label)]),
	      " \"%s\"", (*shm)->getShortName());
    }

  return label;
}

int ControlsWin::getHeight(MainWin *mainWin)
{
  int h = CONTROLS_HEIGHT;

  Plot **plot = mainWin->canvas->plot;
  for(;*plot; plot++)
    h += PLOTWIDGET_HEIGHT +
      CHANNELWIDGET_HEIGHT*(*plot)->num_channels;

  return h;
}

ControlsWin::ControlsWin(MainWin *mainWin_in, dtkSharedMem **shm_in)
  :   Fl_Window(CHANNELWIDGET_WIDTH,
		getHeight(mainWin_in),  getLabel(shm_in))
{
  mainWin = mainWin_in;
  canvas = mainWin_in->canvas;
  shm = shm_in;

  Fl_Button *b = new Fl_Button(10,10, 50, CONTROLS_HEIGHT - 20, "Close");
  b->callback(close_callback);

  rateValueInput = new
    Fl_Value_Input(80,20, 70, CHANNELWIDGET_HEIGHT, "pixels / sec");
  rateValueInput->callback(pix_per_sec_callback);
  rateValueInput->range(MAX_RATE, MIN_RATE);
  rateValueInput->step(0.5);
  rateValueInput->value(rate);
  rateValueInput->align(FL_ALIGN_TOP);

  time_stepValueInput = new
    Fl_Value_Input(200,20, 50, CHANNELWIDGET_HEIGHT, "frame (sec)");
  time_stepValueInput->callback(time_step_callback);
  time_stepValueInput->range(MAX_T_STEP, MIN_T_STEP);
  time_stepValueInput->step(0.01);
  time_stepValueInput->value(time_step);
  time_stepValueInput->align(FL_ALIGN_TOP);

  Fl_Simple_Counter *counter = new
    Fl_Simple_Counter(290,20, 80, CHANNELWIDGET_HEIGHT, "line width");
  counter->align(FL_ALIGN_TOP);
  counter->step(1.0);
  counter->range(MAX_LINE_WIDTH, MIN_LINE_WIDTH);
  counter->callback(set_line_width_cb);
  counter->value((double) line_width);

  counter = new
    Fl_Simple_Counter(390,20, 80, CHANNELWIDGET_HEIGHT, "point size");
  counter->align(FL_ALIGN_TOP);
  counter->step(1.0);
  counter->range(MAX_POINT_RADIUS, MIN_POINT_RADIUS);
  counter->callback(set_point_radius_cb);
  counter->value((double) point_radius);

  int y = CONTROLS_HEIGHT;
  Plot **plot = mainWin->canvas->plot;
  for(;*plot; plot++)
    {
      new PlotWidget(y, *plot);
      y += PLOTWIDGET_HEIGHT;
      Channel **channel = (*plot)->channel;
      int i=0;
      for(;*channel; channel++)
	{
	  new ChannelWidget(y, *channel, *plot);
	  y += CHANNELWIDGET_HEIGHT;
	  i++;
	}
    }

  end();
  show();
}

ControlsWin::~ControlsWin(void)
{
  if(label)
    {
      free(label);
      label = NULL;
    }
}
