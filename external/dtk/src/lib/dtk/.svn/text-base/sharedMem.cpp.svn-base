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

#if defined DTK_ARCH_IRIX || defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
#  include <sys/socket.h>
#endif
 
#ifdef DTK_ARCH_CYGWIN
# include <netinet/ip.h>
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


#ifdef DTK_ARCH_WIN32_VCPP
unsigned int dtkSharedMem_byte_order = 0;
#else
# define INVALID_HANDLE_VALUE  -1
// Set the bit DTK_BYTE_ORDER if this is a BIG_ENDIAN (or net byte
// order) machine.
unsigned int dtkSharedMem_byte_order =
 ((BYTE_ORDER == BIG_ENDIAN)?DTK_BYTE_ORDER:0);
#endif

// May want to add env's DTK_SHAREDMEM_DIR_MODE and DTK_SHAREDMEM_FILE_MODE
// or like to set modes for creating directories and files.


dtkSharedMem::dtkSharedMem(size_t size, const char *filename,
			   void *initBuff, int flag)
  : dtkBase(DTKSHAREDMEM_TYPE)
{
  init(size, filename, initBuff, flag);
}

dtkSharedMem::dtkSharedMem(size_t size, const char *filename, int flag)
  : dtkBase(DTKSHAREDMEM_TYPE)
{
  init(size, filename, NULL, flag);
}

// just works for DTK_CONNECT
dtkSharedMem::dtkSharedMem(const char *filename, int do_connect_spew)
  : dtkBase(DTKSHAREDMEM_TYPE)
{
  init(DTKSHAREDMEM_NOSIZE, filename, NULL, DTK_CONNECT, do_connect_spew);      
}

void dtkSharedMem::cleanup(void)
{
  if(cleanUpFilters())
    dtkMsg.append("Failed to clean up DSO filters when "
		  "deleting dtkSharedMem object.\n");

  // If there are no more sharedMem objects in this process than
  // remove any sockets that may be do to the push write.
  // See: sharedMem_network.cpp.
  number_of_dtkSharedMem_objs--;
  checkRemoveSockets();

#ifdef DTK_ARCH_WIN32_VCPP

  if(rwlock)
  {
    free(rwlock);
    rwlock = NULL;
  }

  if(header)
  {
    UnmapViewOfFile(header);
    header = NULL;
  }

  if(fileMappingHandle)
  {
    CloseHandle(fileMappingHandle);
    fileMappingHandle = NULL;
  }

  if(fd != INVALID_HANDLE_VALUE)
  {
    CloseHandle(fd);
    fd = INVALID_HANDLE_VALUE;
  }

#else

  if(rwlock)
  {
    rwlock = NULL;
  }

  if(header)
    {
      if(munmap(header, file_size))
	dtkMsg.add(DTKMSG_WARN, 1,
		   "munmap(%p, %u) failed in deleting "
		   "a dtkSharedMem object for file "
		   "\"%s\".\n",
		   header, file_size, fullPath);
      header = NULL;
    }

   if(fd > INVALID_HANDLE_VALUE)
    {
      close(fd);
      fd = INVALID_HANDLE_VALUE;
    }

#endif


  if(addrList)
    {
      delete addrList;
      addrList = NULL;
    }
  if(fullPath)
    {
      free(fullPath);
      fullPath = NULL;
    }
  if(name)
    {
      free(name);
      name = NULL;
    }
  if(net_data_buf)
    {
      free(net_data_buf);
      net_data_buf = NULL;
    }
  if(filterList)
    {
      delete filterList;
      filterList = NULL;
    }
  if(filterDSOPath)
    {
      free(filterDSOPath);
      filterDSOPath = NULL;
    }

  invalidate();
}

dtkSharedMem::~dtkSharedMem(void)
{
  cleanup();
}

/* get_createOrConnect_type() sets up the following object data:
 *
 * fullPath
 * name
 *
 * get_createOrConnect_type() returns the 1 for if the shared memory is being created,
 * returns 0 if this is connecting, and returns -1 on error.
 */

