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
# include <sys/un.h>
# include <netinet/in.h>
# include <stdarg.h>
# include <sys/types.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <errno.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <limits.h>
# include <sys/time.h>
#endif /* #else #ifdef DTK_ARCH_WIN32_VCPP */

#ifdef DTK_ARCH_IRIX
# include <bstring.h>
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkSocket.h"
#include "_private_socket.h"

#ifdef DTK_ARCH_WIN32_VCPP
int dtkSocket::number_of_objects = 0;
struct WSAData dtkSocket::wsaData;
#endif


dtkSocket::dtkSocket(void)
{
#ifdef DTK_ARCH_WIN32_VCPP
  number_of_objects++;
  if(number_of_objects == 1 && 
     WSAStartup(MAKEWORD(2,0), &wsaData))
    {
      dtkMsg.add(DTKMSG_WARN, "dtkSocket::dtkSocket() failed:\n"
		 "WSAStartup() failed: No windows socket for you.\n");
      return;
    }
#endif /* #ifdef DTK_ARCH_WIN32_VCPP */
	validate();
}

dtkSocket::~dtkSocket(void)
{
#ifdef DTK_ARCH_WIN32_VCPP
  number_of_objects--;
  if(number_of_objects == 0)
    WSACleanup();
#endif /* #ifdef DTK_ARCH_WIN32_VCPP */
}



// RETURN 0 on failure

static DTK_port_type get_portNumber(const char *port_in, const char *type)
{
  struct servent  *pse;   /* pointer to service information entry */

  if ((pse = getservbyname(port_in,type)))
    return pse->s_port;

  int i = atoi(port_in);
  unsigned short ushort_max = (unsigned short) -1;
  if(i > ((int) ushort_max)) // port number is too large.
    {
      dtkMsg.add(DTKMSG_WARN,
		 "Getting port number failed: "
		 "port \"%s\" is an invalid port.\n",
		 port_in);
      return 0; // failure
    }

  errno = 0;
  return htons((DTK_port_type) i);
}

#define SIZEOFUNIXPATHM1 ((size_t) 103)
#define SIZEOFUNIXPATH   ((size_t) 104)

#ifndef DTK_ARCH_WIN32_VCPP

/* See /usr/include/sys/un.h .  Linux has 108.  IRIX has 104. So I use
 * the less of the two minus one. I don't know why they don't define
 * it as a MICRO.  */

static int set_UNIX_address(const char *path, struct sockaddr_un *addr)
{
  addr->sun_family = AF_UNIX;

  if(path[0] == '/') /* have full pathname */
    {
      if(strlen(path) > SIZEOFUNIXPATHM1)
        return -1;
      strcpy(addr->sun_path,path);
    }
  else /* append pwd to make full path */
    {
      char s[SIZEOFUNIXPATHM1+1], pwd[SIZEOFUNIXPATHM1+1];

      if(!getcwd(pwd,SIZEOFUNIXPATHM1))
	{
	  errno = 0;
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "getcwd(%p,%d) failed.\n",
		     pwd,SIZEOFUNIXPATHM1);
	  return -1;
	}
      snprintf(s,SIZEOFUNIXPATHM1,"%.148s/%.148s",pwd,path);
      if(strlen(s) > SIZEOFUNIXPATHM1)
        return dtkMsg.add(DTKMSG_ERROR, 0, -1,
			  "address \"%s\" is longer than"
			  " %d.\n",path, SIZEOFUNIXPATHM1);
      strcpy(addr->sun_path,s);
    }
  return 0;
} 
#endif



DTK_address_type get_num_address(const char *address)
{
  if(address == NULL)
    return INADDR_NONE;

  struct hostent *h;

  if((h = gethostbyname(address)))
    {
      DTK_address_type *x = (DTK_address_type *) h->h_addr_list[0];
      /* printf("address = %s address length = %d %d\n",
	   address,h->h_length,sizeof(DTK_address_type));
      */
  
      return (DTK_address_type) *x;
    }
  DTK_address_type i = (DTK_address_type) inet_addr(address);
  errno = 0;
  return i;
}



