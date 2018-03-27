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
 */
#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <stdlib.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <sys/un.h>
# include <netinet/in.h>
# include <limits.h>
#endif

#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "_private_socket.h"

// I'm trying to make this give me a unique string that IDs a
// connection to a DTK server.  The form is: "128.173.49.141:34219".
// The input, ap, may be like "foo.com" and the default port is added.

char *dtk_getCommonAddressPort(const char *ap)
{
  if(!ap || ap[0] == '\0') return NULL;
  if(strlen(ap) > 80) return NULL;

  static char address[80];
  char port[30];
  port[0] = '\0';

  { // get address string
    strcpy(address, ap);

    char *s = address;
    for(;*s && *s != ':';s++);
    if(*s == ':')
      {
	*s = '\0';
	s++;
	if(strlen(s) < 30)
	  strcpy(port,s);
	else
	  return NULL;
      }
  }

  struct hostent *h = gethostbyname(address);
  if(!h) h = gethostbyaddr(address, strlen(address), AF_INET);

  if(h) // Convert the address to a number and then back to a string.
    // I've tried easyer ways, but they didn't work on Windoz.
    {
      if(h->h_length <= 0 || h->h_addrtype != AF_INET)
	return NULL;

      struct sockaddr_in in;
      in.sin_addr.s_addr = *((DTK_address_type *) h->h_addr_list[0]);
      strncpy(address, inet_ntoa(in.sin_addr), 79);

#if 0
#ifdef DTK_ARCH_WIN32_VCPP
	  if(h->h_length > 79)
		  return NULL;
	 strncpy(address, h->h_name, 79);
#else
      if(!inet_ntop(AF_INET, h->h_addr_list[0], address, 70))
	return NULL;
#endif
#endif

    }
  else
    errno = 0;

  size_t i = strlen(address);
  address[i++] = ':';
  address[i] = '\0';

  if(!port || port[0] == '\0')
    {
      strcpy(port, DTK_DEFAULT_SERVER_PORT);
    }

  struct servent  *pse = getservbyname(port,"tcp");
  if(pse)
    {
      sprintf(&(address[i]),"%d", ntohs(pse->s_port));
    }
  else
    {
      long p = strtol(port,NULL,10);
#ifdef DTK_ARCH_WIN32_VCPP
# define USHRT_MAX ((unsigned short) -1)
#endif
      if(p <= (long) 0 || p > ((long) USHRT_MAX)) return NULL;
      sprintf(&(address[i]), "%d", (int) p);
    }

  return address;
}
