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
#include <string.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h>
#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/un.h>
# include <fcntl.h>
# include <unistd.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdarg.h>
# include <sys/types.h>
# include <sys/time.h>
#endif

#ifdef DTK_ARCH_IRIX
# include <bstring.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkSocket.h"
#include "dtkUDPSocket.h"
#include "_private_socket.h"


// from socket.cpp
extern DTK_address_type get_num_address(const char *address);


// If bind_port is set, this will bind to binding_port.
dtkUDPSocket::dtkUDPSocket(int domain, const char *binding_port)
{
  init(domain, binding_port);
}


// This will set the remote address and port to write to.
dtkUDPSocket::
dtkUDPSocket(const char *retmote_address_port, int domain)
{
  init(domain, NULL);
  if(isInvalid()) return;
  if(setRemote(retmote_address_port))
    {
      remove_object();
      return;
    }
}


// If bind_port is set, this will bind to binding_port.
void dtkUDPSocket::init(int domain_in, const char *binding_port)
{
  int i = isInvalid();
  setType(DTKUDPSOCKET_TYPE);
  if(i)
	return; // dtkSocket::dtkSocket() failed

  // check domain
  if(domain_in == PF_INET || domain_in == PF_UNIX)
    domain = domain_in;
  else
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkUDPSocket initialization failed:"
		 " domain %d is unsupported.\n"
		 "dtkUDPSocket supported domains"
		 " (or Protocol families) are"
		 "PF_INET=%d and PF_UNIX=%d as defined in"
		 " /usr/include/sys/socket.h .\n",
		 domain_in, PF_INET, PF_UNIX);
      return;
    }

  fd = socket(domain, SOCK_DGRAM, 0);
  is_connected = 0;
  remote_addressPort.i.sin_family = AF_UNSPEC;
  bind_addressPort.i.sin_family   = AF_UNSPEC;

  if(fd == -1)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkUDPSocket initialization "
		 "failed: socket() failed:");
      return;
    }
#ifndef DTK_ARCH_WIN32_VCPP
  // bind() to a port (or file for UNIX domain)
  if(domain == PF_UNIX && /* UNIX domain UDP must be bond to recieve data */
     (binding_port == NULL || binding_port[0] == '\0'))
    {
      char *s = dtk_tempname("/tmp/dtkUDPSocket");
      if(s == NULL)
	{
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtkUDPSocket initialization failed:");
	  return;
	}
      if(Bind(s)) { free(s); return;} // Bind() declared in socket.C
      free(s);
    }
  else 
#endif /* #ifndef DTK_ARCH_WIN32_VCPP */
	  if(binding_port != NULL && binding_port[0] != '\0' && 
	  Bind(binding_port)) return; // Bind() declared in socket.C

  // set the socket to be blocking
  if(setBlocking())
    {
      remove_object();
      return;
    }

  validate();
}


dtkUDPSocket::~dtkUDPSocket(void)
{
  remove_object(); // from _socket.C in base class
}


ssize_t dtkUDPSocket::read(void *data, size_t size)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
           "dtkUDPSocket::read() failed: read() failed.\n"
           "Invalid dtkUDPSocket object.\n");
      return -1;
    }

  if(is_connected == 1)
    {
#ifdef DTK_ARCH_WIN32_VCPP
      int i = recv(fd, (char *) data,(int) size,0);
#else
      int i = recv(fd, data,(int) size,0);
#endif
      if(i == -1)
	dtkMsg.add(DTKMSG_WARN, 1,
		   "dtkUDPSocket::read() failed: recv() failed.\n");
      return (ssize_t) i;
    }
  //else // is_connected == 0 just udp or   == 3 multicast

  dtkSocklen_t fromlen;

#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_INET)
#endif
    fromlen = sizeof(struct sockaddr_in);
#ifndef DTK_ARCH_WIN32_VCPP
  else // domain == PF_UNIX
    fromlen = sizeof(struct sockaddr_un);
#endif
  
#ifdef DTK_ARCH_WIN32_VCPP
  int i = recvfrom(fd, (char *) data, (int) size, 0,
		   (struct sockaddr *) &remote_addressPort,
		   &fromlen);
#else
  int i = recvfrom(fd, data, (int) size, 0,
		   (struct sockaddr *) &remote_addressPort,
		   &fromlen);
#endif
  if(i == -1)
    dtkMsg.add(DTKMSG_WARN, 1,
	       "dtkUDPSocket::read() failed: recvfrom() failed.\n");
  return (ssize_t) i;
}

