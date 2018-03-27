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
// dtkInButton named "buttons".  The size of the inputs will be used
// as rates for translations and rotations, i.e. it's a simple rate
// based controller which has no momentum. It intrpits the inputs as
// the rate of navigation location change and than outputs the new
// navigation location due to changes at that rate.

#define DEFAULT_SPEED 1.0f /* in ether units per second */

#define DEAD_ZONE (0.05f)

// TWIST= degrees/second
#define TWIST     (45.0f)

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

// the default mode is FLY
#ifndef FLY
#  define FLY
#endif

#ifdef HOVER
#  undef FLY
#  define NAVNAME        desktopHoverNav
#  define NAVNAMESTRING  "desktopHoverNav"
#endif

#ifdef ORIGIN
#  undef FLY
#  define NAVNAME        desktopOriginNav
#  define NAVNAMESTRING  "desktopOriginNav"
#endif

#ifdef FLY
#  define NAVNAME        desktopFlyNav
#  define NAVNAMESTRING  "desktopFlyNav"
#endif


class NAVNAME : public dtkNav
{
public:
  NAVNAME(dtkManager *);

  int postConfig(void);
  int postFrame(void);
  int activate(void);

private:

  dtkManager *mgr ;
  // dtkTime is a gettimeofday() wrapper
  dtkTime t;

  dtkInValuator *pointer;
  dtkInButton *button;

};


NAVNAME::NAVNAME(dtkManager *m) :
  dtkNav(m, NAVNAMESTRING)
{
  mgr = m ;
  setDescription("simple desktop navigation with"
		 " the mouse pointer and buttons"
#ifdef FLY
		 " that flys along the pitch plane"
#endif
#ifdef HOVER
		 " that flys in x-y plane"
#endif
		 );

  if(isInvalid()) return;

  button = NULL;
  pointer = NULL;

}


int NAVNAME::activate(void)
{
  t.reset(0.0,1);
  // Install the call-backs.
  return dtkNav::activate() ;
}


int NAVNAME::postConfig(void)
{
  if(!(pointer = (dtkInValuator *) 
       mgr->get("pointer", DTKINVALUATOR_TYPE)))
    return ERROR_;

  if(pointer->number() < 2)
    {
      dtkMsg.add(DTKMSG_ERROR, NAVNAMESTRING"::postConfig()"
                 " dtkInValuator named \"pointer\" with"
                 " two or more values was not found.\n");
      return ERROR_;
    }

  button =  (dtkInButton *) mgr->get("buttons", DTKINBUTTON_TYPE);
  if(!button) return ERROR_;
  if (button->number() < 3)
    {
      dtkMsg.add(DTKMSG_ERROR, NAVNAMESTRING"::postConfig():"
		 " the numbers of buttons in the dtkInButton "
		 "device is less than 3.\n");
      return ERROR_;
    }

  _response = DEFAULT_SPEED;

  t.reset(0.0,1);

#ifdef ORIGIN
	_pivot = new dtkCoord(0.f, 0.0f, 0.f, 0.f, 0.f, 0.f);
#endif

  return dtkNav::postConfig();
}


