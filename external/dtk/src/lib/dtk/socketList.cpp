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
#include <sys/types.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef DTK_ARCH_WIN32_VCPP
#  include <winsock2.h>
#  include <memory.h>
#else
#  include <sys/un.h>
#  include <netinet/in.h>
#endif


#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkSocket.h"
#include "utils.h"
#include "dtkSocketList.h"

dtkSocketList::dtkSocketList(void)
{
  first = NULL;
  //delete_in_destructor = 0;
}

// This will delete any sockets that are in the list.
dtkSocketList::~dtkSocketList(void)
{
  struct dtkSocketList_list *l = first;
  for(;l;l=first)
    {
      first = l->next;
      if(l->sock)
	delete l->sock;
      free(l->addressPort);
      free(l);
    }
}

dtkSocket *dtkSocketList::get(const char *addressPort)
{
  struct dtkSocketList_list *l = first;
  //  char *addressPort = dtk_getCommonAddressPort(addressPort_in);
  //  if(!addressPort) addressPort = (char *) addressPort_in;
  for(;l;l=l->next)
    {
      if(!strcmp(l->addressPort,addressPort))
	return l->sock;
    }
  return NULL;
}

dtkSocket *dtkSocketList::get(void)
{
  if(first)
    return first->sock;
  return NULL;
}

char *dtkSocketList::get(const dtkSocket *s)
{
  struct dtkSocketList_list *l = first;
  for(;l;l=l->next)
    {
      if(l->sock == s)
	return l->addressPort;
    }
  return NULL;
}

// This will not check if it is present; if your that sloppy with
// sockets you need to rethink what you're doing.

int dtkSocketList::add(dtkSocket *s, const char *addressPort_in)
{
  char *addressPort;
  if(!addressPort_in || addressPort_in[0] == '\0')
    {
      addressPort_in = s->getRemote();
      if(!addressPort_in || addressPort_in[0] == '\0') return -1; // error
      addressPort = dtk_getCommonAddressPort(addressPort_in);
      if(!addressPort)
	addressPort = (char *) addressPort_in;
    }
  else
    addressPort = (char *) addressPort_in;

  struct dtkSocketList_list *l =
    (struct dtkSocketList_list *)
    dtk_malloc(sizeof(struct dtkSocketList_list));

  l->addressPort = dtk_strdup(addressPort);
  l->sock = s;

  // add it to the first in the stack

  l->next = first;
  first = l;

  return 0;
}

int dtkSocketList::remove(const char *addressPort)
{
  if(!addressPort || addressPort[0] == '\0') return -1;
  struct dtkSocketList_list *l = first;
  struct dtkSocketList_list *prev = NULL;
  for(;l;l=l->next)
    {
      if(!strcmp(l->addressPort,addressPort))
	break;
      prev = l;
    }
  if(l)
    {
      if(prev) prev->next = l->next;
      else first = l->next;
      free(l->addressPort);
      free(l);
      return 0;
    }
  return -1; // error not found
}

int dtkSocketList::remove(const dtkSocket *sock)
{
  struct dtkSocketList_list *l = first;
  struct dtkSocketList_list *prev = NULL;
  for(;l;l=l->next)
    {
      if(l->sock == sock)
	break;
      prev = l;
    }
  if(l)
    {
      if(prev) prev->next = l->next;
      else first = l->next;
      free(l->addressPort);
      free(l);
      return 0;
    }
  return -1; // error not found
}
