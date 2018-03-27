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
// dtk DSO for making keyboard input and optionally mouse input
// This works with just one X server.

#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <dtk.h>
#include <dtkX11.h>
#include <dtk/dtkDSO_loader.h>

class XKeyboard : public dtkAugment
{
public:

  XKeyboard(dtkManager *manager);
  ~XKeyboard(void);

  int postConfig(void);
  int postFrame(void);

private:

  void wait_for_windowMapping(void);

  dtkManager *manager;
  void clean_up(void);
  dtkInXKeyboard *keyboard;
  dtkXEvent *xe;

  // This is the key state after the last event processed.  It's just
  // like the data returned from XQueryKeymap, but it will just have
  // the key state that was noticed by this class object, not from the
  // X server, so that you can check for multiple key press
  // combinations.  There is a 32 bytes of data for each display.
  char *keyState;

#ifdef MOUSE
  dtkInButton *button;
  dtkInValuator *pointer;
#endif
};


XKeyboard::XKeyboard(dtkManager *manager_in):
  dtkAugment(
#ifdef MOUSE
	     "XKeyboardMouseInput"
#else
	     "XKeyboardInput"
#endif
	     )
{
  // dtkAugment starts out invalid.

  xe = NULL;
  keyboard = NULL;
  manager = manager_in;
  keyState = NULL ; 

#ifdef MOUSE
  button = NULL;
  pointer = NULL;
#endif

  setDescription("Read X Events and stuff them into a"
		 " dtkInXKeyboard object named \"xkeyboard\""
#ifdef MOUSE
		 "\nand a dtkInButton object named \"buttons\""
		 " and a dtkInValuator object named \"pointer\""
#endif
		 );

#ifdef MOUSE

  if(manager->check("pointer"))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "XKeyboard::XKeyboard() this is a dtkAugment"
		 " named \"pointer\" already loaded.\n");
      return;
    }

  /*********************** setup button and pointer **************/

  if(!(button = (dtkInButton *) manager->check("buttons")))
    {
      button = new dtkInButton(manager->record(), 4, "buttons");
      if(!button || button->isInvalid())
	{
	  dtkMsg.add(DTKMSG_ERROR,"XKeyboard::XKeyboard() failed to"
		     " get dtkInButton.\n");
	  return;
	}
      button->setDescription("X mouse buttons with 4 buttons");
      if(manager->add(button)) return;
    }

  pointer = new dtkInValuator(manager->record(), 2, "pointer");
  if(!pointer || pointer->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,"XKeyboard::XKeyboard() failed to"
		 " get dtkInValuator.\n");
      return;
    }
  pointer->setDescription("X mouse pointer");
  if(manager->add(pointer)) return;

#endif

  if(manager->isPostConfiged())
    if(postConfig()) return;

  // dtkAugment starts out invalid.
  validate();
}


void XKeyboard::clean_up(void)
{
  if(xe)
    {
      delete xe;
      xe = NULL;
    }

  if(keyState)
    {
      free(keyState);
      keyState = NULL;
    }

  if(keyboard)
    {
      manager->remove(keyboard);
      delete keyboard;
      keyboard = NULL;
    }

#ifdef MOUSE
  if(button)
    {
      manager->remove(button);
      delete button;
      button = NULL;
    }
  if(pointer)
    {
      manager->remove(pointer);
      delete pointer;
      pointer = NULL;
    }
#endif

  invalidate();
}

XKeyboard::~XKeyboard(void)
{
  clean_up();
}


int XKeyboard::postConfig(void)
{
  if(manager->check("xkeyboard"))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "XKeyboard::postConfig() this is a dtkAugment"
		 " named \"xkeyboard\" already loaded.\n");
      return ERROR_;
    }

  /******************** setup a dtkXEvent ******************/

  // dynamic_cast would be nice here.

  dtkAugment *a = NULL;
  dtkXWindowList *windowList;

#if 1 /* RedHat 7.2 broken dynamic_cast<>() g++ */
  do 
    if((windowList = dynamic_cast<dtkXWindowList *>
	(a = manager->getNext(a))))
      break;
  while(a);
#else /* This we feel would be better than forcing the user
       * to use a different g++ than the default RedHat 7.2 one.
       */
  do
    if((windowList = static_cast<dtkXWindowList *>
	(a = manager->getNext(a, DTKXWINDOWLIST_TYPE))))
      break;
  while(a);