int NAVNAME::postFrame(void)
{
  float delta_t = (float) t.delta();
  
  // button->read(int ) is a polling read on the buttons
 
  if(button->read(0) || button->read(1) || button->read(2))
    {
      float pointer_xy[2];
      // poll current mouse pointer value
      pointer->read(pointer_xy);
      
      // Change the translation
      if (button->read(0) && fabs(pointer_xy[1])>DEAD_ZONE)
	{
	  float dy = (pointer_xy[1] > 0.0f) ? 
	    ((pointer_xy[1] - DEAD_ZONE) / (1.0f - DEAD_ZONE)):
	    ((pointer_xy[1] + DEAD_ZONE) / (1.0f - DEAD_ZONE));
	  dy *= dy*dy ;
	  
#ifdef HOVER
	  // move in xy plane of world
	  dtkMatrix locMat;

	  locMat.translate(0.f, -delta_t*_response*dy, 0.f) ;
	  locMat.rotateHPR(-location.h, 0.f, 0.f) ; 
	  locMat.rotateHPR(location.h, location.p, location.r) ; 

	  dtkCoord locCoord ;
	  locMat.coord(&locCoord) ;
	  location.y += locCoord.y ;
	  location.z += locCoord.z ;
#endif
#if defined(FLY) || defined(ORIGIN)
	  // move in y of dpf coords
	  location.y +=  -delta_t*_response*dy ;
#endif	 
	}
      
      // Change heading rotation about navigated axis
      if ((button->read(0) || button->read(2)) && 
	  fabs(pointer_xy[0])>DEAD_ZONE)
	{
	  float dy = (pointer_xy[0] > 0.0f) ?
	    ((pointer_xy[0] - DEAD_ZONE) / (1.0f - DEAD_ZONE)):
	    ((pointer_xy[0] + DEAD_ZONE) / (1.0f - DEAD_ZONE));
	  dy *= dy*dy ;

	  dtkMatrix locMat ;

	  // make nav locator into a matrix
	  locMat.coord(location);

	  dtkMatrix rotMat, rotMatInv ;

#ifdef ORIGIN

	if( button->read(2) )
	{
	  rotMat.rotateHPR(delta_t*TWIST*dy, 0.f, 0.f) ;

	  // move to pivot position
	  locMat.translate(-location.x,
			   -location.y,
			   -location.z);

	  // rotate nav locator 
	  locMat.mult(&rotMat) ;

	  // move back from pivot position
	  locMat.translate(location.x,
			   location.y,
			   location.z);
	}
	else
	{
	  rotMatInv.rotateHPR(location.h, location.p, location.r) ;
	  rotMat =  rotMatInv ;
	  rotMat.invert() ;
	  
	  rotMat.rotateHPR(delta_t*TWIST*dy, 0.f, 0.f) ;
	  rotMat.mult(&rotMatInv) ;

	  // move to pivot position
	  locMat.translate(-(*_pivot).x,
			   -(*_pivot).y,
			   -(*_pivot).z);

	  // rotate nav locator 
	  locMat.mult(&rotMat) ;

	  // move back from pivot position
	  locMat.translate((*_pivot).x,
			   (*_pivot).y,
			   (*_pivot).z);
	}
#else
	  rotMatInv.rotateHPR(location.h, location.p, location.r) ;
	  rotMat =  rotMatInv ;
	  rotMat.invert() ;
	  
	  rotMat.rotateHPR(delta_t*TWIST*dy, 0.f, 0.f) ;
	  rotMat.mult(&rotMatInv) ;

	  // move to pivot position
	  locMat.translate(-(*_pivot).x,
			   -(*_pivot).y,
			   -(*_pivot).z);

	  // rotate nav locator 
	  locMat.mult(&rotMat) ;

	  // move back from pivot position
	  locMat.translate((*_pivot).x,
			   (*_pivot).y,
			   (*_pivot).z);
#endif

	  // make nav locator back to a dtkCoord
	  locMat.coord(&location);
	  
	}

      // Change pitch rotation about dpf axis
      if (button->read(2) && fabs(pointer_xy[1])>DEAD_ZONE)
	{
	  float dy = (pointer_xy[1] > 0.0f) ?
	    ((pointer_xy[1] - DEAD_ZONE) / (1.0f - DEAD_ZONE)):
	    ((pointer_xy[1] + DEAD_ZONE) / (1.0f - DEAD_ZONE));
	  dy *= dy*dy ;

	  dtkMatrix rotMat ;

	  rotMat.rotateHPR(0.f, delta_t*TWIST*dy, 0.f) ;

	  dtkMatrix locMat ;
	  // make nav locator into a matrix
	  locMat.coord(location);
	  
#ifdef ORIGIN
	  // move to pivot position
	  locMat.translate(-location.x,
			   -location.y,
			   -location.z);

	  // rotate nav locator 
	  locMat.mult(&rotMat) ;

	  // move back from pivot position
	  locMat.translate(location.x,
			   location.y,
			   location.z);
#else
	  // move to pivot position
	  locMat.translate(-(*_pivot).x,
			   -(*_pivot).y,
			   -(*_pivot).z);
	  
	  // rotate nav locator 
	  locMat.mult(&rotMat) ;
	  
	  // move back from pivot position
	  locMat.translate((*_pivot).x,
			   (*_pivot).y,
			   (*_pivot).z);
#endif
	  
	  // make nav locator back to a dtkCoord
	  locMat.coord(&location);
	  
	}
      
      // Change the translation up and down in dpf coordinates
      if (button->read(1) && fabs(pointer_xy[1])>DEAD_ZONE)
	{
	  float dy = (pointer_xy[1] > 0.0f) ? 
	    ((pointer_xy[1] - DEAD_ZONE) / (1.0f - DEAD_ZONE)):
	    ((pointer_xy[1] + DEAD_ZONE) / (1.0f - DEAD_ZONE));
	  dy *= dy*dy;
	  location.z +=  -delta_t*_response*dy ;
	}
      // Change the translation left and right in dpf coordinates
      if (button->read(1) && fabs(pointer_xy[0])>DEAD_ZONE)
	{
	  float dy = (pointer_xy[0] > 0.0f) ? 
	    ((pointer_xy[0] - DEAD_ZONE) / (1.0f - DEAD_ZONE)):
	    ((pointer_xy[0] + DEAD_ZONE) / (1.0f - DEAD_ZONE));
	  dy *= dy*dy;
	  
	  location.x +=  -delta_t*_response*dy ;
	}
      
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
  return new NAVNAME(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


