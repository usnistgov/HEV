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
#ifdef _WIN32
# include <stdlib.h>
# define usleep(x)  _sleep((x)/1000)
# define sleep(x)   _sleep(x)
#else
# include <unistd.h>
# include <stdlib.h>
#endif

#include <string.h>
#include <stdio.h>

#define SMAX 256*256*256   // seconds
#define SMIN 0.01

void usage(void)
{
  printf("Usage: dtk-fsleep SECONDS\n"
         "\n SECONDS can be a float. The minimum SECONDS value is 0.01\n"
         "The maximum SECONDS value is %d.  This is a small\n"
	 "usleep (3) wrapper.\n",SMAX);
}


int main(int argc, char **argv)
{
  unsigned int t = 1;

  if(argc > 1 && strcmp("-h",argv[1]) &&
     strcmp("--help",argv[1]))
    {
      double num = atof(argv[1]);
      if(num > ((double) SMAX))
        {
          usage();
          return 1;
        }
      if(num >= 1.0)
        {
          t = (unsigned int) num;
          //printf("calling sleep(%d)\n",t);
          sleep(t);
          num = num - t;
          if(num < SMIN)
            return 0;
        }
      if(num >= SMIN)
        {
          t = (unsigned int) (num*1000000);
          //printf("calling usleep(%d)\n",t);
          usleep(t);
          return 0;
        }
      usage();
      return 1;
    }
  usage();
  return 0;
}
