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
# include <malloc.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/time.h>
# include <sys/un.h>
# include <netinet/in.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
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
#include "dtkRWLock.h"
#include "dtkConditional.h"
#include "dtkSegAddrList.h"
#include "dtkSocket.h"
#include "dtkUDPSocket.h"
#include "dtkSocketList.h"
#include "dtkSharedMem.h"

#include "utils.h"

#include "_private_sharedMem.h"

/* 
 * memory of the write lists is after the user entry data as follows:
 *
 * It starts at (header + header->write_list_offset). If
 * header->write_list_offset is zero than there are no entrees.
 *
 * then each entry is: { int32_t offset_to_next (plus padding to CHUNKSIZE),
 *                       "address", "name" }
 *
 * offset_to_next is the number of bytes from the top of current to
 * the next entry top, or else it's zero if this is the last one.
 * offset_to_next may be negitive.
 *
 * */

// addrList must be non-NULL, must be read or write locked and
// reconnect() (if needed) before calling and check
// address_change_count before.
void dtkSharedMem::sync_addrList(void)
{
  addrList->removeAll();

  if(header->write_list_offset) // There are addresses present.
    {
      // get the current list of write addresses from shared memory to
      // the dtkSegAddrList object.
      char *p = ((char *) header) + header->write_list_offset;
      while(1)
	{
	  char *addr = p + SIZE_PLUS(sizeof(int32_t));
	  char *nam = addr + (strlen(addr) + 1);
	  addrList->add(addr, nam);
	  if(*((int32_t *)p))
	    p += *((int32_t *)p);
	  else
	    break;
	}
    }

  address_change_count = header->address_change_count;
}


// For the DTK server.
/* do not write to the returned memory. */
dtkSegAddrList *dtkSharedMem::getSegAddrList(void)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkSharedMem::getSegAddrList() failed: "
		 "invalid dtkSharedMem object.\n");
      return NULL;
    }

  if(dtkRWLock_rlock(rwlock))
    {
      dtkMsg.add(DTKMSG_ERROR, 
		 "dtkSharedMem::getSegAddrList() failed.\n");
      return NULL;
    }

  if(!addrList)
    addrList = new dtkSegAddrList(fullPath);

  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
       dtkRWLock_runlock(rwlock);
       dtkMsg.append("dtkSharedMem::getSegAddrList() failed.\n");
	   cleanup();
	   return NULL;
	}

  /**** this is how to update the write list ********************/
  if(address_change_count != header->address_change_count)
    sync_addrList(); 

  dtkRWLock_runlock(rwlock);

  return addrList;
}


