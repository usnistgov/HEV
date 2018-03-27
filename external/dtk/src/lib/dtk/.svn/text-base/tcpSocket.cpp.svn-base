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
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
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
# include <sys/ioctl.h>
#endif

#ifdef DTK_ARCH_IRIX // for bzero()
# include <bstring.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkSocket.h"
#include "dtkTCPSocket.h"
#include "_private_socket.h"

void dtkTCPSocket::
init(int domain_in, const char *binding_port, int backlog)
{
  setType(DTKTCPSOCKET_TYPE);
  is_connected = 0;
  remote_addressPort.i.sin_family = AF_UNSPEC;
  bind_addressPort.i.sin_family   = AF_UNSPEC;

  // check domain
  if(domain_in == PF_INET || domain_in == PF_UNIX)
    domain = domain_in;
  else
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkTCPSocket initialization failed:"
		 " domain %d is unsupported.\n"
		 "dtkTCPSocket supported domains"
		 " (or Protocol families) are"
		 "PF_INET=%d and PF_UNIX=%d .\n",
		 domain_in, PF_INET, PF_UNIX);
      return;
    }

  fd = socket(domain, SOCK_STREAM, 0);
  if(fd < 0)
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkTCPSocket initialization failed: "
		 "socket() failed:");
      return;
    }
#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_INET)
    {
#endif
      // Turn the Nagle algorithm off. This means that packets are
      // always sent as soon as possible and no unnecessary delays are
      // introduced, at the cost of more packets in the network.
#if defined DTK_ARCH_LINUX || defined DTK_ARCH_WIN32_VCPP || defined DTK_ARCH_CYGWIN
      int data = 1;
      if(setsockopt(fd, 
#ifdef DTK_ARCH_WIN32_VCPP
		  SOL_SOCKET, TCP_NODELAY, (const char *) &data
#else /* other OS */
	      SOL_TCP, TCP_NODELAY, &data
#endif
		  , sizeof(int)))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTCPSocket initialization failed:\n"
		     "setsockopt(%d, SOL_TCP, TCP_NODELAY, &1,) failed.\n",
		     fd);
	  remove_object();
	  return;
	}
#endif

#if 0
      data = 0;
      if(ioctl(fd, FIOASYNC, &data))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtkTCPSocket initialization failed:\n"
		     "ioctl(%d, FIOASYNC, &0) failed.\n",
		     fd);
	  remove_object();
	  return;
	}
#endif


#ifndef DTK_ARCH_WIN32_VCPP
    }
#endif

  // bind() to a port
  // Bind() declared in socket.cpp
  if(binding_port != NULL && binding_port[0] != '\0' && Bind(binding_port))
    return;

  // if bound set backlog
  if(bind_addressPort.i.sin_family != AF_UNSPEC && listen (fd, backlog))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkTCPSocket initialization failed: "
		 "listen(%d,%d) failed.\n",
		 fd,backlog);
      remove_object();
      return;
    }

  // set the socket to be blocking
  if(setBlocking())
    {
      remove_object();
      return;
    }

  validate();
}

dtkTCPSocket::dtkTCPSocket(int domain, const char *binding_port,
			   int backlog)
{
  init(domain, binding_port, backlog);
}

dtkTCPSocket::dtkTCPSocket(int domain)
{
  init(domain, NULL, 0);
}

dtkTCPSocket::dtkTCPSocket(const char *address_port, int domain)
{
  init(domain, NULL, 0);
  if(isInvalid() ||
     setRemote(address_port) ||
     connect())
    {
      remove_object();
      invalidate();
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkTCPSocket::dtkTCPSocket(%s, \"%s\")"
		 " failed.\n",
		 (domain==PF_INET)?"PF_INET":
		 ((domain==PF_UNIX)?"PF_UNIX":"?"),
		 address_port);
    }
}

dtkTCPSocket::~dtkTCPSocket(void)
{
  remove_object();
}

// constructor for slave sockets

dtkTCPSocket::dtkTCPSocket(int domain_in, int fd_in,
			   struct sockaddr *a, int addr_length)
{
  is_connected = 2; // connected slave socket
  fd = fd_in;
  domain = domain_in;
  setType(DTKTCPSOCKET_TYPE);

  memcpy((void *) &(remote_addressPort.i),(const void *) a,
	 (size_t) addr_length);
  validate();
}


