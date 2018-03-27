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

#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
# include <memory.h>
# include <malloc.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/time.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <unistd.h>
# include <wctype.h>
# include <errno.h>
# include <sys/mman.h>
# include <sys/un.h>
# include <netinet/in.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
#  include <strings.h>
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


int dtkSharedMem::writeCount(int truth)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::writeCount(%d) failed: "
		 "invalid dtkSharedMem object.\n",
		 truth);
      return -1;
    }

  if(dtkRWLock_wlock(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::writeCount(%d) failed.\n",
		 truth);
      return -1;
    }

  int return_val = 0;
  if(_writeCount(truth)) return_val = -1;

  dtkRWLock_wunlock(rwlock);
  return return_val;
}

// call this with a wlock
int dtkSharedMem::_writeCount(int truth)
{
  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkMsg.append("dtkSharedMem::writeCount(%d) failed.\n",
		   truth);
	   return -1;
	}

  if(truth)
    {
      if(!(flags & COUNTING))
	counter = 0;
      flags |= COUNTING;
    }
  else
    flags &= ~COUNTING;

  if(flags != header->flags)
    {
      int i = recreate();
      if(i)
	{
	  dtkMsg.append("dtkSharedMem::writeCount(%d) failed.\n",
			truth);
	  return i;
	}
    }
  return 0;
}


int dtkSharedMem::timeStamp(int truth)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::timeStamp(%d) failed: "
		 "invalid dtkSharedMem object.\n",
		 truth);
      return -1;
    }
  if(dtkRWLock_wlock(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::timeStamp(%d) failed.\n",
		 truth);
      return -1;
    }

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkRWLock_wunlock(rwlock);
	   dtkMsg.append("dtkSharedMem::timeStamp(%d) failed.\n",
		   truth);
	   cleanup();
	   return -1;
	}

  if(truth)
    {
      if(!(flags & TIMING))
	bzero(&timeVal, sizeof(struct timeval));
      flags |= TIMING;
    }
  else
    flags &= ~TIMING;

  if(flags != header->flags)
    {
      int i = recreate();
      if(i)
	{
	  dtkMsg.append("dtkSharedMem::timeStamp(%d) failed.\n",
			truth);
	  dtkRWLock_wunlock(rwlock);
	  if(i < -1)
	    cleanup();
	  return i;
	}
    }

  dtkRWLock_wunlock(rwlock);

  return 0;
}

// You need a write lock to call this.
int dtkSharedMem::_queue(int queue_length_in)
{
  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkRWLock_wunlock(rwlock);
	   dtkMsg.append("dtkSharedMem::queue(%d) failed.\n",
		   queue_length_in);
	   cleanup();
	   return -1;
	}

  if(queue_length_in < 2 && (header->flags & QUEUING))
    {
      flags &= ~QUEUING;
      queue_length = 1;
    }
  else
    {
      queue_length = (queue_length_in > (int) header->queue_length)?
	queue_length_in: (int) (header->queue_length);
      flags |= QUEUING;
    }

  if(header->queue_length != queue_length)
    {
      int i = recreate();
      if(i)
	{
	  dtkMsg.append("dtkSharedMem::queue(%d) failed.\n",
			queue_length_in);
	  dtkRWLock_wunlock(rwlock);
	  if(i < -1)
	    cleanup();
	  return i;
	}
    }

  return 0;
}

// Returns non-zero on error, -2 for fatal error and -1 for not so bad
// error.
int dtkSharedMem::queue(int queue_length_in)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::queue(%d) failed: "
		 "invalid dtkSharedMem object.\n",
		 queue_length_in);
      return -1;
    }
  if(queue_length_in > 65535) // 65535 is the largest u_int16_t.
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::queue(%d) failed: "
		 "the max queue length is %d, "
		 "%d was requested.\n",
		 queue_length_in, 65535,
		 queue_length_in);
      return -1;
    }

  if(dtkRWLock_wlock(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::queue(%d) failed.\n",
		 queue_length_in);
      return -1;
    }

  int return_val = _queue(queue_length_in);
  if(return_val) return return_val;

  dtkRWLock_wunlock(rwlock);

  return 0;
}