// return 1 if it's not present, returns 0 if it's removed,
// and returns -1 of error.
int dtkSharedMem::removeFromWriteList(const char *address_in)
{
  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::removeFromWriteList(\"%s\") failed: "
		      "invalid dtkSharedMem object.\n",
		      address_in);

  char *address = dtk_getCommonAddressPort(address_in);
  if(!address || !(address[0]))
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkSharedMem::removeFromWriteList(\"%s\") failed: "
		      "can't get common address for \"%s\".\n",
		      address_in, address_in);

  if(!addrList)
    addrList = new dtkSegAddrList(fullPath);

  if(dtkRWLock_wlock(rwlock))
    return dtkMsg.add(DTKMSG_ERROR, 0, -1, 
		      "dtkSharedMem::removeFromWriteList(\"%s\") failed.\n",
		      address_in);

  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
       dtkRWLock_wunlock(rwlock);
       dtkMsg.append("dtkSharedMem::removeFromWriteList(\"%s\") failed.\n",
			address_in);
	   cleanup();
	   return -1;
	}

  /**** this is how to update the write list ********************/
  if(address_change_count != header->address_change_count)
    sync_addrList(); 

  char *remoteName = addrList->get(address);
  if(remoteName) // header->write_list_offset should be non-zero.
    {
      // remove this address from the list in shared memory.
      int32_t *p = (int32_t *)(((char *) header) +
				   header->write_list_offset);
      int32_t *prev = NULL;

      while(1)
	{
	  char *addr = ((char *) p) + SIZE_PLUS(sizeof(int32_t));
	  if(!strcmp(addr,address))
	    {
	      address_change_count = ++(header->address_change_count);

	      if(prev)
		{
		  if(*p)
		    *prev += *p;
		  else // matched the last in the list
		    *prev = 0;
		}
	      else if(*p) // no previous
		header->write_list_offset += *p;
	      else // *p == 0 and no previous
		header->write_list_offset = 0;

	      int return_val = _free(p);
	      dtkRWLock_wunlock(rwlock);
	      dtkMsg.add(DTKMSG_DEBUG,
			 "Removed address=\"%s\"=\"%s\", "
			 " with remoteName=\"%s\"\n"
			 " from DTK shared memory file "
			 "\"%s\" write list.\n",
			 address_in, address, remoteName, fullPath);
	      addrList->remove(address);
	      return return_val;
	    }

	  if(*p)
	    {
	      prev = p;
	      p = (int32_t *)(((char *) p) + *p);
	    }
	  else // At the end of the list and the address has not been
	    // found.
	    {
	      dtkRWLock_wunlock(rwlock);
	      dtkMsg.add(DTKMSG_WARN,
			 "dtkSharedMem::removeFromWriteList(\"%s\"):"
			 " address write list may be corrupted.\n",
			 address_in);
	      return -1; // error
	    }
	}
    }

  dtkRWLock_wunlock(rwlock);
  dtkMsg.add(DTKMSG_INFO,
	     "dtkSharedMem::removeFromWriteList(\"%s\") notice: "
	     "address \"%s\" is not in the remote write list "
	     "for DTK shared memory \"%s\".\n",
	     address_in, address, fullPath);

  return 0;
}


// return 1 if it's already present, returns 0 if it's added,
// and returns -1 on error.
int dtkSharedMem::addToWriteList(const char *address_in,
				 const char *remoteName_in)
{
  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::addToWriteList(\"%s\", \"%s\") failed: "
		      "invalid dtkSharedMem object.\n",
		      address_in, remoteName_in);

  char *address = dtk_getCommonAddressPort(address_in);
  if(!address)
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkSharedMem::addToWriteList(\"%s\", \"%s\") failed: "
		      "can't get common address for \"%s\".\n",
		      address_in, remoteName_in, address_in);

  char *remoteName;
  if(!remoteName_in || !(remoteName_in[0]))
    remoteName = dtk_strdup(name);
  else
    remoteName = dtk_strdup(remoteName_in);

  if(!addrList)
    addrList = new dtkSegAddrList(fullPath);

  if(dtkRWLock_wlock(rwlock))
    {
      free(remoteName);
      return dtkMsg.add(DTKMSG_ERROR, 0, -1, 
			"dtkSharedMem::addToWriteList(\"%s\", "
			"\"%s\") failed.\n",
			address_in, remoteName_in);
    }

  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
       dtkRWLock_wunlock(rwlock);
       dtkMsg.append("dtkSharedMem::addToWriteList(\"%s\", \"%s\") failed.\n",
			address_in, remoteName_in);
	   cleanup();
	   return -1;
	}

  /**** this is how to update the write list ********************/
  if(address_change_count != header->address_change_count)
    sync_addrList(); 


  // Check if this address is already in the list.
  {
    char *n = addrList->get(address);
    if(n)
      {
	// bump the address_change_count counter to force an update to
	// the writers that may have closed broken sockets.
	header->address_change_count = ++address_change_count;
	dtkRWLock_wunlock(rwlock);
	free(remoteName);
	dtkMsg.add(DTKMSG_NOTICE,
		   "dtkSharedMem::addToWriteList"
		   "(\"%s\", \"%s\") notice.\n"
		   " Address=\"%s\" is already "
		   "present in shared memory file\n"
		   " \"%s\" write list with remote file name \"%s\".\n",
		   address_in, remoteName_in, address, fullPath, n);
	return 1;
      }
  }

  /**** add address and remoteName to the list in shared Memory ****/

  // get more shared memory.
  char *str = _alloc(SIZE_PLUS(sizeof(int32_t)) +
		     SIZE_PLUS(strlen(address) + strlen(remoteName) + 2));
  if(!str)
    {
      dtkRWLock_wunlock(rwlock);
      free(remoteName);
      dtkMsg.add(DTKMSG_WARN,
		 "dtkSharedMem::addToWriteList"
		 "(\"%s\", \"%s\") failed.\n",
		 address_in, remoteName_in);
      return -1;
    }

  if(header->write_list_offset)
    *((int32_t *) str) =
