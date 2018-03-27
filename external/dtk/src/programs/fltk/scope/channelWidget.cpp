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
#include <stdio.h>
#include <FL/Fl_Input.H>
#include <dtk.h>
#include "scope.h"

#ifdef DTK_ARCH_WIN32_VCPP
# define snprintf  _snprintf
#endif

static void on_off_callback(Fl_Widget *w, void *data)
{
  Channel *channel = static_cast<Channel *>(data);
  Fl_Light_Button *lb = static_cast<Fl_Light_Button *>(w);
  if(lb->value())
    channel->isPlotted = 1;
  else
    channel->isPlotted = 0;
}

static void lineColor_callback(Fl_Widget *w, void *data)
{
  Channel *channel = static_cast<Channel *>(data);
  ColorButton *cb = static_cast<ColorButton *>(w);
  uchar r, g, b;
  channel->getLineColor(r, g, b);
  if(fl_color_chooser((const char *) "Line Color", r, g, b))
    {
      channel->setLineColor(r, g, b);
      cb->setColor(r, g, b);
    }
  cb->value(0);
}

static void pointColor_callback(Fl_Widget *w, void *data)
{
  Channel *channel = static_cast<Channel *>(data);
  ColorButton *cb = static_cast<ColorButton *>(w);
  uchar r, g, b;
  channel->getPointColor(r, g, b);
  if(fl_color_chooser((const char *) "Point Color", r, g, b))
    {
      channel->setPointColor(r, g, b);
      cb->setColor(r, g, b);
    }
  cb->value(0);
}

// changes the shared memory float index that is plotted for this
// channel.
static void index_callback(Fl_Widget *w, void *data)
{
  Fl_Input *input = static_cast<Fl_Input *>(w);
  ChannelWidget *channelWidget = ((struct ChannelWidget_ *) data)->channelWidget;
  int i = atoi(input->value());
  if(i > -1 && i < channelWidget->plot->num_floats)
    {
      channelWidget->channel->shm_index = i;
      snprintf(channelWidget->shm_index, 5, "%d", i);
      canvas->rescale();
    }
  input->value(channelWidget->shm_index);
}

static void min_callback(Fl_Widget *w, void *data)
{
  Channel *channel = static_cast<Channel *>(data);
  Fl_Value_Input *input = static_cast<Fl_Value_Input *>(w);
  if(input->value() < (double) channel->max)
    {
      channel->min = (float) input->value();
      canvas->rescale();
      double delta = (double) channel->max - (double) channel->min;
      input->range(((double) channel->min) - delta, (double) channel->max);
      input->step(delta/200);
    }
  else 
    input->value((double) channel->min);
}

static void max_callback(Fl_Widget *w, void *data)
{
  Channel *channel = static_cast<Channel *>(data);
  Fl_Value_Input *input = static_cast<Fl_Value_Input *>(w);
  if(input->value() > (double) channel->min)
    {
      channel->max = (float) input->value();
      canvas->rescale();
      double delta = (double) channel->max - (double) channel->min;
      input->range(((double) channel->min), (double) channel->max+delta);
      input->step(delta/200);
    }
  else
    input->value((double) channel->max);
}

ChannelWidget::ChannelWidget(int y, Channel *ch, Plot *pl)
  : Fl_Group(0, y, CHANNELWIDGET_WIDTH, CHANNELWIDGET_HEIGHT)
{
  channel = ch;
  plot = pl;

  Fl_Light_Button *lb = new
    Fl_Light_Button(0,y, 40, CHANNELWIDGET_HEIGHT, "S");
  lb->callback(on_off_callback, (void *) channel);
  lb->value(1);

  uchar r, g, b;

  channel->getLineColor(r, g, b);
  Fl_Button *colorbutton = new 
    ColorButton(40, y, r, g, b, "L");
  colorbutton->callback(lineColor_callback, (void *) channel);
 
  channel->getPointColor(r, g, b);
  colorbutton = new
    ColorButton(40+COLORBUTTON_WIDTH, y, r, g, b, "P");
  colorbutton->callback(pointColor_callback, (void *) channel);

  this_.channelWidget = this;

  Fl_Input *input = new
    Fl_Input(40+2*COLORBUTTON_WIDTH,y, 30, CHANNELWIDGET_HEIGHT);
  snprintf(shm_index,5,"%d",channel->shm_index);
  input->value(shm_index);
  input->callback(index_callback, (void *) &this_);

  Fl_Value_Input *v_input = new 
    Fl_Value_Input(70+2*COLORBUTTON_WIDTH,y, 80, CHANNELWIDGET_HEIGHT);
  v_input->callback(min_callback, (void *) channel);
  double delta = (double) channel->max - (double) channel->min;
  v_input->range(((double) channel->min) - delta, (double) channel->max);
  v_input->step(delta/200);
  v_input->value((double) channel->min);

  v_input = new 
    Fl_Value_Input(150+2*COLORBUTTON_WIDTH,y, 80, CHANNELWIDGET_HEIGHT);
  v_input->callback(max_callback, (void *) channel);
  v_input->range(((double) channel->min), (double) channel->max+delta);
  v_input->step(delta/200);
  v_input->value((double) channel->max);

  end();
}
