/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef DTK_ARCH_WIN32_VCPP
#else
# include <sys/time.h>
#endif
#include <string.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkAugment.h"
#include "dtkRecord.h"
#include "dtkInput.h"
#include "dtkInButton.h"
#include "dtkInValuator.h"
#include "dtkInLocator.h"
#include "dtkDisplay.h"
#include "dtkManager.h"
#include "dtkInCave.h"


#define HEAD      "head"
#define WAND      "wand"
#define JOYSTICK  "joystick"
#define BUTTON    "buttons"


// This class sets up an interface to devices but does not write to
// them.

dtkInCave::dtkInCave(dtkManager *manager) : dtkAugment("caveInputDevices")
{
  setType(DTKINCAVE_TYPE);
  setDescription("creates standard CAVE Inputs: dtkLocator named \"%s\",\n"
		 " dtkLocator named \"%s\", dtkValuator named"
		 " \"%s\" and a dtkButton named \"%s\".",
		 HEAD, WAND, JOYSTICK, BUTTON);
  button = NULL;
  joystick = NULL;
  head = NULL;
  wand = NULL;

  if(manager == NULL) return; // error

  if(!(head = static_cast<dtkInLocator *>
       (manager->check(HEAD, DTKINLOCATOR_TYPE))))
    {
      head = new dtkInLocator(manager->record(), HEAD);
      if(!head || head->isInvalid()) return; // error
      head->setDescription("a dtkInLocator for head tracker");
      if(manager->add(head)) return; // error
    }

  if(!(wand = static_cast<dtkInLocator *>
       (manager->check(WAND, DTKINLOCATOR_TYPE))))
    {
      wand = new dtkInLocator(manager->record(), WAND);
      if(!wand || wand->isInvalid()) return; // error
      wand->setDescription("a dtkInLocator for wand tracker");
      if(manager->add(wand)) return; // error
    }

  if(!(joystick = static_cast<dtkInValuator *>
       (manager->check(JOYSTICK, DTKINVALUATOR_TYPE))))
    {
      joystick = new dtkInValuator(manager->record(), 2, JOYSTICK);
      if(!joystick || joystick->isInvalid()) return; // error
      joystick->setDescription("wand joystick with 2 valuators");
      if(manager->add(joystick)) return; // error
    }

  if(!(button = static_cast<dtkInButton *>
       (manager->check("buttons", DTKINBUTTON_TYPE))))
    {
      button = new dtkInButton(manager->record(), 4, BUTTON);
      if(!button || button->isInvalid()) return; // error
      button->setDescription("wand buttons with 4 buttons");
      if(manager->add(button)) return; // error
    }

  validate();
}

dtkInCave::~dtkInCave(void) {}