dtkSocket *dtkSocket::accept(void)
{
  // For UDP socket there is no connecting.
  // For TCP socket this will be over-written.
  return this;
}


int dtkSocket::setBlocking(void)
{
#ifdef DTK_ARCH_WIN32_VCPP
  u_long arg = 0;
  int i=ioctlsocket(fd, FIONBIO, &arg);
  if(i)
  { 
      dtkMsg.add(DTKMSG_ERROR,
		 "dtk*Socket::setBlocking() failed:\n"
		 "Can't set socket to be blocking "
		 "ioctlsocket(%d, FIONBIO, %p)=%d failed:\n"
		 "winSock error number %d.\n",
		 fd, &arg, i, WSAGetLastError());
      return -1;
    }

#else
  int i;
  // set the socket to be blocking
  if(-1 == (i = fcntl(fd,F_GETFL, 0)) || 
     -1 == fcntl(fd,F_SETFL, i & (~O_NONBLOCK)))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtk*Socket::setBlocking() failed:\n"
		 "Can't set socket to be blocking "
		 "fcntl(%d,) failed.\n",
		 fd);
      return -1;
    }
#endif

  return 0;
}


int dtkSocket::setNonblocking(void)
{
#ifdef DTK_ARCH_WIN32_VCPP
  u_long arg = 1;
  int i=ioctlsocket(fd, FIONBIO, &arg);
  if(i)
  {
	dtkMsg.add(DTKMSG_ERROR,
		 "dtk*Socket::setNonblocking() failed:\n"
		 "Can't set socket to be nonblocking "
		 "ioctlsocket(%d, FIONBIO, %p)=%d failed:\n"
		 "winSock error number %d.\n",
		 fd, &arg, i, WSAGetLastError());
     return -1;
    }
#else
  int i;
  // set the socket to be nonblocking
  if(-1 == (i = fcntl(fd,F_GETFL, 0)) || 
     -1 == fcntl(fd,F_SETFL, i | (O_NONBLOCK)))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtk*Socket::setBlocking() failed:\n"
		 "Can't set socket to be nonblocking "
		 "fcntl(%d,) failed.\n",
		 fd);
      return -1;
    }
#endif
  return 0;
}


// dtkSocket::Bind() is protected.
// It's just called by constuctors.

int dtkSocket::Bind(const char *port)
{

/* I don't think windows supports UNIX domain sockets */
#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_INET)
    {
#endif
      char type[4] = {"udp"};
      if(getType() == DTKTCPSOCKET_TYPE)
	  { type[0] = 't'; type[1] = 'c'; type[2] = 'p'; }


      if((bind_addressPort.i.sin_port =
	 get_portNumber(port,(const char *)type)) == 0)
	  {
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk*Socket initialization failed: "
		     "can't bind to port %s\n",
		     port);
	  remove_object();
	  return -1;
	  }

      bind_addressPort.i.sin_addr.s_addr = htonl(INADDR_ANY);
      bind_addressPort.i.sin_family = AF_INET;
      // Make it so that it can bind on the port if it was
      // resently used.  Doesn't work on Linux.
      int sockopt_data;
#define OPT_TEST 0 /* for debuging */
#if(OPT_TEST)
      socklen_t size;
      if(getsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
#ifdef DTK_ARCH_WIN32_VCPP
		  (const char *)
#endif
                     &sockopt_data, &size))
	  {
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk*Socket initialization failed:\n"
		     "getsockopt(%d,SOL_SOCKET,SO_REUSEADDR,,) failed.\n",
		     fd);
	  remove_object();
	  return -1;
	  }
      printf("SO_REUSEADDR is set to 0x%x  size = %d\n",sockopt_data,size);
#endif
      // Indicates that the rules used in validating addresses
      // supplied in a bind(2) call should allow reuse of local
      // addresses. For PF_INET sockets this means that a socket may
      // bind, except when there is an active listening socket bound
      // to the address. When the listening socket is bound to
      // INADDR_ANY with a specific port then it is not possible to
      // bind to this port for any local address.
      sockopt_data = 1;
      if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
