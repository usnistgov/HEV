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
#include <stdio.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Positioner.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Return_Button.H>
#include <dtk.h>

#include "trackerWidget.h"

/************************* from dtk-caveDeviceSimulator.C ******************/

extern int running;

#ifndef DTK_ARCH_WIN32_VCPP
 // VRCO system V shared memory pointers
 extern dtkVRCOSharedMem *sysV;
 //extern float *sysVhead;
 //extern float *sysVwand;
 extern float *sysVjoystick;
 extern int32_t *sysVbutton;
#endif

// DTK shared memory objects
extern dtkSharedMem *headShm, *wandShm, *joystickShm, *buttonShm;
extern float head[6];
extern float wand[6];
extern float joystick[2];
extern unsigned char button;

extern float reset_head[6];
extern float reset_wand[6];
extern float reset_joystick[2];
extern unsigned char reset_button;

/********************* end - from dtk-caveDeviceSimulator.C ***************/

// from helpButton.C
extern void helpMenu_tracker(Fl_Widget *w, void *data);
extern void helpMenu_joystick(Fl_Widget *w, void *data);
extern void helpMenu_buttons(Fl_Widget *w, void *data);
extern void helpMenu_Top(Fl_Widget *w, void *data);
extern void helpMenu_About(Fl_Widget *w, void *data);


static Tracker *headTracker, *wandTracker;
static Fl_Light_Button *buttons_Button[4];
static unsigned char init_button;
static Fl_Positioner *joystick_Positioner;
static Fl_Value_Input *xjoystick_Value_Input, *yjoystick_Value_Input;
static float init_joystick[2];
static Fl_Check_Button *joystickSpring_Check_Button;


static void resetJoystick_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  joystick[0] = reset_joystick[0];
  joystick[1] = reset_joystick[1];

  joystickShm->write(joystick);
  joystick_Positioner->xvalue((double)  joystick[0]);
  joystick_Positioner->yvalue((double) -joystick[1]);
  xjoystick_Value_Input->value((double) joystick[0]);
  yjoystick_Value_Input->value((double) joystick[1]);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVjoystick)
    {
      sysVjoystick[0] = joystick[0];
      sysVjoystick[1] = joystick[1];
    }
#endif
}

static void joystick_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  joystick[0] = (float) joystick_Positioner->xvalue();
  joystick[1] = (float) -joystick_Positioner->yvalue();
  joystickShm->write(joystick);
  xjoystick_Value_Input->value((double) joystick[0]);
  yjoystick_Value_Input->value((double) joystick[1]);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVjoystick)
    {
      sysVjoystick[0] = joystick[0];
      sysVjoystick[1] = joystick[1];
    }
#endif
  if(joystickSpring_Check_Button->value() && !Fl::pushed())
    resetJoystick_callback();
}

static void valueInputJoystick_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  joystick[0] = (float) xjoystick_Value_Input->value();
  joystick[1] = (float) yjoystick_Value_Input->value();
  joystickShm->write(joystick);
  joystick_Positioner->xvalue((double) joystick[0]);
  joystick_Positioner->yvalue((double) -joystick[1]);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVjoystick)
    {
      sysVjoystick[0] = joystick[0];
      sysVjoystick[1] = joystick[1];
    }
#endif
  if(joystickSpring_Check_Button->value() && !Fl::pushed())
    resetJoystick_callback();
}

static void updateJoystick_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  joystickShm->read(joystick);
  joystick_Positioner->xvalue((double)  joystick[0]);
  joystick_Positioner->yvalue((double) -joystick[1]);
  xjoystick_Value_Input->value((double) joystick[0]);
  yjoystick_Value_Input->value((double) joystick[1]);

  if(joystickSpring_Check_Button->value() && !Fl::pushed())
    resetJoystick_callback();
}

static void initJoystick_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  joystick[0] = init_joystick[0];
  joystick[1] = init_joystick[1];
  joystickShm->write(joystick);
  joystick_Positioner->xvalue((double)  joystick[0]);
  joystick_Positioner->yvalue((double) -joystick[1]);
  xjoystick_Value_Input->value((double) joystick[0]);
  yjoystick_Value_Input->value((double) joystick[1]);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVjoystick)
    {
      sysVjoystick[0] = joystick[0];
      sysVjoystick[1] = joystick[1];
    }
#endif
  if(joystickSpring_Check_Button->value() && !Fl::pushed())
    resetJoystick_callback();
}


