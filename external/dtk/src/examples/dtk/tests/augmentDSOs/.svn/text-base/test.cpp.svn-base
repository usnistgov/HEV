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
#include <dtk/dtkDSO_loader.h>

#define NAME "Test"


class Test : public dtkAugment
{
public:
  Test(dtkManager *m);
  virtual ~Test(void);

  int postFrame(void);
};


Test::Test(dtkManager *m) : dtkAugment(NAME)
{
  printf("calling \"%s\" constructor\n", NAME);

  setDescription("A test dtkAugment");
  validate();
}

Test::~Test(void)
{
  printf("calling \"%s\" destructor\n", NAME);
}

int Test::postFrame(void)
{
  printf("calling %s postFrame()\n", __FILE__);
  return CONTINUE;
}


static dtkAugment *dtkDSO_loader(dtkManager *manager, void *p)
{
  return new Test(manager);
  //return DTKDSO_LOAD_ERROR;
  //return DTKDSO_LOAD_CONTINUE;
  //return DTKDSO_LOAD_UNLOAD;
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  printf("called dtkDSO_unloader(%p)\n", augment);
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
  //return DTKDSO_UNLOAD_ERROR;
}