#ifdef DTK_ARCH_WIN32_VCPP
		  (const char *)
#endif
                    &sockopt_data, sizeof(int)))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk*Socket initialization failed:\n"
		     "setsockopt(%d,SOL_SOCKET,SO_REUSEADDR,,) failed.\n",
		     fd);
	  remove_object();
	  return -1;
	}
#if(OPT_TEST)
      if(getsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
#ifdef DTK_ARCH_WIN32_VCPP
		  (const char *)
#endif
                     &sockopt_data, &size))
	{
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk*Socket initialization failed:\n"
		     "setsockopt(%d,SOL_SOCKET,SO_REUSEADDR,,) failed.\n",
		     fd);
	  remove_object();
	  return -1;
	}
      printf("SO_REUSEADDR is set to 0x%x  size = %d\n",sockopt_data,size);
#endif
      if(bind(fd,(struct sockaddr *) &(bind_addressPort.i),
	      sizeof(struct sockaddr_in)))
	  {
	    dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk*Socket initialization failed:\n"
		     "bind() to internet port %s failed.\n",
		     port);
	   remove_object();
	   return -1;
	  }
      return 0;
#ifndef DTK_ARCH_WIN32_VCPP
    }
  else // if(domain == PF_UNIX)
    {
      if(set_UNIX_address(port,&(bind_addressPort.u)))
	  {
	  dtkMsg.add(DTKMSG_ERROR,
		     "dtk*Socket initialization failed:\n"
		     "bind() to UNIX domain (local) "
		     "port %s failed.\n"
		     "Can't set socket address to %s .\n",
		     port,port);
	  remove_object();
	  return -1;
	  }
      int len = sizeof(bind_addressPort.u);
      if(bind(fd,(struct sockaddr *) &(bind_addressPort.u),len))
	  {
	  dtkMsg.add(DTKMSG_ERROR, 1,
		     "dtk*Socket initialization failed: "
		     "bind() to UNIX domain file %s failed.\n",
		     port);
	  remove_object();
	  return -1;
	  }
      return 0;
    }
#endif /* #ifndef DTK_ARCH_WIN32_VCPP */
}


// address_in is like "moo.cte.com:2134"
// or filename for UNIX domain

int dtkSocket::setRemote(const char *addressPort_in)
{
  if(isValid(DTKUDPSOCKET_TYPE) &&
     isValid(DTKTCPSOCKET_TYPE))
    return -1;

  if(addressPort_in == NULL || addressPort_in[0] == '\0')
    {
      remote_addressPort.i.sin_family = AF_UNSPEC;
#ifndef DTK_ARCH_WIN32_VCPP
      if(domain == PF_INET)
	{
#endif

	  remote_addressPort.i.sin_addr.s_addr = 0;
	  remote_addressPort.i.sin_port = 0;
#ifndef DTK_ARCH_WIN32_VCPP
	}
      else // domain == PF_UNIX
	remote_addressPort.u.sun_path[0] = '\0';
#endif
    }
  else if(domain == PF_INET)
    {
      remote_addressPort.i.sin_family = AF_UNSPEC;
      remote_addressPort.i.sin_addr.s_addr = 0;
      remote_addressPort.i.sin_port = 0;

      if(addressPort_in == NULL || addressPort_in[0] == '\0')
	return 0;
      
      /*********************** get host and port ***************/
      
      char *host = strdup(addressPort_in);
      if(host == NULL)
	{
	  dtkMsg.add(DTKMSG_WARN, 1,
		     "dtk*Socket::setRemoteAddress() failed: "
		     "strdup() failed.\n");
	  return -1;
	}

      char *port;
      for(port = host;*port && *port != ':';port++);
      if(*port)
	{
	  *port = '\0';
	  port++;
	}
      
      //printf("%s %s\n",host,port);
      char type[4] = {"udp"};
       if(getType() == DTKTCPSOCKET_TYPE)
	{ type[0] = 't'; type[1] = 'c'; type[2] = 'p'; }
       remote_addressPort.i.sin_port =
	 get_portNumber(port, (const char *) type);
      if((remote_addressPort.i.sin_addr.s_addr = get_num_address(host))
	 == INADDR_NONE)
	{
	  dtkMsg.add(DTKMSG_WARN, 1,
		     "dtk*Socket::setRemoteAddress(\"%s\") failed: "
		     "can't validate internet address %s .\n",
		     addressPort_in, host);
	  free(host);
	  return -1;
	}
      remote_addressPort.i.sin_family = AF_INET;
      free(host);
      
      return 0;
    }

#ifndef DTK_ARCH_WIN32_VCPP
  // else if(domain == PF_UNIX)

  if(set_UNIX_address(addressPort_in,&(remote_addressPort.u)))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtk*Socket::setRemoteAddress(\"%s\") failed: "
		 "can't validate UNIX (local) "
		 "domain address %s .\n",
		 addressPort_in, addressPort_in);
      return -1;
    }
