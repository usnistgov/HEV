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

// remove this
#include <stdlib.h>

#include <string.h>

#ifdef DTK_ARCH_IRIX
#  include <strings.h>
   typedef uint64_t u_int64_t;
#endif


#ifdef DTK_ARCH_WIN32_VCPP
#  include <winbase.h>
#  include <winsock2.h> // for struct timeval
#  include <memory.h>
#  include <malloc.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
#  include <sys/time.h>
#  include <sys/un.h>
#  include <netinet/in.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#if defined DTK_ARCH_IRIX || defined DTK_ARCH_DARWIN || defined DTK_ARCH_CYGWIN
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
#include "utils.h"
#include "dtkAugment.h"

#include "_private_sharedMem.h"

#ifdef DTK_ARCH_WIN32_VCPP
// from  gettimeofday.cpp
extern int gettimeofday(struct timeval *t, void *p=NULL);
#endif


int dtkSharedMem::rlock(void)
{
  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::rlock() failed: "
		      "invalid dtkSharedMem object.\n");

  if(rwlockState)
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkSharedMem::rlock() failed: "
		      "This object has a %s lock already.\n",
		      (rwlockState == IS_RLOCKED)?"read":"write");

  if(dtkRWLock_rlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::rlock() failed.\n");
    }

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkRWLock_runlock(rwlock);
	   dtkMsg.append("dtkSharedMem::rlock() failed.\n");
	   cleanup();
	   return -1;
	}

  rwlockState = IS_RLOCKED;

  return 0;
}

int dtkSharedMem::runlock(void)
{
  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::runlock() failed: "
		      "invalid dtkSharedMem object.\n");

  if(rwlockState != IS_RLOCKED)
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkSharedMem::runlock() failed: "
		      "This object has no read lock.\n");

  if(dtkRWLock_runlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::runlock() failed.\n");
    }

  rwlockState = IS_NOTLOCKED;

  return 0;
}

int dtkSharedMem::wlock(const struct timeval *timeStamp)
{
  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::wlock() failed: "
		      "invalid dtkSharedMem object.\n");

  if(rwlockState)
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkSharedMem::rlock() failed: "
		      "This object has a %s lock already.\n",
		      (rwlockState == IS_RLOCKED)?"read":"write");

  if(dtkRWLock_wlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::wlock() failed.\n");
    }

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
	   dtkRWLock_wunlock(rwlock);
	   dtkMsg.append("dtkSharedMem::wlock() failed.\n");
	   cleanup();
	   return -1;
	}

  rwlockState = IS_WLOCKED;

  if(flags & QUEUING)
    (header->write_index)++;

  ptr = top + (((header->write_index)%(header->queue_length))*entry_size);

  if(flags & COUNTING)
    {
      header->counter++;
      memcpy(ptr, &(header->counter), sizeof(u_int64_t));
      ptr += COUNTING_PSIZE; // counter
    }
  if(flags & TIMING)
    {
      if(!timeStamp)
	{
	  struct timeval t;
	  gettimeofday(&t,NULL);
	  // write time stamp
	  memcpy(ptr, &t, sizeof(struct timeval));
	}
      else // user provided time stamp
	memcpy(ptr, timeStamp, sizeof(struct timeval));

      ptr += TIMING_PSIZE; // time stamp
    }
  if(flags & QUEUING)
    {
      // Need to add the reading of the queue count from the "dot
      // header" file, write it to this queue, and than increment it.
      ptr += QUEUING_PSIZE; // queue count
    }

  // write the byte order
#ifdef DTK_ARCH_WIN32_VCPP
  memset(ptr, 0, sizeof(u_int8_t));
#else
  bzero(ptr, sizeof(u_int8_t));
#endif

  *ptr |= dtkSharedMem_byte_order;

  ptr += SIZE_PLUS(sizeof(u_int8_t)); // flags

  return 0;
}

