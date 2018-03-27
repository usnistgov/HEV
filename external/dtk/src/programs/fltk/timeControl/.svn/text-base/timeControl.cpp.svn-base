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
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Input.H>
#include <dtk.h>

// from file dtk-timeControl.C
extern int running;
extern dtkSharedTime *sharedTime;

int isPauseButton;
Fl_Output *time_output;
Fl_Output *timeSec_output;
Fl_Button *pause_resume_Button;

static Fl_Value_Input *reset_Value_Input;
static Fl_Button *reset_Button;
static Fl_Window *window;

static void reset_VI_callback(Fl_Widget *w, void *data)
{
  double val = reset_Value_Input->value();
  if(val < 0.0)
    reset_Value_Input->value(0.0);
}

static void quit_callback(Fl_Widget *w, void *data)
{
  running = 0;
}

static void resume_callback(Fl_Widget *w, void *data);

static void pause_callback(Fl_Widget *w, void *data)
{
  pause_resume_Button->label("resume");
  pause_resume_Button->color(FL_GREEN);
  isPauseButton = 0;
  pause_resume_Button->callback(resume_callback);
  if(!data) sharedTime->pause();
  //else printf("not calling sharedTime->pause()\n");
}

static void resume_callback(Fl_Widget *w, void *data)
{
  pause_resume_Button->label("pause");
  pause_resume_Button->color(FL_RED);
  isPauseButton = 1;
  pause_resume_Button->callback(pause_callback);
  if(!data) sharedTime->resume();
  //else printf("not calling sharedTime->resume()\n");
}



static void reset_callback(Fl_Widget *w, void *data)
{
  sharedTime->reset((long double) reset_Value_Input->value());
}

int make_window(int argc, char **argv, char *winlabel)
{
  window = new Fl_Window(130, 87+28, winlabel);

  time_output = new Fl_Output(3, 3, 124, 25);
  time_output->value("0.0");
  time_output->color(FL_BLACK, FL_BLUE);
  time_output->textcolor(FL_GREEN);
  time_output->textfont(FL_COURIER_BOLD);

  timeSec_output = new Fl_Output(3, 31, 124, 25);
  timeSec_output->value("0.0");
  timeSec_output->color(FL_BLACK, FL_BLUE);
  timeSec_output->textcolor(FL_GREEN);
  timeSec_output->textfont(FL_COURIER_BOLD);

  reset_Button =
    new Fl_Button(3, 31+28, 50, 25, "reset");
  reset_Button->callback(reset_callback, NULL);
  reset_Button->color(FL_YELLOW);

  reset_Value_Input =
    new Fl_Value_Input(56, 31+28, 71, 25);
  reset_Value_Input->cursor_color(FL_RED);
  reset_Value_Input->maximum(600.0);
  reset_Value_Input->minimum(0.0);
  reset_Value_Input->step(0.1);
  reset_Value_Input->soft((uchar) 1);
  reset_Value_Input->value(0.0);
  reset_Value_Input->callback(reset_VI_callback);

  pause_resume_Button =
    new Fl_Button(3, 59+28, 65, 25, "pause");
  pause_resume_Button->callback(pause_callback);
  // pause_resume_Button->label("pause");
  pause_resume_Button->color(FL_RED);
  isPauseButton = 1;
  int i = sharedTime->isRunning();
  if(i == -1) return 1; // error
  if(i == 0) pause_callback(NULL, (void *) 1);

  Fl_Button *quitButton =
    new Fl_Button(71, 59+28,  56, 25, "quit");
  quitButton->callback(quit_callback);

  window->show(argc, argv);
  window->end();
  return 0;
}