#ifdef DTK_ARCH_IRIX
#  if (_MIPS_SIM==_MIPS_SIM_ABI64)
      ((int32_t)(u_int64_t) ((((char *) header) +
			      header->write_list_offset))) -
      ((int32_t)(u_int64_t) str);
#  else
      ((int32_t) ((((char *) header) + header->write_list_offset))) -
      ((int32_t) str);
#  endif
#else
#  ifdef DTK_USE_64_BIT
      ((int32_t)(u_int64_t) ((((char *) header) +
			      header->write_list_offset))) -
      ((int32_t)(u_int64_t) str);
#  else
      ((int32_t) ((((char *) header) + header->write_list_offset))) -
      ((int32_t) str);
#  endif
#endif
  else
    *((int32_t *) str) = 0;

  header->write_list_offset = (u_int32_t)(str - ((char *) header));

  str += SIZE_PLUS(sizeof(int32_t));
  strcpy(str, address);

  str += strlen(address) + 1;
  strcpy(str, remoteName);

  addrList->add(address, remoteName);
  header->address_change_count = ++address_change_count;

  dtkRWLock_wunlock(rwlock);
  dtkMsg.add(DTKMSG_DEBUG,
	     "Added address=\"%s\", and remoteName=\"%s\" "
	     "to\n DTK shared memory file "
	     "\"%s\" write list.\n",
	     address, remoteName, fullPath);
  free(remoteName);

  return 0;
}

// for debugging.
int dtkSharedMem::printWriteList(FILE *file)
{
  file = (file) ? ((FILE *) file) : stdout;

  if(isInvalid())
    return dtkMsg.add(DTKMSG_ERROR, 0, -1,
		      "dtkSharedMem::printWriteList(%p) failed"
		      " invalid object.\n",
		      file);

  if(dtkRWLock_rlock(rwlock))
    return dtkMsg.add(DTKMSG_ERROR, 0, -1, 
		      "dtkSharedMem::printWriteList(%p) failed.\n",
		      file);

  if(flags != header->flags || queue_length != header->queue_length)
    if(reconnect())
	{
       dtkRWLock_runlock(rwlock);
       dtkMsg.append("dtkSharedMem::printWriteList(%p) failed.\n",
			file);
	   cleanup();
	   return -1;
	}

  if(header->write_list_offset)
    {
      fprintf(file, "Write Addresses for shared memory %s%s%s\n",
	      dtkMsg.color.red, fullPath, dtkMsg.color.end);
      fprintf(file, "__memory__ %s_______address_______%s"
	      " %s__remote_file_name__%s\n",
	      dtkMsg.color.grn, dtkMsg.color.end,
	      dtkMsg.color.tur, dtkMsg.color.end);

      char *p = ((char *) header) + header->write_list_offset;
      while(1)
	{
	  char *addr = p + SIZE_PLUS(sizeof(int32_t));
	  char *nam = addr + (strlen(addr) + 1);
	  fprintf(file, "%d %s%s%s %s%s%s\n",
#ifdef DTK_ARCH_IRIX
#  if (_MIPS_SIM==_MIPS_SIM_ABI64)
		  (int32_t)(u_int64_t) p,
#  else
                  (int32_t) p,
#  endif
#else
#  ifdef DTK_USE_64_BIT
		  (int32_t)(u_int64_t) p,
#  else
		  (int32_t) p,
#  endif
#endif
		  dtkMsg.color.grn, addr, dtkMsg.color.end,
		  dtkMsg.color.tur, nam, dtkMsg.color.end);
	  if(*((int32_t *)p))
	    p += *((int32_t *)p);
	  else
	    break;
	}
    }

  dtkRWLock_runlock(rwlock);

  return 0;
}
