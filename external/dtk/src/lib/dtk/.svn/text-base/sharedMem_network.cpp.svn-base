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

#ifdef DTK_ARCH_WIN32_VCPP
# include <winbase.h>
# include <winsock2.h> // for struct timeval
# include <malloc.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/un.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/time.h>
#endif /* #else  #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
  typedef uint64_t u_int64_t;
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
#include "dtkTCPSocket.h"
#include "dtkSocketList.h"
#include "dtkSharedMem.h"


#include "utils.h"
#include "_private_sharedMem.h"
#include "serverProtocol.h"

// 'dtkSharedMem_udpList' and 'dtkSharedMem_tcpList' are shared by all
// objects in this process.  Among other things, this will also make
// this code not be thread safe.  dtkSharedMem_tcpList is global so
// that the dtk-server can use it and the TCP sockets in it.  In
// ../server/dtk-server.cpp dtkSharedMem_tcpList set to a local
// statically declared object pointer

static dtkSocketList *dtkSharedMem_udpList = NULL;
DTKAPI dtkSocketList *dtkSharedMem_tcpList = NULL;
int dtkSharedMem::number_of_dtkSharedMem_objs = 0;

// __this_is_the_dtk_server__ so that the code in the libdtk can tell
// if the dtk-server is using libdtk.  It's set to 1 in main() in
// ../server/dtk-server.cpp.  It's hoped that DTK users will not use
// this.
DTKAPI int __this_is_the_dtk_server__ = 0;

// deletes the sockets and lists of sockets if this is the last
// dtkSharedMem object being deleted and this is not dtk-server.
// checkRemoveSockets() is called from dtkSharedMem::clean() in
// sharedMem.cpp.
void dtkSharedMem::checkRemoveSockets(void)
{
  if(number_of_dtkSharedMem_objs == 0)
    {
      // The dtk-server uses it's own dtkSharedMem_tcpList and sets
      // dtkSharedMem_tcpList to point to it so that shared memory
      // socket writing will use some of the same TCP sockets that the
      // server code uses.
      if(dtkSharedMem_tcpList && (!__this_is_the_dtk_server__))
	{
	  dtkMsg.add(DTKMSG_DEBUG,
		     "Removing the remote shared memory write TCP sockets\n");
	  // delete dtkSharedMem_tcpList will cleanup sockets and the
	  // list
	  delete dtkSharedMem_tcpList;
	  dtkSharedMem_tcpList = NULL;
	}
      if(dtkSharedMem_udpList)
	{
	  dtkMsg.add(DTKMSG_DEBUG,
		     "Removing the remote shared memory write UDP sockets\n");
	  // delete dtkSharedMem_udpList will cleanup sockets and the
	  // list
	  delete dtkSharedMem_udpList;
	  dtkSharedMem_udpList = NULL;
	}
    }
}

#if 0
// prints commands in or out of the server.
static void serverDebugPrintCommand(int isInput, const char *address, 
				    const char *buff, size_t size)
{
  if(!dtkMsg.isSeverity(DTKMSG_DEBUG)) return;
  if(isInput)
    dtkMsg.add(DTKMSG_DEBUG,"%s READ%s(size=%d) from %s%s%s (",
	       dtkMsg.color.rgrn, dtkMsg.color.end, size,
	       dtkMsg.color.yel, address,
	       dtkMsg.color.end);
  else
    dtkMsg.add(DTKMSG_DEBUG,"%sWROTE%s(size=%d)  to  %s%s%s (",
	       dtkMsg.color.rvil, dtkMsg.color.end, size,
	       dtkMsg.color.yel, address,
	       dtkMsg.color.end);

  size_t i=0;
  FILE *file = dtkMsg.file();
  for(;i<size;i++)
    {
      if(buff[i])
	putc(buff[i],file);
      else
	fprintf(file,"\\0");
    }
  dtkMsg.append(")\n");
}
#endif

int dtkSharedMem::setNetworkType(int type)
{
  if(dtkRWLock_wlock(rwlock))
    return
      dtkMsg.add(DTKMSG_ERROR, 0, -1,
		 "dtkSharedMem::setNetworkType(%d) failed.\n",
		 type);

  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
	   dtkRWLock_wunlock(rwlock);
	   dtkMsg.append("dtkSharedMem::setNetworkType() failed.\n");
	   cleanup();
	   return -1;
	}

  flags |= ((type)? 00: NETWORK_TYPE);
  header->flags = flags;

  dtkRWLock_wunlock(rwlock);

  // At this point I may want to push this property across the network
  // to all remote shared memory.  -- add later ?? --
  return 0;
}

