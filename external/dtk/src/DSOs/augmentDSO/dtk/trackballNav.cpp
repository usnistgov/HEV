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
/* This file was originally written by Lance Arsenault.  Many
 * modifications have been made by John Kelso.
 */

/* real-time navagation based on clock time, not frame rate */

// Takes control inputs from a dtkInValuator named "pointer" and a
// dtkInButton named "buttons" with at least 3 buttons.  It makes a
// navigation that is simular to the performer track ball as in the
// program perfly.

// By default the trackball spins around the world origin.  
// this uses a nondefault pivot if set, but doesn't change the default
// pivot- it's always at the world origin

#define DEFAULT_SPEED 1.0f /* in ether units per second */

#define TWIST (90.0f)

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>


class trackballNav : public dtkNav
{
public:
  trackballNav(dtkManager *);
  ~trackballNav(void);

  int postConfig(void);
  int postFrame(void);
  int activate(void);

private:

  dtkManager *mgr ;

  // dtkTime is a gettimeofday() wrapper
  dtkTime t;

  dtkInValuator *pointer;
  dtkInButton *button;

  dtkDequeuer *dequeuer;

  u_int32_t last_button_state;

  float last_pointer_xy[2];
};


trackballNav::trackballNav(dtkManager *m) :
  dtkNav(m, "trackballNav")
{
  mgr = m ;

  setDescription("simple desktop trackball navigation");

  if(isInvalid()) return;

  dequeuer = NULL;
  button = NULL;
  pointer = NULL;
  last_button_state = 0;

  dequeuer = new dtkDequeuer(mgr->record());
  if(!dequeuer || dequeuer->isInvalid())
    return;

  t.reset(0.0,1);

  // dtkNav::dtkNav() will get validated.  So there is no need to
  // call validate() here.
}

trackballNav::~trackballNav(void)
{
  if(dequeuer) {
    delete dequeuer;
    dequeuer = NULL;
  }
  
}

int trackballNav::activate(void)
{
  // flush the queue
  while(dequeuer->getNextEvent());
  // reset the clock.
  t.reset(0.0,1);
  // Install the call-backs.
  return dtkNav::activate() ;
}


int trackballNav::postConfig(void)
{
  dtkDisplay *display = (dtkDisplay *) mgr->getNext(DTKDISPLAY_TYPE) ;
  if(!display) return ERROR_;

  if(!(pointer = (dtkInValuator *) 
       mgr->get("pointer", DTKINVALUATOR_TYPE)))
    return ERROR_;

  if(pointer->number() < 2)
    {
      dtkMsg.add(DTKMSG_ERROR, "trackballNav::postConfig()"
                 " dtkInValuator named \"pointer\" with"
                 " two or more values was not found.\n");
      return ERROR_;
    }

  button =  (dtkInButton *) mgr->get("buttons", DTKINBUTTON_TYPE);
  if(!button) return ERROR_;
  if (button->number() < 3)
    {
      dtkMsg.add(DTKMSG_ERROR, "trackballNav::postConfig():"
		 " the numbers of buttons in the dtkInButton "
		 "device is less than 3.\n");
      return ERROR_;
    }
  // make the button be queued
  button->queue();

  _response = DEFAULT_SPEED;

  t.reset(0.0,1);

  return dtkNav::postConfig();
}

