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

int main(int argc, char **argv)
{
  if(argc < 2)
    return dtkMsg.add(DTKMSG_ERROR,0,1,"Usage: %s SEG_NAME\n",
		      argv[0]);

  dtkSharedMem shm(sizeof(float)*6, argv[1]);
  if(shm.isInvalid()) return 1;

  //shm._memPrint();
  //shm.printWriteList();
  //return 0;

  char *addr[] = {"localhost", "bleen", "poo", "foo", NULL };
  char *nam[] = {"0", "1", "2", "3", NULL };

  char **address= addr;
  char **name = nam;
  while(*address)
    {
      shm.addToWriteList(*address,*name);
      printf("\n---------- shm.addToWriteList(\"%s\", \"%s\") -----------------\n\n",
	     *address,*name);
      //shm._memPrint();
      shm.printWriteList();
      address++;
      name++;
    }
  shm.removeFromWriteList("bleen");
  printf("\n---------- shm.removeFromWriteList(\"%s\") -----------------\n\n",
	"bleen");

  shm.printWriteList();


  shm.addToWriteList("PP","5");
  shm.printWriteList();
  shm.removeFromWriteList("localhost");

  shm.printWriteList();


  return 0;
}
