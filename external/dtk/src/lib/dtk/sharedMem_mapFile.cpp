/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2004  Lance Arsenault
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



// Call this if the shared memory file changes size from another
// processes actions.  You need at least a read lock before calling
// this, if you can.  It's not a good idea to decrease the shared
// memory file size, that could cause other programs to crash.  The
// resize_flag should be set for the first process to call this so
// that the file size is changed of the UNIX version of this.

int dtkSharedMem::connect_mapFile(size_t size, int resize_flag)
{
#ifdef DTK_ARCH_WIN32_VCPP

  /* change the size of the memory that is mapped from the mapped file 
   * and keep the contents preserved.  The file_size value must be the
   * same as the value in the shared memory file.
   */
  SetLastError(0);
  HANDLE new_fileMappingHandle =
    CreateFileMapping((HANDLE) fd, NULL, PAGE_READWRITE,
		      0, size, NULL);
  if(!new_fileMappingHandle)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::*() failed:\n"
		 "CreateFileMapping(,,,,size=%u bytes,) failed "
		 "to get another mapping object from file \"%s\".\n"
		 "GetLastError()=%u.\n",
		 size,
		 fullPath,
		 GetLastError());
      return -1;
    }

  struct dtkSharedMem_header *new_header =
    (struct dtkSharedMem_header *)
    MapViewOfFile(new_fileMappingHandle, FILE_MAP_WRITE, 0, 0, size);
  if(!new_header)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::*() failed: "
		 "can't remap shared memory file \"%s\":\n"
		 "MapViewOfFile(,,,size=%d) failed: "
		 "GetLastError()=%d.\n", fullPath, size, GetLastError());
      CloseHandle(new_fileMappingHandle);
      return -1;
    }

  if(header)
    {
      if(!UnmapViewOfFile(header))
	// What do I do if this fails anyway...?
	dtkMsg.add(DTKMSG_WARN, 
		   "dtkSharedMem::*() failed: "
		   "can't unmap (to remap) shared memory file \"%s\":\n"
		   "UnmapViewOfFile() failed: "
		   "GetLastError=%d.\n", fullPath, GetLastError());
    }

  if(fileMappingHandle)
    if(!CloseHandle(fileMappingHandle))
      {
	dtkMsg.add(DTKMSG_WARN, 
		   "dtkSharedMem::*() failed: "
		   "can't CloseHandle (to remap) shared memory file \"%s\":\n"
		   "CloseHandle(fileMappingHandle) failed: "
		   "GetLastError=%d.\n", fullPath, GetLastError());
      }
  
  fileMappingHandle = new_fileMappingHandle;

#else /* #ifdef DTK_ARCH_WIN32_VCPP *//* UNIX */

#if 0
  /*************** map the file to memory ******************************/
  struct dtkSharedMem_header *new_header = (struct dtkSharedMem_header *)
      mmap(NULL, 1024*4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#else
  struct dtkSharedMem_header *new_header = (struct dtkSharedMem_header *)
      mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
#endif
  if(MAP_FAILED == (void *) new_header)
#ifdef DTK_ARCH_CYGWIN
  {
    char* buf = new char[size];
    bzero( buf, size );
    ::write( fd, buf, size );
    delete[] buf;
    new_header = (struct dtkSharedMem_header *)
        mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  }
  if(MAP_FAILED == (void *) new_header)
#endif
    {
      dtkMsg.add(DTKMSG_ERROR, 1, 
		 "dtkSharedMem::*() failed: "
		 "mmap(NULL, size=%u bytes, "
		 "PROT_READ|PROT_WRITE, MAP_SHARED, fd=%d, 0) failed:\n"
		 "to re-map shared memory file \"%s\"\n",
		 size, fd, fullPath);
      return -1;
    }

  if(resize_flag)
    // change the file size.
    if(ftruncate(fd, (off_t) size))
      return dtkMsg.add(DTKMSG_WARN, 1, -1, 
			"dtkSharedMem::*() failed: "
			"can't resize shared memory file \"%s\":\n"
			"ftruncate(%d, %d) failed.\n",
			fullPath, fd, size);

  if(header)
    if(munmap(header, file_size))
      dtkMsg.add(DTKMSG_WARN, 1,
		 "munmap(%p, %u) failed: for remapping file "
		 "\"%s\".\n",
		 header, file_size, fullPath);

  rwlock = &(new_header->rwlock);

#endif /* #else #ifdef DTK_ARCH_WIN32_VCPP */

  /* fix all the pointers that point to the shared memory and object data. */

#ifdef DTK_ARCH_IRIX
# if (_MIPS_SIM==_MIPS_SIM_ABI64)
  ptr += (u_int64_t) ((unsigned char *) new_header) - ((u_int64_t) header);
# else
  ptr += (u_int32_t) ((unsigned char *) new_header) - ((u_int32_t) header);
# endif
#else
# ifdef DTK_USE_64_BIT 
  ptr += (u_int64_t) ((unsigned char *) new_header) - ((u_int64_t) header);
# else
  ptr += (u_int32_t) ((unsigned char *) new_header) - ((u_int32_t) header);
# endif
#endif

  header = new_header;
  top = ((unsigned char *) header) +
    SIZE_PLUS(sizeof(struct dtkSharedMem_header));
  header->file_size = file_size = size;


  return 0; // success
}