int dtkSharedMem::get_createOrConnect_type(size_t size,
	                   const char *filename, void *initBuff,
					   int flag, int do_connect_spew)
{
  /********* We'll do this by looking for failure modes first ***********/

  if(size < (size_t) 1)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem"
		 "(size=%u, filename=\"%s\", %p, flag=%d) "
		 "failed: cannot have size < 1 byte.\n",
		 size, filename, initBuff, flag);
      return -1;
    }
  if(flag & DTK_CREATE && size == DTKSHAREDMEM_NOSIZE)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::dtkSharedMem"
		 "(size=%u, filename=\"%s\", %p, flag=%d) "
		 "failed: cannot have size==DTKSHAREDMEM_NOSIZE with "
		 "flag & DTK_CREATE.\n",
		 size, filename, initBuff, flag);
      return -1;
    }

  /************ construct name and fullPath **********************/

  {
    char *dtk_sharedMem_dir = getenv("DTK_SHAREDMEM_DIR");
    if(!dtk_sharedMem_dir)
      dtk_sharedMem_dir = const_cast<char*>(DTK_DEFAULT_SHAREDMEM_DIR);

    if(!filename || !(filename[0]))
      { // generate a fullPath
	char *pre = (char *)
	  dtk_malloc(strlen(dtk_sharedMem_dir) + strlen(DIR_SEP"dtkSharedMem") + 1);
	sprintf(pre, "%s"DIR_SEP"dtkSharedMem", dtk_sharedMem_dir);
	fullPath = dtk_tempname(pre);
	name = dtk_strdup(&(fullPath[strlen(dtk_sharedMem_dir)+1]));
	free(pre);
      }
    else if(!IS_FULL_PATH(filename))
      { // get the full path file name, fullPath
	fullPath = (char *)
	  dtk_malloc(strlen(dtk_sharedMem_dir) +
		     strlen(DIR_SEP) + strlen(filename) + 1);
	sprintf(fullPath, "%s"DIR_SEP"%s", dtk_sharedMem_dir, filename);
      }
    else // full path file name, fullPath was passed in filename
      fullPath = dtk_strdup(filename);
    
    if(!name)
      name = dtk_strdup(filename);
  }

  int create_flag = 0;

  /* check that the existance of the file is compatible with creating
     or connecting */
  struct stat s;
  if(!stat(fullPath, &s)) // fullPath does exist
    {
      // not a regular file
            
      if(!(S_ISREG(s.st_mode)))
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		     "failed: file \"%s\" exists and is not "
		     "a regular file.\n",
		     size, filename, initBuff, flag, fullPath);
	  cleanup();
	  return -1;
	}
      // Can't create if it exists.
      if(flag & DTK_CREATE && !(flag & DTK_CONNECT))
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)"
		     " failed: file \"%s\" exists.\n",
		     size, filename, initBuff, flag, fullPath);
	  cleanup();
	  return -1;
	}
      // Is it smaller than the user thinks it is.
      if(size != DTKSHAREDMEM_NOSIZE
	 &&
	 s.st_size <
	 (off_t) (SIZE_PLUS(sizeof(struct dtkSharedMem_header)) +
		  SIZE_PLUS(size)))
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		     "failed: file \"%s\" exists size=%d (<%d) "
		     "which is too small.\n",
		     size, filename, initBuff, flag,
		     fullPath,
		     s.st_size,
		     SIZE_PLUS(sizeof(struct dtkSharedMem_header)) +
		     SIZE_PLUS(size));
	  cleanup();
	  return -1;
	}
      
      // Is is just plan too small for any case.
      if(s.st_size < (off_t) SIZE_PLUS(sizeof(struct dtkSharedMem_header)))
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d)"
		     " failed: file \"%s\" is too small (%d bytes) "
		     "to connect to.\n",
		     size, filename, initBuff, flag, fullPath, s.st_size);
	  cleanup();
	  return -1;
	}
    } 
  else if(errno == ENOENT) // fullPath does not exist
    { 
      errno = 0; // reset errno
      if(!(flag & DTK_CREATE) && flag & DTK_CONNECT)
	{
	  if(do_connect_spew)
	    dtkMsg.add(DTKMSG_ERROR,
		       "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		       "failed: file \"%s\" does not exist.\n",
		       size, filename, initBuff, flag, fullPath);
	  cleanup();
	  return -1;
	}
      else
	create_flag = 1;
    }
  else
    {
      if(do_connect_spew)
	dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkSharedMem::dtkSharedMem(%d, \"%s\", %p, %d) "
		   "failed: stat(\"%s\", %p) failed.\n",
		   size, filename, initBuff, flag,
		   fullPath, &s);
      cleanup();
      return -1;
    }

  return create_flag;
}

void dtkSharedMem::init(size_t size, const char *filename,
			void *initBuff, int flag, int do_connect_spew)
{
  /***** Initialize the data first *******/
#ifdef DTK_ARCH_WIN32_VCPP
  fileMappingHandle = NULL;
#endif
  header = NULL;
  read_index = 1;
  file_size = 0;
  fd = INVALID_HANDLE_VALUE;
  name = NULL;
  fullPath = NULL;
  addrList = NULL;
  filterList = NULL;
  user_size = size;
  byte_swap_size = 0;
  rwlockState = IS_NOTLOCKED;
  address_change_count = 0;
  filter_change_count = 0;
  net_data_buf_size = 0;
  net_data_buf = NULL;
  byteOrderChar = (DTK_BYTE_ORDER & dtkSharedMem_byte_order)?
    _DTK_NET_BYTEORDER:_DTK_NONNET_BYTEORDER;
  rwlock = NULL;
  filterDSOPath = NULL;
  have_blocking_read_lock = 0;
  filter_flags = (ALLOW_READ_FILTER | ALLOW_WRITE_FILTER);
  number_of_dtkSharedMem_objs++;

  int create_flag =
    get_createOrConnect_type(size, filename, initBuff, flag, do_connect_spew);

  if(create_flag < 0)
    return; // error
  else if(create_flag)
    create(size, filename, initBuff, flag, do_connect_spew);
  else
    connect(size, filename, initBuff, flag, do_connect_spew);
}