static void initButtons_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  int i;
  buttonShm->read(&button);
  // unset the first 4 bits
  button &= ~(017);
  // set the first four bits to reset value
  button |= init_button & 017;

  buttonShm->write(&button);
  for(i=0;i<4;i++)
    buttons_Button[i]->value((button & (01 << i))? 01: 00);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVbutton)
    for(i=0;i<4;i++)
      sysVbutton[i] = (button & (01 << i))? 01: 00;
#endif
}

static void resetButtons_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  int i;
  buttonShm->read(&button);
  // unset the first 4 bits
  button &= ~(017);
  // set the first four bits to reset value
  button |= reset_button & 017;

  buttonShm->write(&button);
  for(i=0;i<4;i++)
    buttons_Button[i]->value((button & (01 << i))? 01: 00);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVbutton)
    for(i=0;i<4;i++)
      sysVbutton[i] = (button & (01 << i))? 01: 00;
#endif
}

static void updateButtons_callback(Fl_Widget *w=NULL , void *data=NULL)
{
  int i;
  buttonShm->read(&button);
  for(i=0;i<4;i++)
    buttons_Button[i]->value((button & (01 << i))? 01: 00);
}

static void button_callback(Fl_Widget *w=NULL, void *data=NULL)
{
  int i;
  buttonShm->read(&button);
  // unset the first 4 bits
  button &= ~(017);
  for(i=0;i<4;i++)
    button |=  (buttons_Button[i]->value()? 01 : 00) << i;
  buttonShm->write(&button);
#ifndef DTK_ARCH_WIN32_VCPP
  if(sysVbutton)
    for(i=0;i<4;i++)
      sysVbutton[i] = (button & (01 << i))? 01: 00;
#endif
}

static void quit_callback(Fl_Widget*, void *data)
{
  running = 0;
}

static void reset_all_callback(Fl_Widget *w, void *data)
{
  headTracker->reset();
  wandTracker->reset();
  resetButtons_callback();
  resetJoystick_callback();
}

static void init_all_callback(Fl_Widget *w, void *data)
{
  headTracker->init();
  wandTracker->init();
  initButtons_callback();
  initJoystick_callback();
}

static void update_mem_callback(Fl_Widget *w, void *data)
{
  headTracker->update_sharedMem();
  wandTracker->update_sharedMem();
  button_callback();
  joystick_callback();
}

static void update_widget_callback(Fl_Widget *w, void *data)
{
  headTracker->update_sliders();
  wandTracker->update_sliders();
  updateButtons_callback();
  updateJoystick_callback();
}

static Fl_Menu_Item HelpMenu[] =
{
  {"      Help       ",0, NULL, (void *) 0, FL_SUBMENU,FL_EMBOSSED_LABEL},
  {"&Head Tracker",    0, helpMenu_tracker, (void *) 0,0,FL_EMBOSSED_LABEL},
  {"&Wand Tracker",    0, helpMenu_tracker, (void *) 1,0,FL_EMBOSSED_LABEL},
  {"&Joystick",        0, helpMenu_joystick, NULL,0,FL_EMBOSSED_LABEL},
  {"&Buttons",         0, helpMenu_buttons, NULL,0,FL_EMBOSSED_LABEL},
  {"&Tool Bar",        0, helpMenu_Top, NULL, FL_MENU_DIVIDER,FL_EMBOSSED_LABEL},
  {"&About dtk-CAVE Devices Simulator",0, helpMenu_About, NULL,0,FL_EMBOSSED_LABEL},
  {0}, {0}
};

