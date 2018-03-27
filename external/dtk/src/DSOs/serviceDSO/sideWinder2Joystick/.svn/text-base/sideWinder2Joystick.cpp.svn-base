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
// This was originally copied from sideWinderJoystick/ and edited by
// John Kelso.
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

#define RAW 0 // Set to 1 to debug and look at raw data from the joystick.

// normalize a value to -1 to 1, with a resting point of 0
static float normalizeRest(float data, float min, float max, float rest) {

  if (data == rest)
    return 0.f ;
  
  float value ;
  if (max>min)
    if (data>rest)
      value =  ((data-rest)/(max-rest)) ;
    else
      value =  ((data-rest)/(rest-min)) ;
  else 
    if (data>rest)
      value =  ((data-rest)/(rest-min)) ;
    else
      value =  ((data-rest)/(max-rest)) ;
  
#if 1
  if (value>1.f)
    value = 1.f ;
  else if (value<-1.f) 
    value = -1.f ;
#endif
  return value ;
}

// normalize a value from 0 to 1
static float normalize(float data, float min, float max) {
  float value ;
  value =  (data-min)/(max-min) ;
  
#if 1
  if (value>1.f)
    value = 1.f ;
  else if (value<0.f) 
    value = 0.f ;
#endif

  return value ;
}

// modify a hat value to go from raw hat data to xy coordinate data
static void hatToXY(char hat[2], char xy[2]) {

#if 1 /* this seems to work for Red Hat 7.3 */

  xy[0] = hat[0];
  xy[1] = -hat[1];

#else /* this seems to work for Red Hat 7.2 */

  if (hat[0]==-1)
    if (hat[1]==-1) {
      xy[0]=0 ; xy[1]=0 ;
    } else if (hat[1]==1) {
      xy[0]=-1 ; xy[1]=0 ;
    } else { // zero
      xy[0]=-1 ; xy[1]=1 ;
    }
  else if (hat[0]==1)
    if (hat[1]==-1) {
      xy[0]=1 ; xy[1]=0 ;
    } else if (hat[1]==1) {
      xy[0]=0 ; xy[1]=-1 ;
    } else { // zero
      xy[0]=1 ; xy[1]=-1 ;
    }
  else // zero
    if (hat[1]==-1) {
      xy[0]=1 ; xy[1]=1 ;
    } else if (hat[1]==1) {
      xy[0]=-1 ; xy[1]=-1 ;
    } else { // zero
      xy[0]=0 ; xy[1]=1 ;
    }

#endif

}

class MSSideWinder2Joystick : public dtkService {
public:
  MSSideWinder2Joystick(void) ;
  ~MSSideWinder2Joystick(void) ;
  int serve() ;
  
private:
  
  dtkSharedMem joystickShm, throttleShm, hatShm, buttonsShm ;
  
  float joystick[3], throttle ;
  unsigned char buttons ;
  char hat[2] ;
  char xy[2] ;
} ;


MSSideWinder2Joystick::MSSideWinder2Joystick(void) :
  joystickShm(sizeof(float)*3, "sideWinder2/joystick"),
  throttleShm(sizeof(float), "sideWinder2/throttle"),
  hatShm(sizeof(char)*2, "sideWinder2/hat"),
  buttonsShm(sizeof(unsigned char), "sideWinder2/buttons") {
  // Initialize local state data.
  joystick[0] = 0.0f ;
  joystick[1] = 0.0f ;
  joystick[2] = 0.0f ;
  throttle = 0.0 ;
  buttons = 0 ;
  hat[0] = -1 ;
  hat[1] = 1 ;
  xy[0] = 0 ;
  xy[1] = 0 ;

  if(joystickShm.isInvalid() ||
     throttleShm.isInvalid() ||
     hatShm.isInvalid() ||
     buttonsShm.isInvalid())
    return ; // error
  
  // open USB device.
  // auto detect the first correct USB device.
  fd = dtkUSBInput_findID(device_id) ;
  
  if(fd < 0)
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "MSSideWinder2Joystick::MSSideWinder2Joystick() failed"
	       " to find the correct USB device.\n") ;
}

MSSideWinder2Joystick::~MSSideWinder2Joystick(void) {
  if(fd >= 0) {
    close(fd) ;
    fd = -1 ;
  }
}

int MSSideWinder2Joystick::serve(void) {
  struct input_event ev[64] ;
  
  int rd = read(fd, ev, sizeof(struct input_event) * 64) ;
  
  if (rd < (int) sizeof(struct input_event)) {
    dtkMsg.add(DTKMSG_ERROR, 1,
	       "MSSideWinder2Joystick::serve() failed: "
	       "read() failed.\n") ;
    return DTKSERVICE_CONTINUE ;
  }

  for (int i = 0 ; i < (int) (rd / sizeof(struct input_event)) ; i++) {
    if(ev[i].type == EV_ABS) {
      if (ev[i].code == ABS_X) {
	int value = ev[i].value ;
#if RAW
	joystick[0] =(float) value ;
#else
	joystick[0] = normalizeRest((float) value, JOY_X_MIN, JOY_X_MAX, JOY_X_REST) ;
#endif
	if(joystickShm.write(joystick)) return DTKSERVICE_ERROR ;
      } else if (ev[i].code == ABS_Y) {
	int value = ev[i].value ;
#if RAW
	joystick[1] =(float) value ;
#else
	joystick[1] = -normalizeRest((float) value, JOY_Y_MIN, JOY_Y_MAX, JOY_Y_REST) ;
#endif
	if(joystickShm.write(joystick)) return DTKSERVICE_ERROR ;
      } else if (ev[i].code == ABS_RZ) {
	int value = ev[i].value ;
#if RAW
	joystick[2] = (float) value ;
#else
	joystick[2] = normalizeRest((float) value, JOY_Z_MIN, JOY_Z_MAX, JOY_Z_REST) ;
#endif
	if(joystickShm.write(joystick)) return DTKSERVICE_ERROR ;
      } else if (ev[i].code == ABS_THROTTLE) {
	int value = ev[i].value ;
#if RAW
	throttle = (float) value ;
#else
	throttle = normalize((float) value, THROTTLE_MIN, THROTTLE_MAX) ;
#endif
	if(throttleShm.write(&throttle)) return DTKSERVICE_ERROR ;

      } else if (ev[i].code == ABS_HAT0X) {
	hat[0] = ev[i].value ;
	hatToXY(hat,xy) ;
	if(hatShm.write(xy)) return DTKSERVICE_ERROR ;
      } else if (ev[i].code == ABS_HAT0Y) {
	hat[1] = ev[i].value ;
	hatToXY(hat,xy) ;
	if(hatShm.write(xy)) return DTKSERVICE_ERROR ;
      }


    } else if(ev[i].type == EV_KEY) {
      if (ev[i].code >= 288 && ev[i].code <= 295) {
	if(ev[i].value)
	  buttons |= 01 << (ev[i].code-288) ;
	else
	  buttons &= ~(01 << (ev[i].code-288)) ;
	if(buttonsShm.write(&buttons)) return DTKSERVICE_ERROR ;
      }
    }
  }
  
  return DTKSERVICE_CONTINUE ;
}


// The DTK C++ loader/unloader functions

static dtkService *dtkDSO_loader(const char *arg) {
  return new MSSideWinder2Joystick ;
}

static int dtkDSO_unloader(dtkService *msSideWinder2Joystick) {
  delete msSideWinder2Joystick ;
  return DTKDSO_UNLOAD_CONTINUE ; // success
}
