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


class CaveInputPrint : public dtkAugment
{
public:
  CaveInputPrint(dtkManager *m);

  int preFrame(void);
  int postFrame(void);

private:

  dtkManager *manager;
  dtkInCave *cave;
};

CaveInputPrint::CaveInputPrint(dtkManager *m) : dtkAugment("caveInputPrint")
{
  manager = m;
  setDescription("polls and prints CAVE devices data to stdout");

  if(manager->isFramed())
    if(preFrame()) return;

  validate();
}

int CaveInputPrint::preFrame(void)
{
  cave = (dtkInCave *)
    manager->get("caveInputDevices", DTKINCAVE_TYPE);
  if(!cave) return ERROR_;

  return REMOVE_CALLBACK;
}

int CaveInputPrint::postFrame(void)
{
  float *f; 
  f = cave->wand->read();
  printf("                                                "
	 "                      "
	 "w=%+2.2f %+2.2f %+2.2f  %+2.2f %+2.2f %+2.2f  \r",
	 f[0], f[1], f[2],  f[3], f[4], f[5]);

  f = cave->head->read();
  printf("                        "
	 "h=%+2.2f %+2.2f %+2.2f  %+2.2f %+2.2f %+2.2f\r",
	 f[0], f[1], f[2],  f[3], f[4], f[5]);
  
  f = cave->joystick->read();
  printf("         j=%+2.2f,%+2.2f\r",f[0], f[1]);

  static int count;
  static const char flipper[5] = "|/-\\";
  count++;
  if(count >= 4) count = 0;
  printf("%c b=%1.1d%1.1d%1.1d%1.1d\r",flipper[count],
	 cave->button->read(0),
	 cave->button->read(1),
	 cave->button->read(2),
	 cave->button->read(3));

  fflush(stdout);

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
  return new CaveInputPrint(manager);
}

static int dtkDSO_unloader(dtkAugment *augment)
{
  delete augment;
  return DTKDSO_UNLOAD_CONTINUE;
}
