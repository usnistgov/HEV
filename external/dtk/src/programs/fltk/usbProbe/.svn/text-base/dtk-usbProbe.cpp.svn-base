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
#include <dtk.h>
#ifdef DTK_ARCH_LINUX /* include dtk.h before this */
# ifndef __USE_UNIX98
#  define __USE_UNIX98
# endif
#else
#  error "This is only ported to GNU/Linux"
#endif

#include <signal.h>
#include <stdio.h>
#include <FL/Fl.H>

#include "programName.h"

// from mainWindow.cpp
extern int make_mainWindow(int argc, char **argv);


// Global flags
int running = 1;


static int Usage(int returnVal)
{
  printf("\n"
	 "  Usage: %s [-h|--help] [FLTK_OPTIONS]\n"
	 "\n"
	 "  %s will check all input USB devices files for hardware devices\n"
	 " that are plugged in. The device files checked are of the form: /dev/input/eventX,\n"
	 " where X is a number. It will then make GUI buttons to launch and \"probe\" each\n"
	 " USB device file that it detects USB hardware on.  When a button is pressed it will\n"
	 " print the USP device information to stdandard output.\n\n", PROGRAM_NAME, PROGRAM_NAME);
  dtkFLTKOptions_print(stdout);
  printf(" System requirements:\n\n"
	 "  1) You must have the Linux modules: evdev, usb-uhci or usb-ohci, and hid, and\n"
	 "     joydev loaded.  Use `lsmod' to check and `modprobe' (as root) to load them.\n\n"
	 "     For example run:\n"
	 "       /sbin/lsmod\n"
	 "       /sbin/modprobe evdev\n"
	 "       /sbin/modprobe usb-uhci\n"
	 "       /sbin/modprobe hid\n"
	 "       /sbin/modprobe joydev\n"
	 "       /sbin/lsmod\n\n"
	 "  2) You must have read and write access to the /dev/input/event* device files.\n"
	 "     Use `chmod og+rw /dev/input/event*' (as root) to make this so.\n\n"
	 "  3) You must have one or more UBS input devices plugged into your computer.\n\n"
	 );
  
  printf("\n This program is based on Vojtech Pavlik's Event device test program `evtest' which\n"
	 " is Licensed under the GNU General Public License like this program.\n\n");
  return returnVal;
}

static void signal_caughter(int sig)
{
  dtkMsg.add(DTKMSG_INFO, "caught signal %d ... exiting.\n",
	     sig);
  running = 0;
}


int main(int argc, char** argv)
{
  dtkMsg.setPreMessage("%s%s%s",dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);

  int fltk_argc, other_argc;
  char **fltk_argv, **other_argv;

  if(dtkFLTKOptions_get(argc, (const char**) argv,
                        &fltk_argc, &fltk_argv,
                        &other_argc, &other_argv) ||
     other_argc > 1 || make_mainWindow(fltk_argc, fltk_argv))
    return Usage(1);

  signal(SIGTERM, signal_caughter);
  signal(SIGINT, signal_caughter);

  while (Fl::wait() && running);

  return 0;
}
