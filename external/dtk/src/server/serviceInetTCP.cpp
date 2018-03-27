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
#include "serviceSlaveTCP.h"
#include "serviceInetTCP.h"
#define __NOT_LOADED_DTKSERVICE__
#include "dtk/dtkService.h"
#include "serverRespond.h"
#include "dtk-server.h"

ServiceInetTCP::ServiceInetTCP(const char *port) :
  dtkTCPSocket(PF_INET, port)
{
  if(isInvalid()) return;

  dtkService::fd = 
#ifdef DTK_ARCH_WIN32_VCPP
	  (HANDLE)
#endif
	     getFileDescriptor();

  if(dtkMsg.isSeverity(DTKMSG_INFO))
    dtkSocket::print(dtkMsg.file());
}

int ServiceInetTCP::serve(void)
{
  dtkTCPSocket *slave = (dtkTCPSocket *) accept();

  if(!slave || slave->isInvalid())
    {
      dtkMsg.add(DTKMSG_WARN,"ServiceInetTCP::serve() "
		 "failed to accept() TCP connection.\n");
      if(slave) delete slave;
      return DTKSERVICE_CONTINUE;
    }

  // There is not much to fail with SlaveInetTCP::SlaveInetTCP().
  // This will add a service to the server.
  dtkService *s = new ServiceSlaveTCP(slave, slave->getRemote(), PF_INET);
  if(s->fd < 0)
    delete s; // error

  return DTKSERVICE_CONTINUE;
}

#ifdef DTK_ARCH_WIN32_VCPP
void ServiceInetTCP::stop(void)
{
  if(dtkService::fd != NULL)
  {
    closesocket((unsigned int) dtkService::fd);
    dtkService::fd = NULL;
  }
}
#endif