// Here's where DTK shared memory can become remote shared memory.
int dtkSharedMem::wunlock(int isLocal)
{
  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::wunlock() failed: "
		      "invalid dtkSharedMem object.\n");

  if(rwlockState != IS_WLOCKED)
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkSharedMem::wunlock() failed: "
		      "This object has no write lock.\n");

  int write_network = 0;

  if(!isLocal && header->address_change_count)
    {
      if(!addrList)
	addrList = new dtkSegAddrList(fullPath);

      /**** this is how to update the write list ********************/
      if(address_change_count != header->address_change_count)
	{
	  sync_addrList();
	  if(addrList->start())
	    realloc_net_data_buf();
	}
      if(addrList->start())
	{
	  // ptr should have been set in to point to the current
	  // entries user data dtkSharedMem::_write().
	  memcpy(&(net_data_buf[net_data_buf_size - user_size]),
		 ptr, user_size);
	  write_network = 1;
	}
    }

  if(dtkRWLock_wunlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::wunlock() failed.\n");
    }

  rwlockState = IS_NOTLOCKED;
  
  // DTK shared memory using a "push write" for the networked sharing
  // of shared memory contents.
  if(write_network)
    _write_network();

  return 0;
}

int dtkSharedMem::blockingQRead(void *buf, size_t nbyte, size_t offset)
{
  if(isInvalid()) return -1; // error

  int wasLocked = rwlockState;
  if(wasLocked == IS_RLOCKED && dtkRWLock_runlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingQRead() failed.\n");
      return -1;
    }

  // even though this is a read like thing we need to start with a
  // write lock so that we can safely write the dtkConditional struct
  // and the flag bit BLOCKING_RD to shared memory.  So we need to
  // first write some signal like data and then we'll use read locks
  // when we read.
  if(wasLocked != IS_WLOCKED && dtkRWLock_wlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingQRead() failed.\n");
      return -1;
    }

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
      {
	dtkRWLock_wunlock(rwlock);
	dtkMsg.append("dtkSharedMem::blockingQRead() failed.\n");
	cleanup();
	return -1;
      }

  int return_val = 0;

  // make sure it's queued
  if(!(flags & QUEUING) && _queue())
    // shared memory well.
    {
      dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
      if(wasLocked != IS_WLOCKED) dtkRWLock_wunlock(rwlock);
      if(wasLocked == IS_RLOCKED) dtkRWLock_rlock(rwlock);
      return -1;
    }

  // check the queue if the queue has data to read in it.
  if(((int) (header->write_index - read_index + 1)) > 0)
    {
      if(wasLocked != IS_WLOCKED && dtkRWLock_wunlock(rwlock))
	{
	  invalidate();
	  dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
	  return -1;
	}
      // put the read lock back if the user had one before this method
      // call.
      if(wasLocked == IS_RLOCKED && dtkRWLock_rlock(rwlock))
	{
	  invalidate();
	  dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
	  return -1;
	}
 
      // No need to block (wait).
      return qRead(buf, nbyte, offset);
    }

  if(!(header->flags & BLOCKING_RD))
    {
      flags |= BLOCKING_RD;
      header->flags |= BLOCKING_RD;
    }
  
  // get an check the blocking/signaling dtkConditional thingy.
  if(!have_blocking_read_lock  && !return_val)
    {
      if(header->blocking_read_lock.magic_number != DTKCONDITIONAL_TYPE)
	{
	  if(dtkConditional_create(&(header->blocking_read_lock)))
	    {
	      dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
	      return_val = -1;
	    }
	  else
	    have_blocking_read_lock = 1;
	}
      else if(dtkConditional_connect(&(header->blocking_read_lock)))
	{
	  dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
	  return_val = -1;
	}
      else
	have_blocking_read_lock = 1;
    }

  if(dtkRWLock_wunlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
      return -1;
    }

  if(return_val) return return_val;

  // This will block as until dtkSharedMem::write() is called by
  // another process (or thread, but I don't thing DTK is thread
  // safe).  Note: this will get hosed up if the shared memory is
  // destroyed while this is blocking.  I need a read lock since the
  // blocking_read_lock is in shared memory.

  // Do I need a read lock to call this?  But we cannot because that
  // would block the writing process and would give you a dead lock.
  if(dtkConditional_wait(&(header->blocking_read_lock)))
    {
      dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
      return -1;
    }

  // If the caller was dumb enough to have a write lock before this
  // call put it back.
  if(wasLocked == IS_WLOCKED && dtkRWLock_wlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingQRead() failed\n");
      return -1;
    }

  // put the read lock back if the user had one before this method
  // call.
  if(wasLocked == IS_RLOCKED && dtkRWLock_rlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::blockingQRead() failed.\n");
    }

  return qRead(buf, nbyte, offset);
}
// qPeek() returns the number of events that are in the queue
// qPeek() returns -1 on error.
int dtkSharedMem::qPeek(void *buf, size_t nbyte, size_t offset)
{
  if(isInvalid()) return -1; // error

  if(!(flags & QUEUING)) // if it's not queuing yet queue it now.
    if(queue())
      {
	dtkMsg.append("dtkSharedMem::qread() failed to setup queuing.\n");
	return -1;
      }

  if(nbyte == DTK_ALL || offset>user_size || nbyte>user_size)
    {
      nbyte = user_size;
      offset = 0;
    }
  else if(nbyte>user_size-offset)
    nbyte = user_size-offset;

  int doLock = (!rwlockState);
  if(doLock) if(rlock()) return -1; // error

  // no entry
  if(header->write_index < read_index)
    {
      if(doLock) if(runlock()) return -1; // error
      return 0; // no entries
    }

  // Check if the queue wrapped.
  if(header->write_index >= read_index + queue_length)
    read_index = header->write_index + 1 - queue_length;

  int return_val = (int) (header->write_index - read_index + 1);

  ptr = top + (read_index%queue_length)*entry_size;

  if(flags & COUNTING)
    {
      memcpy(&counter, ptr, sizeof(u_int64_t));
      ptr += COUNTING_PSIZE; // counter
    }
  if(flags & TIMING)
    {
      memcpy(&timeVal, ptr, sizeof(struct timeval));
      ptr += TIMING_PSIZE; // time stamp
    }
      
  memcpy(&queue_count, ptr, sizeof(u_int64_t));
  ptr += SIZE_PLUS(sizeof(u_int64_t)); // queue count

  different_byte_order = ( (unsigned int) (*ptr & DTK_BYTE_ORDER) !=
			   (unsigned int) dtkSharedMem_byte_order );
  ptr += SIZE_PLUS(sizeof(u_int8_t)); // flags


  memcpy(buf, ptr + offset, nbyte);

  //read_index++;

  if(doLock) if(runlock()) return -1;// error

  // swap bytes if the user set this to using
  // setAutomaticByteSwapping() and it needs to.
  if(different_byte_order && byte_swap_size)
    for(offset = 0;offset + byte_swap_size <= nbyte;
	offset += byte_swap_size)
      dtk_swapBytes(((char *) buf) + offset, byte_swap_size);

  return return_val;
}

