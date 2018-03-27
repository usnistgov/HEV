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
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>

#include <dtk.h>

#include "trackerWidget.h"

void Tracker::update_sharedMem(void)
{
  float x[6];
  for(int i=0;i<6;i++)
    x[i] = (float) slider[i]->value();
  shm->write(x);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVstate)
    sysVstate->writeTracker(x, which_VRCOtracker);
#endif
}


static void slider_callback(Fl_Widget *w, void *data)
{
  ((struct callbackData *)data)->t->update_sharedMem();
}

void init_callback(Fl_Widget *w, void *data)
{
  Tracker *t = ((struct callbackData *)data)->t;
  int which = ((struct callbackData *)data)->which;
  t->slider[which]->value((double) t->initState[which]);
  t->update_sharedMem();
}

void reset_callback(Fl_Widget *w, void *data)
{
  Tracker *t = ((struct callbackData *)data)->t;
  int which = ((struct callbackData *)data)->which;
  t->slider[which]->value((double) t->resetState[which]);
  t->update_sharedMem();  
}

void Tracker::init(void)
{
  for(int i=0;i<6;i++)
    slider[i]->value((double) initState[i]);
  update_sharedMem();
}

void Tracker::reset(void)
{
  for(int i=0;i<6;i++)
    slider[i]->value((double) resetState[i]);
  update_sharedMem();
}

void Tracker::update_sliders(void)
{
  float x[6];
  shm->read(x);

  for(int i=0;i<6;i++)
    slider[i]->value((double) x[i]);
}


Tracker::Tracker(int x, int y,
		 dtkSharedMem *shm_in,
		 float *state_in,
#ifndef DTK_ARCH_WIN32_VCPP
		 dtkVRCOSharedMem *sysVstate_in,
		 int which_tracker,
#endif
		 float *resetState_in,
		 const char *label, Fl_Color labelColor):
  Fl_Group(x,y,310, 185,label)
{
  shm = shm_in;
  state = state_in;
#ifndef DTK_ARCH_WIN32_VCPP
  sysVstate = sysVstate_in;
  which_VRCOtracker = which_tracker;
#endif

  color(42);
  labeltype(FL_EMBOSSED_LABEL);
  labelcolor(labelColor);

  {
    for(int i=0;i<6;i++)
      {
	cbData[i].which = i;
	cbData[i].t = this;
	initState[i] = state[i];
	resetState[i] = resetState_in[i];
      }
  }
  {
    Fl_Box* o = new Fl_Box(x,y, 215, 175);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->align(FL_ALIGN_TOP);
  }
  {
    for(int i=0;i<6;i++)
      {
	{
	  Fl_Button* o = new Fl_Button(x+215, y+10+i*25, 40, 25, "Init");
	  o->callback(init_callback, (void *) &(cbData[i]));
	  o->labeltype(FL_EMBOSSED_LABEL);
	  o->when(FL_WHEN_RELEASE);
	}
	{
	  Fl_Button* o = new Fl_Button(x+255, y+10+i*25, 50, 25, "Reset");
          o->callback(reset_callback, (void *) &(cbData[i]));
	  o->labeltype(FL_EMBOSSED_LABEL);
	  o->when(FL_WHEN_RELEASE);
	}
      }
  }
  
  {
    static const char *label[] = { "X  ", "Y  ", "Z  "};
    for(int i=0;i<3;i++)
      {
	Fl_Value_Slider *o = slider[i] =
	  new Fl_Value_Slider(x+30,y+10+i*25, 185, 25, label[i]);
	o->type(1);
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(labelColor);
	o->step(0.001);
	o->minimum(-1.0);
	o->maximum(1.0);
	o->value((double) state[i]);
	o->callback(slider_callback, (void *) &(cbData[i]));
	o->align(FL_ALIGN_LEFT);
      }
  }
  {
    static const char *label[] = { "H  ", "P  ", "R  "};
    for(int i=0;i<3;i++)
      {
	Fl_Value_Slider* o = slider[i+3] =
	  new Fl_Value_Slider(x+30,y+85+i*25, 185, 25, label[i]);
	o->type(1);
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(labelColor);
	o->step(0.1);
	o->minimum(-180.0);
	o->maximum(180.0);
	o->value((double) state[i+3]);
	o->callback(slider_callback, (void *) &(cbData[i+3]));
	o->align(FL_ALIGN_LEFT);
      }
  }

  end();
}
