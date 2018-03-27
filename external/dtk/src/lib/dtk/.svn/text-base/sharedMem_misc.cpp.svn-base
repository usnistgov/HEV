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

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>


#ifdef DTK_ARCH_WIN32_VCPP
#  include <winbase.h>
#  include <winsock2.h> // for struct timeval
#  include <malloc.h>
#  include <memory.h>
#  include <direct.h>
#  include <io.h>
#define _WIN32_WINNT 0x0500
#  include <sddl.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
#  include <sys/time.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <sys/un.h>
#  include <netinet/in.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
   typedef uint64_t u_int64_t;
#endif

#if defined DTK_ARCH_DARWIN || defined DTK_ARCH_IRIX || defined DTK_ARCH_CYGWIN
#  include <sys/socket.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkFilter.h"
#include "filterList.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkRWLock.h"
#include "dtkConditional.h"
#include "dtkSegAddrList.h"
#include "dtkSocket.h"
#include "dtkUDPSocket.h"
#include "dtkSocketList.h"
#include "dtkSharedMem.h"

#include "_private_sharedMem.h"

#ifdef DTK_ARCH_WIN32_VCPP
inline static int mkdir(const char *f, unsigned int m)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = false;
	char* szSD = TEXT("D:")
	TEXT("(A;OICI;GA;;;BA)")
	TEXT("(A;OICI;GA;;;AU)");
	PULONG nSize = 0;

	if( !ConvertStringSecurityDescriptorToSecurityDescriptor(szSD,
		SDDL_REVISION_1, &(sa.lpSecurityDescriptor), nSize) )
	{
		DWORD err = GetLastError();
		return dtkMsg.add( DTKMSG_ERROR, 1, -1,
			"mkdir(\"%s\") failed with error %d\n", err );
	}

	return !CreateDirectory( f, &sa );
}
#endif


int dtkSharedMem::differentByteOrder(void) const
{
  return different_byte_order;
}

size_t dtkSharedMem::getSize(void) const
{
  if(isValid()) return user_size;
  else return (size_t) 0; // error
}

// This makes the directory and parent directories as needed by using
// recursion.
int dtk_recMkdir(const char *dir, mode_t mode)
{

  if(!IS_FULL_PATH(dir))
    return -1; // error

  if(!mkdir(dir, mode))
    {
#ifndef DTK_ARCH_WIN32_VCPP
      chmod(dir, mode);
#endif
      return dtkMsg.add(DTKMSG_DEBUG, 0, 0,
			"made directory \"%s\".\n", dir);
    }

  char *parent_dir = dtk_strdup(dir);
  char *s = parent_dir + strlen(parent_dir);
  for(;*s;s++);

  for(;*s != DIR_CHAR;s--);
  *s = '\0';

  if(!IS_FULL_PATH(parent_dir))
    {
      free(parent_dir);
      return -1; // error
    }

  struct stat st;
  int i = stat(parent_dir,&st);
  if(i && errno == ENOENT && !dtk_recMkdir(parent_dir, mode))
    // parent_dir does not exist and I made it by recursing.
    {
      free(parent_dir);
      if(!mkdir(dir, mode))
	  {
#ifndef DTK_ARCH_WIN32_VCPP
	    chmod(dir, mode);
#endif
	    return dtkMsg.add(DTKMSG_DEBUG, 0, 0,
			    "made directory \"%s\".\n", dir);
	  }
      else
		return -1;
  }

  free(parent_dir);
  return -1; // error
}

// getTimeStamp() gets the Time Stamp from the last qread() or read().
int dtkSharedMem::getTimeStamp(struct timeval *t) const
{
  if(isInvalid()) return -1; // error

  if(!(flags & TIMING))
    {
      dtkMsg.add(DTKMSG_WARN,"dtkSharedMem::getTimeStamp() failed:"
		 " this shared memory is not time stamped."
		 " Call dtkSharedMem::timeStamp().\n");
      return -1;
    }

  memcpy(t, &timeVal, sizeof(struct timeval));
  return 0;
}

