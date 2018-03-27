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
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/input.h>

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Box.H>
#include <dtk.h>

#include "widget.h"

#define FL_WAIT_TIME (0.02) // seconds

// from dtk-usbProbe.cpp
extern int running;

char *events[EV_MAX + 1] =
  { 
    const_cast<char*>("Reset"), const_cast<char*>("Key"), const_cast<char*>("Relative"), const_cast<char*>("Absolute"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, const_cast<char*>("LED"), const_cast<char*>("Sound"), NULL, const_cast<char*>("Repeat"), const_cast<char*>("ForceFeedback"), NULL, const_cast<char*>("ForceFeedbackStatus")
  };

char *keys[KEY_MAX + 1] =
  { 
    const_cast<char*>("Reserved"), const_cast<char*>("Esc"), const_cast<char*>("1"), const_cast<char*>("2"), const_cast<char*>("3"), const_cast<char*>("4"), const_cast<char*>("5"), const_cast<char*>("6"), const_cast<char*>("7"), const_cast<char*>("8"), const_cast<char*>("9"), const_cast<char*>("0"), const_cast<char*>("Minus"), const_cast<char*>("Equal"), const_cast<char*>("Backspace"),
    const_cast<char*>("Tab"), const_cast<char*>("Q"), const_cast<char*>("W"), const_cast<char*>("E"), const_cast<char*>("R"), const_cast<char*>("T"), const_cast<char*>("Y"), const_cast<char*>("U"), const_cast<char*>("I"), const_cast<char*>("O"), const_cast<char*>("P"), const_cast<char*>("LeftBrace"), const_cast<char*>("RightBrace"), const_cast<char*>("Enter"), const_cast<char*>("LeftControl"), const_cast<char*>("A"), const_cast<char*>("S"), const_cast<char*>("D"), const_cast<char*>("F"), const_cast<char*>("G"),
    const_cast<char*>("H"), const_cast<char*>("J"), const_cast<char*>("K"), const_cast<char*>("L"), const_cast<char*>("Semicolon"), const_cast<char*>("Apostrophe"), const_cast<char*>("Grave"), const_cast<char*>("LeftShift"), const_cast<char*>("BackSlash"), const_cast<char*>("Z"), const_cast<char*>("X"), const_cast<char*>("C"), const_cast<char*>("V"), const_cast<char*>("B"), const_cast<char*>("N"), const_cast<char*>("M"), const_cast<char*>("Comma"), const_cast<char*>("Dot"),
    const_cast<char*>("Slash"), const_cast<char*>("RightShift"), const_cast<char*>("KPAsterisk"), const_cast<char*>("LeftAlt"), const_cast<char*>("Space"), const_cast<char*>("CapsLock"), const_cast<char*>("F1"), const_cast<char*>("F2"), const_cast<char*>("F3"), const_cast<char*>("F4"), const_cast<char*>("F5"), const_cast<char*>("F6"), const_cast<char*>("F7"), const_cast<char*>("F8"), const_cast<char*>("F9"), const_cast<char*>("F10"),
    const_cast<char*>("NumLock"), const_cast<char*>("ScrollLock"), const_cast<char*>("KP7"), const_cast<char*>("KP8"), const_cast<char*>("KP9"), const_cast<char*>("KPMinus"), const_cast<char*>("KP4"), const_cast<char*>("KP5"), const_cast<char*>("KP6"), const_cast<char*>("KPPlus"), const_cast<char*>("KP1"), const_cast<char*>("KP2"), const_cast<char*>("KP3"), const_cast<char*>("KP0"), const_cast<char*>("KPDot"), const_cast<char*>("103rd"),
    const_cast<char*>("F13"), const_cast<char*>("102nd"), const_cast<char*>("F11"), const_cast<char*>("F12"), const_cast<char*>("F14"), const_cast<char*>("F15"), const_cast<char*>("F16"), const_cast<char*>("F17"), const_cast<char*>("F18"), const_cast<char*>("F19"), const_cast<char*>("F20"), const_cast<char*>("KPEnter"), const_cast<char*>("RightCtrl"), const_cast<char*>("KPSlash"), const_cast<char*>("SysRq"),
    const_cast<char*>("RightAlt"), const_cast<char*>("LineFeed"), const_cast<char*>("Home"), const_cast<char*>("Up"), const_cast<char*>("PageUp"), const_cast<char*>("Left"), const_cast<char*>("Right"), const_cast<char*>("End"), const_cast<char*>("Down"), const_cast<char*>("PageDown"), const_cast<char*>("Insert"), const_cast<char*>("Delete"), const_cast<char*>("Macro"), const_cast<char*>("Mute"),
    const_cast<char*>("VolumeDown"), const_cast<char*>("VolumeUp"), const_cast<char*>("Power"), const_cast<char*>("KPEqual"), const_cast<char*>("KPPlusMinus"), const_cast<char*>("Pause"), const_cast<char*>("F21"), const_cast<char*>("F22"), const_cast<char*>("F23"), const_cast<char*>("F24"), const_cast<char*>("KPComma"), const_cast<char*>("LeftMeta"), const_cast<char*>("RightMeta"),
    const_cast<char*>("Compose"), const_cast<char*>("Stop"), const_cast<char*>("Again"), const_cast<char*>("Props"), const_cast<char*>("Undo"), const_cast<char*>("Front"), const_cast<char*>("Copy"), const_cast<char*>("Open"), const_cast<char*>("Paste"), const_cast<char*>("Find"), const_cast<char*>("Cut"), const_cast<char*>("Help"), const_cast<char*>("Menu"), const_cast<char*>("Calc"), const_cast<char*>("Setup"),
    const_cast<char*>("Sleep"), const_cast<char*>("WakeUp"), const_cast<char*>("File"), const_cast<char*>("SendFile"), const_cast<char*>("DeleteFile"), const_cast<char*>("X-fer"), const_cast<char*>("Prog1"), const_cast<char*>("Prog2"), const_cast<char*>("WWW"), const_cast<char*>("MSDOS"), const_cast<char*>("Coffee"), const_cast<char*>("Direction"),
    const_cast<char*>("CycleWindows"), const_cast<char*>("Mail"), const_cast<char*>("Bookmarks"), const_cast<char*>("Computer"), const_cast<char*>("Back"), const_cast<char*>("Forward"), const_cast<char*>("CloseCD"), const_cast<char*>("EjectCD"), const_cast<char*>("EjectCloseCD"), const_cast<char*>("NextSong"), const_cast<char*>("PlayPause"),
    const_cast<char*>("PreviousSong"), const_cast<char*>("StopCD"), const_cast<char*>("Record"), const_cast<char*>("Rewind"), const_cast<char*>("Phone"), const_cast<char*>("ISOKey"), const_cast<char*>("Config"), const_cast<char*>("HomePage"), const_cast<char*>("Refresh"), const_cast<char*>("Exit"), const_cast<char*>("Move"), const_cast<char*>("Edit"), const_cast<char*>("ScrollUp"),
    const_cast<char*>("ScrollDown"), const_cast<char*>("KPLeftParenthesis"), const_cast<char*>("KPRightParenthesis"),
    const_cast<char*>("International1"), const_cast<char*>("International2"), const_cast<char*>("International3"), const_cast<char*>("International4"), const_cast<char*>("International5"),
    const_cast<char*>("International6"), const_cast<char*>("International7"), const_cast<char*>("International8"), const_cast<char*>("International9"),
    const_cast<char*>("Language1"), const_cast<char*>("Language2"), const_cast<char*>("Language3"), const_cast<char*>("Language4"), const_cast<char*>("Language5"), const_cast<char*>("Language6"), const_cast<char*>("Language7"), const_cast<char*>("Language8"), const_cast<char*>("Language9"),
    NULL, const_cast<char*>("PlayCD"), const_cast<char*>("PauseCD"), const_cast<char*>("Prog3"), const_cast<char*>("Prog4"), const_cast<char*>("Suspend"), const_cast<char*>("Close"),
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    const_cast<char*>("Btn0"), const_cast<char*>("Btn1"), const_cast<char*>("Btn2"), const_cast<char*>("Btn3"), const_cast<char*>("Btn4"), const_cast<char*>("Btn5"), const_cast<char*>("Btn6"), const_cast<char*>("Btn7"), const_cast<char*>("Btn8"), const_cast<char*>("Btn9"),
    NULL, NULL,  NULL, NULL, NULL, NULL,
    const_cast<char*>("LeftBtn"), const_cast<char*>("RightBtn"), const_cast<char*>("MiddleBtn"), const_cast<char*>("SideBtn"), const_cast<char*>("ExtraBtn"), const_cast<char*>("ForwardBtn"), const_cast<char*>("BackBtn"),
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    const_cast<char*>("Trigger"), const_cast<char*>("ThumbBtn"), const_cast<char*>("ThumbBtn2"), const_cast<char*>("TopBtn"), const_cast<char*>("TopBtn2"), const_cast<char*>("PinkieBtn"),
    const_cast<char*>("BaseBtn"), const_cast<char*>("BaseBtn2"), const_cast<char*>("BaseBtn3"), const_cast<char*>("BaseBtn4"), const_cast<char*>("BaseBtn5"), const_cast<char*>("BaseBtn6"),
    NULL, NULL, NULL, const_cast<char*>("BtnDead"),
    const_cast<char*>("BtnA"), const_cast<char*>("BtnB"), const_cast<char*>("BtnC"), const_cast<char*>("BtnX"), const_cast<char*>("BtnY"), const_cast<char*>("BtnZ"), const_cast<char*>("BtnTL"), const_cast<char*>("BtnTR"), const_cast<char*>("BtnTL2"), const_cast<char*>("BtnTR2"), const_cast<char*>("BtnSelect"), const_cast<char*>("BtnStart"), const_cast<char*>("BtnMode"),
    const_cast<char*>("BtnThumbL"), const_cast<char*>("BtnThumbR"), NULL,
    const_cast<char*>("ToolPen"), const_cast<char*>("ToolRubber"), const_cast<char*>("ToolBrush"), const_cast<char*>("ToolPencil"), const_cast<char*>("ToolAirbrush"), const_cast<char*>("ToolFinger"), const_cast<char*>("ToolMouse"), const_cast<char*>("ToolLens"), NULL, NULL,
    const_cast<char*>("Touch"), const_cast<char*>("Stylus"), const_cast<char*>("Stylus2")
  };

char *absval[5] =
  {
    const_cast<char*>("Value"), const_cast<char*>("Min  "), const_cast<char*>("Max  "), const_cast<char*>("Fuzz "), const_cast<char*>("Flat ")
  };
char *relatives[REL_MAX + 1] =
  { 
    const_cast<char*>("X"), const_cast<char*>("Y"), const_cast<char*>("Z"), NULL, NULL, NULL, const_cast<char*>("HWheel"), const_cast<char*>("Dial"), const_cast<char*>("Wheel")
  };
char *absolutes[ABS_MAX + 1] =
  {
    const_cast<char*>("X"), const_cast<char*>("Y"), const_cast<char*>("Z"), const_cast<char*>("Rx"), const_cast<char*>("Ry"), const_cast<char*>("Rz"), const_cast<char*>("Throttle"), const_cast<char*>("Rudder"), const_cast<char*>("Wheel"), const_cast<char*>("Gas"), const_cast<char*>("Brake"),
    NULL, NULL, NULL, NULL, NULL,
    const_cast<char*>("Hat0X"), const_cast<char*>("Hat0Y"), const_cast<char*>("Hat1X"), const_cast<char*>("Hat1Y"), const_cast<char*>("Hat2X"), const_cast<char*>("Hat2Y"), const_cast<char*>("Hat3X"), const_cast<char*>("Hat 3Y"), const_cast<char*>("Pressure"), const_cast<char*>("Distance"), const_cast<char*>("XTilt"), const_cast<char*>("YTilt")
  };
char *leds[LED_MAX + 1] =
  {
    const_cast<char*>("NumLock"), const_cast<char*>("CapsLock"), const_cast<char*>("ScrollLock"), const_cast<char*>("Compose"), const_cast<char*>("Kana"), const_cast<char*>("Sleep"), const_cast<char*>("Suspend"), const_cast<char*>("Mute")
  };
char *repeats[REP_MAX + 1] =
  {
    const_cast<char*>("Delay"), const_cast<char*>("Period")
  };
char *sounds[SND_MAX + 1] =
  {
    const_cast<char*>("Bell"), const_cast<char*>("Click")
  };

char **names[EV_MAX + 1] =
  {
    events, keys, relatives, absolutes, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, leds, sounds, NULL, repeats, NULL, NULL, NULL
  };

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)


