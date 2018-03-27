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
#include <sys/types.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
#include <FL/Fl_Box.H>

#include <dtk.h>

// from getcwd.cpp
extern char *Getcwd(void);


// from launchButtons.cpp
extern int buildLaunchButtons(void);
extern int buildLaunchButtons_withListFile(const  char *listFile);
extern pid_t launch_pid;
extern char *launch_procfile;

// from dtk-demoRunner.cpp
extern int running;
extern char *listFile;
extern int switchLaunchToKill;
extern int switchKillToLaunch;


char *topDir = NULL;

Fl_Output *cwd_output = NULL;
Fl_Scroll *launch_scroll = NULL;
Fl_Button* killButton= NULL;
Fl_Box* killLabel = NULL;

static void quitCallback(Fl_Widget *who, void *data) { running = 0; }
static void killAndQuitCallback(Fl_Widget *who, void *data)
{
  if(launch_pid)
    {
      //dtkMsg.add(DTKMSG_NOTICE, "calling kill(%d, SIGTERM)\n", launch_pid); 
      kill(launch_pid, SIGTERM);
    }
  running = 0;
}

static void killCallback(Fl_Widget *who, void *data)
{
  if(launch_pid)
    {
      int fd = open(launch_procfile, O_RDONLY);
      if(fd == -1)
	{
	  launch_pid = (pid_t) 0;
	  switchKillToLaunch = 1;
	}
      else
	close(fd);
    }
  else // Messed up.  This should not happen.
    {
      switchKillToLaunch = 1;
    }

  if(launch_pid)
    {
      //dtkMsg.add(DTKMSG_NOTICE, "calling kill(%d, SIGTERM)\n", launch_pid);
      kill(launch_pid, SIGTERM);
    }
}


Fl_Window* make_mainWindow(int argc, char **argv)
{
  char *str = Getcwd();
  if(!str) return NULL;
  topDir = strdup(str);

#define PRE_WINTITLE "dtk-demoRunner: "
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

  Fl_Window* w = new Fl_Window(365, 326, winTitle);
  w->size_range(365, 201, 365, 990, 1, 1, 0);
  w->box(FL_ENGRAVED_BOX);

  {
    cwd_output = new Fl_Output (10, 257, 345, 25,
				"Current Working Directory");
    cwd_output->labeltype(FL_ENGRAVED_LABEL);
    cwd_output->align(FL_ALIGN_TOP_LEFT);
    cwd_output->value(topDir);
    cwd_output->color(FL_BLACK);
    cwd_output->textcolor(FL_WHITE);
    //cwd_output->textfont(FL_COURIER_BOLD);
  }
  {
    Fl_Button* o = new Fl_Button(10, 289, 90, 25, "Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(quitCallback);
  }
  {
    Fl_Button* o = new Fl_Button(198, 289, 156, 25, "Kill and Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(killAndQuitCallback);
  }
  {
    killLabel = new Fl_Box(0, 6, 365, 26, "Running: ");
    killLabel->labeltype(FL_EMBOSSED_LABEL);
    //killLabel->labelsize(18);
    killLabel->align(FL_ALIGN_INSIDE);
    killLabel->hide();
  }
  {
    killButton = new Fl_Button(10, 30, 345, 179+25, "Kill");
    killButton->labeltype(FL_ENGRAVED_LABEL);
    killButton->labelsize(18);
    killButton->callback(killCallback);
    // killButton->font(FL_COURIER_BOLD);
    killButton->hide();
    Fl_Group::current()->resizable(killButton);
  }
  {
    launch_scroll = new Fl_Scroll(10, 30, 345, 179+25,
				  "Available Actions");
    launch_scroll->box(FL_DOWN_BOX);
    launch_scroll->color(FL_BLACK);
    launch_scroll->labeltype(FL_ENGRAVED_LABEL);
    launch_scroll->labelsize(18);
    launch_scroll->type(Fl_Scroll::VERTICAL_ALWAYS);
    launch_scroll->end();
    Fl_Group::current()->resizable(launch_scroll);
    //launch_scroll->hide();
  }

  if(listFile)
    {
      if(buildLaunchButtons_withListFile((const  char *) listFile))
	return NULL; // failure
    }
  else if(buildLaunchButtons())
    return NULL; // failure

  // will parse the -geometry and other X window options
  w->show(argc, argv);
  w->end();

  return w;
}
