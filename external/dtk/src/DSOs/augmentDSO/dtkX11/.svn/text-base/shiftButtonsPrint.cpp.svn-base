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
// Poll the shift keys and print to the "buttons" values to stdout if
// a shift key is pressed.

#include <stdio.h>
#include <dtk.h>
#include <dtkX11.h>
#include <dtk/dtkDSO_loader.h>


class ShiftButtonsPrint : public dtkAugment
{
public:
  ShiftButtonsPrint(dtkManager *m);
  ~ShiftButtonsPrint(void);
  int preFrame(void);
  int postFrame(void);

private:

  dtkManager *manager;
  dtkInXKeyboard *keyboard;
  dtkInButton *button;
  dtkDequeuer *dequeuer;

  int numButtons;

  KeyCode xkc_shift_l;
  KeyCode xkc_shift_r;
};

ShiftButtonsPrint::ShiftButtonsPrint(dtkManager *m) :
  dtkAugment("xkeyboardPrint")
{
  manager = m;
  button = NULL;
  dequeuer = NULL;
  keyboard = NULL;
  setDescription("print X key events to stdout");

  dequeuer = new dtkDequeuer(manager->record());
  if(!dequeuer || dequeuer->isInvalid()) return;

  if(manager->isFramed())
    if(preFrame()) return;

  validate();
}

ShiftButtonsPrint::~ShiftButtonsPrint(void)
{
  if(dequeuer)
    {
      delete dequeuer;
      dequeuer = NULL;
    }

  invalidate();
}

int ShiftButtonsPrint::preFrame(void)
{
  button = (dtkInButton *) manager->get("buttons", DTKINBUTTON_TYPE);
  if(!button) return ERROR_;
  button->queue();

  numButtons = button->number();

  keyboard = (dtkInXKeyboard *)
    manager->get("xkeyboard", DTKINXKEYBOARD_TYPE);
  if(!keyboard) return ERROR_;

  if(!(xkc_shift_l = keyboard->getKeyCode(XK_Shift_L))
     ||
     !(xkc_shift_r = keyboard->getKeyCode(XK_Shift_R))
     )
    {
      dtkMsg.add(DTKMSG_ERROR,
                 "ShiftButtonsPrint::preFrame() no Keycap to "
                 "match one of the X Key Syms in file %s",
                 __FILE__);
      return ERROR_;
    }

  return REMOVE_CALLBACK;
}


int ShiftButtonsPrint::postFrame(void)
{
  dtkRecord_event *event;

  while((event = dequeuer->getNextEvent(button)))
    {
      u_int32_t buttonVal = button->read(event);
      int i;
      for(i=0;i<numButtons;i++)
	printf("%d ", buttonVal>>i & 01);
      printf("\n");
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
  return new ShiftButtonsPrint(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
