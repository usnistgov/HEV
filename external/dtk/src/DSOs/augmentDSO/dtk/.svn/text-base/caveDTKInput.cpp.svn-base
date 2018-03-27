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

// This class reads from DTK shared memory and writes to the
// dtkInput CAVE device objects.


class CaveDTKInput : public dtkAugment
{
public:
  CaveDTKInput(dtkManager *manager);
  ~CaveDTKInput(void);

  int preFrame(void);

private:

  int doFloats(dtkInValuator *v, dtkSharedMem *shm, int number);

  dtkInLocator *head;
  dtkInLocator *wand;
  dtkInButton *button;
  dtkInValuator *joystick;

  dtkSharedMem *head_shm;
  dtkSharedMem *wand_shm;
  dtkSharedMem *joystick_shm;
  dtkSharedMem *button_shm;

  unsigned char button_state;
  dtkInCave *caveInput;
  dtkManager *manager;
};



CaveDTKInput::~CaveDTKInput(void)
{
  if(head_shm)
    {
      delete head_shm;
      head_shm = NULL;
    }
  if(head_shm)
    {
      delete head_shm;
      head_shm = NULL;
    }
  if(button_shm)
    {
      delete button_shm;
      button_shm = NULL;
    }
  if(joystick_shm)
    {
      delete joystick_shm;
      joystick_shm = NULL;
    }
  if(caveInput)
    {
      // caveInput is set if this object makes it.
      manager->remove(caveInput);
      delete caveInput;
      caveInput = NULL;
    }
}


CaveDTKInput::CaveDTKInput(dtkManager *manager_in) : dtkAugment("caveDTKInput")
{
  setDescription("dtkAgument container for standard CAVE Inputs that"
		 " reads DTK shared memory and writes dtkIn* devices");
  manager = manager_in;

  float H[] = {0.0f,0.0f, 0.0f,0.0f,0.0f,0.0f};
  float W[] = {0.0f,0.2f,-0.2f,0.0f,0.0f,0.0f};
  float J[] = {0.0f,0.0f};
  button_state = 0;

  head_shm =   new dtkSharedMem(sizeof(float)*6,"head", H);
  wand_shm =   new dtkSharedMem(sizeof(float)*6,"wand", W);
  joystick_shm = new dtkSharedMem(sizeof(float)*2,"joystick", J);
  button_shm = new dtkSharedMem(sizeof(unsigned char),
				"buttons", &button_state);

  if(head_shm->isInvalid() || wand_shm->isInvalid() ||
     joystick_shm->isInvalid() || button_shm->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,"CaveOutput::CaveDTKInput() failed.\n");
      return;
    }

  head_shm->setAutomaticByteSwapping(sizeof(float));
  wand_shm->setAutomaticByteSwapping(sizeof(float));
  joystick_shm->setAutomaticByteSwapping(sizeof(float));


  // flush old button events
  button_shm->flush() ;


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

  // initialize the values
  if(preFrame() == ERROR_) return;

  // dtkAugment starts out invalid.
  validate();
}

int CaveDTKInput::
doFloats(dtkInValuator *v, dtkSharedMem *shm, int num)
{
  int queued = v->isQueued();
  if(queued)
    if(!shm->isQueued())
      if(shm->queue())
	return -1; // error

  float x[6];

  if(queued)
    {
      int i;
      do
	{
	  i = shm->qread(x);
          if(i<0) return 1; // error
	  if(i==0) return 0; // no record
	  v->write(x);
	}
      while(i>1);
    }
  else
    {
      if(shm->read(x)) return 1; // error
      v->write(x);
    }

  return 0;
}


int CaveDTKInput::preFrame(void)
{
  if(doFloats(head, head_shm, 6)) return ERROR_;
  if(doFloats(wand, wand_shm, 6)) return ERROR_;
  if(doFloats(joystick, joystick_shm, 2)) return ERROR_;

  // the buttons
  int queued = button->isQueued();
  if(queued)
    if(!button_shm->isQueued())
      if(button_shm->queue())
	return ERROR_;

  if(queued)
    {
      int i;
      do
	{
	  unsigned char button_buffer[4] = {0,0,0,0};
	  if((i = button_shm->qread(button_buffer)))
	    {
	      // find the bits that changed
	      int j, k;
	      for(j=0;j<8;j++)
		{
		  k = 01 << j;
		  if((button_state & k) ^ (button_buffer[0] & k))
		    {
		      k &= button_buffer[0];
		      if(k)
			button_state |= 01 << j;
		      else
			button_state &= ~(01 << j);
		      button->write(j, k);
		    }
		}
	    }
	}
      while (i > 1);
    }
  else
    {
      button_shm->read(&button_state);
      u_int32_t s = button_state;
      button->write(s);
    }

  return  CONTINUE;
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
  return new CaveDTKInput(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
