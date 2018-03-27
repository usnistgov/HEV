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
/* char *dtkSharedMem::_alloc(size_t size) and
 * int dtkSharedMem::_free(void *pt) are used to manage the shared
 * memory at the end of the mmaped() file, the file footer.
 *
 * This managed memory is used by things that change size, like the
 * remote address write lists.  Relative offsets should be used in
 * shared memory rather than pointers, since the absolute addresses
 * may be different in different processes.  */

#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/time.h>
# include <unistd.h>
# include <sys/un.h>
# include <netinet/in.h>
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
#include "dtkRWLock.h"
#include "dtkConditional.h"
#include "dtkSegAddrList.h"
#include "dtkSocket.h"
#include "dtkUDPSocket.h"
#include "dtkSocketList.h"
#include "dtkSharedMem.h"

#include "_private_sharedMem.h"

// This struct is put before each shared memory segment.
struct _mem
{
  u_int32_t free_size; // This free_size is the size of memory available
  // if it is free, or zero if not.
  u_int32_t size; // The size of usable, "user", memory.
};



// You must call a write lock before and write unlock after _alloc()
// and _free().  This is to manage memory after the user entry data,
// where the write lists are kept and (maybe) other stuff.
char *dtkSharedMem::_alloc(size_t size)
{
  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
	   dtkMsg.append("dtkSharedMem::_alloc(%d) failed.\n",
		   size);
	   return NULL;
	}


  size = SIZE_PLUS(size);
  u_int32_t add_size = SIZE_PLUS(sizeof(struct _mem)) + size;

  struct _mem *p = (struct _mem *) (top + entry_size*queue_length);
  struct _mem *end = (struct _mem *) (((char *) header) +
				      header->file_size);

  /************ first check to see if there is free memory available ******/

  while(p < end)
    {
      if(p->free_size >= (u_int32_t) size)
	{
	  // I'll use this memory.
	  p->free_size = 0; // mark it as not free.
	  if(p->size > add_size)
	    {
	      // Split this piece of memory into two pieces.
	      struct _mem *q = (struct _mem *) (((char *) p) + add_size);
	      q->free_size = q->size = p->size - add_size;
	      p->size = (u_int32_t) size;
	    }
	  /****** return this memory ********/
	  return (char *) (((char *) p) + SIZE_PLUS(sizeof(struct _mem)));
	}
      else
	p = (struct _mem *)(((char *) p) +
			    (p->size + SIZE_PLUS(sizeof(struct _mem))));
    }  

  /*** allocate some memory at the end of the file ***/

  u_int32_t new_file_size = header->file_size + add_size;

  if(connect_mapFile(new_file_size, 1))
    {
      dtkMsg.append("dtkSharedMem::_alloc(%d) failed:\n",
		    new_file_size);
      return NULL;
    }

  p = (struct _mem *) (((char *) header) + (file_size - add_size));
  p->size = size;
  p->free_size = 0; // It's not free.

  return ((char *) p) + SIZE_PLUS(sizeof(struct _mem));
}

/* This will insure that there will never be to adjacent free memory
 * chunks.
 */
int dtkSharedMem::_free(void *pt)
{
  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
      {
	dtkMsg.append("dtkSharedMem::_free() failed.\n");
	return -1;
      }


  struct _mem *p = (struct _mem *) (top + entry_size*queue_length);
  struct _mem *end = (struct _mem *) (((u_int8_t *) header) +
				      header->file_size);

  if(pt > end || pt < p)
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkSharedMem::_free(%p) failed, "
		      "%p is out of range.\n", pt, pt);

  struct _mem *before = NULL;
  struct _mem *x = (struct _mem *) (((char *) pt) -
				    SIZE_PLUS(sizeof(struct _mem)));

  while(p < x)
    {
      struct _mem *a = (struct _mem *)
	(((char *) p) + (SIZE_PLUS(sizeof(struct _mem)) + p->size));
      if(p->free_size && x == a)
	before = p;
      p = a;
    }

  struct _mem *after = NULL;

  if(p != x) // memory was over written.
    return dtkMsg.add(DTKMSG_WARN,0,-1,
		      "dtkSharedMem::_free(%p) failed, "
		      "memory was corrupted.\n", pt);

  p = (struct _mem *)(((char *) p) +
		      (SIZE_PLUS(sizeof(struct _mem)) + p->size));

  if(p < end && p->free_size)
    after = p;

  u_int32_t free_size = 0;
  if(before)
    free_size += before->free_size + SIZE_PLUS(sizeof(struct _mem));
  free_size += x->size;
  if(after)
    free_size += after->free_size + SIZE_PLUS(sizeof(struct _mem));

  if(before)
    {
      before->free_size = before->size = free_size;
      x = before;
    }
  else
    x->free_size = x->size = free_size;

  if(((char *) x) + (SIZE_PLUS(sizeof(struct _mem)) + free_size) ==
     ((char *) header) + header->file_size)
    {
      // Remove the free memory that is at the end of the file.
      u_int32_t new_file_size = header->file_size - 
	(SIZE_PLUS(sizeof(struct _mem)) + free_size);

      if(connect_mapFile(new_file_size, 1))
	{
	  dtkMsg.append("dtkSharedMem::_free(%p) failed:\n",
			pt);
	  return -1;
	}
    }

  return 0;
}

// for debugging.
int dtkSharedMem::_memPrint(FILE *file)
{
  file = (file) ? ((FILE *) file) : stdout;
  dtkColor c(file);

  if(dtkRWLock_rlock(rwlock))
    return dtkMsg.add(DTKMSG_ERROR, 0, -1, 
		      "dtkSharedMem::dtkSharedMem::_memPrint(%p) failed.\n",
		      file);

  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
	   dtkMsg.append("dtkSharedMem::_memPrint(%p) failed.\n",
		   file);
	   return -1;
	}

  if(header->file_size > (u_int32_t) (SIZE_PLUS(sizeof(dtkSharedMem_header)) +
				      queue_length*entry_size))
    fprintf(file, "%s%d bytes of footer out of %d bytes total "
	    "for shared memory "
	    "file%s \"%s%s%s\"\n",
	    c.grn, 
	    header->file_size - SIZE_PLUS(sizeof(dtkSharedMem_header)) -
	    queue_length*entry_size,
	    header->file_size,
	    c.end, c.red, fullPath, c.end);
  else
    {
      dtkRWLock_runlock(rwlock);
      fprintf(file, "%sThere is no footer for shared "
	      "memory file%s \"%s%s%s\"\n",
	      c.grn, c.end, c.red, fullPath, c.end);
      return 0;
    }

  unsigned char *p = top + entry_size*queue_length;
  unsigned char *end = ((unsigned char *) header) + header->file_size;

  while(p < end)
    {
      struct _mem *a = (struct _mem *) p;

      fprintf(file, "address %s%lu%s size %d free_size = %d\n",
	      c.grn, (unsigned long) (p + SIZE_PLUS(sizeof(struct _mem))),
	      c.end, a->size, a->free_size);
      p += a->size + SIZE_PLUS(sizeof(struct _mem));
    }

  dtkRWLock_runlock(rwlock);

  return 0;
}