#endif

  return 0;
}



void dtkSocket::remove_object(void)
{
#ifndef DTK_ARCH_WIN32_VCPP
  if(fd > -1 &&
     (is_connected == 2 ||
      (domain == PF_INET && isValid(DTKTCPSOCKET_TYPE)))
     )
    {
      shutdown(fd, 2);
      is_connected = 0;
    }

  if(fd > -1)
    {
      close(fd);
      fd = -1;
    }
  
  if(domain == PF_UNIX && bind_addressPort.u.sun_family == AF_UNIX)
    {
      dtkMsg.add(DTKMSG_DEBUG, "removing UNIX socket file \"%s\"\n",
		 bind_addressPort.u.sun_path);
      unlink(bind_addressPort.u.sun_path);
      bind_addressPort.u.sun_family = AF_UNSPEC;
    }

#else /* #ifndef DTK_ARCH_WIN32_VCPP */

  if(fd != NULL)
  {
	  closesocket(fd);
	  fd = NULL;
  }

#endif /* #ifndef DTK_ARCH_WIN32_VCPP */
  invalidate();
}


int dtkSocket::connect(void)
{
  if(isInvalid())
    return -1;

  int i;
  size_t len;

  // You should not connect a TCP socket more then once
  if(isValid(DTKTCPSOCKET_TYPE) && is_connected > 0)
    {
      // shutdown(fd, 2);
      return -1;
    }
  // multicast is set if is_connected == 3
  else if(isValid(DTKUDPSOCKET_TYPE) && is_connected == 3)
    return -1;

#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_INET)
#endif
    len = sizeof(struct sockaddr_in);

#ifndef DTK_ARCH_WIN32_VCPP
  else  // domain == PF_UNIX // not on windows
    len = sizeof(struct sockaddr_un);
#endif

  int will_disconnect_datagram =
    (isValid(DTKUDPSOCKET_TYPE) &&
     remote_addressPort.i.sin_family == AF_UNSPEC)?1:0;

  if((i = ::connect(fd, (struct sockaddr *) &(remote_addressPort),len)))
    {
      dtkMsg.add(DTKMSG_WARN, 1,
		 "dtk*Socket::connect() failed: connect() failed.\n");
      is_connected = 0;
    }
  else if(will_disconnect_datagram)
    is_connected = 0;
  else
    is_connected = 1;

  return i;
}


char *dtkSocket::getRemote(void)
{
  if(isInvalid()) return NULL;

  static char addressPort[SIZEOFUNIXPATH+1];

#ifndef DTK_ARCH_WIN32_VCPP
  if(domain == PF_INET)
    {
#endif
      if(remote_addressPort.i.sin_family == AF_UNSPEC)
	     return NULL;
      snprintf(addressPort,SIZEOFUNIXPATH,"%s:%d",
	      inet_ntoa(remote_addressPort.i.sin_addr),
	      ntohs(remote_addressPort.i.sin_port));
      return addressPort;
#ifndef DTK_ARCH_WIN32_VCPP
    }
  else // domain == PF_UNIX
    {
      if(remote_addressPort.i.sin_family == AF_UNSPEC)
	return NULL;
      snprintf(addressPort,SIZEOFUNIXPATH,"%s",
	      remote_addressPort.u.sun_path);
      return addressPort;
    }
#endif
}


