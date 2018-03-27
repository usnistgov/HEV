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
#include <string.h>
#include <dtk.h>
#include "valuators.h"


extern dtkSharedMem *shm;
extern FLOAT_TYPE *buffer;
extern int update_continous;

Fl_Light_Button *update_contButton;
Fl_Window *mainWindow;

static VALUATOR_TYPE **valuator = NULL;
static Fl_Output **output = NULL;
static Fl_Button **reset = NULL;
static Fl_Button **start = NULL;
static int *valuator_num = NULL;
static struct ValuatorLimits *S = NULL;
static int numberValuators = 0;
static size_t bytes_written = 0;

#ifdef DTK_ARCH_WIN32_VCPP
inline void bzero(void *p, size_t s)
{
	memset(p, 0, s);
}
#endif /* #ifdef DTK_ARCH_WIN32_VCPP */


// write from shared memory to valuators
void update_valuators(Fl_Widget *w, void *data)
{
  if(shm->read(buffer)) { return_val = 1; is_running = 0; return; }
  int i;

  for(i=0; i<numberValuators; i++)
    {
      valuator[i]->value((double) valuator[i]->clamp((double)buffer[i]));

      char s[16];
      snprintf(s,15,"%.6g",(double) buffer[i]);
      output[i]->value(s);
    }
}

// put a value in DTK shared memory
// or print to stdout.
static void put_values(void)
{
  if(shm)
    {
      if(!bytes_written)
	{
	  if(shm->write(buffer)) { return_val = 1; is_running = 0; return; }
	}
      else // need the number of bytes that the valuators write
	// so it does not write to more memory than there are valuators.
	{
	  if(shm->write(buffer, (size_t) 0, bytes_written))
	    { return_val = 1; is_running = 0; return; }
	}
    }
  else
    {
      for(int i=0; i<numberValuators; i++)
	printf("%f ", buffer[i]);
      printf("\n");
      fflush(stdout) ;
    }
  for(int i=0; i<numberValuators; i++)
    {
      char s[16];
      snprintf(s,15,"%.6g",(double) buffer[i]);
      output[i]->value(s);
    }
}

static void reset_callback(Fl_Widget *w, void *data)
{
  int num = * (int *) data;
  valuator[num]->value(S[num].reset);
  buffer[num] = (FLOAT_TYPE) S[num].reset;
  put_values();
}

static void start_callback(Fl_Widget *w, void *data)
{
  int num = * (int *) data;
  valuator[num]->value(S[num].start);
  buffer[num] = (FLOAT_TYPE) S[num].start;
  put_values();
}

static void valuator_callback(Fl_Widget *w, void *data)
{
  int num = * (int *) data;
  buffer[num] = (FLOAT_TYPE) valuator[num]->value();
  put_values();
}

static void start_all(Fl_Widget *w, void *data)
{
  for(int i=0;i<numberValuators;i++)
    {
      valuator[i]->value(S[i].start);
      buffer[i] = (FLOAT_TYPE) S[i].start;
    }
  put_values();
}

static void reset_all(Fl_Widget *w, void *data)
{
  for(int i=0;i<numberValuators;i++)
    {
      valuator[i]->value(S[i].reset);
      buffer[i] = (FLOAT_TYPE) S[i].reset;
    }
  put_values();
}

// write from valuators to shared memory
static void update_seg(Fl_Widget *w, void *data)
{
  for(int i=0; i<numberValuators; i++)
    buffer[i] = (FLOAT_TYPE) valuator[i]->value();

  put_values();
}


static void quit_callback(Fl_Widget *w, void *data)
{
  return_val = 1;
  is_running = 0;
}


int make_window(int num, struct ValuatorLimits *s, const char *wlabel,
		int update_continuous, int fltk_argc, char **fltk_argv)
{
  numberValuators = num;
  S = s;

  valuator = (VALUATOR_TYPE **) dtk_malloc(sizeof(VALUATOR_TYPE *)*num);
  output = (Fl_Output **) dtk_malloc(sizeof(Fl_Output *)*num);
  reset = (Fl_Button **) dtk_malloc(sizeof(Fl_Button *)*num);
  start = (Fl_Button **) dtk_malloc(sizeof(Fl_Button *)*num);
  valuator_num = (int *) dtk_malloc(sizeof(int)*num);

  Fl_Window* w = mainWindow = new Fl_Window(460, 30 + num*25, wlabel);

  Fl_Button *o;

  o = new Fl_Button(0,   0,  40, 25, "quit");
  o->callback(quit_callback, NULL);

  update_contButton = new
    Fl_Light_Button(40,   0, 145, 25, "update continuous");
  if(!shm) update_contButton->deactivate();
  else if(update_continuous) update_contButton->value(1);

  o = new Fl_Button(185, 0, 85, 25, "update once");
  if(shm) o->callback(update_valuators, NULL);
  else o->deactivate();

  o = new Fl_Button(270, 0,  85, 25, "update mem");
  o->callback(update_seg, NULL);

  o = new Fl_Button(355, 0,  46, 25, "init all");
  o->callback(start_all, NULL);

  o = new Fl_Button(401,  0,  59, 25, "reset all");
  o->callback(reset_all, NULL);

  for(int i=0;i<num;i++)
    {
      valuator_num[i] = i;
      new Fl_Box(0, 30 + i*25, 50, 25, s[i].label);

      output[i] = new Fl_Output(50, 30 + i*25, 105, 25);

      char str[16];
      snprintf(str,15,"%.6g",(double) s[i].start);
      output[i]->value(str);

      VALUATOR_TYPE* o = valuator[i] =
	new VALUATOR_TYPE(155, 30 + i*25, 225, 25);

      o->callback(valuator_callback, (void *) &valuator_num[i]);
      o->type(1);

#ifdef sliders
      o->step((s[i].max -s[i].min)/(226));
#endif
#ifdef adjusters
      o->step((s[i].max -s[i].min)/(100*225));
#endif
#ifdef counters
      o->step((s[i].max -s[i].min)/(20));
#endif
#ifdef rollers
      o->step((s[i].max -s[i].min)/(2*225));
#endif
#ifdef valueInputs
      o->step((s[i].max -s[i].min)/(2*225));
      o->soft('1');
#endif
      o->maximum(s[i].max);
      o->minimum(s[i].min);
      o->value(s[i].start);

      start[i] = new Fl_Button(380, 30 + i*25, 40, 25, "init");
      start[i]->callback(start_callback, (void *) &valuator_num[i]);

      reset[i] = new Fl_Button(420, 30 + i*25, 40, 25, "reset");
      reset[i]->callback(reset_callback, (void *) &valuator_num[i]);
    }

  if(shm && numberValuators != (int) (shm->getSize()/sizeof(FLOAT_TYPE)))
    bytes_written = numberValuators*sizeof(FLOAT_TYPE);

  w->end();
  w->show(fltk_argc, fltk_argv);
  return 0;
}