int dtkSharedMem::getNetworkType(void)
{
  if(dtkRWLock_wlock(rwlock))
    return
      dtkMsg.add(DTKMSG_ERROR, 0, -1,
		 "dtkSharedMem::getNetworkType() failed.\n");

  if(flags != header->flags || queue_length != header->queue_length)
    // Make sure that the size and shape of the file is consistant.
    if(reconnect())
	{
	   dtkRWLock_wunlock(rwlock);
	   dtkMsg.append("dtkSharedMem::getNetworkType() failed.\n");
	   cleanup();
	   return -1;
	}


  header->flags = flags;
  dtkRWLock_wunlock(rwlock);

  return (flags & NETWORK_TYPE)? DTK_TCP: DTK_UDP;
}


// addrList->start() is called before this.  realloc_net_data_buf()
// sets up a buffer large enough to write all the remote addresses
// given the format from _DTKREQ_WRITE command in the file
// serverProtocol.h.
void dtkSharedMem::realloc_net_data_buf(void)
{
  size_t max_remoteName_size = 0;
  char *remoteName;

  while(addrList->getNextAddressPort(&remoteName))
    {
      size_t len = strlen(remoteName);
      if(max_remoteName_size < len)
	max_remoteName_size = len;
    }

  char segSize_string[10];
  sprintf(segSize_string, "%d", (int) user_size);
  sizeStringLength = strlen(segSize_string);
  size_t maxLength = (size_t)
    (1+ 1+ max_remoteName_size+1+ sizeStringLength+1+ user_size);

  // senddata={'W',byte_order,"remoteFilename","size",data} and
  // "remoteFileName" may be different for each remote address.
  if(net_data_buf_size < maxLength)
    {
      net_data_buf = (char *)
	dtk_realloc(net_data_buf, net_data_buf_size = maxLength);
      // We'll put the ("size",data) at the end of the buffer so that
      // it will not change while writing to different addresses.
      strcpy(&(((char *) net_data_buf)[net_data_buf_size - 
				      (strlen(segSize_string)+1+user_size)]),
	     segSize_string);

      // The user data may change for each network write so it is
      // copied into this net_data_buf at each and every network
      // write by dtkSharedMem::wunlock().
    }     
}

