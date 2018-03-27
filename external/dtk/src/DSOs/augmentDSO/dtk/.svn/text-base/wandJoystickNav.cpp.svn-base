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
 * modifications and additions have been made by John Kelso.
 */

/* real-time navagation based on clock time, not frame rate */

// Takes control inputs from a dtkInValuator, and a dtkInLocator.

#define TRANS_DEAD_ZONE (0.05f)
#define ROT_DEAD_ZONE (0.1f)

// TWIST = degrees per second
#define TWIST     (45.0f)

#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

#undef NAVNAME

// by default navigation rotates around the Z axis.  different versions are
// created for rotating about different axes

#ifdef PITCH
#  undef HEADING
#  undef ROLL
#  undef HPR
#  undef XY
#  undef ORIGIN
#  define NAVNAME        wandJoystickPitchNav
#  define NAVNAMESTRING  "wandJoystickPitchNav"
#endif

#ifdef ROLL
#  undef HEADING
#  undef PITCH
#  undef HPR
#  undef XY
#  undef ORIGIN
#  define NAVNAME        wandJoystickRollNav
#  define NAVNAMESTRING  "wandJoystickRollNav"
#endif

#ifdef HPR
#  undef HEADING
#  undef PITCH
#  undef ROLL
#  undef XY
#  undef ORIGIN
#  define NAVNAME        wandJoystickHPRNav
#  define NAVNAMESTRING  "wandJoystickHPRNav"
#endif

#ifdef XY
#  undef HEADING
#  undef PITCH
#  undef ROLL
#  undef HPR
#  undef ORIGIN
#  define NAVNAME        wandJoystickXYNav
#  define NAVNAMESTRING  "wandJoystickXYNav"
#endif

#ifdef ORIGIN
#  undef HEADING
#  undef PITCH
#  undef ROLL
#  undef HPR
#  undef XY
#  define NAVNAME        wandJoystickOriginNav
#  define NAVNAMESTRING  "wandJoystickOriginNav"
#endif

// the default rotation is HEADING
#ifndef NAVNAME 
#  ifndef HEADING
#    define HEADING
#  endif
#  undef PITCH
#  undef ROLL
#  undef HPR
#  undef XY
#  define NAVNAME        wandJoystickNav
#  define NAVNAMESTRING  "wandJoystickNav"
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
  dtkInLocator *wand ;
  dtkInValuator *joystick ;
  dtkCoord wand_xyzhpr ;
  dtkCoord pivot_xyzhpr ;
};


NAVNAME::NAVNAME(dtkManager *m) :
  dtkNav(m, NAVNAMESTRING)
{
  mgr = m ;
  setDescription("simple wand and joystick navigation "
#ifdef HEADING
		 "with rotation about the Z axis"
#endif
#ifdef PITCH
		 "with rotation about the X axis"
#endif
#ifdef ROLL
		 "with rotation about the Y axis"
#endif
#ifdef HPR
		 "with rotation about the wand's Y axis"
#endif
#ifdef XY
		 "with rotation about the wand's Z axis, but no translation in Z"
#endif
#ifdef ORIGIN
		 "with rotation about the origin"
#endif
		 );

}

int NAVNAME::postConfig(void)
{
  wand = (dtkInLocator *) mgr->get("wand", DTKINLOCATOR_TYPE);
  joystick = (dtkInValuator *) mgr->get("joystick", DTKINVALUATOR_TYPE);

  if(!wand || !joystick)
    return ERROR_;
  if(joystick->number() != 2)
    {
      dtkMsg.add(DTKMSG_ERROR,
	     "The dtkInValuator named \"joystick\" has %d values"
	     " which is not 2 values.",joystick->number());
      return ERROR_;
    }

  // init pivot to be wand position
  float pos[6];
  wand->read(pos);
  defPivot->set(pos) ;
  t.reset(0.0,1);
  return dtkNav::postConfig();
}


int NAVNAME::activate(void)
{
  t.reset(0.0,1);
  // Install the call-backs.
  return dtkNav::activate() ;
}

