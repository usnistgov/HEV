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
#include <dtk.h>
#include <dtkX11.h>
#include <dtk/dtkDSO_loader.h>


class XKeyboardPrint : public dtkAugment
{
public:
  XKeyboardPrint(dtkManager *m);
  ~XKeyboardPrint(void);
  int preFrame(void);
  int postFrame(void);

private:

  dtkManager *manager;
  dtkDequeuer *dequeuer;
  dtkInXKeyboard *keyboard;
  dtkInValuator *pointer;
  dtkInButton *button;

  int numPointerVals;
  int numButtons;
};

XKeyboardPrint::XKeyboardPrint(dtkManager *m) :
  dtkAugment("xkeyboardPrint")
{
  dequeuer = NULL;
  manager = m;
  keyboard = NULL;
  pointer = NULL;
  button = NULL;

  dequeuer = new dtkDequeuer(manager->record());
  if(!dequeuer || dequeuer->isInvalid()) return;

  setDescription("print X key events to stdout");
  validate();
}

XKeyboardPrint::~XKeyboardPrint(void)
{
  if(dequeuer)
    {
      delete dequeuer;
      dequeuer = NULL;
    }

  invalidate();
}

int XKeyboardPrint::preFrame(void)
{
  keyboard = (dtkInXKeyboard *)
    manager->get("xkeyboard", DTKINXKEYBOARD_TYPE);
  if(!keyboard) return ERROR_;

  pointer = (dtkInValuator *)
    manager->check("pointer", DTKINVALUATOR_TYPE);
  button = (dtkInButton *)
    manager->check("buttons", DTKINBUTTON_TYPE);

  // keyboard may not be queued yet.
  keyboard->queue();

  if(pointer)
    {
      pointer->queue();
      numPointerVals = pointer->number();
    }

  if(button)
    {
      numButtons = button->number();
      button->queue();
    }

  return REMOVE_CALLBACK;
}


int XKeyboardPrint::postFrame(void)
{
  dtkRecord_event *event;
  dtkInput *input;
  int i;

  while((i = dequeuer->getNextEvent(&event, &input)))
    {
      if(input == (dtkInput *) keyboard)
	{
	  KeyCode key;
	  int pressed = keyboard->read(&key,event);
	  if(pressed)
	    printf("  pressed key %d\n",key);
	  else
	    printf(" released key %d\n",key);
	}
      else if(input == (dtkInput *) pointer)
	{
	  float *x = pointer->read(event);
	  printf("pointer = ");
	  int i=0;
	  for(;i<numPointerVals;i++)
	    printf("%f ", x[i]);
	  printf("\n");
	}
      else if(input == (dtkInput *) button)
	{
	  printf("buttons = ");
	  int i=0;
	  for(;i<numButtons;i++)
	    printf("%d ",button->read(i,event));
	  printf("\n");
	}
      if(i == 1) break;
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
  return new XKeyboardPrint(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
