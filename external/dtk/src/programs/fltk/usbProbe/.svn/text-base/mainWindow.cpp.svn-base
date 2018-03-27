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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/input.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Group.H>
#include <dtk.h>

#include "widget.h"
#include "programName.h"


// from dtk-usbProbe.cpp
extern int running;

static void quitCallback(Fl_Widget *who, void *data)
{
  running = 0;
}

static void helpCallback(Fl_Widget *who, void *data) 
{
  fl_message("Pressing a button will cause this to print USB device "
	     "information and the current device input events to "
	     "standard output.");
}

int make_mainWindow(int argc, char **argv)
{
  Fl_Window* w = new Fl_Window(MAIN_WIDGET_WIDTH, 318, "DTK - Probe USB Input Devices");

  {
    Fl_Button* o = new Fl_Button(12, 288, MAIN_WIDGET_WIDTH/2-16, 25, "Quit");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(quitCallback);
  }
  {
    Fl_Button* o = new Fl_Button(MAIN_WIDGET_WIDTH/2, 288, MAIN_WIDGET_WIDTH/2-12, 25, "Help");
    o->labeltype(FL_ENGRAVED_LABEL);
    o->callback(helpCallback);
  }
  
  Fl_Scroll *launch_scroll = new Fl_Scroll(0, 30, MAIN_WIDGET_WIDTH, 253);
  launch_scroll->box(FL_DOWN_BOX);
  launch_scroll->labeltype(FL_ENGRAVED_LABEL);
  launch_scroll->labelsize(18);
  launch_scroll->type(Fl_Scroll::VERTICAL_ALWAYS);
  launch_scroll->end();
  Fl_Group::current()->resizable(launch_scroll);
  
  int Launcher_count = 0;
  int device_count = 0;
  {
    char fileName[64];
    int fd;
    errno = 0;
    do
      {
	sprintf(fileName, "/dev/input/event%d", device_count++); 
	fd = open(fileName, O_RDONLY);

	if(fd < 0)
	  {
	    dtkMsg.add(DTKMSG_DEBUG, 1, "failed to open file \"%s\":\n"
		       " open(\"%s\", O_RDONLY)=%d failed\n",
		       fileName, fileName, fd);
	  }
	else
	  {
	    char name[256] = "unknown name";
	    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	    char launcherLabel[512];
	    snprintf(launcherLabel, 511, "read   \"%s\"   on:  %s",
		     name, fileName);
	    Launcher_Widget *o = new Launcher_Widget(Launcher_count, launcherLabel, fileName);

	    dtkMsg.add(DTKMSG_INFO, "found USB hardware input device: \"%s%s%s\" on \"%s%s%s\"\n",
		       dtkMsg.color.vil, name, dtkMsg.color.end,
		       dtkMsg.color.grn, fileName, dtkMsg.color.end);
	    dtkMsg.add(DTKMSG_DEBUG, "making button labeled: \"%s%s%s\"\n",
		       dtkMsg.color.tur, launcherLabel, dtkMsg.color.end);
	    launch_scroll->add(o);
	    Launcher_count++;
	    close(fd);
	  }
      }
    while (fd >= 0 || errno == ENODEV);
  }
  
  if(!Launcher_count)
    {
      dtkMsg.add(DTKMSG_WARN, "No USB input hardware found.\n");
      return 1; // error
    }

  static char scroll_label[128];
  snprintf(scroll_label, 127, "Probe USB Input Devices -- %d devices found",
	   Launcher_count);
  launch_scroll->label(scroll_label);

  
  int maxHeight = 69 + Launcher_count*32;
  if(maxHeight > 709) maxHeight = 709;
  w->size_range(MAIN_WIDGET_MIN_WIDTH, 101, MAIN_WIDGET_MAX_WIDTH, maxHeight, 1, 1, 0);
  w->end();
  w->size(MAIN_WIDGET_WIDTH, maxHeight);
  w->show(argc, argv);
  
  return 0; // success
}
