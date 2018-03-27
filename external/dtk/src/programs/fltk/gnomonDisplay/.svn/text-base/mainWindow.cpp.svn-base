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
#include <stdlib.h>
#include <stdio.h>
#include <dtk/_config.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif
#include <math.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Roller.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Return_Button.H>

#ifdef DTK_ARCH_DARWIN
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

#include <dtk.h>
#include "gnomonWindow.h"

#define PI  (3.141592654)



// from dtk-gnomonDisplay.C
extern int running;
extern int connectShm(const char *shmFileName);
extern int readIn_xyzhpr(double *xyzhpr);
extern double scale[6];
extern double offset[6];

// from about.C
extern void aboutCallback(Fl_Widget *who, void *data);


static Fl_Output       *xyzhpr_Output[6];
static Fl_Input        *sharedMemName_Input;
static Fl_Value_Input  *sharedMemIndex_Input;
static GnomonWindow    *gnomonWindow;
static Fl_Roller       *horz_Roller;
static Fl_Roller       *vert_Roller;
static Fl_Roller       *zoom_Roller;


Fl_Window       *mainWindow = NULL;
Fl_Light_Button *connect_Button;
Fl_Light_Button *continous_Button;


void update_xyzhpr_Output(Fl_Widget *who, void *data)
{
  double x[6]; 
  readIn_xyzhpr(x);
  int i;
  for(i=0;i<6;i++)
    {
      char s[16];
      snprintf(s,15,"%.8g",x[i]);
      xyzhpr_Output[i]->value(s);

      gnomonWindow->xyzhpr[i] = x[i]*scale[i] + offset[i];
    }
  gnomonWindow->redraw();
}

int getIndex(void)
{
  if(!sharedMemIndex_Input) return 0;
  int i = (int) sharedMemIndex_Input->value();
  if(i<0) return 0;
  return i;
}

static void viewZoom_cb(Fl_Widget *who, void *data)
{
  gnomonWindow->Scale = ((Fl_Roller *) who)->value();
  update_xyzhpr_Output(NULL, NULL);
}

static void viewHorz_cb(Fl_Widget *who, void *data)
{
  double val = ((Fl_Roller *) who)->value();
  gnomonWindow->Horz = val;
  update_xyzhpr_Output(NULL, NULL);
  ((Fl_Roller *) who)->value(fmod(val, 2.0*PI));
  //printf("val = %g\n",((Fl_Roller *) who)->value());
}

static void viewVert_cb(Fl_Widget *who, void *data)
{
  gnomonWindow->Vert = ((Fl_Roller *) who)->value();
  update_xyzhpr_Output(NULL, NULL);
  //printf("val = %g\n",((Fl_Roller *) who)->value());
}

static void resetView_cb(Fl_Widget *who, void *data)
{
  gnomonWindow->resetView();
  update_xyzhpr_Output(NULL, NULL);
  horz_Roller->value(PI/2.0);
  vert_Roller->value(PI/2.0);
  zoom_Roller->value(2.0);
}

static void quit_Callback(Fl_Widget *who, void *data)
{
  running = 0;
}

static void connect_Callback(Fl_Widget *who, void *data)
{
  connectShm(sharedMemName_Input->value());
  if(connect_Button->value())
    update_xyzhpr_Output(NULL, NULL);
}

