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
#include <signal.h>
#include <FL/Fl.H>
#include <dtk.h>
#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
# include <string.h>
#endif

#define PROGRAM_NAME  "dtk-caveDeviceSimulator"

extern int make_mainWindow(int fltk_argc, char **fltk_argv);


int running = 1;

#ifndef DTK_ARCH_WIN32_VCPP
// VRCO system V shared memory pointers
dtkVRCOSharedMem *sysV = NULL;
float *sysVhead = NULL;
float *sysVwand = NULL;
float *sysVjoystick = NULL;
int32_t *sysVbutton = NULL;
#endif


// DTK shared memory objects
dtkSharedMem *headShm, *wandShm, *joystickShm, *buttonShm;

float head[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
float wand[6] = { 0.0f, 0.2f, -0.2f, 0.0f, 0.0f, 0.0f };
float joystick[2] = { 0.0f, 0.0f };
unsigned char button = 0;

float reset_head[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
float reset_wand[6] = { 0.0f, 0.2f, -0.2f, 0.0f, 0.0f, 0.0f };
float reset_joystick[2] = { 0.0f, 0.0f };
unsigned char reset_button = 0;



static void catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,
             "caught signal %d: exiting.\n",sig);
  running = 0;
}

static int Usage(int returnVal)
{
  printf("Usage: "PROGRAM_NAME" "
#ifndef DTK_ARCH_WIN32_VCPP
	  "[-v|--vrco] "
#endif
	 "[FLTK_OPTIONS]|[-h|--help]\n"
	 "\n"
	 " dtk-caveDeviceSimulator is a GUI CAVE device simulator.\n"
	 "It writes to DTK shared memory files:\n"
	 "  name       size              description\n"
	 " ------    --------    -------------------------\n"
	 "  head     6 floats    XYZHPR of head tracker\n"
	 "  wand     6 floats    XYZHPR of wand tracker\n"
	 " joystick  2 floats    XY of wand joystick\n"
	 " buttons   1 byte      4 buttons bits 0,1,2,3\n"
	 "\n"
	 "   OPTIONS\n\n"
	 " -h|--help print this help then exit\n"
#ifndef DTK_ARCH_WIN32_VCPP
	 " -v|--vrco write to old Trackd VRCO system "
	 "V shared memory too\n"
#endif
	 "\n");
  dtkFLTKOptions_print(stdout);
  return returnVal;
}

static int do_args(int argc, char **argv)
{
  for(int i=1;i<argc;)
    {
      if(!strcmp("-h",argv[i]) || !strcmp("--help",argv[i]))
	return  Usage(1);
#ifndef DTK_ARCH_WIN32_VCPP
      if(!strcmp("-v",argv[i]) || !strcmp("--vrco",argv[i]))
	{
	  sysV = new dtkVRCOSharedMem;
	  if(!sysV || sysV->isInvalid())
	    return 1;
	  sysVhead = &(sysV->head->x);
	  sysVwand = &(sysV->wand->x);
	  sysVbutton = sysV->button;
	  sysVjoystick =  sysV->val;
	  i++;
	}
#endif
      else
	return  Usage(1);
    }
  return 0;
}


// main
int main(int argc, char **argv)
{
  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);
  int fltk_argc;
  char **fltk_argv;
  int other_argc;
  char **other_argv;
  // dtkFLTKOptions_get will pull out FLTK options into its return pointer.
  if(dtkFLTKOptions_get(argc, (const char **) argv,
                        &fltk_argc,  &fltk_argv,
                        &other_argc, &other_argv)) return 1;

  if(do_args(other_argc, other_argv)) return 1;

  headShm = new dtkSharedMem(sizeof(float)*6, "head", head);
  if(headShm->isInvalid()) return 1;
  wandShm = new dtkSharedMem(sizeof(float)*6, "wand", wand);
  if(wandShm->isInvalid()) return 1;
  joystickShm = new dtkSharedMem(sizeof(float)*2, "joystick", joystick);
  if(joystickShm->isInvalid()) return 1;
  buttonShm = new dtkSharedMem(sizeof(unsigned char), "buttons", &button);
  if(buttonShm->isInvalid()) return 1;

  if(make_mainWindow(fltk_argc, fltk_argv)) return 1;

  signal(SIGINT,catcher);
  signal(SIGTERM,catcher);

  while ((running) && Fl::wait());

#ifndef DTK_ARCH_WIN32_VCPP
  if(sysV)
    delete sysV;
#endif

  return 0;
}
