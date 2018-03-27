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
/* This file was originally written by John Kelso.  Some
 * modifications have been made by Lance Arsenault.
 */

#include <math.h>

#include <dtk.h>
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_IRIX
# include <values.h>
#endif

#ifdef DTK_ARCH_WIN32_VCPP
# define M_PI  (3.1415926f)
#endif


#include <dtk/dtkDSO_loader.h>

#define TWIST (40.0f)
#define SPEED (0.25f)

//these work on Virginia Tech's Immersive workbench.  You may need to
//reassign them.

#define TRIGGER 4
#define LEFT 2
#define RIGHT 1

int direction = -1 ;

//#define DEBUG

class WandButtonNav : public dtkNav
{
public:
  WandButtonNav(dtkManager *);
  ~WandButtonNav(void);
  int postConfig(void);
  int postFrame(void) ;
  int activate(void);

private:
  dtkManager *mgr ;
  dtkDequeuer *dequeuer ;
  dtkInLocator *_wand ;
  dtkInButton *_buttons ;

  dtkCoord _resetLocation;
  dtkTime t;
  dtkCoord wand_xyzhpr ;
  dtkCoord pivot_xyzhpr ;
} ;

WandButtonNav::WandButtonNav(dtkManager *m) :
  dtkNav(m, "wandButtonNav")
{
  mgr = m ;
  setDescription("dtk Navigation using wand and buttons");

  _buttons = NULL ;
  _wand = NULL ;

}

WandButtonNav::~WandButtonNav(void)
{
  if(dequeuer)
    {
      delete dequeuer;
      dequeuer = NULL;
    }

}


int WandButtonNav::postConfig(void)
{
  dequeuer = new dtkDequeuer(mgr->record());
  if(!dequeuer || dequeuer->isInvalid()) {
    dtkMsg.add(DTKMSG_ERROR,
	       "WandButtonNav::postConfig(): couldn't get dequeuer\n") ;
    invalidate() ;
    return ERROR_;
  }

  _wand = (dtkInLocator *) mgr->get("wand", DTKINLOCATOR_TYPE) ;
  _buttons =  (dtkInButton *) mgr->get("buttons", DTKINBUTTON_TYPE) ;
  if (!_wand || !_buttons || _buttons->number() < 3) {
    dtkMsg.add(DTKMSG_ERROR,"WandButtonNav::postConfig(): "
	       "couldn't get valid wand or button\n") ;
    invalidate() ;
    return ERROR_ ;
  }
  _buttons->queue() ;

  _resetLocation = location ;

  t.reset(0.0,1);

  return dtkNav::postConfig();
}


// for navigation update
int WandButtonNav::postFrame(void)
{  
  dtkRecord_event *event_rec;

  static enum {NONE, ROT, TRANS} transType ;
  float delta_t = (float) t.delta() ;

  while((event_rec = dequeuer->getNextEvent(_buttons)) > 0)
    {
      u_int32_t button_state = _buttons->read(event_rec);
      
#ifdef DEBUG
      printf("%s %d: button_state = %d\n",  __FILE__, __LINE__, button_state) ;
      if (button_state & (TRIGGER|LEFT|RIGHT))
	  printf("button\n") ;
#endif

      if ((button_state & (TRIGGER|LEFT|RIGHT)) == (TRIGGER|LEFT|RIGHT)) {
	location = _resetLocation ;
	transType = NONE ;
      } else if ((button_state & (TRIGGER|LEFT)) == (TRIGGER|LEFT)) {
	transType = ROT ;
	direction = 1 ;
      } else if ((button_state & (TRIGGER|RIGHT)) == (TRIGGER|RIGHT)) {
	transType = TRANS ;
	direction = 1 ;
      } else if ((button_state & (LEFT)) == (LEFT)) {
	transType = ROT ;
	direction = -1 ;
      } else if ((button_state & (RIGHT)) == (RIGHT)) {
	transType = TRANS ;
	direction = -1 ;
      } else {
	transType = NONE ;
      }
    }
  
  if (transType == ROT) {
    
#ifdef DEBUG
    printf ("-----ROT\n") ;
#endif
    
    float pos[6];
    _wand->read(pos);
    wand_xyzhpr.set(pos) ;
    
    // grab heading and convert it to a speed control
    float s = -sinf(M_PI*wand_xyzhpr.h/180.f) * delta_t * TWIST * direction;
    
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
    // UPDATE THE DEFAULT PIVOT POINT WHETHER IT'S BEING USED OR NOT
    *defPivot = wand_xyzhpr ;
    
    //////////////////////////////////////////
    // Change heading rotation about the pivot
    // rotation depends on pivot point
    //////////////////////////////////////////
    
    // set an intial heading rotation and then rotate it again about
    // correct axis
    dtkMatrix rotMat ;
    rotMat.rotateHPR(s, 0.f, 0.f) ;
    
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
    
  } else if (transType == TRANS) {
    
#ifdef DEBUG
    printf ("-----TRANS\n") ;
#endif
    
    /////////////////////////////////////////
    // Change the translation.
    // translation is based on wand hpr, regardless of pivot point
    /////////////////////////////////////////

    // grab heading and make it the translation direction
    float pos[6];
    _wand->read(pos);
    
    dtkMatrix xlateMat;
    // get the translation distance along y
    xlateMat.translate(0.f,  direction*delta_t*_response*SPEED, 0.f) ;
    
    // make the translation go in the direction of the wand
    xlateMat.rotateHPR(pos[3],
		       pos[4],
		       pos[5]);
    
    
    // add the translation into the location
    float x, y, z ;
    xlateMat.translate(&x, &y, &z) ;
    location.x += x ;
    location.y += y ;
    location.z += z ;

#if 0
  } else {
    printf ("-----NONE\n") ;
#endif

  }

  return dtkNav::postFrame();
}

int WandButtonNav::activate(void)
{
  // reset the clock.
  t.reset(0.0,1);
  return dtkNav::activate() ;
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
  return new WandButtonNav(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