int make_mainWindow(char *sharedMemName, int fltk_argc, char **fltk_argv)
{

  mainWindow = new Fl_Window(370, 365, "dtk-Gnomon Graphical Display");
  mainWindow->size_range(356, 300);
  { 
    Fl_Box* o = new Fl_Box(25, 60, 250, 250);
    o->box(FL_DOWN_FRAME);
    gnomonWindow = new GnomonWindow(25,60,250,250);
    Fl_Group::current()->resizable(o);
  }
  {
    Fl_Roller* o = zoom_Roller = new Fl_Roller(5,60,15,250);
    o->step(.02);
    o->value(2.0);
    o->minimum(0.2);
    o->maximum(100);
    o->callback((Fl_Callback*)viewZoom_cb);
  }
  {
    Fl_Roller* o = vert_Roller = new Fl_Roller(280,60,15,250);
    o->step(0.01);
    o->value(PI/2.0);
    o->minimum(0.00001);
    o->maximum(PI-0.00001);
    o->callback((Fl_Callback*)viewVert_cb);
  }
  {
    Fl_Roller* o = horz_Roller = new Fl_Roller(25,315,250,15); 
    o->type(FL_HORIZONTAL);
    o->align(FL_ALIGN_RIGHT);
    o->step(0.01);
    o->value(PI/2.0);
    o->minimum(-10*PI);
    o->maximum(10.0*PI);        
    o->callback((Fl_Callback*)viewHorz_cb);
  }
  {
    static char *label[] = { const_cast<char*>("X"), const_cast<char*>("Y"), const_cast<char*>("Z"), const_cast<char*>("H"), const_cast<char*>("P"), const_cast<char*>("R") };
    for (int i=0;i<6;i++) 
      {
	Fl_Output* o = xyzhpr_Output[i] 
	  = new Fl_Output(300, 85+i*35, 50, 25, label[i]);
	o->labeltype(FL_EMBOSSED_LABEL);
	o->textsize(11);
	o->align(FL_ALIGN_RIGHT|FL_ALIGN_CLIP);
      }
  }
  xyzhpr_Output[0]->labelcolor(FL_RED);
  xyzhpr_Output[1]->labelcolor(FL_GREEN);
  xyzhpr_Output[2]->labelcolor(FL_BLUE);
  xyzhpr_Output[3]->labelcolor(FL_BLUE);
  xyzhpr_Output[4]->labelcolor(FL_RED);
  xyzhpr_Output[5]->labelcolor(FL_GREEN);
  {
    connect_Button = new Fl_Light_Button(80, 0, 130, 25, "Connect");
    connect_Button->labeltype(FL_EMBOSSED_LABEL);
    connect_Button->callback((Fl_Callback*) connect_Callback);
  }
  {
    sharedMemName_Input =
      new Fl_Input(115, 30, 130, 25, "Connect To: ");
    sharedMemName_Input->labeltype(FL_EMBOSSED_LABEL);
    sharedMemName_Input->callback((Fl_Callback*)connect_Callback);
    sharedMemName_Input->when(FL_WHEN_ENTER_KEY);
  }
  {
    sharedMemIndex_Input =
      new Fl_Value_Input(310, 30, 50, 25, "at index: ");
    sharedMemIndex_Input->labeltype(FL_EMBOSSED_LABEL);
    sharedMemIndex_Input->value(0.0);
  }
  {
    Fl_Button* o = new Fl_Button(210, 0, 80, 25, "About");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback((Fl_Callback*)aboutCallback);
  }
  {
    Fl_Light_Button* o = continous_Button =
      new Fl_Light_Button(5, 335, 155, 25,"Update Continuous");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->value(0);
  }
  {
    Fl_Button* o =
      new Fl_Button(165, 335, 115, 25,"Update Widget");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback((Fl_Callback*) update_xyzhpr_Output);
  }
  {
    Fl_Button* o = new Fl_Button(285,335,80,25,"Reset View");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback((Fl_Callback*)resetView_cb);
  }
  {
    Fl_Button* o = new Fl_Button(5,0,75,25,"Quit");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback((Fl_Callback*) quit_Callback);
  }

  if(sharedMemName && sharedMemName[0])
    {
      sharedMemName_Input->value(sharedMemName);
      connect_Callback(NULL, NULL);
      if(!connect_Button->value()) return 1;
    }

  update_xyzhpr_Output(NULL,NULL);
  mainWindow->end();
  mainWindow->show(fltk_argc, fltk_argv);
  gnomonWindow->show();
  gnomonWindow->initQuad();

  return 0;
}
