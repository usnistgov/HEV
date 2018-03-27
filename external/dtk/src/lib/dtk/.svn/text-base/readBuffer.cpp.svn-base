/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <io.h>
#else
# include <unistd.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkReadBuffer.h"


dtkReadBuffer::dtkReadBuffer(int fileDescriptor, size_t minRecordSize_in,
			     size_t bufferSize_in,
			     size_t (*checkFunc)(unsigned char *buffer, size_t size))
  : dtkBase(DTKREADBUFFER_TYPE)
{
  buffer = NULL;

  bufferSize = (bufferSize_in > ((size_t) 32)) ? bufferSize_in : ((size_t) 32);
  if(bufferSize < minRecordSize_in*4)
    bufferSize = minRecordSize_in*4;

  if(bufferSize > bufferSize_in)
    {
      dtkMsg.add(DTKMSG_ERROR, "dtkReadBuffer::dtkReadBuffer():"
		 "the requested read buffer size, %d,"
		 " was too small: try setting read "
		 "buffer size to %d or larger.\n",
		 bufferSize_in, bufferSize);
      return;
    }

  buffer = (unsigned char *) malloc(bufferSize);
  ptr = end = buffer;
  fd = fileDescriptor;
  minRecordSize = minRecordSize_in;
  callback = checkFunc;
  validate();
}

dtkReadBuffer::~dtkReadBuffer(void)
{
  if(buffer)
    {
      free(buffer);
      buffer = NULL;
    }
}

int dtkReadBuffer::read(void)
{
  if(end >  buffer + 3 * bufferSize / 4)
    // wrap buffer
    {
      memmove(buffer, ptr, (size_t) (end - ptr));
      end = buffer + (size_t) (end - ptr);
      ptr = buffer;
    }

  ssize_t sizeRead = ::read(fd, end, bufferSize - (end - buffer));

#if 0
  printf("dtkReadBuffer::read() size= %d buffer=\n",(int) end + sizeRead - (int) ptr);
  for(unsigned char *p = ptr; p < end + sizeRead; p++)
    printf("0%3.3o ",*p);
  printf("\n");
#endif

  if(sizeRead > 0)
    {
      for(end += sizeRead; ptr + minRecordSize <= end;)
	{
	  size_t s = callback(ptr, (size_t) ((size_t) end - (size_t) ptr));
	  if(s) ptr += s;
	  else ptr++;
	}
      return 0;
    }
  else if(sizeRead < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1, "dtkReadBuffer::read(): "
		 "read() failed.\n");
      return -1;
    }
  else // sizeRead == 0
    {
      dtkMsg.add(DTKMSG_WARN, "dtkReadBuffer::read(): "
		 "read() returned 0.\n");
      return -1;
    }
}
