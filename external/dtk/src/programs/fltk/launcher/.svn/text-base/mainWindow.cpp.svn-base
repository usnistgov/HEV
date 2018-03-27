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
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>

#include "common.h"

// from getcwd.C
extern char *Getcwd(void);


// from launchButtons.C
extern int buildLaunchButtons(void);
extern int buildLaunchButtons_withListFile(const  char *listFile);
extern void reset_tile_position(void);

// from dtk-launcher.C
extern int running;
extern char *listFile;

// from killButtons.C
extern void killAll(Fl_Widget *who, void *data);

char *topDir = NULL;

Fl_Input *commandLine_input = NULL;
Fl_Output *cwd_output = NULL;
Fl_Scroll *launch_scroll = NULL;
Fl_Scroll *kill_scroll = NULL;
Fl_Tile *tile = NULL;
Fl_Window *kill_window = NULL;
Fl_Window *main_window = NULL;

static void quitCallback(Fl_Widget *who, void *data) { running = 0; }

static void killAllandQuitCallback(Fl_Widget *who, void *data)
{
  killAll(NULL, NULL);
  quitCallback(NULL, NULL);
}


Fl_Window* make_mainWindow(int argc, char **argv)
{
  char *str = Getcwd();
  if(!str) return NULL;
  topDir = strdup(str);

#define PRE_WINTITLE "dtk-Launcher: "
#define TITLE_SIZE ((size_t)41)

  static char winTitle[TITLE_SIZE+1];
  {
    if(listFile)
      {
	size_t len = strlen(PRE_WINTITLE) + strlen(listFile) + 1;
	if(len > (size_t) TITLE_SIZE)
	  {
	    len = 4 + strlen(PRE_WINTITLE) + strlen(listFile) - TITLE_SIZE ;
	    snprintf(winTitle, TITLE_SIZE, "%s...%s",
		     PRE_WINTITLE, &listFile[len]);
	  }
	else
	  snprintf(winTitle, TITLE_SIZE, "%s%s", PRE_WINTITLE, listFile);
      }
    else
      {
	size_t len = strlen(PRE_WINTITLE) + strlen(topDir) + 1;
	if(len > (size_t) TITLE_SIZE)
	  {
	    len = 4 + strlen(PRE_WINTITLE) + strlen(topDir) - TITLE_SIZE ;
	    snprintf(winTitle, TITLE_SIZE, "%s...%s",
		     PRE_WINTITLE, &topDir[len]);
	  }
	else
	  snprintf(winTitle, TITLE_SIZE, "%s%s", PRE_WINTITLE, topDir);
      }
  }

  main_window = new Fl_Window(345, 504, winTitle);
  main_window->size_range(MAIN_W, MAIN_H_MIN, MAIN_W, MAIN_H_MAX);

  {
    Fl_Button* o = new Fl_Button(0, 0, 100, 25, "Kill All");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback((Fl_Callback*)killAll);
  }
  {
    Fl_Button* o = new Fl_Button(100, 0, 100, 25, "Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(quitCallback);
  }
  {
    Fl_Button* o = new Fl_Button(200, 0, 145, 25, "Kill All and Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(killAllandQuitCallback);
  }

  {
    cwd_output = new Fl_Output (0, 50, MAIN_W, 25,
				"Current Working Directory");
    cwd_output->labeltype(FL_ENGRAVED_LABEL);
    cwd_output->align(FL_ALIGN_TOP_LEFT);
    cwd_output->value(topDir);
    cwd_output->color(FL_BLACK);
    cwd_output->textcolor(FL_WHITE);
    //cwd_output->textfont(FL_COURIER_BOLD);
  }
  {
    commandLine_input = new Fl_Input(0, 100, MAIN_W, 25,
				     "Additional Command Line Arguments");
    commandLine_input->labeltype(FL_ENGRAVED_LABEL);
    commandLine_input->align(FL_ALIGN_TOP_LEFT);
    commandLine_input->cursor_color(FL_RED);
    commandLine_input->color(FL_BLACK, FL_BLUE);
    commandLine_input->textcolor(FL_GREEN);
    //commandLine_input->textfont(FL_COURIER_BOLD);
  }

  tile = new Fl_Tile(0, RESIZABLE_H+3, MAIN_W, 504-RESIZABLE_H-3);
  {
    launch_scroll = new Fl_Scroll(0, RESIZABLE_H, MAIN_W, 260);
    launch_scroll->box(FL_DOWN_BOX);
    launch_scroll->color(FL_BLACK);
    launch_scroll->labeltype(FL_ENGRAVED_LABEL);
    launch_scroll->labelsize(18);
    launch_scroll->type(Fl_Scroll::VERTICAL_ALWAYS);
    launch_scroll->end();
  }

  kill_window = new Fl_Window(0, RESIZABLE_H+260, MAIN_W, 99);
  {
    kill_scroll = new Fl_Scroll(0, 0, MAIN_W, 99);
    kill_scroll->box(FL_DOWN_BOX);
    kill_scroll->color(FL_BLACK);
    kill_scroll->labeltype(FL_ENGRAVED_LABEL);
    kill_scroll->labelsize(18);
    kill_scroll->type(Fl_Scroll::VERTICAL_ALWAYS);
    kill_scroll->end();
  }
  kill_window->resizable(kill_scroll);
  kill_window->end();

  tile->end();
  main_window->resizable(tile);

  if(listFile)
    {
      if(buildLaunchButtons_withListFile((const  char *) listFile))
	return NULL; // failure
    }
  else if(buildLaunchButtons())
    return NULL; // failure

  main_window->end();
  // will parse the -geometry and other X window command line options
  main_window->show(argc, argv);

  reset_tile_position();
  return main_window;
}