// qread() returns the number of events that are in the queue before
// the qread() call.  qread() returns -1 on error.
int dtkSharedMem::qRead(void *buf, size_t nbyte, size_t offset)
{
  if(isInvalid()) return -1; // error

  if(!(flags & QUEUING)) // if it's not queuing yet queue it now.
    if(queue())
      {
	dtkMsg.append("dtkSharedMem::qread() failed to setup queuing.\n");
	return -1;
      }

  if(nbyte == DTK_ALL || offset>user_size || nbyte>user_size)
    {
      nbyte = user_size;
      offset = 0;
    }
  else if(nbyte>user_size-offset)
    nbyte = user_size-offset;

  int doLock = (!rwlockState);
  if(doLock) if(rlock()) return -1; // error

  // no entry
  if(header->write_index < read_index)
    {
      if(doLock) if(runlock()) return -1; // error
      return 0; // no entries
    }

  // Check if the queue wrapped.
  if(header->write_index >= read_index + queue_length)
    read_index = header->write_index + 1 - queue_length;

  int return_val = (int) (header->write_index - read_index + 1);

  ptr = top + (read_index%queue_length)*entry_size;

  if(flags & COUNTING)
    {
      memcpy(&counter, ptr, sizeof(u_int64_t));
      ptr += COUNTING_PSIZE; // counter
    }
  if(flags & TIMING)
    {
      memcpy(&timeVal, ptr, sizeof(struct timeval));
      ptr += TIMING_PSIZE; // time stamp
    }
      
  memcpy(&queue_count, ptr, sizeof(u_int64_t));
  ptr += SIZE_PLUS(sizeof(u_int64_t)); // queue count

  different_byte_order = ( (unsigned int) (*ptr & DTK_BYTE_ORDER) !=
			   (unsigned int) dtkSharedMem_byte_order );
  ptr += SIZE_PLUS(sizeof(u_int8_t)); // flags


  memcpy(buf, ptr + offset, nbyte);

  read_index++;

  if(doLock) if(runlock()) return -1;// error

  // swap bytes if the user set this to using
  // setAutomaticByteSwapping() and it needs to.
  if(different_byte_order && byte_swap_size)
    for(offset = 0;offset + byte_swap_size <= nbyte;
	offset += byte_swap_size)
      dtk_swapBytes(((char *) buf) + offset, byte_swap_size);

  return return_val;
}


