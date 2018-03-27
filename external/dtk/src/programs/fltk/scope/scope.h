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
#include <FL/Fl.H>
#include <FL/Fl_Valuator.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Light_Button.H>
#include <dtk.h>
#include "type.h"
#include "channel.h"
#include "plot.h"
#include "canvas.h"
#include "mainWin.h"
#include "controls.h"
#include "channelWidget.h"
#include "plotWidget.h"
#include "colorButton.h"

#define CONTROLS_HEIGHT   55
#define CHANNELWIDGET_WIDTH  550
#define CHANNELWIDGET_HEIGHT  25
#define PLOTWIDGET_WIDTH   (CHANNELWIDGET_WIDTH)
#define PLOTWIDGET_HEIGHT  (65)

#define T_LEN_STEP  8

#define MIN_WIN_WIDTH  160
#define MENU_HEIGHT    25

#define MAX_RATE  1000.0 // pixels/second
#define MIN_RATE  1.0     // pixels/second

#define MAX_T_STEP 1000.0 // frame period
#define MIN_T_STEP 0.01   // frame period

#define MAX_LINE_WIDTH 10
#define MIN_LINE_WIDTH 1

#define MAX_POINT_RADIUS 10
#define MIN_POINT_RADIUS 1


extern dtkSharedMem **parseArgs(int argc, char **argv);
int parse_scaleOptions(MainWin *mainWin, int argc, char **argv);
extern dtkSharedMem **Usage(void);

// from checkRates.cpp
extern double set_rate(double rate_in);
extern double set_time_step(double time_step_in);
extern int set_line_width(int line_width_in);
extern int set_point_radius(int point_radius_in);

// from controls.cpp
void controls_callback(Fl_Widget *w, void *data);

// from dtk-floatScope.cpp
extern int running;
extern int queue_read;
extern double time_step; // render frame period
extern double rate;      // sweep rate in pixels/second
extern int swipe;        // wipes this numbers of pixels each draw frame
extern int line_width;
extern int point_radius;

extern Canvas *canvas;
