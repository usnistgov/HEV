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

#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#include <dtk/dtkDSO_loader.h>

/* #ifdef DTK_ARCH_WIN32_VCPP
# define usleep(x)	SleepEx((x)/1000, false)
#endif *//* #ifdef DTK_ARCH_WIN32_VCPP */


class SleepSyncDisplay : public dtkDisplay
{
public:
  SleepSyncDisplay(dtkManager *m);

  int sync(void);
};

SleepSyncDisplay::SleepSyncDisplay(dtkManager *m) :
  dtkDisplay("sleepSyncDisplay")
{
  setDescription("calls usleep(100000) at sync()");
  validate();
}


int SleepSyncDisplay::sync(void)
{
  usleep(100000);
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
  return new SleepSyncDisplay(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
