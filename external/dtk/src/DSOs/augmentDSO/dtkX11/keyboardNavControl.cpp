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
/* This file was written by John Kelso.
 */
// Controls the current navigation speed and reset using key presses
// '+', '-', and '0'.  Read on to see now.


#define DEFAULT_SPEED 1.0f
#define SPEED_FACTOR 2.0f

// the max speed is the speed of light if the model units are meters.
#define MAX_SPEED 3.0e+8f
#define MIN_SPEED 3.0e-8f

#include <dtk.h>
#include <dtkX11.h>
#include <dtk/dtkDSO_loader.h>


class keyboardNavControl : public dtkAugment
{
public:
  keyboardNavControl(dtkManager *);
  ~keyboardNavControl(void);

  int preFrame(void);
  int postFrame(void);

private:

  dtkInXKeyboard *keyboard;
  KeyCode xkc_plus;
  KeyCode xkc_minus;
  KeyCode xkc_zero;
  KeyCode xkc_nine;
  KeyCode xkc_eight;
  KeyCode xkc_p;

  dtkManager *mgr ;
  dtkNavList *navList;
  dtkDequeuer *dequeuer;

  dtkCoord reset_location;
  float reset_scale;
  float reset_speed;  
};


keyboardNavControl::keyboardNavControl(dtkManager *m) :
  dtkAugment("keyboardNavControl")
{
  setDescription("adds speed and reset navigation control"
		 " to the current dtkNav navigation:"
		 " '+' or '-' = speed up or down, '0' = reset location,"
		 " '+' and '-' = reset speed, '9' = zero pitch and roll");
  mgr = m ;
  dequeuer = NULL;
  navList = NULL;
  keyboard = NULL;
  
  // dtkAugment::dtkAugment() will not validate the object
  validate() ;
}

keyboardNavControl::~keyboardNavControl(void)
{
  if(dequeuer)
    {
      delete dequeuer;
      dequeuer = NULL;
    }
}


// This tries to find a reset location and scale
int keyboardNavControl::preFrame(void)
{
  dequeuer = new dtkDequeuer(mgr->record());
  if(!dequeuer || dequeuer->isInvalid()) return ERROR_;

  navList = (dtkNavList *) mgr->check("dtkNavList", DTKNAVLIST_TYPE) ;
  if(!navList)
    {
      navList = new dtkNavList;
      if(mgr->add(navList))
	{
	  delete navList;
	  navList = NULL;
	  return ERROR_;
	}
    }

  // get the first xkeyboard
  keyboard = (dtkInXKeyboard *) mgr->getNext(DTKINXKEYBOARD_TYPE);
  if(!keyboard) return ERROR_;

  //////////////// get X key codes from key syms ///////////////////////

  if(!(xkc_plus = keyboard->getKeyCode(XK_plus)) ||
     !(xkc_minus = keyboard->getKeyCode(XK_minus)) ||
     !(xkc_zero  = keyboard->getKeyCode(XK_0)) ||
     !(xkc_eight = keyboard->getKeyCode(XK_8)) ||
     !(xkc_nine = keyboard->getKeyCode(XK_9)) ||
     !(xkc_p = keyboard->getKeyCode(XK_P))
     )
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "keyboardNavControl::preFrame() no Keycap to "
		 "match one of the X Key Syms in file %s",
		 __FILE__);
      return ERROR_;
    }
  keyboard->queue();

  if(navList->current())
    {
      reset_location = navList->current()->location ;
      reset_scale = navList->current()->scale;
      reset_speed = navList->current()->response();
    }
  else
    {
      //reset_location.zero() ; // constructor zeros it
      reset_scale = 1.0f;
      reset_speed = DEFAULT_SPEED;
    }

  return REMOVE_CALLBACK;
}

