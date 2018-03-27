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
#include "serverProtocol.h"


void dtkSharedMem::create(size_t size, const char *filename, void *initBuff,
			  int flag, int do_connect_spew)
{
  /************** check and/or create the directory part of the file ********/
  
  char *dir = dtk_strdup(fullPath);
  char *str = dir;
  for(;*str;str++);
  for(;*str != DIR_CHAR;str--);
  *str = '\0';
  struct stat s;
  int i = stat(dir,&s);
  if(i && errno == ENOENT)
    {
#ifdef DTK_ARCH_WIN32_VCPP
      if(dtk_recMkdir(dir, (mode_t) _S_IREAD | _S_IWRITE))
#else
      if(dtk_recMkdir(dir, (mode_t) 0777))
#endif
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)"
		     " failed: failed to make directory \"%s\".\n",
		     size, filename, initBuff, flag, dir);
	  errno = 0; // reset errno
	  free(dir);
	  cleanup();
	  return;
	}
    }
  else if(i)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedMem::dtkSharedMem"
		 "(%d, \"%s\", %p, %d) failed:\n"
		 "looking for the directory part of \"%s\": "
		 "stat(\"%s\", %p) failed.\n",
		 size, filename, initBuff, flag, fullPath, dir, &s);
      errno = 0; // reset errno
      free(dir);
      cleanup();
      return;
    }
  else if(!(S_ISDIR(s.st_mode)))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem"
		 "(%d, \"%s\", %p, %d) failed:\n"
		 "\"%s\" is not a directory.\n",
		 size, filename, initBuff, flag, dir);
      free(dir);
      cleanup();
      return;
    }
  free(dir);

  size_t new_file_size = // size without timing counting or queuing yet.
    SIZE_PLUS(sizeof(struct dtkSharedMem_header))
    + SIZE_PLUS(sizeof(u_int8_t)) // flags
    + SIZE_PLUS(user_size);

#ifdef DTK_ARCH_WIN32_VCPP
  fd = CreateFile(fullPath, GENERIC_READ|GENERIC_WRITE|FILE_ALL_ACCESS,
		  FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,
		  NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY, 0);
  if(fd == INVALID_HANDLE_VALUE)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		 "failed: CreateFile(\"%s\",,,) "
		 "failed.\n"
		 "GetLastError()=%u.\n",
		 size, filename, initBuff, flag, fullPath,
		 GetLastError());
      cleanup();
      return;
    }


  /* for Windoz we use a local piece of memory for the rwlock */
  rwlock = (struct dtkRWLock *) dtk_malloc(sizeof(struct dtkRWLock));

#else /* #ifdef DTK_ARCH_WIN32_VCPP *//******** UNIX *********/

#ifdef DTK_ARCH_CYGWIN
  fd = open(fullPath, O_RDWR|O_CREAT, (mode_t) 0666);
  chmod(fullPath, (mode_t) 0666 ); 
#else
  fd = open(fullPath, O_RDWR|O_CREAT|O_TRUNC, (mode_t) 00);
#endif
  if(fd < 0)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		 "failed: open(\"%s\",O_RDWR|O_CREAT|O_TRUNC, 0%o) "
		 "failed.\n",
		 size, filename, initBuff, flag, fullPath, 00);
      cleanup();
      return;
    }

#endif /* #else #ifdef DTK_ARCH_WIN32_VCPP */

  if(connect_mapFile(new_file_size, 1))
    {
      dtkMsg.append("dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)\n"
		    "failed: open(\"%s\",O_RDWR|O_CREAT|O_TRUNC, 0%o) "
		    "failed.\n",
		    size, filename, initBuff, flag, fullPath, 00);
      cleanup();
      return;
    }

  header->magic_number = DTKSHAREDMEM_HEADER;
  header->blocking_read_lock.magic_number = 0;
  header->write_index = 0;
  header->counter = 0;
  header->address_change_count = address_change_count;
  header->user_size = user_size;
  header->queue_length = queue_length = 1;
  header->flags = flags = 0;
  header->write_list_offset = 0;
  
  if(dtkRWLock_create(rwlock))
    {
      unlink(fullPath);
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

  if(initBuff && write(initBuff))
    {
      dtkRWLock_destroy(rwlock);
      unlink(fullPath);
      dtkMsg.append("dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		    "failed.\n",
		    size, filename, initBuff, flag);
      cleanup();
      return;
    }

#ifdef DTK_ARCH_WIN32_VCPP
  
      // Now make the file accessable by other processes.
  if(_chmod(fullPath, _S_IREAD | _S_IWRITE))
    {
      dtkRWLock_destroy(rwlock);
      unlink(fullPath);
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) failed:\n"
		 "_chmod(\"%s\", 0%o) failed.\n",
		 size, filename, initBuff, flag,
		 fullPath, _S_IREAD | _S_IWRITE);
      cleanup();
      return;
    }
  
  memcpy(&(header->rwlock), rwlock, sizeof(struct dtkRWLock));
  
#else /* #ifdef DTK_ARCH_WIN32_VCPP *//* UNIX */
  
      // Now make the file accessable by other processes.
  if(fchmod(fd, DTKSHAREDMEM_FILE_MODE))
    {
      dtkRWLock_destroy(rwlock);
      unlink(fullPath);
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) failed:\n"
		 "chmod(%d, 0%o) failed for file \"%s\".\n",
		 size, filename, initBuff, flag,
		 fd, DTKSHAREDMEM_FILE_MODE, fullPath);
      cleanup();
      return;
    }
  
#endif /* #else #ifndef DTK_ARCH_WIN32_VCPP */
  
  dtkMsg.add(DTKMSG_DEBUG,
	     "     created DTK shared memory \"%s%s%s\""
	     " of size %s%d%s.\n",
	     dtkMsg.color.tur, fullPath, dtkMsg.color.end,
	     dtkMsg.color.grn, user_size, dtkMsg.color.end);
}
