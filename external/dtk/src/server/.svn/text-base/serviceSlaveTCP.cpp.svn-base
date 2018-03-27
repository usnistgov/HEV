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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h>
#endif

#include "if_windows.h"
#include "serviceSlaveTCP.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"

// The socket in 'dtkTCPSocket *s' is a accept()-connected slave
// socket.

ServiceSlaveTCP::
ServiceSlaveTCP(dtkTCPSocket *s,
		const char *addressPort_in, int domain)
{
  addressPort = NULL;
  sock = s;
#ifdef DTK_ARCH_WIN32_VCPP
  fd = (HANDLE) 1;
#else
  fd = sock->getFileDescriptor();
#endif

  dtkMsg.add(DTKMSG_DEBUG,"made ServiceSlaveTCP: ");
  if(dtkMsg.isSeverity(DTKMSG_DEBUG))
    sock->print(dtkMsg.file());

  // Add this socket to the usable connected socket list.
  if(domain == PF_UNIX)
    {
      socketListMutex.lock();
      serverSocketList.add(sock, addressPort_in);
      socketListMutex.unlock();
      addressPort = dtk_strdup(addressPort_in);
    }
  else // domain == PF_INET
    {
      if(serverSocketList.add(sock))
	{
	  dtkMsg.add(DTKMSG_ERROR, "failed to find Inet slave"
		     " TCP socket address.\n");
#ifdef DTK_ARCH_WIN32_VCPP
	  fd = NULL;
#else
	  fd = -1; // Tells the serviceManager this failed.
#endif
	  return;
	}
      else
	{
	  socketListMutex.lock();
	  addressPort = dtk_strdup(serverSocketList.get(sock));
	  socketListMutex.unlock();
	}
    }

  // Add this to the list of DTK services.  This needs to be done last
  // in this constructor, just so the serviceManager can tell if this
  // service as a valid fd, and can fail and delete for me.
  serviceManager.add(this);
  serviceManager.deleteOnRemove(this);
}

ServiceSlaveTCP::~ServiceSlaveTCP(void)
{
  // This object had it's own copy of addressPort so that it could
  // change it easily.
  if(addressPort)
    {
      dtkMsg.add(DTKMSG_DEBUG,
		 "removing connection from address %s%s%s.\n",
		 dtkMsg.color.yel, addressPort, dtkMsg.color.end);

      // Remove this the associated connection lists for all shared
      // memory.
      socketListMutex.lock();
      serverRespond.removeAddressFromWroteLists(addressPort);
      socketListMutex.unlock();

      free(addressPort);
      addressPort = NULL;
    }

  if(sock)
    {
      // Remove the socket from the list of usable connected sockets.
      socketListMutex.lock();
      serverSocketList.remove(sock);
      socketListMutex.unlock();
      delete sock;
      sock = NULL;
    }

  // This object, like all services will get removed from the
  // serverManager by the serverManager.  So we don't do that here.
}

int ServiceSlaveTCP::serve(void)
{
  char serverBuffer[DTKSERVER_BUFFER_SIZE + 1];
  ssize_t size = sock->read(serverBuffer, DTKSERVER_BUFFER_SIZE);

  if(size < 0)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "ServiceSlaveTCP::serve() failed.\n");
      return DTKSERVICE_UNLOAD;
    }
  else if(size == 0)
    {
      // Reading no data seems to mean that the socket connection has
      // been broken from the other side.
      dtkMsg.add(DTKMSG_DEBUG,
		 "ServiceSlaveTCP::serve() read no "
		 "data, assuming connection was broken.\n");
      return DTKSERVICE_UNLOAD;
    }

  // This is the only server command request that is not parsed in
  // serverParseRequest(), because it needs to change data in this
  // object.  It changes the port number of this as refered to in the
  // serverSocketList to a known server port number in place of what
  // the connection port is.
  if(*serverBuffer == _DTKSREQ_CONNECTSERVERS &&
     (ssize_t) size < (ssize_t) DTKSERVER_BUFFER_SIZE - 2 &&
     serverBuffer[size-1] == '\0')
    {
      serverDebugPrintCommand(1,addressPort,serverBuffer,size);
      // serverBuffer = {_DTKSREQ_CONNECTSERVERS,"port","clientReplyAddr"}
      // respond with   {_DTKSSREQ_CONNECTSERVERS,"port","clientReplyAddr",""}
      serverBuffer[size] = '\0';
      size++;
      *serverBuffer = _DTKSSREQ_CONNECTSERVERS;
      if(sock->write(serverBuffer, (size_t) size) != size)
	return DTKSERVICE_CONTINUE; // failed to write back.

      serverDebugPrintCommand(0,addressPort,serverBuffer,size);

      // success change addressPort.
      char *port = addressPort;
      for(;*port && *port != ':';port++);
      if(*port)
	{
	  *port = '\0';
	  port++;
	}
      char *oldAddr = addressPort;
      addressPort = (char *)
	dtk_malloc(strlen(oldAddr) + strlen(&(serverBuffer[1])) + 2);
      sprintf(addressPort,"%s:%s",oldAddr,&(serverBuffer[1]));
      serverSocketList.remove(sock);
      serverSocketList.add(sock, addressPort);
      dtkMsg.add(DTKMSG_DEBUG,"reseting Slave sockets listed "
		 "address from \"%s:%s\" to \"%s\" on fd=%d.\n",
		 oldAddr, port, addressPort, fd);
      free(oldAddr);

      return DTKSERVICE_CONTINUE;
    }
  serverMutex.lock();
  size_t size_out;
  char *response =
    serverParseRequest(serverBuffer, size, &size_out, addressPort);

  if(response)
    {
      sock->write(response, size_out);
      serverDebugPrintCommand(0,addressPort,response,size_out);
    }

  serverMutex.unlock();
  return DTKSERVICE_CONTINUE;
}

#ifdef DTK_ARCH_WIN32_VCPP
void ServiceSlaveTCP::stop(void)
{
	if(dtkService::fd != NULL)
  {
	  closesocket((unsigned int) dtkService::fd);
	  dtkService::fd = NULL;
  }
}
#endif
