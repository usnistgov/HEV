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
// This class reads DTK shared memory and writes to the the current
// navigation object, from dtkDisplay::currentNav.

// Classes that inherit dtkNav and override postConfig(void) or
// preFrame(void) need to call dtkNav::postConfig(void) or
// dtkNav::preFrame(void) respectively.

#include <dtk.h>
#include <dtk/dtkDSO_loader.h>

class dtkShmNav : public dtkNav
{
public:
  dtkShmNav(dtkManager *);
  ~dtkShmNav(void);
  int postConfig(void);
  int postFrame(void);

private:

  dtkSharedMem *nav_shm;
};

dtkShmNav::~dtkShmNav(void)
{
  if(nav_shm)
    {
      delete nav_shm;
      nav_shm = NULL;
    }
}


dtkShmNav::dtkShmNav(dtkManager *m) : dtkNav(m, "dtkShmNav")
{
  setDescription("reads a DTK shared memory segment named \"nav\""
		 " and then writes it to dtkNav location and scale");
}


int dtkShmNav::postConfig(void)
{
  // default initial navigation position
  float x[7] = {
    0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 1.0f
  } ; /* x,y,z,h,p,r,scale */

  nav_shm = new dtkSharedMem(sizeof(float)*7,"nav",x);
  if(!nav_shm || nav_shm->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,"dtkShmNav::postConfig() failed.\n");
      return ERROR_;
    }
  nav_shm->setAutomaticByteSwapping(sizeof(float));

  dtkNav::postConfig();

  // initialize the values
  postFrame();

  return REMOVE_CALLBACK;
}


// just copy the navigation location and scale from DTK shared memory
int dtkShmNav::postFrame(void)
{
  float x[7]; /* x,y,z,h,p,r,scale */

  if(nav_shm->read(x)) return ERROR_;

  location.set(x) ;
  scale = x[6];

  //  printf("%3.3f %3.3f %3.3f  %3.3f %3.3f %3.3f  %3.3f\n",
  // x[0], x[1], x[2], x[3], x[4], x[5], x[6]);

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
  return new dtkShmNav(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}


