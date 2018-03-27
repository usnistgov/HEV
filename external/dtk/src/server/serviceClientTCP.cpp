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

#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h>
#endif

#include "if_windows.h"
#include "serviceClientTCP.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"

// The socket in 'dtkTCPSocket' is a "client-like" connect()ed socket.

// This class removes itself from the list so that it can be
// self-destorying if the service is removed by the serviceManager.

ServiceClientTCP::
ServiceClientTCP(const char *addressPort_in):
  dtkTCPSocket(addressPort_in)
{
  addressPort = NULL;
  if(isInvalid()) // dtkTCPSocket::isInvalid()
    {
      dtkMsg.add(DTKMSG_WARN,
		 "ServiceClientTCP::ServiceClientTCP(\"%s\")"
		 " failed.\n", addressPort_in);
      return; // error
    }

  dtkMsg.add(DTKMSG_DEBUG,"made ServiceClientTCP: ");
  if(dtkMsg.isSeverity(DTKMSG_DEBUG))
    dtkTCPSocket::print(dtkMsg.file());

  dtkService::fd = 
#ifdef DTK_ARCH_WIN32_VCPP
	  (HANDLE)
#endif
	    dtkTCPSocket::getFileDescriptor();

  // Add this to the list of DTK services.
  serviceManager.add(this);
  serviceManager.deleteOnRemove(this);

  // Add the socket to the list of usable connected sockets.
  socketListMutex.lock();
  serverSocketList.add(this);

  // Get my common address and port.
  addressPort = serverSocketList.get(this);
  socketListMutex.unlock();
}

ServiceClientTCP::~ServiceClientTCP(void)
{
  if(addressPort)
    {
      dtkMsg.add(DTKMSG_DEBUG,
		 "removing connection from address %s%s%s.\n",
		 dtkMsg.color.yel, addressPort, dtkMsg.color.end);

      // Remove this the associated connection lists for all DTK
      // shared memory.
      serverRespond.removeAddressFromWroteLists(addressPort);

      addressPort = NULL;
    }
  socketListMutex.lock();
  serverSocketList.remove(this);
  socketListMutex.unlock();
}

int ServiceClientTCP::serve(void)
{
  // dtkTCPSocket::read()
  char serverBuffer[DTKSERVER_BUFFER_SIZE + 1];
  ssize_t size = read(serverBuffer, DTKSERVER_BUFFER_SIZE);

  if(size < 0)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "ServiceClientTCP::serve() failed.\n");
      return DTKSERVICE_UNLOAD;
    }
  else if(size == 0)
    {
      // Reading no data seems to mean that the socket connection has
      // been broken from the other side.
      dtkMsg.add(DTKMSG_DEBUG,
		 "ServiceClientTCP::serve() read no "
		 "data, assuming connection was broken.\n");
      return DTKSERVICE_UNLOAD;
    }

  serverMutex.lock();
  size_t size_out;
  char *response = serverParseRequest(serverBuffer, size,
				      &size_out, addressPort);
  if(response)
    {
      write(response, size_out);
      serverDebugPrintCommand(0,addressPort,response,size_out);
    }

  serverMutex.unlock();
  return DTKSERVICE_CONTINUE;
}

#ifdef DTK_ARCH_WIN32_VCPP
void ServiceClientTCP::stop(void)
{
	if(dtkService::fd != NULL)
  {
	  closesocket((unsigned int) dtkService::fd);
	  dtkService::fd = NULL;
  }
}
#endif
