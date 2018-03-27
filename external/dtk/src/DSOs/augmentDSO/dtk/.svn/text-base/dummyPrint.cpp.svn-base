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

#define NAME "dummyPrint"


class DummyPrint : public dtkAugment
{
public:
  DummyPrint(dtkManager *m);
  virtual ~DummyPrint(void);

  int preConfig(void);
  int postConfig(void);
  int preFrame(void);
  int postFrame(void);
};

DummyPrint::DummyPrint(dtkManager *m) : dtkAugment(NAME)
{
  printf("calling \"%s\" constructor\n", NAME);

  setDescription("A test dtkAugment that prints to stdout");
  validate();
}

DummyPrint::~DummyPrint(void)
{
  printf("calling \"%s\" destructor\n", NAME);
}

int DummyPrint::preConfig(void)
{
  printf("calling \"%s\" preConfig()\n", NAME);
  return REMOVE_CALLBACK;
}
	
int DummyPrint::postConfig(void)
{
  printf("calling \"%s\" postConfig()\n", NAME);
  return REMOVE_CALLBACK;
}
	
int DummyPrint::preFrame(void)
{
  printf("calling \"%s\" preFrame()\n", NAME);
  return CONTINUE;
}

int DummyPrint::postFrame(void)
{
  printf("calling \"%s\" postFrame()\n", NAME);
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
  return new DummyPrint(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
