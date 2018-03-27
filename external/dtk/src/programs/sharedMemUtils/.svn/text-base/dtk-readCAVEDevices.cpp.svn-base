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
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

int Usage(void)
{
  dtkColor c(stdout);
  printf("\n"
	 " Usage: dtk-readCAVEDevices [-h|--help]\n\n"
	 "  dtk-readCAVEDevices reads the DTK shared memory %shead%s,\n"
	 " %swand%s, %sjoystick%s, and %sbuttons%s and than prints\n"
	 " to stdout with a carrage return (not new line).\n\n",
	 c.vil, c.end, c.vil, c.end, c.vil, c.end, c.vil, c.end);
  return 1;
}

int main(int argc, char **argv)
{
  if(argc > 1) return Usage();

  dtkSharedMem *headShm = new dtkSharedMem(sizeof(float)*6, "head");
  if(headShm->isInvalid()) return 1;
  headShm->setAutomaticByteSwapping(sizeof(float));
  dtkSharedMem *wandShm = new dtkSharedMem(sizeof(float)*6, "wand");
  if(wandShm->isInvalid()) return 1;
  wandShm->setAutomaticByteSwapping(sizeof(float));
  dtkSharedMem *joystickShm = new dtkSharedMem(sizeof(float)*2, "joystick");
  if(joystickShm->isInvalid()) return 1;
  joystickShm->setAutomaticByteSwapping(sizeof(float));
  dtkSharedMem *buttonShm = new dtkSharedMem(sizeof(unsigned char), "buttons");
  if(buttonShm->isInvalid()) return 1;

  for(;;)
    {
      float x[6];
      if(headShm->read(x)) return 1;

      printf("                                 "
	     "                                 "
	     "                                 "
	     "| %+2.2f %+2.2f %+2.2f)   \r",
	     x[3],x[4],x[5]);

      printf("                                     "
	     "                                     "
	     "head=(%+2.2f %+2.2f %+2.2f \r",
	     x[0],x[1],x[2]);
      

      if(wandShm->read(x)) return 1;

      printf("                                           "
	     "| %+2.2f %+2.2f %+2.2f) \r",
	     x[3],x[4],x[5]);
      printf("                       "
	     "(%+2.2f %+2.2f %+2.2f \r",
	     x[0],x[1],x[2]);

      unsigned char b;
      if(buttonShm->read(&b)) return 1;
      if(joystickShm->read(x)) return 1;

      printf(" wand=%d%d%d%d %+1.1f,%+1.1f \r",
	     (b&01)?1:0,(b&02)?1:0,(b&04)?1:0,(b&010)?1:0,
	     x[0], x[1]);

      fflush(stdout);
      usleep(100000);
    }
}