#endif

  if(!windowList)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "XKeyboard::postConfig() failed to "
		 "get a dtkXWindowList object from"
		 " the dtkAugment manager.\n");
      return ERROR_;
    }

  xe = new dtkXEvent(windowList);
  if(!xe || xe->isInvalid())
    {
      clean_up();
      return ERROR_;
    }

  // Get the first X Display connection struct thingy from XOpenDisplay().
  Display *dpy = xe->getDisplay();

  // xe->setKeyGrabing() will make it so that you can get key release
  // events after a key press and your pointer loses focus.
  // xe->setKeyGrabing();

  /****************** now get a dtkInKeyboard ******************/

  int num_displays = 1;

  Display *disArray[2] = { dpy, NULL };

  keyboard = new dtkInXKeyboard(manager->record(), disArray, manager);

  if(!keyboard || keyboard->isInvalid() || manager->add(keyboard))
    {
      clean_up();
      return ERROR_;
    }

  keyState = (char *) malloc(32*num_displays);

  bzero(keyState, 32*num_displays);

  return CONTINUE;
}


int XKeyboard::postFrame(void)
{
  XEvent e;
  while(xe->getNextEvent(&e))
    {
      switch (e.type)
	{
	case KeyPress:
	  {
	    //printf("got key press: %d\n", ((XKeyEvent *)&e)->keycode);
	    KeyCode key = e.xkey.keycode;
	    // set this key bit.
	    keyState[key/8] |= (01 << (key%8));
	    keyboard->write((const char *) keyState,
		      key,
		      1, e.xany.window, 0);
	    break;
	  }
	case KeyRelease:
	  {
	    //printf("got key release: %d\n",((XKeyEvent *)&e)->keycode);
	    KeyCode key = e.xkey.keycode;
	    // unset this key bit.
	    keyState[key/8] &= ~(01 << (key%8));
	    keyboard->write((const char *) keyState,
		      key,
		      0, e.xany.window, 0);
	    break;
	  }
#ifdef MOUSE

/* This could get interesting if there is more than one X display
 * connection with more than one keyboard and mouse. */

	case MotionNotify:
	  {
	    float xy[2];
	    // get a new value of the pointer
	    // The value is between +1 and -1 .	    
	    xy[0] = xe->getWindowWidth()/2;
	    xy[1] = xe->getWindowHeight()/2;
	    xy[0] =  (e.xmotion.x - xy[0])/xy[0];
	    xy[1] = - (e.xmotion.y - xy[1])/xy[1];
	    if(xy[0] > 1.0f) xy[0] = 1.0f;
	    else if(xy[0] < -1.0f) xy[0] = -1.0f;
	    if(xy[1] > 1.0f) xy[1] = 1.0f;
	    else if(xy[1] < -1.0f) xy[1] = -1.0f;
	    pointer->write(xy);
	    break;
	  }
	case ButtonPress:
	  if(e.xbutton.button == Button1)
	    button->write(0,1);
	  else if(e.xbutton.button == Button2)
	    button->write(1,1);
	  else if(e.xbutton.button == Button3)
	    button->write(2,1);
	  else if(e.xbutton.button == Button4)
	    button->write(3,1);
	  break;
	case ButtonRelease:
	  if(e.xbutton.button == Button1)
	    button->write(0,0);
	  else if(e.xbutton.button == Button2)
	    button->write(1,0);
	  else if(e.xbutton.button == Button3)
	    button->write(2,0);
	  else if(e.xbutton.button == Button4)
	    button->write(3,0);
	  break;

#endif /* #ifdef MOUSE */

	  case UnmapNotify:
	    if(manager->state & DTK_ICONSLEEP &&
	       !(xe->isWindowMapped()))
	      wait_for_windowMapping();
	    break;
	}
    }
  return CONTINUE;
}


void XKeyboard::wait_for_windowMapping(void)
{
  dtkMsg.add(DTKMSG_DEBUG, "No windows mapped sleeping ...\n");
  int i = 1;
  while(i)
    {
      usleep(100000);
      XEvent e;
      while(xe->getNextEvent(&e))
	{
	  if(e.type == MapNotify && xe->isWindowMapped())
	    {
	      i = 0;
	      break;
	    }
	}
    }
  dtkMsg.add(DTKMSG_DEBUG, "Windows mapped waking up ...\n");
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
  return new XKeyboard(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
