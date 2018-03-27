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

#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
# include <fcntl.h>
# include <sys/stat.h>
# include <io.h>
# include <memory.h>
# include <malloc.h>
# include <direct.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/time.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <unistd.h>
# include <wctype.h>
# include <sys/mman.h>
# include <sys/un.h>
# include <netinet/in.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
#  include <sys/endian.h>
   typedef uint64_t u_int64_t;
#endif

#if defined DTK_ARCH_IRIX || defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
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


void dtkSharedMem::connect(size_t size, const char *filename, void *initBuff,
			  int flag, int do_connect_spew)
{
#ifdef DTK_ARCH_WIN32_VCPP
  fd = CreateFile(fullPath, GENERIC_READ|GENERIC_WRITE,
		  FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY, 0);
  if(fd == INVALID_HANDLE_VALUE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		 "failed: CreateFile(\"%s\",,,,OPEN_EXISTING,,) "
		 "failed.\n"
		 "GetLastError()=%u.\n",
		 size, filename, initBuff, flag,
		 fullPath,
		 GetLastError());
      cleanup();
      return;
    }

  { /* check that the header is ok and get the file size from the
     * DTK shared memory header in the file */
    struct dtkSharedMem_header tmp_header;
    DWORD len = sizeof(struct dtkSharedMem_header), rlen;
    BOOL rtn = ReadFile(fd, &tmp_header, len, &rlen, NULL);
    if(!rtn || len != rlen)
      {
	dtkMsg.add(DTKMSG_ERROR,
		   "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		   "failed: ReadFile() failed reading file \"%s\":\n"
		   "GetLastError()=%u.\n",
		   size, filename, initBuff, flag,
		   fullPath,
		   GetLastError());
	cleanup();
	return;
      }

    // This checks that this is a DTK shared memory file.
    if(tmp_header.magic_number != DTKSHAREDMEM_HEADER)
      {
	dtkMsg.add(DTKMSG_ERROR,
		   "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) failed: "
		   "bad magic number in file \"%s\".\n",
		   size, filename, initBuff, flag, fullPath);
	cleanup();
	return;
      }

    if(SetFilePointer(fd, (DWORD) 0, NULL, FILE_BEGIN) != (DWORD) 0)
      {
	dtkMsg.add(DTKMSG_ERROR,
		   "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		   "failed: SetFilePointer() failed for file \"%s\": "
		   "GetLastError()=%u.\n",
		   size, filename, initBuff, flag,
		   fullPath,
		   GetLastError());
	cleanup();
	return;
      }

    file_size = tmp_header.file_size;

    /* for Windoz we use a local piece of memory for the rwlock */
    rwlock = (struct dtkRWLock *) dtk_malloc(sizeof(struct dtkRWLock));
    /* create or connect to the Windoz shared memory mapping object */
    
    /* We need to get the name of the Windoz semaphore from the file. */
    memcpy(rwlock, &(tmp_header.rwlock), sizeof(struct dtkRWLock));
  }

#else /* #ifdef DTK_ARCH_WIN32_VCPP *//* UNIX */

  fd = open(fullPath, O_RDWR);
  if(fd < 0)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		 "failed: open(\"%s\", O_RDWR) failed.\n",
		 size, filename, initBuff, flag, fullPath);
      cleanup();
      return;
    }

  { /* check that the header is ok and get the file size from the
     * DTK shared memory header in the file */

    struct dtkSharedMem_header tmp_header;
     size_t len = sizeof(struct dtkSharedMem_header);
     errno = 0;
     ssize_t rlen = ::read(fd, &tmp_header, len);
    if((size_t) rlen != len)
      {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		   "failed: read() failed to read %u bytes from file \"%s\".\n",
		   size, filename, initBuff, flag,
		   len, fullPath);
	cleanup();
	return;
      }

    // This checks that this is a DTK shared memory file.
    if(tmp_header.magic_number != DTKSHAREDMEM_HEADER)
      {
	dtkMsg.add(DTKMSG_ERROR,
		   "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) failed: "
		   "bad magic number in file \"%s\".\n",
		   size, filename, initBuff, flag, fullPath);
	cleanup();
	return;
      }

    if(lseek(fd, SEEK_SET, 0))
      {
	dtkMsg.add(DTKMSG_ERROR, 1,
		   "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		   "failed: lseek() failed for file \"%s\".\n",
		   size, filename, initBuff, flag,
		   fullPath);
	cleanup();
	return;
      }

    file_size = tmp_header.file_size;
  }

#endif /* #else #ifdef DTK_ARCH_WIN32_VCPP */

  if(connect_mapFile(file_size))
    {
      dtkMsg.append("dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		 "failed.\n",
		 size, filename, initBuff, flag);
      cleanup();
      return;
    }

/* On UNIX we use the data in shared memory for the read/write lock */
#ifndef DTK_ARCH_WIN32_VCPP
  rwlock = &(header->rwlock);
#endif


  /*************** finish initializing *******************************/
  
  if(dtkRWLock_connect(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		 "failed.\n",
		 size, filename, initBuff, flag);
      cleanup();
      return;
    }
  
  if(dtkRWLock_rlock(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		 "failed.\n",
		 size, filename, initBuff, flag);
      cleanup();
      return;
    }
  
  if(size != header->user_size && size != DTKSHAREDMEM_NOSIZE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) failed:\n"
		 "this DTK shared memory \"%s\" "
		 "is of size %u and not the requested size "
		 "of %u.\n",
		 size, filename, initBuff, flag,
		 fullPath,
		 header->user_size, size);
      dtkRWLock_runlock(rwlock);
      cleanup();
      return;
    }
  
  flags = header->flags;
  user_size = header->user_size;
  queue_length = header->queue_length;
  
  if(dtkRWLock_runlock(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		 "failed.\n",
		 size, filename, initBuff, flag);
      cleanup();
      return;
    }
  
  // for create or connect
  
  top = ((unsigned char *) header) +
    SIZE_PLUS(sizeof(struct dtkSharedMem_header));
  
  entry_size =
    ((flags & COUNTING) ? COUNTING_PSIZE : 0)
    + ((flags & TIMING)   ? TIMING_PSIZE   : 0)
    + ((flags & QUEUING)  ? QUEUING_PSIZE  : 0)
    + SIZE_PLUS(sizeof(u_int8_t)) // flags
    + SIZE_PLUS(user_size);
  
  /***** initialize data after making methods read() and write() usable *****/

  validate();

  if(initBuff && read(initBuff))
    {
      dtkMsg.add(DTKMSG_ERROR, "dtkSharedMem::dtkSharedMem"
		 "(%d, \"%s\", %p, %d) failed.\n",
		 size, filename, initBuff, flag);
      cleanup();
      return;
    }

  dtkMsg.add(DTKMSG_DEBUG,
	     "connected to DTK shared memory \"%s%s%s\""
	     " of size %s%d%s.\n",
	     dtkMsg.color.tur, fullPath, dtkMsg.color.end,
	     dtkMsg.color.grn, user_size, dtkMsg.color.end);
}