ssize_t dtkUDPSocket::write(const void *data, size_t size)
{
  ssize_t i;

  if(is_connected == 1)
    {
#ifdef DTK_ARCH_WIN32_VCPP
      i = (ssize_t) send(fd, (char *) data, (int) size, 0);
#else
      i = (ssize_t) send(fd, data, (int) size, 0);
#endif
      if(i != (ssize_t) size)
	{
	  if(i == -1)
	    dtkMsg.add(DTKMSG_WARN, 1,
		       "dtkUDPSocket::write() failed: send() failed.\n");
	  else
	    dtkMsg.add(DTKMSG_WARN, 1,
		       "dtkUDPSocket::write() failed:"
		       " send() wrote only %d bytes.\n",i);
	}	
    }
  else // is_connected == 0 just udp or 3 multicast
    {
#ifndef DTK_ARCH_WIN32_VCPP
      if(domain == PF_INET)
#endif
#ifdef DTK_ARCH_WIN32_VCPP
	i = (ssize_t) sendto(fd, (char *) data, size, 0,
			     (struct sockaddr *) &(remote_addressPort.i),
			     sizeof(struct sockaddr_in));
#else
	i = (ssize_t) sendto(fd, data, size, 0,
			     (struct sockaddr *) &(remote_addressPort.i),
			     sizeof(struct sockaddr_in));
#endif
#ifndef DTK_ARCH_WIN32_VCPP
      else  // domain == PF_UNIX
	i = (ssize_t) sendto(fd, data, size, 0,
			     (struct sockaddr *) &(remote_addressPort.u),
			     sizeof(remote_addressPort.u.sun_family) +
			     strlen(remote_addressPort.u.sun_path));
#endif
      if(i != (ssize_t)size)
	{
	  if(i == -1)
	    dtkMsg.add(DTKMSG_WARN, 1,
		       "dtkUDPSocket::write() failed: sendto() failed.\n");
	  else
	    dtkMsg.add(DTKMSG_WARN, 1,
		       "dtkUDPSocket::write() failed:"
		       " sendto() wrote only %d bytes.\n",i);
	}
    }
  return i;
}

int dtkUDPSocket::setMulticast(const char *address, int ttl_in)
{
#ifndef DTK_ARCH_WIN32_VCPP
  unsigned char ttl = ttl_in;
  struct ip_mreq mreq;

  // set default in case of failure
  remote_addressPort.i.sin_family = AF_UNSPEC;

  if(domain != PF_INET)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkUDPSocket::setMulticast(\"%s\",%d) failed:\n"
		 "This is not a internet domain socket.\n",
		 address, ttl_in);
      return -1;
    }

#if(0)
  /* turn off loopback for MULTICAST */
  unsigned char c = 0;
  if(setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &c, 1) < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkUDPSocket::setMulticast(\"%s\",%d) failed:\n"
		 "setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP,"
		 " &c, 1) failed.\n",
		 address, ttl_in);
      return -1;
    }
#endif

  remote_addressPort.i.sin_addr.s_addr =
  mreq.imr_multiaddr.s_addr = get_num_address(address);

  /* This one I don't get. What is the interface address? */
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if(setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		&mreq,sizeof(struct ip_mreq)) < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkUDPSocket::setMulticast(\"%s\",%d) failed:\n"
		 "setsockopt() for joining the MULTICAST group "
                  "failed.\n",
		 address, ttl_in);
      return -1;
    }

  if(setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkUDPSocket::setMulticast(\"%s\",%d) failed:\n"
		 "setsockopt() setting TTL to %d failed.\n",
		 address, ttl_in, ttl_in);
      return -1;
    }
  remote_addressPort.i.sin_family = AF_INET;
  is_connected = 3;
#endif
  return 0; // success
}

// read with a select() timeout.

ssize_t dtkUDPSocket::read(void *data, size_t size,
			   unsigned int seconds)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
           "dtkUDPSocket::read() failed: read() failed.\n"
           "Invalid dtkUDPSocket object.\n");
      return -1;
    }

  fd_set rfds;
  FD_ZERO(&rfds);
#ifdef DTK_ARCH_WIN32_VCPP
  FD_SET((unsigned int) fd, &rfds);
#else
  FD_SET(fd, &rfds);
#endif
  struct timeval tv = { seconds, 0};

  int j = select(fd+1, &rfds, NULL, NULL, &tv);

  // check timeout
  if(j == 0)
    {
      dtkMsg.add(DTKMSG_DEBUG,
		 "dtkUDPSocket::read() failed: "
		 "select() timed out in %d seconds.\n",
		 (int) seconds);
      return DTK_SOCKET_TIMEOUT;
    }
  if(j < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkUDPSocket::read() failed: "
		 "select() failed.\n");
      return j;
    }

  if(is_connected == 1)
    {
#ifdef DTK_ARCH_WIN32_VCPP
      int i = recv(fd, (char *) data,(int) size,0);
#else
      int i = recv(fd, data,(int) size,0);
#endif
      if(i == -1)
	dtkMsg.add(DTKMSG_WARN, 1,
		   "dtkUDPSocket::read() failed: recv() failed.\n");
      return (ssize_t) i;
    }
  //else // is_connected == 0 just udp or   == 3 multicast

  dtkSocklen_t fromlen;

#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_INET)
#endif
    fromlen = sizeof(struct sockaddr_in);
#ifndef DTK_ARCH_WIN32_VCPP
  else // domain == PF_UNIX
    fromlen = sizeof(struct sockaddr_un);
#endif

#ifdef DTK_ARCH_WIN32_VCPP
  int i = recvfrom(fd, (char *) data, (int) size, 0,
		   (struct sockaddr *) &remote_addressPort,
		   &fromlen);
#else
  int i = recvfrom(fd, data, (int) size, 0,
		   (struct sockaddr *) &remote_addressPort,
		   &fromlen);
#endif
  if(i == -1)
    dtkMsg.add(DTKMSG_WARN, 1,
	       "dtkUDPSocket::read() failed: recvfrom() failed.\n");
  return (ssize_t) i;
}