int trackballNav::postFrame(void)
{
  float delta_t = (float) t.delta();
  u_int32_t button_state = button->read();
  float pointer_xy[2];
  pointer->read(pointer_xy);

  
  // BUTTON 0 and not 1 and not 2
  // translation along the screen with the pointer
  // only is using default Velocity
  if(button_state & 01 && last_button_state & 01 &&
     !(button_state & 06 || last_button_state & 06) &&
     (_velocity == defVelocity))
    {
      _velocity->x = (pointer_xy[0] - last_pointer_xy[0])/delta_t;
      _velocity->z = (pointer_xy[1] - last_pointer_xy[1])/delta_t;
    }
  
  // BUTTON 1 and not 0 (2 or not 2)
  else if(button_state & 02 && last_button_state & 02 &&
	  !(button_state & 01 || last_button_state & 01) &&
	  (_velocity == defVelocity))
    {
      // BUTTON 1 and not 2
      // pointer x = heading   pointer y = pitch
      if(!(button_state & 04 && last_button_state & 04))
	{
	  _velocity->h = 
	    (pointer_xy[0] - last_pointer_xy[0])*TWIST/delta_t;
	  
	  _velocity->p = 
	    -(pointer_xy[1] - last_pointer_xy[1])*TWIST/delta_t;
	  
	  _velocity->r = 0.0f;
	}
      // BUTTON 1 and 2
      // max of pointer x and y = roll
      else if(button_state & 04 && last_button_state & 04)
	{
	    _velocity->r = 
	    (pointer_xy[1] - last_pointer_xy[1])*TWIST/delta_t;

	  _velocity->h = 0.0f;
	  _velocity->p = 0.0f;
	}
    }

  // BUTTON 2 and not (0 and 1)
  // in and out of screen translation = pointer y
  // into screen = pointer y+
  else if(button_state & 04 && last_button_state & 04 &&
	  !(button_state & 03 || last_button_state & 03) &&
	  (_velocity == defVelocity))
    {
      _velocity->y = (pointer_xy[1] - last_pointer_xy[1])/delta_t;
    }

  struct dtkRecord_event *event;

  while((event = dequeuer->getNextEvent(button)))
    {
      // Don't miss button events that stop one motion and start
      // another kind of motion.  This is one thing that performer
      // pfiTDFXformer can't do, because it does read queued X events,
      // it just polls.
      int buttonNum;
      
      // if pressed button 0 or 1 or 2     STOP ALL
      if((button->read(&buttonNum, event)))
	if((buttonNum == 0 || buttonNum == 1 || buttonNum == 2) &&
	   (_velocity == defVelocity))
	  _velocity->zero() ;
    }
  
  // now that velocities are set, navigate using updated values

  // _velocity->r uses two buttons, so I override the single button
  // rates here.
  if((_velocity->r != 0.0f) &&
     (_velocity == defVelocity)) 
    {
      _velocity->h = 0.0f;
      _velocity->p = 0.0f;
      _velocity->y = 0.0f;
    }
  
  if((_velocity == defVelocity) && 
     (_velocity->h != 0.0f || _velocity->p != 0.0f || 
      _velocity->r != 0.0f))
    {
      // general rotation

      // set an intial heading rotation and then rotate it again
      dtkMatrix hprMat, hprMatInv ;
      
      hprMat.rotateHPR(_pivot->h,
		       _pivot->p,
		       _pivot->r) ;
      hprMatInv = hprMat ;
      hprMatInv.invert() ;
      
      dtkMatrix rotMat = hprMat ;

      if(_velocity->r != 0.0f)
	// rotate about Y
	rotMat.rotateHPR(0.f, 0.f, _velocity->r*delta_t) ;
      else // _velocity->h != 0.0f || _velocity->p != 0.0f
	{
	  // rotate about X
	  rotMat.rotateHPR(0.f, _velocity->p*delta_t, 0.f) ;
	  // rotate about Z
	  rotMat.rotateHPR(_velocity->h*delta_t, 0.f, 0.f) ;
	}
      
      rotMat.mult(&hprMatInv) ;
      
      dtkMatrix locMat;
      // make nav locator into a matrix
      locMat.coord(location);
      
      // move to pivot position
      locMat.translate(-_pivot->x,
		       -_pivot->y,
		       -_pivot->z);
      
      // rotate nav locator 
      locMat.mult(&rotMat) ;
      
      // move back from pivot position
      locMat.translate(_pivot->x,
		       _pivot->y,
		       _pivot->z);
      
      // make nav locator back to a dtkCoord
      locMat.coord(&location);

#if 0
      printf("l=(%f,%f,%f)(%f,%f,%f) v=(%f,%f,%f)(%f,%f,%f)\n",
	     location.x, location.y, location.z, 
	     location.h, location.p, location.r,
	     _velocity->x, _velocity->y, _velocity->z, 
	     _velocity->h, _velocity->p, _velocity->r) ;
#endif      
    } else {
      //printf("%s %d: one of h, p, r != 0\n", __FILE__, __LINE__) ;
    }
  
  
  // general translation
  if(_velocity->x != 0.0f || _velocity->y != 0.0f || 
     _velocity->z != 0.0f )
    {

      location.x += _velocity->x*_response*delta_t ;
      location.y += _velocity->y*_response*delta_t ;
      location.z += _velocity->z*_response*delta_t ;
    }
  
  
  // save the state of the buttons
  last_button_state = button_state;
  last_pointer_xy[0] = pointer_xy[0];
  last_pointer_xy[1] = pointer_xy[1];
  
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
  return new trackballNav(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


