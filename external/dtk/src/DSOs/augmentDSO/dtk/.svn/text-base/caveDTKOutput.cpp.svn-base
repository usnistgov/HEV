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
#include <sys/types.h>
#include <dtk.h>
#include <dtk/dtkDSO_loader.h>


// This class writes to DTK shared memory and reads from the
// dtkInput objects.


class CaveDTKOutput : public dtkAugment
{
public:
  CaveDTKOutput(dtkManager *manager);
  ~CaveDTKOutput(void);

  int postFrame(void);

private:

  dtkInLocator *head;
  dtkInLocator *wand;
  dtkInButton *button;
  dtkInValuator *joystick;

  dtkSharedMem head_shm;
  dtkSharedMem wand_shm;
  dtkSharedMem joystick_shm;
  dtkSharedMem button_shm;

  dtkDequeuer *dequeuer;
  dtkInCave *caveInput;
  dtkManager *manager;
};


CaveDTKOutput::CaveDTKOutput(dtkManager *manager_in)
  : dtkAugment("caveDTKOutput"),
    head_shm(sizeof(float)*6,"head"),
    wand_shm(sizeof(float)*6,"wand"),
    joystick_shm(sizeof(float)*2,"joystick"),
    button_shm(sizeof(unsigned char),"buttons")
{
  manager = manager_in;
  dequeuer = NULL;
  caveInput = NULL;

  setDescription("dtkAugment container: reads dtkIn* devices and writes "
		 "to DTK shared memory for standard CAVE(TM) Devices");

  if(head_shm.isInvalid() ||
     wand_shm.isInvalid() || 
     joystick_shm.isInvalid() || 
     button_shm.isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "CaveDTKOutput::CaveDTKOutput() failed.\n");
      return;
    }

  // See if cave input is already present
  dtkInCave *CaveInput = static_cast<dtkInCave *>
    (manager->check("caveInputDevices", DTKINCAVE_TYPE));

  if(!CaveInput)
    {
      // new dtkInCave(manager) will add dtkAugment objects for the
      // CAVE devices.  caveInput is set if this object makes it.
      CaveInput = caveInput = new dtkInCave(manager);
      if(!caveInput || caveInput->isInvalid() || manager->add(caveInput))
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "CaveDTKOutput::CaveDTKOutput() failed:"
		     " in file \"%s\"\n",__FILE__);
	  return;
	}
    }

  joystick = CaveInput->joystick;
  wand = CaveInput->wand;
  head = CaveInput->head;
  button = CaveInput->button;
  // they need to be queued in order for this
  // CaveDTKOutput::postFrame() to work
  head->queue();
  wand->queue();
  joystick->queue();
  button->queue();
  dequeuer = new dtkDequeuer(manager->record());

  // initialize the values
  postFrame();

  // dtkAugment starts out invalid.
  validate();
}


CaveDTKOutput::~CaveDTKOutput(void)
{
  if(dequeuer)
    {
      delete dequeuer;
      dequeuer = NULL;
    }
}


int CaveDTKOutput::postFrame(void)
{
  struct dtkRecord_event *event_rec;
  dtkInput *input;

  while((dequeuer->getNextEvent(&event_rec, &input) > 0))
    {
      float x[6];
      unsigned char c;
      if(static_cast<dtkInput *>(button) == input)
	{
	  c = (unsigned char) button->read(event_rec);
	  if(button_shm.write(&c)) return ERROR_;
	}
      else if(static_cast<dtkInput *>(joystick) == input)
	{
	  joystick->read(x, event_rec);
	  if(joystick_shm.write(x)) return ERROR_;
	}
      else if(static_cast<dtkInput *>(head) == input)
	{
	  head->read(x, event_rec);
	  if(head_shm.write(x)) return ERROR_;
	}
      else if(static_cast<dtkInput *>(wand) == input)
	{
	  wand->read(x, event_rec);
	  if(wand_shm.write(x)) return ERROR_;
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
  return new CaveDTKOutput(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