long double dtkSharedMem::getTimeStamp(void) const
{
  if(isInvalid()) return LDBL_MAX; // error
  if(!(flags & TIMING))
    {
      dtkMsg.add(DTKMSG_WARN,"dtkSharedMem::getTimeStamp() failed:"
		 " this shared memory is not time stamped."
		 " Call dtkSharedMem::timeStamp().\n");
      return LDBL_MAX;
    }

  return ((long double) timeVal.tv_sec +
	  (long double) timeVal.tv_usec * 1.0e-6);
}

u_int64_t dtkSharedMem::getWriteCount(void) const
{
  if(isInvalid()) return (u_int64_t) -1; // error
  if(!(flags & COUNTING))
    {
      dtkMsg.add(DTKMSG_WARN,"dtkSharedMem::getWriteCount() failed:"
		 " this shared memory is not write counted."
		 " Call dtkSharedMem::writeCount().\n");
      return (u_int64_t) -1;
    }

  return counter;
}

int dtkSharedMem::isQueued(void)
{
  if(isInvalid()) return 0;

   if(dtkRWLock_rlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::isQueued() failed.\n");
    }

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkRWLock_runlock(rwlock);
	   dtkMsg.append("dtkSharedMem::isQueued() failed.\n");
	   cleanup();
	   return -1;
	}

  dtkRWLock_runlock(rwlock);

  return (flags & QUEUING)?1:0;
}

int dtkSharedMem::isCounted(void)
{
  if(isInvalid()) return 0;

   if(dtkRWLock_rlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::isCounted() failed.\n");
    }

   if(flags != header->flags || queue_length != header->queue_length)
     if(reconnect())
       {
	 dtkRWLock_runlock(rwlock);
	 dtkMsg.append("dtkSharedMem::isQueued() failed.\n");
	 cleanup();
	 return -1;
       }
   
  dtkRWLock_runlock(rwlock);

  return (flags & COUNTING)?1:0;
}

int dtkSharedMem::isTimeStamped(void)
{
  if(isInvalid()) return 0;

   if(dtkRWLock_rlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::isTimeStamped() failed.\n");
    }

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkRWLock_runlock(rwlock);
	   dtkMsg.append("dtkSharedMem::isQueued() failed.\n");
	   cleanup();
	   return -1;
	}

  dtkRWLock_runlock(rwlock);

  return (flags & TIMING)?1:0;
}


char *dtkSharedMem::getName(void) const
{
  return fullPath;
}

char *dtkSharedMem::getShortName(void) const
{
  return name;
}

int dtkSharedMem::flush(void)
{
  if(isInvalid()) return -1; // error
  if(dtkRWLock_rlock(rwlock))
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::flush() failed.\n");

  // Check if the queue wrapped.
  if(header->write_index >= read_index + queue_length)
    // Unwrap the queue.
    read_index = header->write_index + 1 - queue_length;

  int num = (int) (header->write_index + 1 - read_index); // get number in queue
  read_index = header->write_index + 1; // flush

  dtkRWLock_runlock(rwlock);

  return num;
}

// Determine the full path of a DTK shared memory file if it
// exists or not.  Don't mess with the memory returned.
char *dtkSharedMem_getFullPathName(const char *filename)
{
  static char *fullPath = NULL;
  static size_t size = 0;

  if(!filename || filename[0] == '\0')
    return NULL;

  char *dtk_sharedMem_dir = getenv("DTK_SHAREDMEM_DIR");
  if(!dtk_sharedMem_dir)
    dtk_sharedMem_dir = const_cast<char*>(DTK_DEFAULT_SHAREDMEM_DIR);

  if(!IS_FULL_PATH(filename))
    { // get the full path file name, fullPath
      size_t new_size = strlen(dtk_sharedMem_dir) +
	strlen(DIR_SEP) + strlen(filename) + 1;
      if(new_size > size)
	fullPath = (char *) dtk_realloc(fullPath, size=new_size);
      sprintf(fullPath, "%s%s%s",
		  dtk_sharedMem_dir, DIR_SEP, filename);
    }
  else // full path file name, fullPath was passed in filename
    {
      size_t new_size = strlen(filename) + 1;
      if(new_size > size)
	fullPath = (char *) dtk_realloc(fullPath, size=new_size);
      strcpy(fullPath, filename);
    }
  return fullPath;
}


void dtkSharedMem::setAutomaticByteSwapping(size_t element_size)
{
  byte_swap_size = element_size;
}