// recreate() uses flags and resize/recreate this.  Get a write lock
// before and release a write lock after calling this.  Returns
// non-zero on error, -1 for fatal error and -2 for not so bad error.
int dtkSharedMem::recreate(void)
{
  // get a copy of all the old data related to this shared memory.
  struct dtkSharedMem_header *old_header = (struct dtkSharedMem_header *)
    dtk_malloc(header->file_size);

  memcpy(old_header, header, header->file_size);

  u_int32_t old_entry_size = entry_size;

  { // check and fix the file size
    u_int32_t new_entry_size =
        ((flags & COUNTING) ? COUNTING_PSIZE : 0)
      + ((flags & TIMING)   ? TIMING_PSIZE   : 0)
      + ((flags & QUEUING)  ? QUEUING_PSIZE  : 0)
      + SIZE_PLUS(sizeof(u_int8_t)) // flags
      + SIZE_PLUS(user_size);

    size_t new_file_size = header->file_size +
      new_entry_size * queue_length -
      old_entry_size * old_header->queue_length;

    if(new_file_size != (size_t) old_header->file_size)
      {
	if(connect_mapFile(new_file_size, 1))
	  {
	    dtkMsg.append("dtkSharedMem::recreate() failed.\n");
	    free(old_header);
	    flags = header->flags;
	    queue_length = header->queue_length;
	    return -1;
	  }
      }

    header->flags = flags;
    header->queue_length = queue_length;
    entry_size = new_entry_size;
    if(header->write_list_offset)
      header->write_list_offset +=
	new_entry_size * queue_length -
	old_entry_size * old_header->queue_length;
    if(header->filter_list_offset)
      header->filter_list_offset +=
	new_entry_size * queue_length -
	old_entry_size * old_header->queue_length;
  }

  // Copy the entries from the old queue to the new, while keeping
  // the same queue write index value in the new queue as is in
  // the old queue, so that reconnecting objects will not lose
  // events.  Start writing at the current write index which is
  // the same in the old queue as the new, and move back in the
  // queue to where there is no data in the old queue and write
  // zeros in the addition entries (if any) in the new queue.
  // This will work even if the queue gets smaller.

  u_int8_t *old_entree_ptr_min = (u_int8_t *) old_header +
    SIZE_PLUS(sizeof(struct dtkSharedMem_header));

  u_int8_t *old_entree_ptr = old_entree_ptr_min +
    ((old_header->write_index)%(old_header->queue_length))*old_entry_size;

  u_int8_t *entree_ptr_min = (u_int8_t *) header +
    SIZE_PLUS(sizeof(struct dtkSharedMem_header));

  u_int8_t *entree_ptr = entree_ptr_min +
    ((header->write_index)%(queue_length))*entry_size;

  u_int16_t i;
  // Fill in the queue one entree at a time.  Write from the old
  // queue to the new.
  for(i=0; i<queue_length; i++)
    {
      if(i >= old_header->queue_length)
	{
	  // Write zero if old queue is smaller than the new
	  // queue.
	  bzero(entree_ptr, entry_size);
	}
      else
	{
	  if(flags & COUNTING)
	    {
	      u_int64_t count;
	      if(old_header->flags & COUNTING)
		memcpy(&count, old_entree_ptr, sizeof(u_int64_t));
	      else
		count = 0;
	      memcpy(entree_ptr, &count, sizeof(u_int64_t));
	    }
	  if(flags & TIMING)
	    {
	      struct timeval t = {0,0};
	      if(old_header->flags & TIMING)
		memcpy(&t, old_entree_ptr +
		       ((old_header->flags & COUNTING)?SIZE_PLUS(sizeof(u_int64_t)):0), // counter
		       sizeof(struct timeval));

	      memcpy(entree_ptr +
		     ((flags & COUNTING)?SIZE_PLUS(sizeof(u_int64_t)):0), // counter
		     &t, sizeof(struct timeval));
	    }
	  if(flags & QUEUING)
	    {
	      u_int64_t queue_count;
	      if(old_header->flags & QUEUING)
		memcpy(&queue_count,
		       old_entree_ptr +
		       (((old_header->flags & COUNTING)?SIZE_PLUS(sizeof(u_int64_t)):0) +   // counter
			((old_header->flags & TIMING)?SIZE_PLUS(sizeof(struct timeval)):0)), // time_stamp
		       sizeof(u_int64_t));
	      else
		queue_count = 0;
	      
	      // queue count
	      memcpy(entree_ptr +
		     (((flags & COUNTING)?SIZE_PLUS(sizeof(u_int64_t)):0) +  // counter
		      ((flags & TIMING)?SIZE_PLUS(sizeof(struct timeval)):0)), // time_stamp
		     &queue_count, sizeof(u_int64_t));
	    }

	  // flags + user_data
	  memcpy(entree_ptr + (
		   ((flags & COUNTING) ? COUNTING_PSIZE : 0)
		 + ((flags & TIMING)   ? TIMING_PSIZE   : 0)
		 + ((flags & QUEUING)  ? QUEUING_PSIZE  : 0)),
		 old_entree_ptr + (
		   ((old_header->flags & COUNTING) ? COUNTING_PSIZE : 0)
		 + ((old_header->flags & TIMING)   ? TIMING_PSIZE   : 0)
		 + ((old_header->flags & QUEUING)  ? QUEUING_PSIZE  : 0)),
		 SIZE_PLUS(sizeof(u_int8_t)) + SIZE_PLUS(user_size));
	}
      
      // Backup the pointers one entry in both queues.
      old_entree_ptr -= old_entry_size;

      // check for queue wrapping.
      if(old_entree_ptr < old_entree_ptr_min)
	old_entree_ptr = old_entree_ptr_min +
	  (old_header->queue_length - 1)*old_entry_size;
      
      entree_ptr -= entry_size;
      
      // check for queue wrapping.
      if(entree_ptr < entree_ptr_min)
	entree_ptr = entree_ptr_min +
	  (queue_length - 1)*entry_size;
    }


#if 0
  // Fix read_index to be at non-zero entrees.
  // This will make the number entrees in the queue be less than
  // of equal to the old queue length.
  if(header->write_index > (u_int64_t) old_header->queue_length &&
     queue_length > old_header->queue_length &&
     read_index + old_header->queue_length < header->write_index + 1)
    read_index = header->write_index + 1 - old_header->queue_length;
#else
  // flush the queue for the object that made the request
  read_index = header->write_index + 1;
  
#endif

  /* copy all the memory after the user (entree) data, where the write
   * lists are kept.  */

  if(header->file_size > (u_int32_t) (SIZE_PLUS(sizeof(dtkSharedMem_header)) +
				      queue_length*entry_size))
    memcpy(top + queue_length*entry_size,
	   ((char *) old_header) +
	   (SIZE_PLUS(sizeof(dtkSharedMem_header)) +
	    old_header->queue_length * old_entry_size),
	   (size_t) (header->file_size -
		     SIZE_PLUS(sizeof(dtkSharedMem_header)) -
		     queue_length*entry_size));

  free(old_header);

  dtkMsg.add(DTKMSG_DEBUG,
	     "DTK shared memory \"%s\" of size %d is now: %s, %s, and %s\n",
	     fullPath, user_size,
	     flags & COUNTING ? "counted": "not counted",
	     flags & TIMING   ? "time stamped": "not time stamped",
	     flags & QUEUING  ? "queued.": "not queued.");

  return 0;
}

// Get a read lock before and release a read lock after calling this.
int dtkSharedMem::reconnect(void)
{
  /* change the size of the memory that is mapped in the mapped file */
  if(file_size !=  header->file_size)
    if(connect_mapFile(header->file_size))
      {
	dtkMsg.append("dtkSharedMem::reconnect() failed.\n");
	return -1;
      }

  flags = header->flags;
  u_int16_t old_queue_length = queue_length;
  queue_length = header->queue_length;

  // Fix read_index to be at non-zero entrees.  This will make the
  // number entrees in the queue be less than of equal to the old
  // queue length.
  if(header->write_index > (u_int64_t) old_queue_length &&
     queue_length > old_queue_length &&
     read_index + old_queue_length < header->write_index + 1)
    read_index = header->write_index + 1 - old_queue_length;

   entry_size =
      ((flags & COUNTING) ? COUNTING_PSIZE : 0)
    + ((flags & TIMING)   ? TIMING_PSIZE   : 0)
    + ((flags & QUEUING)  ? QUEUING_PSIZE  : 0)
    + SIZE_PLUS(sizeof(u_int8_t)) // flags
    + SIZE_PLUS(user_size);

   return 0;
}
