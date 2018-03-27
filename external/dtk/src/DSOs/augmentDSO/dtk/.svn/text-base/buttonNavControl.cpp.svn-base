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
// Controls the current navigation speed and reset using button presses.
// Read on to see now.

#define DEFAULT_SPEED 1.0f
#define SPEED_FACTOR 2.0f

// the max speed is the speed of light if the model units are meters.
#define MAX_SPEED 3.0e+8f
#define MIN_SPEED 3.0e-8f

#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

class buttonNavControl : public dtkAugment
{
public:
  buttonNavControl(dtkManager *);
  ~buttonNavControl(void);

  int preFrame(void);
  int postFrame(void);

private:

  dtkManager *mgr ;
  dtkInButton *button;

  dtkNavList *navList;
  dtkDequeuer *dequeuer;

  dtkCoord reset_location;
  float reset_scale;
  float reset_speed;
};


buttonNavControl::buttonNavControl(dtkManager *m) :
  dtkAugment("buttonNavControl")
{
  setDescription("adds speed and reset navigation control"
		 " to the current dtkNav navigation:"
		 " 'button-2' or 'button-0' = speed "
		 "up or down, 'button-1' = reset location,"
		 " 'button-2' and 'button-0' = reset speed");
  mgr = m ;
  dequeuer = NULL;
  navList = NULL;
  button = NULL;

  // dtkAugment::dtkAugment() will not validate the object
  validate();
}


buttonNavControl::~buttonNavControl(void)
{
  if(dequeuer)
    {
      delete dequeuer;
      dequeuer = NULL;
    }
}


#define MIN_BUTTONS 3


// This tries to find a reset location and scale
int buttonNavControl::preFrame(void)
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

  button = (dtkInButton *) mgr->get("buttons", DTKINBUTTON_TYPE);
  if(!button) return ERROR_;

   if(button->number() < MIN_BUTTONS)
     {
       dtkMsg.add(DTKMSG_ERROR,
		  "buttonNavControl::preConfig() failed:"
		  " the dtkInButton named \"%s\" has "
		  "just %d buttons"
		  " which is less than the %d needed.\n",
		  button->getName(), button->number(),
		  MIN_BUTTONS);
      return ERROR_;
    }
  button->queue();

  if(navList->current())
    {
      reset_location = navList->current()->location ;
      reset_scale = navList->current()->scale;
      reset_speed = navList->current()->response();
    }
  else
    {
      //reset_location.zero() ; # constructor zeros
      reset_scale = 1.0f;
      reset_speed = DEFAULT_SPEED;
    }

  // just call buttonNavControl::preFrame() once at the start.
  return REMOVE_CALLBACK;
}


int buttonNavControl::postFrame(void)
{
  dtkRecord_event *e;
  while((e = dequeuer->getNextEvent(button)))
    {
      dtkNav *currentNav = navList->current();
      if(currentNav)
	{
	  int whichButton;
	  int buttonState = button->read(&whichButton, e);
	  if(buttonState && whichButton < 3) // if pressed button 0 or 1 or 2
	    {
	      if((whichButton == 0 || whichButton == 2) && !button->read(1,e))
		{
		  float speed = currentNav->response();
		  if(whichButton == 0) // slow down
		    {
		      if(speed > MIN_SPEED)
			{
			  speed /= SPEED_FACTOR;
			  if(speed > MIN_SPEED)
			    currentNav->response(speed);
			}
		    }
		  else // whichButton == 2
		    {
		      if(speed < MAX_SPEED)
			{
			  speed *= SPEED_FACTOR;
			  if(speed < MAX_SPEED)
			    currentNav->response(speed);
			}
		    }

		  // if button 0 and 2 are pressed == reset speed
		  if(button->read(0,e) && button->read(2,e))
		    {
		      // reset speed (response)
		      currentNav->response(reset_speed);
		      dtkMsg.add(DTKMSG_INFO, "buttonNavControl::postFrame(): "
				 "reset response (speed) to %f in dtkNav named "
				 "\"%s\"\n",
				 currentNav->response(),
				 currentNav->dtkAugment::getName());
		    }
		  else
		    dtkMsg.add(DTKMSG_INFO, "buttonNavControl::postFrame(): "
			       "set response (speed) in dtkNav named "
			       "\"%s\" to %f\n",
			       currentNav->dtkAugment::getName(),
			       currentNav->response());
		}
	      else if(whichButton == 0 && button->read(1,e))
		// location reset for button 1 then 0
		{
		  currentNav->location = reset_location ;
		  currentNav->scale = reset_scale;
		  dtkMsg.add(DTKMSG_INFO, "buttonNavControl::postFrame(): "
			     "reset to initial location in "
			     "dtkNav named \"%s\"\n",
			     currentNav->dtkAugment::getName());
		}
	      else if(whichButton == 2 && button->read(1,e))
		// switch navigation for button 1 then 2
		{
		  float speed = currentNav->response();
		  // remove the speed increase do to button 2 press
		  if(speed > MIN_SPEED)
		    {
		       speed /= SPEED_FACTOR;
		       if(speed > MIN_SPEED)
			 currentNav->response(speed);
		    }

		  dtkNav *nav = currentNav;
		  dtkMsg.add(DTKMSG_INFO,"buttonNavSwitcher::postFrame():"
			     " switching navigation from \"%s%s%s\" to ",
			     dtkMsg.color.vil, nav->getName(), dtkMsg.color.end);
		  // Go to the next naviagtion in the list.
		  float response = nav->response();
		  dtkNav *next_nav = navList->next(nav);
		  if(!next_nav)
		    next_nav = navList->first();
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
  return new buttonNavControl(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


