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
/* This file was written by John Kelso or was it someone else.
 */

/* all it does is translate you in X and y based on joystick X and Y */

#define TRANS_DEAD_ZONE (0.05f)

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

class joystickXYtransNav : public dtkNav
{
public:
  joystickXYtransNav(dtkManager *);

  int postConfig(void);
  int postFrame(void);
  int activate(void);

private:
  
  dtkManager *mgr ;
  // dtkTime is a gettimeofday() wrapper
  dtkTime t;
  dtkInValuator *joystick ;
};

joystickXYtransNav::joystickXYtransNav(dtkManager *m) :
  dtkNav(m, "joystickXYtransNav")
{
  mgr = m ;
  setDescription("simple joystick translational navigation") ;
}

int joystickXYtransNav::postConfig(void)
{
  joystick = (dtkInValuator *) mgr->get("joystick", DTKINVALUATOR_TYPE);

  if(!joystick)
    return ERROR_;
  if(joystick->number() != 2)
    {
      dtkMsg.add(DTKMSG_ERROR,
	     "The dtkInValuator named \"joystick\" has %d values"
	     " which is not 2 values.",joystick->number());
      return ERROR_;
    }

  // init pivot to be wand position
  t.reset(0.0,1);
  return dtkNav::postConfig();
}

int joystickXYtransNav::activate(void)
{
  t.reset(0.0,1);
  // Install the call-backs.
  return dtkNav::activate() ;
}

int joystickXYtransNav::postFrame(void) {
  
  float delta_t = (float) t.delta();
  float joy_xy[2];
  
  // poll current joystick value
  joystick->read(joy_xy);
  
  /////////////////////////////////////////
  // Change the translation.
  /////////////////////////////////////////
  if (fabs(joy_xy[0])>TRANS_DEAD_ZONE ||
      fabs(joy_xy[1])>TRANS_DEAD_ZONE) 
    {
      
      float dx = (joy_xy[0] > 0.0f) ? 
	((joy_xy[0] - TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE)):
	((joy_xy[0] + TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE));
      dx *= -dx*dx*delta_t;
      
      float dy = (joy_xy[1] > 0.0f) ? 
	((joy_xy[1] - TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE)):
	((joy_xy[1] + TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE));
      dy *= -dy*dy*delta_t;
      
      location.x += dx ;
      location.y += dy ;
    }
  return dtkNav::postFrame() ;
}
  
  
 /************ DTK C++ dtkAugment loader/unloader functions ***************
 *
 * All DTK dtkAugment DSO files are required to declare these two
 * functions.  These function are called by the loading program to get
 * your C++ objects loaded.
 *
 *************************************************************************/

static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
  return new joystickXYtransNav(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
