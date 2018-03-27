/*
 * The DIVRRRSE Toolkit
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
// This class writes to DTK shared memory and reads from the
// the current navigation object, from dtkNavList::current().

// This DSO is used as an example in the DPF documention.

#include <dtk.h>
#include <dtk/dtkDSO_loader.h>


class navDTKOutput : public dtkAugment
{

public:

  navDTKOutput(dtkManager *m);
  int postFrame(void);

private:

  dtkSharedMem shm;
  dtkNavList *navList;
};

// Constructor.
navDTKOutput::navDTKOutput(dtkManager *m) :
  dtkAugment("navDTKOutput"), shm(sizeof(float)*7,"nav")
{
  setDescription("writes the current navigation "
		 "location and scale to a DTK "
		 "shared memory segment \"nav\"");

  if(shm.isInvalid()) return; // error

  navList = (dtkNavList *) m->check("dtkNavList", DTKNAVLIST_TYPE) ;
  if(!navList)
    {
      navList = new dtkNavList;
      if(m->add(navList))
	{
	  delete navList;
	  navList = NULL;
	  return;
	}
    }

  // dtkAugment starts out invalid.
  validate();
}

// Gets called after each Frame().
int navDTKOutput::postFrame(void)
{
  dtkNav *nav = navList->current();
  if(nav)
    {
      float x[7]; /* x,y,z,h,p,r,scale */
      nav->location.get(x) ;
      x[6] = nav->scale;

      if(shm.write(x)) return ERROR_;
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
  return new navDTKOutput(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


