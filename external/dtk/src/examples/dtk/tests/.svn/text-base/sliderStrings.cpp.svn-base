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
#include <stdio.h>
#include <signal.h>
#include <dtk.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Output.H>


#define SIZE  ((size_t) 25)

int running = 1;
dtkSharedMem *shm;
Fl_Output *Output;


static void valuator_callback(Fl_Widget *w, void *data)
{
  char buffer[SIZE];
  sprintf(buffer,"%.15g",((Fl_Slider *) w)->value());
  Output->value(buffer);
  if(shm->write(buffer)) running = 0;
}

static void catcher(int sig)
{
  printf("caught signal %d, exiting.\n",
	 sig);
  running = 0;
}

static void quit_callback(Fl_Widget *w, void *data)
{
  running = 0;
}


int main(int argc, char **argv)
{
  if(argc < 2)
    return printf("Usage: %s SHM_FILE\n",
		  argv[0]);

  shm = new dtkSharedMem(SIZE, argv[1]);
  if(!shm) return 1;

  Fl_Window* w = new Fl_Window(450, 30 + 25, argv[1]);

  {
    Fl_Button *o = new Fl_Button(0,   0,  200, 25, "quit");
    o->callback(quit_callback, NULL);
  }
  {
    Fl_Slider* o = new Fl_Slider(150, 30, 300, 25);
    o->callback(valuator_callback);
    o->type(1);
    //o->step(0.01);
    o->maximum(1000.0);
    o->minimum(-1000.0);
    o->value(0.0);
  }
  {
    Output = new Fl_Output(0, 30, 150, 25);
    Output->value("hi");
  }
  w->end();
  w->show();

  signal(SIGINT,catcher);
  signal(SIGTERM,catcher);
#ifndef DTK_ARCH_WIN32_VCPP
  signal(SIGQUIT,catcher);
#endif

  while(running)
    Fl::wait();
}