void dtkSocket::print(FILE *file)
{
  if(!file) file = stdout;
  if(isValid())
    {
      if(isValid(DTKUDPSOCKET_TYPE))
	    fprintf(file,"DTK %sUDP%s ",
		dtkMsg.color.red, dtkMsg.color.end);
      else // TCP
	fprintf(file,"DTK %sTCP%s ",
		dtkMsg.color.red, dtkMsg.color.end);
      if(domain == PF_INET)
	fprintf(file,"%sinternet domain%s ",
		dtkMsg.color.yel, dtkMsg.color.end);
#ifndef DTK_ARCH_WIN32_VCPP
      else // domain == PF_UNIX
	fprintf(file,"%sUNIX local domain%s ",
		dtkMsg.color.yel, dtkMsg.color.end);
#endif

      fprintf(file,"socket file descriptor="
	      "%s%d%s", dtkMsg.color.grn, fd, dtkMsg.color.end);

      if(bind_addressPort.i.sin_family != AF_UNSPEC)
	{
	  if(domain == PF_INET)
	    fprintf(file,"\nis bound to local internet port: %s%d%s\n",
		    dtkMsg.color.grn, (int)ntohs(bind_addressPort.i.sin_port),
		    dtkMsg.color.end);
#ifndef DTK_ARCH_WIN32_VCPP
	  else // domain == PF_UNIX
	    fprintf(file,"\nis bound to local file: %s%s%s\n",
		    dtkMsg.color.grn, bind_addressPort.u.sun_path,
		    dtkMsg.color.end);
#endif
	}
      else
	fprintf(file,"\n");

      if(is_connected == 1) // TCP or UDP (not in below)
	fprintf(file,"connected to ");
      else if(is_connected == 2) // TCP that has returned from accept()
	fprintf(file,"is a slave connected to ");
      else if(is_connected == 3) // UDP MULTICAST
	fprintf(file,"connected to MULTICAST group address: %s%s%s\n",
		dtkMsg.color.tur,
		inet_ntoa(remote_addressPort.i.sin_addr),
		dtkMsg.color.end);

      if(remote_addressPort.i.sin_family == AF_INET && is_connected != 3)
	fprintf(file,"remote internet address: %s%s:%d%s\n",
		dtkMsg.color.tur,
		inet_ntoa(remote_addressPort.i.sin_addr),
		(int) ntohs(remote_addressPort.i.sin_port), dtkMsg.color.end);
#ifndef DTK_ARCH_WIN32_VCPP
      else if(remote_addressPort.u.sun_family == AF_UNIX)
	fprintf(file,"remote UNIX address: %s%s%s\n",
		dtkMsg.color.tur, remote_addressPort.u.sun_path,
		dtkMsg.color.end);
#endif
    }
  else
    fprintf(file,"DTK socket object is invalid\n");
}


int dtkSocket::haveData(void)
{
  if(fd == -1) return -1;
  fd_set rfds;
  FD_ZERO(&rfds);
#ifdef DTK_ARCH_WIN32_VCPP
  FD_SET((unsigned int) fd, &rfds);
#else
  FD_SET(fd, &rfds);
#endif
  struct timeval tv = { 0, 0};
  return select(fd+1, &rfds, NULL, NULL, &tv);
}


int dtkSocket::setMulticast(const char *address, int ttl)
{
  // assume that there is just a class UDPSocket or TCPSocket
	if(isValid(DTKTCPSOCKET_TYPE))
      dtkMsg.add(DTKMSG_WARN,
	     "dtk*Socket::setMulticast(\"%s\",%d) failed: "
	     "MULTICAST is not supported on a TCP socket.\n",
	     address, ttl);
  return -1;
}