int keyboardNavControl::postFrame(void)
{
  dtkRecord_event *e;
  while((e = dequeuer->getNextEvent(keyboard)))
    {
      dtkNav *currentNav = navList->current();
      if(currentNav)
	{
	  KeyCode key;
	  // if pressed
	  if((keyboard->read(&key,e)))
	    {
	      // speed up or slow down
	      if(key == xkc_plus || key == xkc_minus)
		{
		  float speed = currentNav->response();
		  if(key == xkc_minus) // slow down
		    {
		      if(speed > MIN_SPEED)
			{
			  speed /= SPEED_FACTOR;
			  if(speed > MIN_SPEED)
			    currentNav->response(speed);
			}
		    }
		  else // key == xkc_minus
		    {
		      if(speed < MAX_SPEED)
			{
			  speed *= SPEED_FACTOR;
			  if(speed < MAX_SPEED)
			    currentNav->response(speed);
			}
		    }
		  dtkMsg.add(DTKMSG_INFO, "keyboardNavControl::postFrame(): "
			     "set response (speed) in dtkNav named "
			     "\"%s\" to %f\n",
			     currentNav->dtkAugment::getName(),
			     currentNav->response());
		}
	    }
	  else if(key == xkc_zero) // location reset
	    {
	      currentNav->location = reset_location ;
	      currentNav->scale = reset_scale;
	      dtkMsg.add(DTKMSG_INFO, "keyboardNavControl::postFrame(): "
			 "reset to initial location in "
			 "dtkNav named \"%s\"\n",
			 currentNav->dtkAugment::getName());
	    }
	  else if(key == xkc_nine) // location pitch and roll reset
	    {

	      currentNav->location.p = 0.f;
	      currentNav->location.r = 0.f;
	      
	      dtkMsg.add(DTKMSG_INFO, "keyboardNavControl::postFrame(): "
			 "set location pitch and roll to zero with "
			 "dtkNav named \"%s\"\n",
			 currentNav->dtkAugment::getName());
	    }
	  else if(key == xkc_eight) // set reset value to current value
	    {
	      reset_location = currentNav->location ;
	      reset_scale = currentNav->scale;
	      dtkMsg.add(DTKMSG_INFO, "keyboardNavControl::postFrame(): "
			 "set navigation reset location "
			 "to the current location in"
			 "dtkNav named \"%s\"\n",
			 currentNav->dtkAugment::getName());
	    }
	  else if(key == xkc_p) // switch navigation
	    {
	      dtkNav *nav = currentNav;
	      dtkMsg.add(DTKMSG_INFO,"keyboardNavSwitcher::postFrame():"
			 " switching navigation from \"%s%s%s\" to ",
			 dtkMsg.color.vil, nav->getName(), dtkMsg.color.end);
	      // Go to the next naviagtion in the list.
	      float response = nav->response();
	      dtkNav *next_nav = navList->next(nav);
	      if(!next_nav)
		next_nav = navList->first();
	      if(!next_nav) {
		dtkMsg.append("\"%s%s%s\".\n",dtkMsg.color.vil,
			      "a non-existant navigation", dtkMsg.color.end);
		return ERROR_ ;
	      }
		
	      dtkMsg.append("\"%s%s%s\".\n",dtkMsg.color.vil,
			    next_nav->getName(), dtkMsg.color.end);
	      if(next_nav != nav)
		{
		  // move to where it left off
		  next_nav->location = nav->location ;
		  next_nav->scale = nav->scale;
		  next_nav->response(response);

		  // userMatrix is copied only if current is not NULL
		  dtkMatrix *up = nav->userMatrix() ;
		  if (!up) 
		    next_nav->userMatrix(up) ;

		  // pivot is copied only if current pivot isn't the default
		  // get current pivot pointer
		  dtkCoord *cp = nav->pivot() ;
		  // set pivot to default
		  nav->pivot(NULL) ;
		  // get current pivot back out
		  dtkCoord *defcp = nav->pivot() ;
		  // is current not the same as the default?
		  if (cp != defcp) {
		    // set next to same non-default value
		    next_nav->pivot(cp) ;
		  }

		  // velocity is copied only if current velocity isn't the default
		  // get current velocity pointer
		  dtkCoord *vp = nav->velocity() ;
		  // set velocity to default
		  nav->velocity(NULL) ;
		  // get current velocity back out
		  dtkCoord *defvp = nav->velocity() ;
		  // is current not the same as the default?
		  if (vp != defvp) {
		    // set next to same non-default value
		    next_nav->velocity(vp) ;
		  }
		  next_nav->preFrame();
		  navList->switch_(next_nav);
		}
	    }
	  // if keyboard '+' and '-' are pressed == reset speed
	  if(keyboard->read(xkc_plus,e) && keyboard->read(xkc_minus,e))
	    {
	      // reset speed (response)
	      currentNav->response(reset_speed);
	      dtkMsg.add(DTKMSG_INFO, "keyboardNavControl::postFrame(): "
			 "reset response (speed) to %f in dtkNav named "
			 "\"%s\"\n",
			 currentNav->response(),
			 currentNav->dtkAugment::getName());
	    }
	}
    }

  return CONTINUE;
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
  return new keyboardNavControl(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


