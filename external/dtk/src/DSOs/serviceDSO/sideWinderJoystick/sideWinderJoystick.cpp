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

#define CALIBRATE 0


#define DEADBAND(x , x0, FUG)   ((x) > (x0) + FUG)?((x) - (x0) - FUG):(((x) < (x0) - FUG)?((x) - (x0) + FUG):0.0f)


class MSSideWinderJoystick : public dtkService
{
public:
  MSSideWinderJoystick(void);
  ~MSSideWinderJoystick(void);
  int serve();

private:

  dtkSharedMem joystickShm, throttleShm, hatShm, buttonsShm;

  float joystick[3], throttle;
  unsigned char buttons[2];
  char hat[2];
};


MSSideWinderJoystick::MSSideWinderJoystick(void) :
  joystickShm(sizeof(float)*3, "sideWinder/joystick"),
  throttleShm(sizeof(float), "sideWinder/throttle"),
  hatShm(sizeof(char)*2, "sideWinder/hat"),
  buttonsShm(sizeof(unsigned char)*2, "sideWinder/buttons")
{
  // Initialize local state data.
  joystick[0] = 0.0f;
  joystick[1] = 0.0f;
  joystick[2] = 0.0f;
  throttle = 0.0;
  buttons[0] = 0;
  buttons[1] = 0;
  hat[0] = 0;
  hat[1] = 0;

  if(joystickShm.isInvalid() ||
     throttleShm.isInvalid() ||
     hatShm.isInvalid() ||
     buttonsShm.isInvalid())
    return; // error

   // open USB device.
  // auto detect the first correct USB device.
  fd = dtkUSBInput_findID(device_id);

  if(fd < 0)
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "MSSideWinderJoystick::MSSideWinderJoystick() failed"
	       " to find the correct USB device.\n");
}

MSSideWinderJoystick::~MSSideWinderJoystick(void)
{
  if(fd >= 0)
    {
      close(fd);
      fd = -1;
    }
}

int MSSideWinderJoystick::serve(void)
{
  struct input_event ev[64];

  int rd = read(fd, ev, sizeof(struct input_event) * 64);

  if (rd < (int) sizeof(struct input_event))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "MSSideWinderJoystick::serve() failed: "
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
#if CALIBRATE
	      joystick[0] =(float) value;
#else
	      joystick[0] =
		((float) DEADBAND((float) value, ((float) JOY_X_REST_VAL), ((float)JOY_X_FUG)))/
		((float)JOY_X_SCALE);
	      joystick[0] = joystick[0]>1.0f?1.0f:((joystick[0]<-1.0f)?-1.0f:joystick[0]);
#endif
	      if(joystickShm.write(joystick)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_Y)
	    {
	      int value = ev[i].value;
#if CALIBRATE
	      joystick[1] =(float) value;
#else
	      joystick[1] = - 
		((float) DEADBAND((float) value, ((float) JOY_Y_REST_VAL), ((float)JOY_Y_FUG)))/
		((float)JOY_Y_SCALE);
	      joystick[1] = joystick[1]>1.0f?1.0f:((joystick[1]<-1.0f)?-1.0f:joystick[1]);
#endif
	      if(joystickShm.write(joystick)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_RZ)
	    {
	      int value = ev[i].value;
#if CALIBRATE
	      joystick[2] = (float) value;
#else
	      joystick[2] = -
		((float) DEADBAND((float) value, ((float) JOY_Z_REST_VAL), ((float)JOY_Z_FUG)))/
		((float)JOY_Z_SCALE);
		
#endif
	      if(joystickShm.write(joystick)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == ABS_THROTTLE)
	    {
	      int value = ev[i].value;
#if CALIBRATE
	      // To calibrate.
	      
	      throttle = (float) value;
#else
	      if(value > THROTTLE_MAX_VAL)
		throttle = 0.0f;
	      else
		{
		  
		  throttle = - (((float) value) - ((float) THROTTLE_MAX_VAL))/
		    ((float) (THROTTLE_MAX_VAL - THROTTLE_MIN_VAL));
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
	  if (ev[i].code >= 288 && ev[i].code <= 295)
	    {
	      if(ev[i].value)
		buttons[0] |= 01 << (ev[i].code-288);
	      else
		buttons[0] &= ~(01 << (ev[i].code-288));
	      if(buttonsShm.write(buttons)) return DTKSERVICE_ERROR;
	    }
	  else if (ev[i].code == 296)
	    {
	      if(ev[i].value)
		buttons[1] = 01;
	      else
		buttons[1] = 00;
	      if(buttonsShm.write(buttons)) return DTKSERVICE_ERROR;
	    }
	}
    }

  return DTKSERVICE_CONTINUE;
}


// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg)
{
  return new MSSideWinderJoystick;
}

static int dtkDSO_unloader(dtkService *msSideWinderJoystick)
{
  delete msSideWinderJoystick;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
