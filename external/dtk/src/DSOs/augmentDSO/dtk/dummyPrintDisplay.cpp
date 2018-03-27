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

#ifdef DTK_ARCH_WIN32_VCPP
# include <stdlib.h>
# define sleep(x)  Sleep(1000*(x))
#else
# include <unistd.h>
#endif

#include <dtk/dtkDSO_loader.h>

#define NAME "dummyPrintDisplay"

class DummyPrintDisplay : public dtkDisplay
{
public:
  DummyPrintDisplay(dtkManager *m);
  virtual ~DummyPrintDisplay(void);

  int preConfig(void);
  int config(void);
  int postConfig(void);
  int sync(void);
  int preFrame(void);
  int frame(void);
  int postFrame(void);
};

DummyPrintDisplay::DummyPrintDisplay(dtkManager *m) : dtkDisplay(NAME)
{
  printf("calling \""NAME"\" constructor\n");

  setDescription("A test dtkDisplay that prints to stdout");
  validate();
}

DummyPrintDisplay::~DummyPrintDisplay(void)
{
  printf("calling \""NAME"\" destructor\n");
}

int DummyPrintDisplay::preConfig(void)
{
  printf("calling \""NAME"\" preConfig()\n");
  return REMOVE_CALLBACK;
}


int DummyPrintDisplay::config(void)
{
  printf("calling \""NAME"\" config()\n");
  return REMOVE_CALLBACK;
}

int DummyPrintDisplay::postConfig(void)
{
  printf("calling \""NAME"\" postConfig()\n");
  return REMOVE_CALLBACK;
}

int DummyPrintDisplay::sync(void)
{
  printf("calling \""NAME"\" sync() sleeping 1 second\n");
  sleep(1);
  return CONTINUE;
}

int DummyPrintDisplay::preFrame(void)
{
  printf("calling \""NAME"\" preFrame()\n");
  return CONTINUE;
}

int DummyPrintDisplay::frame(void)
{
  printf("calling \""NAME"\" frame()\n");
  return CONTINUE;
}

int DummyPrintDisplay::postFrame(void)
{
  printf("calling \""NAME"\" postFrame()\n");
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
  return new DummyPrintDisplay(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