int make_mainWindow(int fltk_argc, char **fltk_argv)
{
  Fl_Window *mainWindow = new 
    Fl_Window(720, 226, "dtk-CAVE Devices Simulator"); 
  {
    Fl_Menu_Bar* o = new Fl_Menu_Bar(531, 0, 100, 25);
    o->labeltype(FL_NO_LABEL);
    o->selection_color(50);
    o->menu(HelpMenu);
  }
   // toolbar buttons
  {
    Fl_Button* o = new Fl_Button(0, 0, 75, 25, "Quit");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback(quit_callback);
  }
  {
    Fl_Button* o = new Fl_Button(75, 0, 95, 25, "Reset All");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback(reset_all_callback);
    o->when(FL_WHEN_RELEASE);
  }
  {
    Fl_Button* o = new Fl_Button(170, 0, 95, 25, "Init All");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback(init_all_callback);
    o->when(FL_WHEN_RELEASE);
  }
  {
    Fl_Button* o = new Fl_Button(265, 0, 125, 25, "Update Mem");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback(update_mem_callback);
    o->when(FL_WHEN_RELEASE);
  }
  {
    Fl_Button* o = new Fl_Button(390, 0, 140, 25, "Update Widget");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->callback(update_widget_callback);
    o->when(FL_WHEN_RELEASE);
  }
  { //******** head and wand sliders ********
    Fl_Tabs* o = new Fl_Tabs(0, 30, 310, 195);
    o->when(FL_WHEN_CHANGED);
    {
      headTracker = new Tracker(0, 50,
		  headShm, head,
#ifndef DTK_ARCH_WIN32_VCPP
		  sysV, 0,
#endif
		  reset_head,
		  "  HEAD  TRACKER  ", FL_YELLOW);
    }
    {
      Fl_Group *o = wandTracker = new
	Tracker(0, 50,
		wandShm, wand,
#ifndef DTK_ARCH_WIN32_VCPP
		sysV, 1,
#endif
		reset_wand,
		"  WAND  TRACKER  ", FL_BLUE);
      o->color(46);
    }
    o->end();
  }
  {
    // ****************** buttons ******************
    Fl_Box* o = new Fl_Box(518, 25, 75, 35, "BUTTONS");    
    o->labeltype(FL_EMBOSSED_LABEL);
    o->labelcolor(FL_GREEN);
    o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  }
  {
    int i;
    static const char *label[] = { "  0", "  1", "  2", "  3" };
    for(i=0;i<4;i++)
      {
	Fl_Light_Button *o = buttons_Button[i] =
	  new Fl_Light_Button(520, 50+i*25, 80, 25);
	o->labelcolor(FL_GREEN);
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(button_callback);
	o->label(label[i]);
	o->value((button & (01 << i))? 01: 00);
      }
    init_button = button;
  }
  {
    Fl_Button* o = new Fl_Button(607, 80, 110, 23, "Init Buttons");
    o->callback(initButtons_callback);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->when(FL_WHEN_RELEASE);
  }
  {
    Fl_Button* o = new Fl_Button(607, 103, 110, 23, "Reset Buttons");
    o->callback(resetButtons_callback);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->when(FL_WHEN_RELEASE);
  }
  {// ************** joystick **************
    Fl_Box* o = new Fl_Box(310, 50, 200, 175, "JOYSTICK");
    o->box(FL_UP_FRAME);
    o->labelcolor(1);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->align(FL_ALIGN_TOP);
  }
  {
    init_joystick[0] = joystick[0];
    init_joystick[1] = joystick[1];
  }
  {
    Fl_Positioner *o = joystick_Positioner =
      new Fl_Positioner(325,53,168,168);
    o->box(FL_THIN_DOWN_BOX);
    o->color(51);
    o->xbounds(-1,1);
    o->ybounds(-1,1);
    o->xstep(0.001);
    o->ystep(0.001);
    o->xvalue((double) joystick[0]);
    o->yvalue((double) -joystick[1]);
    o->callback(joystick_callback);
    o->when(FL_WHEN_RELEASE_ALWAYS|FL_WHEN_CHANGED);
  }
  {
    Fl_Value_Input* o = xjoystick_Value_Input =
      new Fl_Value_Input(535, 155, 65, 25, "X: ");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->value((double) joystick[0]);
    o->callback(valueInputJoystick_callback);
    o->when(FL_WHEN_RELEASE_ALWAYS|FL_WHEN_CHANGED);
  }
  {
    Fl_Value_Input* o = yjoystick_Value_Input =
      new Fl_Value_Input(535, 185, 65, 25, "Y: ");
    o->labeltype(FL_EMBOSSED_LABEL);
    o->value((double) joystick[1]);
    o->callback(valueInputJoystick_callback);
    o->when(FL_WHEN_RELEASE_ALWAYS|FL_WHEN_CHANGED);
  }
  { 
    Fl_Button* o = new Fl_Button(607, 147, 110, 23, "Init Joystick");
    o->callback(initJoystick_callback);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->when(FL_WHEN_RELEASE);
  }
  {
    Fl_Button* o = new Fl_Button(607, 170, 110, 23, "Reset Joystick");
    o->callback(resetJoystick_callback);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->when(FL_WHEN_RELEASE);
  }
  {
    Fl_Check_Button* o = joystickSpring_Check_Button =
      new Fl_Check_Button(607, 193, 110, 23, "Spring Stick");
    o->callback(resetJoystick_callback);
    o->labeltype(FL_EMBOSSED_LABEL);
    o->when(FL_WHEN_RELEASE);
  }
 

  mainWindow->show(fltk_argc, fltk_argv);
  return 0;
}
