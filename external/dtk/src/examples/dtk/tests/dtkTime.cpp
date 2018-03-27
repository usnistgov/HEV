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
// Example code using class dtkTime

/* dtkTime is a small gettimeofday() wrapper.  It returns time as long
 * double so that it contains the full resolution of the system clock.
 * double will not work.
 */

#include <stdio.h>
#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# define usleep(x)  SleepEx((x)/1000, false)
#endif


int main(void)
{
  dtkTime t;

  int i;
  for(i=0;i<10;i++)
  {
    printf("      time=%.12Lf\n",t.get());
	usleep(10000);
  }
 
  for(i=0;i<5;i++)
  {
    printf("delta time=%.12Lf\n",t.delta());
    usleep(10000);
  }
  printf("reset(0.0, 1) sets time to zero\n");
  t.reset(0.0, 1);
  for(i=0;i<5;i++)
  {
    printf("     time=%.12Lf\n",t.get());
	usleep(10000);
  }
  printf("reset(0.0) Sets offset to zero"
	 " so that time is the number of seconds "
	 "since Jan1,1970\n");
  t.reset(0.0);
  for(i=0;i<5;i++)
    {
      long double T = t.get();
      printf("     time=%.12Lf sec /(365.25*24*3600) = %.15Lg years\n",
	     T,T/(365.25*24*3600));
	  usleep(10000);
    }
  
  return 0;
}