static Launcher_Widget *last_Launcher = NULL;

static void launcher_callback(Fl_Widget *who, void *data)
{
  Launcher_Widget *launcher = (Launcher_Widget *) data;

  if(!running) return;

  dtkMsg.add(DTKMSG_DEBUG, "calling launcher_callback for \"%s\"\n",
	     launcher->deviceFile);

  if(launcher->Button.value())
    {
      if(last_Launcher)
	{
	  last_Launcher->Button.value(0);
	  if(last_Launcher->fd >= 0)
	    {
	      dtkMsg.add(DTKMSG_DEBUG, "closing device \"%s\"\n",
			 launcher->deviceFile);
	      close(last_Launcher->fd);
	      last_Launcher->fd = -1;
	    }
	}

      last_Launcher = launcher;

      launcher->fd = open(launcher->deviceFile, O_RDONLY);

      if(launcher->fd < 0)
	{
	  dtkMsg.add(DTKMSG_WARN, 1, "failed to open file \"%s\":\n"
		     " open(\"%s\", O_RDONLY)=%d failed\n",
		     launcher->deviceFile, launcher->deviceFile, launcher->fd);
	  last_Launcher = NULL;
	  launcher->Button.value(0);
	  return;
	}

      printf("\n%s---------------------------HARDWARE-DESCRIPTION-----------------------------%s\n",
	     dtkMsg.color.yel, dtkMsg.color.end);

      printf("opened device \"%s\"\n",
	     launcher->deviceFile);

      {
	char name[256] = "unknown";
	ioctl(launcher->fd, EVIOCGNAME(sizeof(name)), name);
	printf("Input device name: \"%s%s%s\"\n", dtkMsg.color.rgrn, name, dtkMsg.color.end);

	int version;
	if (ioctl(launcher->fd, EVIOCGVERSION, &version)) 
	  dtkMsg.add(DTKMSG_WARN, "can't get version");
	else
	  printf("Input driver version is %d.%d.%d\n",
		 version >> 16, (version >> 8) & 0xff, version & 0xff);
	
	unsigned short id[4];
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
	int abs[5];

	if(ioctl(launcher->fd, EVIOCGID, id) != -1)
	  printf("Input device ID = { %sbus%s, %svendor%s, %sproduct%s, %sversion%s } = { %s0x%x%s, %s0x%x%s, %s0x%x%s, %s0x%x%s },\n",
		 dtkMsg.color.grn, dtkMsg.color.end,
		 dtkMsg.color.yel, dtkMsg.color.end,
		 dtkMsg.color.tur, dtkMsg.color.end,
		 dtkMsg.color.vil, dtkMsg.color.end,

		 dtkMsg.color.grn, id[ID_BUS], dtkMsg.color.end,
		 dtkMsg.color.yel, id[ID_VENDOR],dtkMsg.color.end,
		 dtkMsg.color.tur, id[ID_PRODUCT],dtkMsg.color.end,
		 dtkMsg.color.vil, id[ID_VERSION],dtkMsg.color.end);
		
	memset(bit, 0, sizeof(bit));
	ioctl(launcher->fd, EVIOCGBIT(0, EV_MAX), bit[0]);
	printf("Supported events:\n");
	
	int i;
	for (i = 0; i < EV_MAX; i++)
	  if (test_bit(i, bit[0])) {
	    printf("  Event type %d (%s)\n", i, events[i] ? events[i] : "?");
	    ioctl(launcher->fd, EVIOCGBIT(i, KEY_MAX), bit[i]);
	    int j;
	    for (j = 0; j < KEY_MAX; j++) 
	      if (test_bit(j, bit[i])) {
		printf("    Event code %d (%s)\n", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?");
		if (i == EV_ABS) {
		  ioctl(launcher->fd, EVIOCGABS(j), abs);
		  int k;
		  for (k = 0; k < 5; k++)
		    if ((k < 3) || abs[k])
		      printf("      %s %6d\n", absval[k], abs[k]);
		}
	      }
	  }
      }
      
      printf("%s----------------------------CURRENT-EVENTS----------------------------------%s\n",
	     dtkMsg.color.yel, dtkMsg.color.end);
      // proccess FLTK events and read device.
      double fl_wait_return = 1.0;
      while(launcher->fd >= 0 && running && fl_wait_return > 0.0)
	fl_wait_return = Fl::wait(0.0);

      while(launcher->fd >= 0 && running && fl_wait_return >= 0.0)
	{
	  int rd = 0;
	  struct input_event ev[64];
	  do
	    {
	      { // The select() call to see if there is data.

		fd_set rfds;
		struct timeval tv = {0,0};
		
		/* Watch file desciptor (launcher->fd) to see when it has input. */
		FD_ZERO(&rfds);
		FD_SET(launcher->fd, &rfds);
		
		int retval = select(launcher->fd+1, &rfds, NULL, NULL, &tv);
		
		if (retval <= 0)
		  {
		    if(retval < 0)
		      {
			dtkMsg.add(DTKMSG_WARN, 1, "reading \"%s\" failed: select() failed.\n",
				   launcher->deviceFile);
			dtkMsg.add(DTKMSG_DEBUG, "closing device \"%s\"\n",
				   launcher->deviceFile);
			close(launcher->fd);
			launcher->fd = -1;
		      }
		    break;
		  }
	      }
	      
	      rd = read(launcher->fd, ev, sizeof(struct input_event)*64);
	      
	      if (rd < (int) sizeof(struct input_event))
		break;
	      
	      int i;
	      for (i=0; (int) i<(int)(rd / sizeof(struct input_event)); i++)
		{
		  printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
			 ev[i].time.tv_sec, ev[i].time.tv_usec, ev[i].type,
			 events[ev[i].type] ? events[ev[i].type] : "?",
			 ev[i].code,
			 names[ev[i].type] ? (names[ev[i].type][ev[i].code] ? names[ev[i].type][ev[i].code] : "?") : "?",
			 ev[i].value);
		}

	    }
	  while(rd > 0);
	  
	  if(rd < 0)
	    {
	      dtkMsg.add(DTKMSG_WARN, 1, "failed to read file \"%s\":\n"
			 " read(%d, %p, %d)=%d failed\n",
			 launcher->deviceFile, launcher->fd, ev,
			 sizeof(struct input_event)*64, rd);
	    }
	  else if(launcher->fd >= 0)
	    {
	      fl_wait_return = 1.0;
	      while(launcher->fd >= 0 && running && fl_wait_return > 0.0)
		fl_wait_return = Fl::wait(FL_WAIT_TIME);
	    }
	}

      launcher->Button.value(0);
      if(launcher->fd >= 0)
	{
	  dtkMsg.add(DTKMSG_DEBUG, "closing device \"%s\"\n",
		     launcher->deviceFile);
	  close(launcher->fd);
	  launcher->fd = -1;
	}
    }
  else // launcher->Button.value() == 0
    {
      if(launcher->fd >= 0)
	{
	  dtkMsg.add(DTKMSG_DEBUG, "closing device \"%s\"\n",
		     launcher->deviceFile);
	  close(launcher->fd);
	  launcher->fd = -1;
	}
      last_Launcher = NULL;
    }
}

Launcher_Widget::~Launcher_Widget(void)
{
  if(deviceFile)
    {
      free(deviceFile);
      deviceFile = NULL;
    }
  if(label)
    {
      free(label);
      label = NULL;
    }
}

#define HEIGHT  32

Launcher_Widget::
Launcher_Widget(int num, char *label_in, char *deviceFile_in):
  Fl_Group(2, yPos = 32 + num*HEIGHT, MAIN_WIDGET_MAX_WIDTH-22, HEIGHT),
  Button(2, yPos, MAIN_WIDGET_MAX_WIDTH-22, HEIGHT, label= dtk_strdup(label_in))
{
  deviceFile = dtk_strdup(deviceFile_in);

  Button.labelsize(12);
  //Fl_Group::current()->resizable(&Button);
  Button.callback(launcher_callback, this);
  add(&Button);
  fd = -1;
}
