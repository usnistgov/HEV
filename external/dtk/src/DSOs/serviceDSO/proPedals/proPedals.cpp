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

// This service will auto detect the first correct CH Pro Pedals USB
// Rudder Pedals device. See config.h.

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

class ProPedals : public dtkService
{
public:
  ProPedals(void);
  ~ProPedals(void);
  int serve();

private:

  dtkSharedMem valuatorShm;

  float valuator[3];
};


ProPedals::ProPedals(void) :
  valuatorShm(sizeof(float)*3, "proPedals/valuator")
{
  // Initialize local state data.
  valuator[0] = 0.0f;
  valuator[1] = 0.0f;
  valuator[2] = 0.0f;

  if(valuatorShm.isInvalid())
    return; // error

  // auto detect the first correct USB device.
  fd = dtkUSBInput_findID(device_id);

  if(fd < 0)
    dtkMsg.add(DTKMSG_ERROR,
	       "ProPedals::ProPedals() failed find a good"
	       " CH (TM) Flight Sim Yoke device on any USB port.\n");
}

ProPedals::~ProPedals(void)
{
  if(fd >= 0)
    {
      close(fd);
      fd = -1;
    }
}

int ProPedals::serve(void)
{
  struct input_event ev[64];

  int rd = read(fd, ev, sizeof(struct input_event) * 64);

  if (rd < (int) sizeof(struct input_event))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "ProPedals::serve() failed: "
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
	      valuator[0] = ((float) value - 1)/254.0f;

	      if(valuatorShm.write(valuator)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_Y)
	    {
	      int value = ev[i].value;
	      valuator[1] = ((float) value - 1)/254.0f;

	      if(valuatorShm.write(valuator)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_THROTTLE)
	    {
	      int value = ev[i].value;
	      valuator[2] = 2.0f*(((float) value)/255.0f) - 1.0f;

	      if(valuatorShm.write(valuator)) return DTKSERVICE_ERROR;
	    }
	}
    }

  return DTKSERVICE_CONTINUE;
}


// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg)
{
  return new ProPedals;
}

static int dtkDSO_unloader(dtkService *proPedals)
{
  delete proPedals;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
