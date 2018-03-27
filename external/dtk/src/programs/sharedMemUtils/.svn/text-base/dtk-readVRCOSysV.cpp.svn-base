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
#include <unistd.h>
#include <dtk.h>

int main(int argc, char **argv)
{
  dtkVRCOSharedMem data;

  if(argc > 1) // -h | --help
    return
      printf("Usage: dtk-readVRCOSysV\n"
	     "\n"
	     "   dtk-readVRCOSysV prints to standard output the state\n"
	     " of the standard VRCO(TM) Trackd shared memory with\n"
	     " two trackers, one joystick, and one set of buttons.\n"
	     " This data is printed with just a carrage return and no\n"
	     " line feed, so that it keep write on one line of the tty.\n"
	     "\n");

  if(data.isInvalid()) return 1;

  for(;;)
    {
      printf("                                 "
	     "                                 "
	     "                                 "
	     "| %+2.2f %+2.2f %+2.2f)   \r",
	     data.head->h,data.head->p,data.head->r);

      printf("                                     "
	     "                                     "
	     "head=(%+2.2f %+2.2f %+2.2f \r",
	     data.head->x,data.head->y,data.head->z);
      
      printf("                                           "
	     "| %+2.2f %+2.2f %+2.2f) \r",
	     data.wand->h,data.wand->p,data.wand->r);
      printf("                       "
	     "(%+2.2f %+2.2f %+2.2f \r",
	     data.wand->x,data.wand->y,data.wand->z);
      printf(" wand=%1.1d%1.1d%1.1d%1.1d %+1.1f,%+1.1f \r",
	     data.button[0], data.button[1], data.button[2],data.button[3],
	     data.val[0], data.val[1]);

      fflush(stdout);
      usleep(100000);
    }
}
