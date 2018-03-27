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

#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"
#include "dtkSegAddrList.h"


dtkSegAddrList::dtkSegAddrList(const char *name_in)
{
  first = current = NULL;
  name = dtk_strdup(name_in);
}

int dtkSegAddrList::removeAll(void)
{
  struct dtkSegAddrList_list *l = first;
  for(;l;l=first)
    {
      first = l->next;
      free(l->name);
      free(l->address);
      free(l);
    }
  first = current = NULL;
  return 0;
}

dtkSegAddrList::~dtkSegAddrList(void)
{
  removeAll();
  if(name)
    {
      free(name);
      name = NULL;
    }
 }

int dtkSegAddrList::start(void)
{
  if((current = first))
    return 1;
  else
    return 0;
}

// returns remote shared memory name.
// return NULL if not found.
char *dtkSegAddrList::get(const char *address)
{
  struct dtkSegAddrList_list *l = first;
  for(;l;l=l->next)
    if(!strcmp(l->address,address))
      return l->name; 
  return NULL;
}

char *dtkSegAddrList::getNextAddressPort(char **remoteName)
{
  if(!current) return NULL;

  if(remoteName)
    *remoteName = current->name;
  char *ret = current->address;
  current = current->next;
  return ret;
}

int dtkSegAddrList::add(const char *address,
			const char *remoteName)
{
  if(!address /* address[0] == '\0' is a local connection */
     || !remoteName || !(remoteName[0])) return -1;

  struct dtkSegAddrList_list *l = first;
  for(;l;l=l->next)
    if(!strcmp(l->address,address))
      return 1; // already there.

  l = (struct dtkSegAddrList_list *)
    dtk_malloc(sizeof(struct dtkSegAddrList_list));
  l->name = dtk_strdup(remoteName);
  l->address = dtk_strdup(address);

  // add to the first in the stack.
  l->next = first;
  first = l;
  
  return 0; // success
}

int dtkSegAddrList::remove(const char *address)
{
  if(!address || !(address[0])) return -1;
  struct dtkSegAddrList_list *l = first, *prev = NULL;
  for(;l;l=l->next)
    {
      if(!strcmp(l->address,address))
	break;
      prev = l;
    }
  if(!l) return -1; // not found in list

  if(current == l) current = current->next;

  if(prev) prev->next = l->next;
  else first = l->next;

  free(l->address);
  free(l->name);
  free(l);

  return 0;
}