int NAVNAME::postFrame(void) {

  float delta_t = (float) t.delta();
  float joy_xy[2];

  // poll current wand and joystick value
  joystick->read(joy_xy);

  float pos[6];
  wand->read(pos);
  wand_xyzhpr.set(pos) ;

  if(fabs(joy_xy[0]) > TRANS_DEAD_ZONE ||
     fabs(joy_xy[1]) > ROT_DEAD_ZONE)
    {

      // if using the default pivot point, use the wand dtkCoord, otherwise
      // use the pivot's dtkCoord.  if using the default pivot, set it too-
      // see below
#if 0
      printf("_pivot = %p  defPivot = %p\n",_pivot,defPivot) ;
      printf("_pivot = ") ;
      if (_pivot) _pivot->print() ;
      printf("defPivot = ") ;
      if (defPivot) defPivot->print() ;
#endif
#ifndef ORIGIN
      if (_pivot == defPivot) {
	// using the wand as pivot point
	pivot_xyzhpr = wand_xyzhpr ;
      } else {
	// using non-default pivot as the pivot point - don't change it!
	pivot_xyzhpr = *_pivot ;
	// but use the wand hpr for the rotation- we get to decide how to
	// use it.
	pivot_xyzhpr.h = wand_xyzhpr.h ;
	pivot_xyzhpr.p = wand_xyzhpr.p ;
	pivot_xyzhpr.r = wand_xyzhpr.r ;
      }
#else
      pivot_xyzhpr = dtkCoord( location );
#endif

      // UPDATE THE DEFAULT PIVOT POINT WHETHER IT'S BEING USED OR NOT
      *defPivot = wand_xyzhpr ;


      //////////////////////////////////////////
      // Change heading rotation about the pivot
      // rotation depends on pivot point
      //////////////////////////////////////////
      if (fabs(joy_xy[0])>ROT_DEAD_ZONE)
	{
	  float dy = (joy_xy[0] > 0.0f) ?
	    ((joy_xy[0] - ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE)):
	    ((joy_xy[0] + ROT_DEAD_ZONE) / (1.0f - ROT_DEAD_ZONE));
	  dy *= dy*dy;

	  // set an intial heading rotation and then rotate it again about
	  // correct axis
	  dtkMatrix rotMat ;

	  // assume that the HEADING, PITCH and ROLL navigations are
	  // user-centric- i.e., users want to feel as if they're
	  // moving through the virtual world.  For HPR, assume that
	  // the navigation is exo-centric- i.e., the object is
	  // manipulated, instead of users moving through the world.
#if defined (HEADING) || defined (XY)
	  rotMat.rotateHPR(delta_t*TWIST*dy, 0.f, 0.f) ;
#endif
#ifdef PITCH
	  rotMat.rotateHPR(0.f, delta_t*TWIST*dy, 0.f) ;
#endif
#ifdef ROLL
	  rotMat.rotateHPR(0.f, 0.f, delta_t*TWIST*dy) ;
#endif
#ifdef HPR
	  // hpr mode sets rotation axis based on wand hpr
	  dtkMatrix hprMat, hprMatInv ;

	  hprMat.rotateHPR(wand_xyzhpr.h,
			   wand_xyzhpr.p,
			   wand_xyzhpr.r) ;
	  hprMatInv =  hprMat ;
	  hprMatInv.invert() ;

	  rotMat = hprMatInv ;
	  rotMat.rotateHPR(0.f, 0.f, delta_t*TWIST*dy) ;
	  rotMat.mult(&hprMat) ;
#endif
#ifdef ORIGIN
	  // origin mode sets rotation axis about origin
	  dtkMatrix hprMat, hprMatInv ;

	  hprMat.rotateHPR(wand_xyzhpr.h,
			   wand_xyzhpr.p,
			   wand_xyzhpr.r) ;
	  hprMatInv =  hprMat ;
	  hprMatInv.invert() ;

	  rotMat = hprMatInv ;
	  rotMat.rotateHPR( 0.f, 0.f, delta_t*TWIST*dy ) ;
	  rotMat.mult(&hprMat) ;
#endif
	  dtkMatrix locMat;
	  // make nav locator into a matrix
	  locMat.coord(location);

	  // move to pivot position
	  locMat.translate(-pivot_xyzhpr.x,
			   -pivot_xyzhpr.y,
			   -pivot_xyzhpr.z);
	  
	  // rotate nav locator 
	  locMat.mult(&rotMat) ;

	  // move back from pivot position
	  locMat.translate(pivot_xyzhpr.x,
			   pivot_xyzhpr.y,
			   pivot_xyzhpr.z);
	  
	  // make nav locator back to a dtkCoord
	  locMat.coord(&location);
	}

      /////////////////////////////////////////
      // Change the translation.
      // translation is based on wand hpr, regardless of pivot point
      /////////////////////////////////////////
      if (fabs(joy_xy[1])>TRANS_DEAD_ZONE)
	{

	  float dy = (joy_xy[1] > 0.0f) ? 
	    ((joy_xy[1] - TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE)):
	    ((joy_xy[1] + TRANS_DEAD_ZONE) / (1.0f - TRANS_DEAD_ZONE));
	  dy *= dy*dy;
	  
	  dtkMatrix xlateMat;
	  // get the translation distance along y
	  xlateMat.translate(0.f, -delta_t*_response*dy, 0.f) ;

	  // make the translation go in the direction of the wand
	  xlateMat.rotateHPR(pos[3],
			     pos[4],
			     pos[5]);


	  // add the translation into the location
	  float x, y, z ;
	  xlateMat.translate(&x, &y, &z) ;
	  location.x += x ;
	  location.y += y ;
#ifndef XY
	  location.z += z ;
#endif
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
