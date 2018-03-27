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

#include <dtk.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h>
#endif

#include "if_windows.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serviceInetUDP.h"
#include "serverRespond.h"
#include "dtk-server.h"


ServiceInetUDP::ServiceInetUDP(const char *port)
{
  sock = new dtkUDPSocket(PF_INET, port);
  if(!sock || sock->isInvalid()) return;

  fd = 
#ifdef DTK_ARCH_WIN32_VCPP
	  (HANDLE)
#endif
	     sock->getFileDescriptor();

  if(dtkMsg.isSeverity(DTKMSG_INFO))
    sock->print(dtkMsg.file());
}

ServiceInetUDP::~ServiceInetUDP(void)
{
  if(sock)
    {
      delete sock;
      sock = NULL;
    }

#ifdef DTK_ARCH_WIN32_VCPP
  fd = NULL;
#else
   fd = -1;
#endif
}

int ServiceInetUDP::serve(void)
{
  char serverBuffer[DTKSERVER_BUFFER_SIZE + 1];
  ssize_t size = sock->read(serverBuffer, DTKSERVER_BUFFER_SIZE);

  if(size < 0)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "ServiceInetUDP::serve() failed.\n");
      return DTKSERVICE_UNLOAD;
    }
  else if(size == 0)
    {
      // Reading no data seems to mean that the socket connection has
      // been broken from the other side.
      dtkMsg.add(DTKMSG_DEBUG,
		 "ServiceInetUDP::serve() read no "
		 "data. What's up with that?\n");
      return DTKSERVICE_CONTINUE;
    }

  serverMutex.lock();
  size_t size_out;
  char *response = serverParseRequest(serverBuffer, size, &size_out);

  if(response)
    {
      sock->write(response, size_out);
      serverDebugPrintCommand(0,"an Inet UDP socket", response, size_out);
    }

  serverMutex.unlock();
  return DTKSERVICE_CONTINUE;
}

#ifdef DTK_ARCH_WIN32_VCPP
void ServiceInetUDP::stop(void)
{
	if(dtkService::fd != NULL)
  {
	  closesocket((unsigned int) dtkService::fd);
	  dtkService::fd = NULL;
  }
}
#endif
