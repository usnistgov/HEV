/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
#include <dtk.h>
#include <dtk/dtkService.h>
#include "config.h"

class UDPEcho : public dtkService
{
public:
  UDPEcho(const char *port);
  virtual ~UDPEcho(void);
  int serve(void);
private:

  dtkSharedMem shm;
  dtkUDPSocket *udp;
};


UDPEcho::UDPEcho(const char *port):
  shm((size_t) DTK_SHARED_MEMORY_SIZE,
      DTK_SHARED_MEMORY_NAME)
{

  if(shm.isInvalid()) return; // error

  if(!port)
    {
      port = DEFAULT_SOCKET_ADDRESS_PORT;
      // strip off the address part in port.
      for(;*port && *port != ':';port++);
      if(!(*port))
	{
	  dtkMsg.add(DTKMSG_ERROR,"UDPEcho::UDPEcho() failed"
		     " the DEFAULT_SOCKET_ADDRESS_PORT is \"%s\"",
		     "\n   DEFAULT_SOCKET_ADDRESS_PORT needs a colon"
		     " (:) in it, but does not.\n",
		     DEFAULT_SOCKET_ADDRESS_PORT);
	  return;
	}
      port++;
    }
  udp = new dtkUDPSocket(PF_INET, port);
  if(!udp || udp->isInvalid()) return; // error

  if(dtkMsg.isSeverity(DTKMSG_INFO))
    {
      dtkMsg.add(DTKMSG_INFO,"");
      udp->print(dtkMsg.file());
    }

  fd = udp->getFileDescriptor();

  return;
}


int UDPEcho::serve(void)
{
  char buffer[DTK_SHARED_MEMORY_SIZE];

  ssize_t size = udp->read(buffer, (size_t) DTK_SHARED_MEMORY_SIZE);
  if(size < 0) return DTKSERVICE_CONTINUE;

  if(shm.write(buffer)) return DTKSERVICE_UNLOAD;

  return DTKSERVICE_CONTINUE;
}


UDPEcho::~UDPEcho(void)
{
  if(udp)
    {
      delete udp;
      udp = NULL;
    }
}


static dtkService *dtkDSO_loader(const char *port)
{
  return new UDPEcho(port);
}

static int dtkDSO_unloader(dtkService *udpEcho)
{
  delete udpEcho;
  return DTKDSO_UNLOAD_CONTINUE; // success
}
