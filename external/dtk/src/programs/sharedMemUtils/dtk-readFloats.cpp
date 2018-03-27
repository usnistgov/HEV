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
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <unistd.h>
#endif

#ifndef HEADER_FILE
# include "floatsOptions.h"
#else
# include HEADER_FILE
#endif

extern int running;
extern int Usage(void);
extern int parse_args(int argc, char **argv, char **name,
		      int **field, int *number,
		      char *endchar, int *count,
		      int *queue, int *time);
extern void sig_catcher(int sig);


int main(int argc, char **argv)
{
  if(argc < 2)
    return Usage();

  int number=0, *field = NULL;
  char endchar = '\n', *name=NULL;
  int count = 0, queue = 0, time = 0;

  dtkMsg.setPreMessage("%s%s%s", dtkMsg.color.vil,
		       PROGRAM_NAME, dtkMsg.color.end);

  if(parse_args(argc, argv, &name, &field, &number,
		&endchar, &count, &queue, &time))
    return 1;


#if(0)
  {
    printf("got name=%s number=%d endchar=<%d>"
	   " count=%d queue=%d time=%d\n",
	   name, number, (int) endchar,
	   count, queue, time);
    if(field)
      {
	printf("printing just fields: ");
	for(int *i=field; *i>-1;i++)
	  printf("%d ",*i);
	printf("\n");
      }
    return 0;
  }
#endif

  dtkSharedMem shm(name);
  if(shm.isInvalid()) return 1;
  if(count && shm.writeCount()) return 1;
  if(time && shm.timeStamp()) return 1;
  if(queue && shm.queue()) return 1;

  int num_types = (int) shm.getSize()/sizeof(TYPE);

  if(num_types < 1)
    return dtkMsg.add(DTKMSG_ERROR,0,1,
		      "shared memory named \"%s\" has"
		      " a size that is less than a %s\n",
		      name, TYPE_NAME);

  if(field)
    for(int *j=field;*j > -1;j++)
      if(num_types <= *j)
	return
	  dtkMsg.add(DTKMSG_ERROR,0,1,
		     "field number %d is not present"
		     " in DTK shared memory \"%s\".\n",
		     *j, name);


  TYPE *val = (TYPE *) dtk_malloc((num_types + 1)*sizeof(TYPE));
  if(val == NULL) return 1;

  signal(SIGINT, sig_catcher);
#ifndef DTK_ARCH_WIN32_VCPP
  signal(SIGQUIT, sig_catcher);
#endif
  signal(SIGTERM, sig_catcher);

  // If your reading from a remote machine
  // (from the writer machine) with a different byte order.
  if(sizeof(TYPE) > (size_t) 1)
    shm.setAutomaticByteSwapping(sizeof(TYPE));

  int i;

  for(i=0;running && ((!number) || i < number);i++)
    {
      int k;
      if(queue)
	k = shm.qread(val); // a polling read
      else
	k = shm.read(val); // a polling read

      if(k < 0) return 1; // error

      if(!queue || k)
	{

	  if(count)
	    printf("%lld ",shm.getWriteCount());
	  
	  if(time)
	    printf("%.21Lg ", shm.getTimeStamp());
	  
#ifdef BUTTONS
	  if(field)  // print just the fields in field[] (bit).
	    {
	      for(int *j=field;*j > -1;j++)
		{
		  printf("%d ", (01 << (*j)%8) & val[*j/8]);
		}
	      printf(" %c",endchar);
	    }
	  else
	    {
	      int l;
	      for(l=0;l<num_types;l++)
		{
		  int j;
		  for(j=0;j<4;j++)
		    printf("%d", ((01<<(j%8)) & val[l])?1:0);
		  printf(" ");
		  for(;j<8;j++)
		    printf("%d", ((01<<(j%8)) & val[l])?1:0);
		  printf("   ");
		}
	      printf("    %c",endchar);
	    }
#else
	  if(field)  // print just the fields in field[] (bit).
	    {
	      for(int *j=field;*j > -1;j++)
		printf(FORMAT,val[*j]);
	      printf(" %c",endchar);
	    }
	  else
	    {
	      int j;
	      for(j=0;j<num_types;j++)
		printf(FORMAT,val[j]);
	      printf("  %c",endchar);
	    }
#endif

	  if(endchar == '\r')
	    fflush(stdout);
	}

      if(queue && k<2) // queued and got data to read
	usleep(20000);
      else if(!queue) // not queued
	usleep(100000);
    }
  return 0;
}