int dtkSharedMem::filter_read(void *buf, size_t nbyte, size_t offset)
{
  if(filter_flags & IS_READ_FILTER_QUEUED)
    {
      int j= qread(buf, nbyte, offset);
      // If there is no data in the queue than poll read.
      if(j == 0)
	if(!_read(buf, nbyte, offset))
	  j = 1;
      while(j > 0)
	{
	  cycle_filtersRead(buf, nbyte, offset);
	  j= qread(buf, nbyte, offset);
	}
      return j; // 0 for success -1 for failure
    }
  else
    {
      if(_read(buf, nbyte, offset)) return -1;
      cycle_filtersRead(buf, nbyte, offset);
    }

  return 0;
}


void dtkSharedMem::cycle_filtersRead(void *buf, size_t nbyte, size_t offset)
{
  // Call the filter callbacks in the order that they there loaded.
  dtkFilter *f = filterList->start();
  while(f)
    {
      if(filterList->checkRead())
	{
	  int i = f->read(buf, nbyte, offset, different_byte_order,
			  &timeVal, counter);
	  if(i == dtkFilter::ERROR_ || i == dtkFilter::REMOVE)
	    {
	      if(i == dtkFilter::ERROR_)
		dtkMsg.add(DTKMSG_WARN,
			   "dtkSharedMem::*read() filter callback failed.\n");
	      filterList->removeRead();
	      // Need to reset the filter_flags.
	      filter_flags &= ~IS_READ_FILTER_QUEUED;
	      filter_flags &= ~IS_READ_FILTER;
	      dtkFilter *filter = filterList->start();
	      while(filter)
		{
		  if(filterList->checkRead())
		    {
		      filter_flags |= IS_READ_FILTER;
		      if(filter->needQueued())
			{
			  filter_flags |= IS_READ_FILTER_QUEUED;
			  // We just need to know that there's at
			  // least one queue requisting read filter.
			  break;
			}
		    }
		  filter = filterList->next();
		}
	      // put the filterList back onto the same filter.
	      filter = filterList->start();
	      while(filter != f && filter)
		filter = filterList->next();
	    }
	}
      f = filterList->next();
    }
}


