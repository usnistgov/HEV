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
#include <FL/Fl_Pixmap.H>
#include <dtk.h>

// declares static char *eric[]
#include "../gnomonDisplay/mugshot.h"


static void Ok_callback(Fl_Widget *w, void *data)
{
  ((Fl_Window *) data)->hide();
}

static Fl_Value_Slider *slider[2] = { NULL, NULL };
static void trackerInit_callback(Fl_Widget *w, void *data)
{
  int i = *((int*) data);
  slider[i]->value(0.0);
}

void helpMenu_tracker(Fl_Widget *w, void *data)
{
  static Fl_Window *trackerWin[2] = { NULL, NULL };
  int i = (int) ((size_t) data);
  // i is 0 or 1 for head or wand

  static const char *winLabel[] =
  { "Help Head Tracker","Help Wand Tracker"
  };
  int color[2] = { FL_YELLOW, FL_BLUE };

  if(!trackerWin[i])
    {
      Fl_Window* o = trackerWin[i] = new Fl_Window(334, 461, winLabel[i]);
      o->labeltype(FL_EMBOSSED_LABEL);
      {
	Fl_Box* o = new Fl_Box(15, 0, 110, 35, &(winLabel[i][5]));
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(color[i]);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	const char *str = (i==0)?
	  "These sliders simulate the tracker data for "
	  "the head tracker which is a 6 float array "
	  "arranged XYZ HPR.  The XYZ values for "
	  "the sliders are from (-1,1).  The HPR "
	  "values for the sliders are in degree "
	  "units and in the range from (-180, 180).":

	  "These sliders simulate the tracker data for "
	  "the wand tracker which is a 6 float array "
	  "arranged XYZ HPR.  The XYZ values for "
	  "the sliders are from (-1,1).  The HPR "
	  "values for the sliders are in degree "
	  "units and in the range from (-180, 180).";

	Fl_Box* o = new
	  Fl_Box(10, 35, 310, 40, str);
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Value_Slider* o = slider[i] = 
	  new Fl_Value_Slider(35, 140, 190, 25, "X ");
	o->type(1);
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(color[i]);
	o->minimum(-1);
	o->step(0.001);
	o->align(FL_ALIGN_LEFT);
      }
      {
	Fl_Box* o = new
	  Fl_Box(10, 175, 310, 80,
		 "This value is written into a DTK shared "
		 "memory whenever "
		 "the slider is moved, or when the "
		 "buttons, Reset, Reset All, Init, Init All "
		 "or Update Memory are pressed.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Button* o = new Fl_Button(225, 140, 40, 25, "Init");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(trackerInit_callback, (void *) i);
      }
      {
	Fl_Box* o = new
	  Fl_Box(60, 265, 260, 95,
		 "Init sets the slider back to the initial "
		 "values XYZ HPR that was read in from the "
		 "DTK shared memory at startup.  "
		 "This value is fixed once program starts "
		 "and cannot be changed.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Button* o = new Fl_Button(10, 375, 50, 25, "Reset");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(trackerInit_callback, (void *) i);
      }
      {
	Fl_Box* o = new
	  Fl_Box(70, 375, 250, 35,
		 "Reset will reset the values of the sliders "
		 "back to the reset value.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Button* o = new Fl_Button(265, 140, 55, 25, "Reset");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(trackerInit_callback, (void *) i);
      }
      {
	Fl_Button* o = new Fl_Button(10, 265, 40, 25, "Init");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(trackerInit_callback, (void *) i);
      }
      {
	Fl_Return_Button* o = new Fl_Return_Button(240, 425, 85, 25, "OK");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(Ok_callback,(void *) trackerWin[i]);
      }
      o->end();
    }
  trackerWin[i]->show();
}

static Fl_Positioner *joystick_Positioner;
static Fl_Value_Input *xjoystick_Value_Input, *yjoystick_Value_Input;
static Fl_Check_Button *joystickSpring_Check_Button;

static void joystickInit_callback(Fl_Widget *w=NULL, void *data=NULL)
{
  joystick_Positioner->xvalue(0.0);
  joystick_Positioner->yvalue(0.0);
  xjoystick_Value_Input->value(0.0);
  yjoystick_Value_Input->value(0.0);
}

static void joystickValInput_callback(Fl_Widget *w, void *data)
{
  joystick_Positioner->xvalue(xjoystick_Value_Input->value());
  joystick_Positioner->yvalue(-yjoystick_Value_Input->value());
  if(joystickSpring_Check_Button->value() && !Fl::pushed())
    joystickInit_callback();
}

static void joystickPositioner_callback(Fl_Widget *w, void *data)
{
  xjoystick_Value_Input->value(joystick_Positioner->xvalue());
  yjoystick_Value_Input->value(-joystick_Positioner->yvalue());
  if(joystickSpring_Check_Button->value() && !Fl::pushed())
    joystickInit_callback();
}

void helpMenu_joystick(Fl_Widget *w, void *data)
{
  static Fl_Window *joystickWin = NULL;
  if(!joystickWin)
    {
      joystickWin =  new Fl_Window(425, 617,"Help Joystick");
      {
	Fl_Box* o = new Fl_Box(10, 5, 75, 35, "JOYSTICK");    
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(1);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(165, 45, 250, 145,
		 "The joystick writes a (X,Y) "
		 "coordinate into a DTK shared memory. These "
		 "values are updated to DTK shared memory "
		 "anytime the joystick is moved or when "
		 "the buttons, Reset Joystick, Init "
		 "Joystick, Init All, Reset All, or Update "
		 "Memory is pressed.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Positioner* o = joystick_Positioner =
	  new Fl_Positioner(15, 45, 140, 135);
	o->box(FL_EMBOSSED_BOX);
	o->xbounds(-1,1);
	o->ybounds(-1,1);
	o->xstep(.001);
	o->ystep(.001);
	o->xvalue(0.000);
	o->yvalue(0.000);
	o->color(51);
	o->callback(joystickPositioner_callback);
	o->when(FL_WHEN_RELEASE_ALWAYS|FL_WHEN_CHANGED);
      }
      {
	Fl_Value_Input* o = xjoystick_Value_Input =
	  new Fl_Value_Input(25, 205, 65, 25, "X: ");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->value(0.000);
	o->callback(joystickValInput_callback);
	o->when(FL_WHEN_CHANGED);
      }
      {
	Fl_Box* o = new
	  Fl_Box(115, 195, 300, 110,
		 "These input/output devices show the "
		 "current (X,Y) coordinates of the "
		 "crosshairs.  These will also allow "
		 "you to directly put in desired "
		 "coordinates.  Anything outside the range "
		 "(-1<=X<=1, -1<=Y<=1) will "
		 "peg the crosshairs but will write the "
		 "values into the DTK shared memory.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Value_Input* o = yjoystick_Value_Input = 
	  new Fl_Value_Input(25, 250, 65, 25, "Y: ");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->value(0.000);
	o->callback(joystickValInput_callback);
	o->when(FL_WHEN_CHANGED);
      }
      {
	Fl_Check_Button* o = joystickSpring_Check_Button =
	  new Fl_Check_Button(10, 315, 25, 25, "Spring Stick");
	o->down_box(FL_DIAMOND_DOWN_BOX);
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(joystickInit_callback);
      }
      {
	Fl_Box* o = new
	  Fl_Box(115, 320, 300, 115,
		 "Spring Stick is a toggle switch that "
		 "will make the crosshairs act more like "
		 "a real joystick by springing back to "
		 "(0,0) when the mouse button is released. "
		 "Also, the DTK shared memory will also "
		 "reflect this spring action by storing (0,0).");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(120, 440, 290, 85,
		 "Init sets the joystick back to the value "
		 "that was read in from the DTK shared "
		 "memory at startup.  This "
		 "initial value is fixed once program "
		 "starts and cannot be changed.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Button* o = new Fl_Button(5, 535, 105, 25, "Reset Joystick");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(joystickInit_callback);
      }
      {
	Fl_Button* o = new Fl_Button(5, 445, 105, 25, "Init Joystick");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(joystickInit_callback);
      }
      {
	Fl_Box* o = new Fl_Box(120, 535, 295, 35,
			       "Reset will reset the values of the "
			       "joystick back to zero.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Return_Button* o = new Fl_Return_Button(335, 580, 80, 25, "OK");
	o->labeltype(FL_EMBOSSED_LABEL);
        o->callback(Ok_callback,(void *) joystickWin);
      }
      joystickWin->end();
    }
  joystickWin->show();
}

static Fl_Light_Button *button;

static void initButton_callback(Fl_Widget *w, void *data)
{
  button->value(0);
}



void helpMenu_buttons(Fl_Widget *w, void *data)
{
  static Fl_Window *Win = NULL;
  if(!Win)
    {
      Win = new Fl_Window(325, 411, "Help Buttons");
      {
	Fl_Box* o = new Fl_Box(10, 0, 75, 35, "BUTTONS");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(2);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(10, 35, 305, 50,
		 "These buttons are generic buttons.  "
		 "Each button is one bit (0 or 1), and are "
		 "written to the DTK shared memory "
		 "file \"buttons\". This memory is stored "
		 "in the first 4 bits of a byte."
		 "These bit values are "
		 "updated whenever the state of any button "
		 "changes or when Init Buttons, Init All, "
		 "Reset Buttons, Reset All, or Update Mem "
		 "is pressed.");
	o->align(133|FL_ALIGN_INSIDE);
      }
#if 0
      {
	Fl_Box* o = new Fl_Box(250, 62, 50, 25, "buttons");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(2);
      }
#endif
      {
	Fl_Light_Button *o = button =
	  new Fl_Light_Button(115, 180, 80, 25, "  2");
	o->labelcolor(FL_GREEN);
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Button* o = new Fl_Button(10, 215, 110, 23, "Init Buttons");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(initButton_callback);
      }
      {
	Fl_Box* o = new Fl_Box(125, 215, 200, 80,
			       "Init sets the buttons back to the "
			       "value that was read in from the DTK shared "
			       "memory at startup.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Button* o = new Fl_Button(10, 310, 110, 23, "Reset Buttons");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(initButton_callback);
      }
      {
	Fl_Box* o = new Fl_Box(125, 310, 200, 40,
			       "Reset will reset the values of the buttons "
			       "back to zero.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Return_Button* o = new Fl_Return_Button(230, 360, 85, 25, "OK");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(Ok_callback,(void*) Win);
      }
      Win->end();
    }
  Win->show();
}

void helpMenu_Top(Fl_Widget *w, void *data)
{
  static Fl_Window *Win = NULL;
  if(!Win)
    {
      Win = new Fl_Window(384, 530 , "Help ToolBar");
      {
	Fl_Box* o = new Fl_Box(10, 5, 80, 35, "TOOLBAR");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	Fl_Button* o = new Fl_Button(15, 50, 70, 25, "Quit");
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Button* o = new Fl_Button(15, 125, 70, 25, "Reset All");
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Button* o = new Fl_Button(15, 180, 70, 25, "Init All");
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Button* o = new Fl_Button(10, 305, 100, 30, "Update Mem");
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Button* o = new Fl_Button(10, 365, 115, 25, "Update Widget");
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Button* o = new Fl_Button(10, 450, 75, 25, "Help");
	o->labeltype(FL_EMBOSSED_LABEL);
      }
      {
	Fl_Box* o = new
	  Fl_Box(95, 50, 295, 70,
		 "exits this program.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new 
	  Fl_Box(95, 125, 290, 50,
		 "sets all components of program back "
		 "to 0.  Maybe later additions will be added "
		 "so users will be able to set reset values.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(95, 185, 285, 115,
		 "sets all components of program back "
		 "to the value that was read in from "
		 "DTK shared mem at startup.  If there was "
		 "no initial values in the DTK shared "
		 "memory, then init value will be "
		 "0.  This initial value is fixed once "
		 "program starts and cannot be changed.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(120, 305, 255, 50,
		 "writes the current state of all "
		 "program components into the DTK shared memory.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(135, 365, 235, 50,
		 "reads all DTK shared memory "
		 "associated with this widget and updates all "
		 "components to reflect the state of the memory.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(95, 450, 280, 35,
		 "gives help for particular groups in the "
		 "widget.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Return_Button* o = new Fl_Return_Button(290, 495, 85, 25, "OK");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(Ok_callback,(void *) Win);
      }
      Win->end();
    }
  Win->show();
}

void helpMenu_About(Fl_Widget *w, void *data)
{
  static Fl_Window *Win = NULL;
  if(!Win)
    {
      Win = new Fl_Window(480, 478,
		    "About DTK CAVE Devices Simulator");
      {
	Fl_Box* o = new Fl_Box(10, 20, 460, 305);
	o->box(FL_EMBOSSED_FRAME);
	o->labeltype(FL_SYMBOL_LABEL);
	o->labelfont(2);
	o->labelsize(20);
	o->labelcolor(4);
	o->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
      }
      {
	Fl_Return_Button* o = new Fl_Return_Button(370,440,100,25,"OK");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->callback(Ok_callback,(void *) Win);
      }
      {
	Fl_Box* o = new Fl_Box(120, 35, 235, 25, "DTK CAVE Device Simulator");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelfont(2);
	o->labelsize(20);
	o->labelcolor(4);
	o->align(FL_ALIGN_TOP_LEFT|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(25, 65, 435, 80,
		 "The DTK CAVE Device Simulator graphical "
		 "user interface (GUI) enables the user "
		 "to simulate data from standard 4 CAVE (TM) "
		 "devices: head, wand trackers, "
		 "wand joystick, and wand buttons.  The data "
		 "is written into a DTK shared memory "
		 "when the corresponding simulated devices "
		 "are acted on by you. ");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Pixmap *pix = new Fl_Pixmap(eric);
	Fl_Box* o = new Fl_Box(55, 340, 80, 120);
	pix->label(o);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new Fl_Box(160, 370, 160, 20, "Written by: Eric Tester");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->align(FL_ALIGN_WRAP|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(40, 160, 300, 95,
		 " Simulated Device\n"
		 "-------------------------");
	o->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_WRAP|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(180, 160, 300, 95,
		 "  Shared Memory File\n"
		 "-------------------------------");
	o->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_WRAP|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(350, 160, 300, 95,
		 "   size\n"
		 "------------\n"
		 "  6 floats\n"
		 "  6 floats\n"
		 "  2 floats\n"
		 "  1 byte");
	o->align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_WRAP|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new
	  Fl_Box(25, 270, 400, 35,
		 "This GUI can be started and stopped while "
		 "other DIVERSE applications are still up and "
		 "running. You can run any number of copies of this"
		 " program at a time.");
	o->align(133|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new Fl_Box(220, 240, 50, 20, "buttons");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(2);
      }
      {
	Fl_Box* o = new Fl_Box(225, 190, 40, 20, "head");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(135);
      }
      {
	Fl_Box* o = new Fl_Box(220, 205, 50, 20, "wand");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(4);
      }
      {
	Fl_Box* o = new Fl_Box(220, 225, 55, 15, "joystick");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(1);
      }
      {
	Fl_Box* o = new Fl_Box(45, 205, 120, 20, "WAND TRACKER");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(4);
      }
      {
	Fl_Box* o = new Fl_Box(50, 185, 110, 25, "HEAD TRACKER");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(135);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new Fl_Box(60, 220, 75, 25, "JOYSTICK");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(1);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      {
	Fl_Box* o = new Fl_Box(60, 240, 75, 20, "BUTTONS");
	o->labeltype(FL_EMBOSSED_LABEL);
	o->labelcolor(2);
	o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      }
      Win->end();
    }
  Win->show();
}