// This is and should be called without the read/write lock set.
// addrList->start() is called before this.
int dtkSharedMem::_write_network()
{
  char *remoteName;
  char *addr = addrList->getNextAddressPort(&remoteName);

  for(;addr;addr = addrList->getNextAddressPort(&remoteName))
    {
      size_t offset = net_data_buf_size -
	(user_size + strlen(remoteName) + sizeStringLength + 2);
      strcpy(&(net_data_buf[offset]), remoteName);
      net_data_buf[--offset] = byteOrderChar;
      net_data_buf[--offset] = _DTKREQ_WRITE;
      // senddata={'W',byte_order,"remoteFilename","size",data}
      ssize_t write_size =
	user_size + strlen(remoteName) + sizeStringLength + 4;
      //////////////////////////////////////////////////////////////
      /*
	This UDP SHM writing needs to be
	rewritten. dtkSharedMem_udp->setRemote(addr) used DNS to get
	the and set the UDP socket address and that takes way to long.
	Don't change the #if 0 or the writing of to remote address
	will suck major CPU usage.  The other method just below the
	#endif uses a list of UDP sockets.  Something needs to keep
	the address structures for all the UDP write addresses and
	than we would not need but one UDP socket.
      */
#if 0 // This is calling dtkSharedMem_udp->setRemote(addr) too much
      if(!(flags & NETWORK_TYPE)) // UDP
	{
	  if(!(dtkSharedMem_udp))
	    {
	      dtkSharedMem_udp = new dtkUDPSocket(PF_INET);
	      if(!dtkSharedMem_udp || dtkSharedMem_udp->isInvalid())
		{
		  // Somethings broken, so you will not be writing to
		  // the network in this call.
		  addrList->removeAll();

		  if(dtkSharedMem_udp)
		    {
		      delete dtkSharedMem_udp;
		      dtkSharedMem_udp = NULL;
		    }
		}
	    }

	  if(dtkSharedMem_udp)
	    {
	      if(dtkSharedMem_udp->setRemote(addr) ||
		 dtkSharedMem_udp->write(&(net_data_buf[offset]),
					 (size_t) write_size) != write_size)
		{
		  addrList->remove(addr);
		  dtkMsg.add(DTKMSG_WARN, "removed address=\"%s\" "
			     "and remote file name=\"%s\" from the"
			     " write list of local shared memory file "
			     "\"%s\".\n",
			     addr, remoteName, fullPath);
		}
	    }
	}
#endif
      if(!(flags & NETWORK_TYPE)) // UDP
	{
	  if(!dtkSharedMem_udpList)
	    dtkSharedMem_udpList = new dtkSocketList;

	  int is_new_sock = 0;

	  dtkSocket *sock = dtkSharedMem_udpList->get(addr);
	  if(!sock)
	    {
	      sock = new dtkUDPSocket(addr);
	      if(!sock || sock->isInvalid())
		{
		  addrList->remove(addr);
		  dtkMsg.add(DTKMSG_WARN, "removed address=\"%s\" "
			     "and remote file name=\"%s\" from the"
			     " write list of local shared memory file "
			     "\"%s\".\n",
			     addr, remoteName, fullPath);
		  if(sock)
		    {
		      delete sock;
		      sock = NULL;
		    }
		}
	      else
		{
		  dtkSharedMem_udpList->add(sock, addr);
		  is_new_sock = 1;
		}
	    }

	  if(sock && sock->write(&(net_data_buf[offset]),
				 (size_t) write_size) != write_size)
	    {
	      dtkSharedMem_udpList->remove(sock);
	      delete sock;
	      sock = NULL;

	      if(!is_new_sock) // it may be an old socket whos server died
		// and a new server is not connected to it.
		{
		  sock = new dtkUDPSocket(addr);
		  if(!sock || sock->isInvalid())
		    {
		      addrList->remove(addr);
		      if(sock)
			{
			  delete sock;
			  sock = NULL;
			}
		    }
		  else
		    dtkSharedMem_udpList->add(sock, addr);
		}
	      else
		addrList->remove(addr);

	      if(sock && sock->write(&(net_data_buf[offset]),
				     (size_t) write_size) != write_size)
		{
		  dtkSharedMem_udpList->remove(sock);
		  addrList->remove(addr);
		  delete sock;
		  sock = NULL;
		}
	      if(!sock)
		dtkMsg.add(DTKMSG_WARN, "removed address=\"%s\" "
			   "and remote file name=\"%s\" from the"
			   " write list of local shared memory"
			   " file \"%s\".\n",
			   addr, remoteName, fullPath);
	    }
#if 0
	  if(sock)
	    serverDebugPrintCommand(0, addr,
				    &(net_data_buf[offset]),
				      (size_t) write_size);
#endif
	}
      else // if(addrList->type == DTK_INETTCPSOCKET)
	// All this logic takes care of removing dead sockets and
	// adding a new one when appropriate.
	{
	  if(!dtkSharedMem_tcpList)
	    dtkSharedMem_tcpList = new dtkSocketList;

	  int is_new_sock = 0;

	  dtkSocket *sock = dtkSharedMem_tcpList->get(addr);
	  if(!sock)
	    {
	      sock = new dtkTCPSocket(addr);
	      if(!sock || sock->isInvalid())
		{
		  addrList->remove(addr);
		  dtkMsg.add(DTKMSG_WARN, "removed address=\"%s\" "
			     "and remote file name=\"%s\" from the"
			     " write list of local shared memory file "
			     "\"%s\".\n",
			     addr, remoteName, fullPath);
		  if(sock)
		    {
		      delete sock;
		      sock = NULL;
		    }
		}
	      else
		{
		  dtkSharedMem_tcpList->add(sock, addr);
		  is_new_sock = 1;
		}
	    }

	  if(sock && sock->write(&(net_data_buf[offset]),
				 (size_t) write_size) != write_size)
	    {
	      dtkSharedMem_tcpList->remove(sock);
	      delete sock;
	      sock = NULL;

	      if(!is_new_sock) // it may be an old socket whos server died
		// and a new server is not connected to it.
		{
		  sock = new dtkTCPSocket(addr);
		  if(!sock || sock->isInvalid())
		    {
		      addrList->remove(addr);
		      if(sock)
			{
			  delete sock;
			  sock = NULL;
			}
		    }
		  else
		    dtkSharedMem_tcpList->add(sock, addr);
		}
	      else
		addrList->remove(addr);

	      if(sock && sock->write(&(net_data_buf[offset]),
				     (size_t) write_size) != write_size)
		{
		  dtkSharedMem_tcpList->remove(sock);
		  addrList->remove(addr);
		  delete sock;
		  sock = NULL;
		}
	      if(!sock)
		dtkMsg.add(DTKMSG_WARN, "removed address=\"%s\" "
			   "and remote file name=\"%s\" from the"
			   " write list of local shared memory"
			   " file \"%s\".\n",
			   addr, remoteName, fullPath);
	    }
#if 0
	  if(sock)
	    serverDebugPrintCommand(0, addr,
				    &(net_data_buf[offset]),
				      (size_t) write_size);
#endif
	}
    }

  return 0;
}