// This is called by read() and filter_read().
int dtkSharedMem::_read(void *buf, size_t nbyte, size_t offset)
{
  int doLock = (!rwlockState);
  if(doLock) if(rlock()) return -1; // error

  int filter_change = 0;

  if(doLock && filter_flags & ALLOW_READ_FILTER &&
     filter_change_count != header->filter_change_count)
    {
      sync_filterList();
      filter_change = 1;
    }

  ptr = top + ((header->write_index)%(header->queue_length))*entry_size;

  if(flags & COUNTING)
    {
      memcpy(&counter, ptr, sizeof(u_int64_t));
      ptr += COUNTING_PSIZE; // counter
    }
  if(flags & TIMING)
    {
      memcpy(&timeVal, ptr, sizeof(struct timeval));
      ptr += TIMING_PSIZE; // time stamp
    }

  if(flags & QUEUING)
    ptr += QUEUING_PSIZE; // queue count

  different_byte_order = ( (unsigned int) (*ptr & DTK_BYTE_ORDER) !=
			   (unsigned int) dtkSharedMem_byte_order );

  ptr += SIZE_PLUS(sizeof(u_int8_t)); // flags

  memcpy(buf, ptr + offset, nbyte);

  if(doLock) if(runlock()) return -1; // error

  if(filter_change)
    unloadAndLoad_filterList();

  // swap bytes if the user set this to using
  // setAutomaticByteSwapping() and it needs to.
  if(different_byte_order && byte_swap_size)
    for(offset = 0;offset + byte_swap_size <= nbyte;
	offset += byte_swap_size)
      dtk_swapBytes(((char *) buf) + offset, byte_swap_size);

  return 0;
}

int dtkSharedMem::blockingRead(void *buf, size_t nbyte, size_t offset)
{
  if(isInvalid()) return -1; // error

  int wasLocked = rwlockState;
  if(wasLocked == IS_RLOCKED && dtkRWLock_runlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingRead() failed.\n");
      return -1;
    }

  // even though this is a read like thing we need to start with a
  // write lock so that we can safely write the dtkConditional struct
  // and the flag bit BLOCKING_RD to shared memory.  So we need to
  // first write some signal like data and then we'll use read locks
  // when we read.
  if(wasLocked != IS_WLOCKED && dtkRWLock_wlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingRead() failed.\n");
      return -1;
    }
  
  int return_val = 0;
  
  if(_writeCount(1)) // make sure it's write counted and connected to
    // shared memory well.
    {
      dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
      if(wasLocked != IS_WLOCKED) dtkRWLock_wunlock(rwlock);
      if(wasLocked == IS_RLOCKED) dtkRWLock_rlock(rwlock);
      return -1;
    }

  // check the write count that will be next with the one last read.
  u_int64_t shm_counter_val;
  memcpy(&shm_counter_val,
	 top + ((header->write_index)%(header->queue_length))*entry_size,
	 sizeof(u_int64_t));
  if(counter != shm_counter_val)
    {
      if(wasLocked != IS_WLOCKED && dtkRWLock_wunlock(rwlock))
	{
	  invalidate();
	  dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
	  return -1;
	}
      // put the read lock back if the user had one before this method
      // call.
      if(wasLocked == IS_RLOCKED && dtkRWLock_rlock(rwlock))
	{
	  invalidate();
	  dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
	  return -1;
	}

      // No need to block (wait).
      return read(buf, nbyte, offset);
    }

  if(!(header->flags & BLOCKING_RD))
    {
      flags |= BLOCKING_RD;
      header->flags |= BLOCKING_RD;
    }
  
  // get an check the blocking/signaling dtkConditional thingy.
  if(!have_blocking_read_lock  && !return_val)
    {
      if(header->blocking_read_lock.magic_number != DTKCONDITIONAL_TYPE)
	{
	  if(dtkConditional_create(&(header->blocking_read_lock)))
	    {
	      dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
	      return_val = -1;
	    }
	  else
	    have_blocking_read_lock = 1;
	}
      else if(dtkConditional_connect(&(header->blocking_read_lock)))
	{
	  dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
	  return_val = -1;
	}
      else
	have_blocking_read_lock = 1;
    }

  if(dtkRWLock_wunlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
      return -1;
    }

  if(return_val) return return_val;

  // This will block as until dtkSharedMem::write() is called by
  // another process (or thread, but I don't thing DTK is thread
  // safe).  Note: this will get hosed up if the shared memory is
  // destroyed while this is blocking.  I need a read lock since the
  // blocking_read_lock is in shared memory.

  // Do I need a read lock to call this?  But we cannot because that
  // would block the writing process and would give you a dead lock.
  if(dtkConditional_wait(&(header->blocking_read_lock)))
    {
      dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
      return -1;
    }

  // If the caller was dumb enough to have a write lock before this
  // call put it back.
  if(wasLocked == IS_WLOCKED && dtkRWLock_wlock(rwlock))
    {
      invalidate();
      dtkMsg.append("dtkSharedMem::blockingRead() failed\n");
      return -1;
    }

  // put the read lock back if the user had one before this method
  // call.
  if(wasLocked == IS_RLOCKED && dtkRWLock_rlock(rwlock))
    {
      invalidate();
      return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			"dtkSharedMem::blockingRead() failed.\n");
    }

  return read(buf, nbyte, offset);
}


