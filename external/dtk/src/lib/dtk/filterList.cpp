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
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#ifdef DTK_ARCH_IRIX
   typedef uint64_t u_int64_t;
#endif

#include "types.h"
#include "dtkBase.h"
#include "dtkFilter.h"
#include "filterList.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"

#include "_private_sharedMem.h"

FilterList::FilterList(void)
{
  first = NULL;
  current = NULL;
}

void FilterList::_free(struct Filter_list *l)
{
  if(l->name) dtk_free(l->name);
  if(l->dso_file) dtk_free(l->dso_file);
  if(l->argv)
    {
      int i=0;
      for(;i<l->argc;i++)
	if((l->argv)[i]) dtk_free((l->argv)[i]);
      dtk_free(l->argv);
    }
}

FilterList::~FilterList(void)
{
  // remove the whole list.
  while(first)
    {
      struct Filter_list *l = first;
      first = first->next;
      _free(l);
    }
  current = NULL;
}

void FilterList::add(dtkFilter *fltr,
		     void *loaderFunc,
		     const char *name,
		     const char *dso_file,
		     int argc, const char **argv, pid_t pid)
{
  struct Filter_list *l = (struct Filter_list *)
    dtk_malloc(sizeof(struct Filter_list));

  l->loaderFunction = loaderFunc;
  l->filter = fltr;
  l->doRead = 1;
  l->doWrite = 1;
  l->pid = pid;
  l->name = dtk_strdup(name);
  l->argc = argc;
  l->dso_file = dtk_strdup(dso_file);
  l->argv = (char **) dtk_malloc((argc+1)*sizeof(char *));
  l->next = NULL;
  int i=0;
  for(;i<argc;i++)
    (l->argv)[i] = dtk_strdup(argv[i]);
  (l->argv)[i] = NULL;

  if(first)
    {
      struct Filter_list *last = first;
      while(last->next) last = last->next;
      last->next = l;
      return;
    }
  else
    current = first = l;
}

void FilterList::remove(dtkFilter *fltr)
{
  struct Filter_list *l = first, *prev = NULL;
  for(;l;l = l->next)
    {
      if(l->filter == fltr)
	{
	  if(prev)
	    prev->next = l->next;
	  else
	    first = l->next;

	  if(current == l)
	    current = l->next;

	  _free(l);
	  return;
	}
      prev = l;
    }
}

void FilterList::remove(const char *name)
{
  struct Filter_list *l = first;
  struct Filter_list *prev = NULL;
  for(;l;l = l->next)
    {
      if(!strcmp(l->name,name))
	{
	  if(prev)
	    prev->next = l->next;
	  else
	    first = l->next;

	  if(current == l)
	    current = l->next;

	  _free(l);
	  return;
	}
      prev = l;
    }
}

// do not mess with the returned memory.
void *FilterList::getLoaderFunction(const char *name)
{
  if(!name)
    {
      if(current) return current->loaderFunction;
      return NULL;
    }

  struct Filter_list *l = first;
  for(;l;l = l->next)
    if(!strcmp(l->name, name))
      return l->loaderFunction;
  return NULL; 
}

dtkFilter *FilterList::getFilter(const char *name)
{
  struct Filter_list *l = first;
  for(;l;l = l->next)
    if(!strcmp(l->name, name))
      return l->filter;
  return NULL; 
}

char *FilterList::getName(dtkFilter *fltr)
{
  if(!fltr)
    {
      if(current) return current->name;
      return NULL;
    }

  struct Filter_list *l = first;
  for(;l;l = l->next)
    if(l->filter == fltr)
      return l->name;
  return NULL;
}

struct Filter_list *FilterList::getFilterList(const char *name)
{
  if(!name)
    return current;

  struct Filter_list *l = first;
  for(;l;l = l->next)
    if(!strcmp(l->name, name))
      return l;
  return NULL; 
}

void FilterList::markNeedUnloading(const char *name)
{
  if(!name)
    {
      if(current)
	{
	  if(current->dso_file) dtk_free(current->dso_file);
	  current->dso_file = NULL;
	  return;
	}
      return;
    }

  struct Filter_list *l = first;
  for(;l;l = l->next)
    if(!strcmp(l->name, name))
      {
	if(l->dso_file) dtk_free(l->dso_file);
	l->dso_file = NULL;
	break;
      }
}
