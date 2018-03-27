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
// This just works on GNU/Linux operating systems.

// This service will auto detect the first correct CH (TM) Flight Sim
// Yoke LE device. See config.h.

// This DTK service will look for the following device_name on the
// following device_files in the order listed until it succeeds in
// opening and identifying the USB hardware device or fails on all
// combinations or device_name and device_files. See config.h.

// See the file README for more on setting up your kernal modules for
// talking to USB ports for joystick and like devices.

// Many thanks to Vojtech Pavlik for his work with/on Linux USB
// drivers.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>

#include <dtk.h>
#include <dtk/dtkService.h>

#include "config.h"

#define CALIBRATE 0

class YokeLE : public dtkService
{
public:
  YokeLE(void);
  ~YokeLE(void);
  int serve();

private:

  dtkSharedMem axisShm, throttleShm, hatShm, buttonsShm;

  float axis[2], throttle;
  char hat[2];
  unsigned char buttons[2];
};


YokeLE::YokeLE(void) :
  axisShm(sizeof(float)*2, "yokeLE/axis"),
  throttleShm(sizeof(float), "yokeLE/throttle"),
  hatShm(sizeof(char)*2, "yokeLE/hat"),
  buttonsShm(sizeof(unsigned char)*2, "yokeLE/buttons")
{
  // Initialize local state data.
  axis[0] = 0.0f;
  axis[1] = 0.0f;
  throttle = 0.0f;
  buttons[0] = 0;
  buttons[1] = 0;
  hat[0] = 0;
  hat[1] = 0;

  if(axisShm.isInvalid() ||
     throttleShm.isInvalid() ||
     hatShm.isInvalid() ||
     buttonsShm.isInvalid())
    return; // error

  // auto detect the first correct USB device.
  fd = dtkUSBInput_findID(device_id);

  if(fd < 0)
    dtkMsg.add(DTKMSG_ERROR,
	       "YokeLE::YokeLE() failed find a good"
	       " CH (TM) Flight Sim Yoke device on any USB port.\n");
}

YokeLE::~YokeLE(void)
{
  if(fd >= 0)
    {
      close(fd);
      fd = -1;
    }
}

int YokeLE::serve(void)
{
  struct input_event ev[64];

  int rd = read(fd, ev, sizeof(struct input_event) * 64);

  if (rd < (int) sizeof(struct input_event))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "YokeLE::serve() failed: "
		 "read() failed.\n");
      return DTKSERVICE_CONTINUE;
    }

  for (int i = 0; i < (int) (rd / sizeof(struct input_event)); i++)
    {
      if(ev[i].type == EV_ABS)
	{
	  if (ev[i].code == ABS_X)
	    {
	      int value = ev[i].value;
	      axis[0] = (float)(value - 127)/125.0f;

	      if(axis[0] > 1.0f)
		axis[0] = 1.0f;
	      else if(axis[0] < - 1.0f)
		axis[0] = - 1.0f;

	      if(axisShm.write(axis)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_Y)
	    {
	      int value = ev[i].value;
	      axis[1] = ((float) (value - 127))/126.0f;

	      if(axis[1] > 1.0f)
		axis[1] = 1.0f;
	      else if(axis[1] < - 1.0f)
		axis[1] = - 1.0f;

	      if(axisShm.write(axis)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_THROTTLE)
	    {
	      int value = ev[i].value;
	      throttle = ((float) value)/255.0f;

	      if(throttleShm.write(&throttle)) return DTKSERVICE_ERROR;
	    }

	  else if (ev[i].code == ABS_HAT0X)
	    {
	      hat[0] = ev[i].value;
	      if(hatShm.write(hat)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_HAT0Y)
	    {
	      hat[1] = - ev[i].value;
	      if(hatShm.write(hat)) return DTKSERVICE_ERROR;
	    }
	}
      else if(ev[i].type == EV_KEY)
	{
	  if (ev[i].code >= 288 && ev[i].code <= 295)
	    {
	      if(ev[i].value)
		buttons[0] |= 01 << (ev[i].code-288);
	      else
		buttons[0] &= ~(01 << (ev[i].code-288));
	      if(buttonsShm.write(buttons)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code >= 296 && ev[i].code <= 303)
	    {
	      if(ev[i].value)
		buttons[1] |= 01 << (ev[i].code-296);
	      else
		buttons[1] &= ~(01 << (ev[i].code-296));
	      if(buttonsShm.write(buttons)) return DTKSERVICE_ERROR;
	    }
	}
    }

  return DTKSERVICE_CONTINUE;
}


// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg)
{
  return new YokeLE;
}

static int dtkDSO_unloader(dtkService *yokeLE)
{
  delete yokeLE;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