int dtkSharedMem::read(void *buf, size_t nbyte, size_t offset)
{
  if(isInvalid()) return -1; // error
  if(nbyte == DTK_ALL || offset>user_size || nbyte>user_size)
    {
      nbyte = user_size;
      offset = 0;
    }
  else if(nbyte > user_size-offset)
    nbyte = user_size-offset;

  // If there are usable filters than let the filters change the
  // returned data.  Do not, if there is a lock held so that the
  // process has no chance of getting hung.
  if(!rwlockState && filter_flags & IS_READ_FILTER && 
     filter_flags & ALLOW_READ_FILTER)
    return filter_read(buf, nbyte, offset);

  return _read(buf, nbyte, offset);
}

int dtkSharedMem::write(const void *buf, size_t bytes, size_t offset)
{
  return _write(buf, NULL, bytes, offset);
}

int dtkSharedMem::write(const void *buf, const struct timeval *timeStamp,
		      size_t nbyte, size_t offset)
{
  return _write(buf, timeStamp, nbyte, offset);
}

int dtkSharedMem::write(const void *buf, long double time,
		      size_t nbyte, size_t offset)
{
  struct timeval t =
  {
    ((long) time), ((long)((time - ((long) time))*1.0e+6))
  };

  return _write(buf, &t, nbyte, offset);
}

// This gets a little hairy if the user calls wlock() on their own.
int dtkSharedMem::_write(const void *buf, const struct timeval *timeStamp,
			 size_t nbyte, size_t offset)
{
  if(isInvalid()) return -1; // error

  if(nbyte == DTK_ALL || offset>user_size || nbyte>user_size)
    {
      nbyte = user_size;
      offset = 0;
    }
  else if(nbyte>user_size-offset)
    nbyte = user_size-offset;

  struct timeval t;
  if(!rwlockState && filter_flags & IS_WRITE_FILTER &&
     filter_flags & ALLOW_WRITE_FILTER )
    {
      if(filter_flags & IS_WRITE_FILTER_TIMESTAMPED)
	{
	  if(timeStamp) memcpy(&t, timeStamp, sizeof(struct timeval));
	  else gettimeofday(&t,NULL);
	  buf = cycle_filtersWrite(buf, &t, nbyte, offset);
	  timeStamp = &t;
	}
      else
	buf = cycle_filtersWrite(buf, NULL, nbyte, offset);
    }

  int doLock = (!rwlockState);
  if(doLock) if(wlock(timeStamp)) return -1; // error

  int filter_change = 0;
  if(doLock && filter_flags & ALLOW_WRITE_FILTER &&
     filter_change_count != header->filter_change_count)
    {
      sync_filterList();
      filter_change = 1;
    }

  // ptr got set in wlock().
  memcpy(ptr + offset, buf, nbyte);

  int return_val = 0;

  // check for blocking readers.
  if(header->flags & BLOCKING_RD)
    {
      // unmark the flags: local object flags and shared header->flags
      flags &= ~BLOCKING_RD;
      header->flags &= ~BLOCKING_RD;

      // get an check the dtkConditional thingy.
      if(!have_blocking_read_lock)
	{
	  if(header->blocking_read_lock.magic_number != DTKCONDITIONAL_TYPE)
	    {
	      // This should not happen seeing that the blockingRead()
	      // should have already created this.
	      if(dtkConditional_create(&(header->blocking_read_lock)))
		{
		  dtkMsg.append("dtkSharedMem::write() failed\n");
		  return_val = -1;
		}
	      else
		have_blocking_read_lock = 1;
	    }
	  else if(dtkConditional_connect(&(header->blocking_read_lock)))
	    {
	      dtkMsg.append("dtkSharedMem::write() failed\n");
	      return_val = -1;
	    }
	  else
	    have_blocking_read_lock = 1;
	}

      if(!return_val && dtkConditional_signal(&(header->blocking_read_lock)))
	{
	  dtkMsg.append("dtkSharedMem::write() failed\n");
	  return_val = -1;
	}
    }

  if(doLock) if(wunlock()) return_val = -1; // error

  if(filter_change)
    unloadAndLoad_filterList();

  return return_val;
}