dtkSocket *dtkTCPSocket::accept(unsigned int seconds)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkTCPSocket::accept() failed: "
		 "Not a valid dtkTCPSocket object.\n"
		 "Bad magic number.\n");
      return NULL;
    }

  // We use select() for adding a timeout or so that accept() will not
  // be called when there is no incomming connect.  select() will
  // return when a signal comes, but accept() will not.
    {
      fd_set rfds;
      FD_ZERO(&rfds);
#ifdef DTK_ARCH_WIN32_VCPP
      FD_SET((unsigned int) fd, &rfds);
#else
      FD_SET(fd, &rfds);
#endif
      int j;
      if(seconds)
	{
	  struct timeval tv = { seconds , 0L };
	  //tv.tv_sec = (time_t) seconds;
	  //tv.tv_usec = 0L;
	  // Block with timeout.
	  j = select(fd+1, &rfds, NULL, NULL, &tv);
	}
      else
	// Block until there is data.
	j = select(fd+1, &rfds, NULL, NULL, NULL);
      
      // check timeout
      if(j == 0)
	{
	  dtkMsg.add(DTKMSG_INFO,
		     "dtkTCPSocket::accept(): "
		     "select() timed out in %d seconds.\n",
		     seconds);
	  return NULL;
	}
      if(j < 0)
	{
	  dtkMsg.add(DTKMSG_WARN, 1,
		     "dtkTCPSocket::accept(%d) failed: "
		     "select() failed.\n",seconds);
	  return NULL;
	}
    }

  dtkSocklen_t fromlen = sizeof(remote_addressPort);

  int new_fd = ::accept(fd, (struct sockaddr *) &remote_addressPort, &fromlen);

  if(new_fd < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkTCPSocket::accept() failed: "
		 "accept(%d,,) failed.\n",
		 fd);
      return NULL;
    }
#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_UNIX)
    {
      remote_addressPort.u.sun_family = AF_UNIX;
      strcpy(remote_addressPort.u.sun_path,"connected_UNIX_domain");
      fromlen = sizeof(remote_addressPort);
    }
#endif

  return new
    dtkTCPSocket(domain, new_fd,
		 (struct sockaddr *) &remote_addressPort, fromlen);
}

dtkSocket *dtkTCPSocket::accept(void)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkTCPSocket::accept() failed: "
		 "Not a valid dtkTCPSocket object.\n"
		 "Bad magic number.\n");
      return NULL;
    }

  dtkSocklen_t fromlen = sizeof(remote_addressPort);
  int new_fd = ::accept(fd, (struct sockaddr *) &remote_addressPort, &fromlen);

  if(new_fd < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkTCPSocket::accept() failed: "
		 "accept(%d,,) failed.\n",
		 fd);
      return NULL;
    }

#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_UNIX)
    {
      remote_addressPort.u.sun_family = AF_UNIX;
      strcpy(remote_addressPort.u.sun_path,"connected_UNIX_domain");
      fromlen = sizeof(remote_addressPort);
    }
#endif

  return new
    dtkTCPSocket(domain, new_fd,
		 (struct sockaddr *) &remote_addressPort, fromlen);
}

ssize_t dtkTCPSocket::read(void *data, size_t size)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
           "dtkTCPSocket::read() failed: read() failed.\n"
           "Invalid dtkTCPSocket object.\n");
      return -1;
    }
  ssize_t i = 
#ifdef DTK_ARCH_WIN32_VCPP
	  recv(fd, (char *) data, size, 0);
#else
	  ::read(fd, data, size);
#endif
  if(i==-1)
    dtkMsg.add(DTKMSG_WARN, 1,
	       "dtkTCPSocket::read() failed: "
	       "read(%d,,) failed.\n",
	       fd);
  return i;
}

ssize_t dtkTCPSocket::write(const void *data, size_t size)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
           "dtkTCPSocket::write() failed: write() failed.\n"
           "Invalid dtkTCPSocket object.\n");
      return -1;
    }
  ssize_t i = 
#ifdef DTK_ARCH_WIN32_VCPP
	  send(fd, (char *) data, size, 0);
#else
	  ::write(fd, data, size);
#endif
  if(i != (ssize_t)size)
    {
      if(i == -1)
	dtkMsg.add(DTKMSG_WARN, 1,
		   "dtkTCPSocket::write() failed: write() failed.\n");
      else
	dtkMsg.add(DTKMSG_WARN, 1,
		   "dtkTCPSocket::write() failed:"
		   " write() wrote only %d bytes "
		   "out of %d.\n",i,size);
    }
  return i;
}


ssize_t dtkTCPSocket::read(void *data, size_t size,
			   unsigned int seconds)
{
  if(isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
           "dtkTCPSocket::read() failed: read() failed.\n"
           "Invalid dtkTCPSocket object.\n");
      return -1;
    }

  fd_set rfds;
  FD_ZERO(&rfds);
#ifdef DTK_ARCH_WIN32_VCPP
  FD_SET((unsigned int) fd, &rfds);
#else
  FD_SET(fd, &rfds);
#endif
  struct timeval tv = { seconds, 0L};
  //tv.tv_sec = (time_t) seconds;
  //tv.tv_usec = 0L;

  int j = select(fd+1, &rfds, NULL, NULL, &tv);

  // check timeout
  if(j == 0)
    {
      dtkMsg.add(DTKMSG_DEBUG,
		 "dtkTCPSocket::read(): "
		 "select() timed out in %d seconds.\n",
		 (int) seconds);
      return DTK_SOCKET_TIMEOUT;
    }
  if(j < 0)
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtkTCPSocket::read() failed: "
		 "select() failed.\n");
      return j;
    }
  ssize_t i =
#ifdef DTK_ARCH_WIN32_VCPP
  recv(fd, (char *) data, size, 0);
#else
  ::read(fd, data, size);
#endif
  if(i==-1)
    dtkMsg.add(DTKMSG_WARN, 1,
	       "dtkTCPSocket::read() failed: "
	       "read(%d,,) failed.\n",
	       fd);
  return i;
}
