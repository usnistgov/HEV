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
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>

#include <dtk.h>
#include <dtk/dtkService.h>

#include "config.h"

#define FUG       ((float) JOY_DEAD_BAND_WIDTH)
#define DEADBAND(x , x0)   ((x) > (x0) + FUG)?((x) - (x0) - FUG):(((x) < (x0) - FUG)?((x) - (x0) + FUG):0.0f)


class WingManJoystick : public dtkService
{
public:
  WingManJoystick(void);
  ~WingManJoystick(void);
  int serve();

private:

  dtkSharedMem joystickShm, throttleShm, hatShm, buttonsShm;

  float joystick[3], throttle;
  unsigned char buttons;
  char hat[2];
};


WingManJoystick::WingManJoystick(void) :
  joystickShm(sizeof(float)*3, "wingMan/joystick"),
  throttleShm(sizeof(float), "wingMan/throttle"),
  hatShm(sizeof(char)*2, "wingMan/hat"),
  buttonsShm(sizeof(unsigned char), "wingMan/buttons")
{
  // Initialize local state data.
  joystick[0] = 0.0f;
  joystick[1] = 0.0f;
  joystick[2] = 0.0f;
  throttle = 0.0;
  buttons = 0;
  hat[0] = 0;
  hat[1] = 0;

  if(joystickShm.isInvalid() ||
     throttleShm.isInvalid() ||
     hatShm.isInvalid() ||
     buttonsShm.isInvalid())
    return; // error
 
  // auto detect the first correct USB device.
  fd = dtkUSBInput_findID(device_id);

  if(fd < 0)
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "WingManJoystick::WingManJoystick() failed"
		 " to find the correct USB device.\n");

}

WingManJoystick::~WingManJoystick(void)
{
  if(fd >= 0)
    {
      close(fd);
      fd = -1;
    }
}

int WingManJoystick::serve(void)
{
  struct input_event ev[64];

  int rd = read(fd, ev, sizeof(struct input_event) * 64);

  if (rd < (int) sizeof(struct input_event))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "WingManJoystick::serve() failed: "
		 "read() failed.\n");
      return DTKSERVICE_CONTINUE;
    }

  for (int i = 0; i < (int) (rd / sizeof(struct input_event)); i++)
    {
      if(ev[i].type == EV_ABS)
	{
	  if (ev[i].code == ABS_X)
	    {
	      joystick[0] =
		((float) DEADBAND((float) ev[i].value, ((float) JOY_X_REST_VAL)))/((float)JOY_X_SCALE);
	      joystick[0] = joystick[0]>1.0f?1.0f:((joystick[0]<-1.0f)?-1.0f:joystick[0]);
	      if(joystickShm.write(joystick)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_Y)
	    {
	      joystick[1] = - 
		((float) DEADBAND((float) ev[i].value, ((float) JOY_Y_REST_VAL)))/((float)JOY_Y_SCALE);
	      joystick[1] = joystick[1]>1.0f?1.0f:((joystick[1]<-1.0f)?-1.0f:joystick[1]);
	      if(joystickShm.write(joystick)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_RZ)
	    {
	      joystick[2] = -
		((float) DEADBAND((float) ev[i].value, ((float) JOY_Z_REST_VAL)))/((float)JOY_Z_SCALE);
	      joystick[2] = joystick[2]>1.0f?1.0f:((joystick[2]<-1.0f)?-1.0f:joystick[2]);
	      if(joystickShm.write(joystick)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_THROTTLE)
	    {
#if 0
	      // To calibrate.
	      throttle = ((float) ev[i].value);
#else
	      if(ev[i].value > THROTTLE_MAX_VAL)
		throttle = 0.0f;
	      else
		{
		  throttle = - (((float) ev[i].value) - ((float) THROTTLE_MAX_VAL))/
		    ((float) THROTTLE_MAX_VAL - THROTTLE_MIN_VAL);
		  if(throttle > 1.0f)
		    throttle = 1.0f;
		}
	      if(throttle < 0.0f)
		throttle = 0.0f;
#endif
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
	  if (ev[i].code >= 288 && ev[i].code <= 294)
	    {
	      if(ev[i].value)
		buttons |= 01 << (ev[i].code-288);
	      else
		buttons &= ~(01 << (ev[i].code-288));
	      if(buttonsShm.write(&buttons)) return DTKSERVICE_ERROR;
	    }
	}
    }

  return DTKSERVICE_CONTINUE;
}


// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg)
{
  return new WingManJoystick;
}

static int dtkDSO_unloader(dtkService *wingManJoystick)
{
  delete wingManJoystick;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