// Used by DTK server to write and set the byte order as shared memory
// is written to to the DTK server from the network.  I need to figure
// out how to deal with timestamps across the network.  Some day when
// I have time. :)
int dtkSharedMem::writeLocal(const void *buf, int net_byte_order)
{
  if(isInvalid()) return -1; // error

  struct timeval *tv_ptr = NULL;

  // Should the writes to shared memory from the network be filtered
  // on the way in?  Or should it just be filtered on the way out?

  int doLock = (!rwlockState);
  if(doLock) if(wlock(tv_ptr)) return -1; // error

  // ptr got set in wlock().
  // We need to unset the byte_order that was set in wlock()
  // and then set it to a value from net_byte_order.

  ptr -= SIZE_PLUS(sizeof(u_int8_t)); // flags

  bzero(ptr, sizeof(u_int8_t));

  *ptr |= (u_int8_t) ((net_byte_order)?DTK_BYTE_ORDER:00);

  // Move back to just past the flags in the entry.
  ptr += SIZE_PLUS(sizeof(u_int8_t)); // flags
  memcpy(ptr, buf, user_size);

  if(doLock) if(wunlock(1)) return -1; // error
  return 0;
}


void *dtkSharedMem::cycle_filtersWrite(const void *buf,
				       struct timeval *timeStamp,
				       size_t nbyte, size_t offset)
{
  // We use the net_data_buf as a dummy buffer to pass data to and
  // from the write filters.

  if(net_data_buf_size < user_size)
    {
      net_data_buf_size = user_size;
      if(net_data_buf) dtk_free(net_data_buf);
      net_data_buf = (char *) dtk_malloc(user_size);
    }
  
  memcpy(net_data_buf + offset, buf, nbyte);
  
  dtkFilter *f = filterList->start();
  while(f)
    {
      if(filterList->checkWrite())
	{
	  int i = f->write((void *) buf, nbyte, offset, timeStamp);
 
	  if(i == dtkFilter::ERROR_ || i == dtkFilter::REMOVE)
	    {
	      if(i == dtkFilter::ERROR_)
		dtkMsg.add(DTKMSG_WARN,
			   "dtkSharedMem::write() filter callback failed.\n");
  	      filterList->removeWrite();
  
	      // Need to reset the filter_flags.
	      filter_flags &= ~IS_WRITE_FILTER;
	      filter_flags &= ~IS_WRITE_FILTER_TIMESTAMPED;
	      dtkFilter *filter = filterList->start();
	      while(filter)
		{
  		  if(filterList->checkWrite())
		    {
		      filter_flags |= IS_WRITE_FILTER;
		      if(filter->needTimeStamp())
		      {
			filter_flags |= IS_WRITE_FILTER_TIMESTAMPED;
			// We just need to know that there's at least
			// one write filter that requests a timestamp.
			break;
		      }
		    }
		  filter = filterList->next();
		}
	      // put the filterList back onto the same filter.
	      filter = filterList->start();
	      while(filter != f && filter)
		filter = filterList->next();
	    }
  	}
      f = filterList->next();
      }
    return net_data_buf;
}
