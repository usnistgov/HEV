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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

#include "dtkColor.h"
#include "dtkMessage.h"
#include "types.h"
#include "dtkBase.h"
#include "dtkAugment.h"
#include "dtkDisplay.h"
#include "dtkRecord.h"
#include "dtkDSO.h"
#include "dtkManager.h"
#include "__manager_magicNums.h"
#include "utils.h"

#define  __DTKAUGMENT_NOT_LOADED__
#include "dtkDSO_loader.h"

#ifdef DTK_ARCH_WIN32_VCPP
# define LIST_SEPERATOR  ';'
#else
# define LIST_SEPERATOR  ':'
#endif


#define DUMMYAUGMENT_TYPE  ((u_int32_t) 0x02700043)

class DummyAugment: public dtkAugment
{
public:
  DummyAugment(void);
};


DummyAugment::DummyAugment(void)
{
  setDescription("Dummy Augment");
  setType(DUMMYAUGMENT_TYPE);
  validate();
}
// Set the PATH to look for DSO files.
void dtkManager::path(const char *dso_path)
{
  if(DSOPath)
    free(DSOPath);
  DSOPath = dtk_strdup(dso_path);
}

// path() gets the PATH to look for DSO files.  Don't mess with the
// memory that this returned.
char *dtkManager::path(void) const
{
  return DSOPath;
}

int dtkManager::load(const char **file, void *arg)
{
  for(;*file;file++)
    if(load(*file, arg)) return -1;
  return 0;    // success
}

int dtkManager::load(const char *file_in, void *arg)
{
  char *s = (char *) file_in;
  for(;*s;s++)
    if(*s == LIST_SEPERATOR)
      break;

  if(!(*s))
    return _load(file_in, arg);
  else
    {
      // There is a colon (windoZ: semicolon) seperated
	  // list of files.
      char *file = dtk_strdup(file_in);
      char *mem = file;
      char *s = file;
      while(*s)
	{
	  if(*s == LIST_SEPERATOR)
	    {
	      *s = '\0';
	      if(_load(file, arg))
		{
		  free(mem);
		  return -1; // error
		}
	      file = ++s;
	    }
	  else
	    s++;
	}
      if(*file && _load(file, arg))
	{
	  free(mem);
	  return -1; // error
	}
      free(mem);
    }
  return 0;
}


// returns 0 on success and -1 on error.
int dtkManager::_load(const char *file, void *arg)
{
  dtkAugment *(*loaderFunc)(dtkManager *, void *) =
    (dtkAugment * (*)(dtkManager *, void *))
    dtkDSO_load(DSOPath, file, this, arg);

  if(!loaderFunc)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::load() failed to load DSO file"
		 " \"%s\".\n",file);
      return -1;
    }

  dtkAugment *a = loaderFunc(this, arg);

  if(a == DTKDSO_LOAD_ERROR)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::load() failed: DSO file \"%s\"\n"
		 " entry function returned an error code.\n",file);
      _unload(a, (void *) loaderFunc);
      return -1; // error
    }

  if(a == DTKDSO_LOAD_UNLOAD)
    {
      dtkMsg.add(DTKMSG_DEBUG,
		 "dtkManager::load(): DSO file \"%s\"\n"
		 " entry function returned telling to unload DSO.\n",
		 file);

      return _unload(a, (void *) loaderFunc);
    }

  dtkAugment *dummyAugment = NULL;
  if(a == DTKDSO_LOAD_CONTINUE)
    dummyAugment = a = new DummyAugment;

  if(add(a))
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::load() failed to load DSO file"
		 " \"%s\".\n",file);
      _unload(a, (void *) loaderFunc);
      return -1;
    }
  augment_list_last->loaderFunc = (void *) loaderFunc;

  if(dummyAugment)
    augment_list_last->flag = 0; // no callbacks called

  return 0;
}

// Does not remove it from the augment list.  Just calls the C++
// unloader function dtkDSO_unloader(dtkAugment *a) and unloads the
// DSO file.
int dtkManager::_unload(dtkAugment *a, void *p)
{
  int (*unloaderFunc)(dtkAugment *) = 
    (int (*)(dtkAugment *)) dtkDSO_getUnloader(p);

  if(!unloaderFunc)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::load(): unloading "
		 "DSO file failed to get "
		 "unloader function.\n");
      dtkDSO_unload(p);
      return -1; // error
    }

  if(unloaderFunc(a) == DTKDSO_UNLOAD_ERROR)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::load(): unloading DSO file "
		 "failed: unloader function failed.\n");
      dtkDSO_unload(p);
      return -1; // error
    }

  // dtkDSO_unload() returns -1 on error or 0 on success
  return dtkDSO_unload(p); 
}

// returns 0 on success and -1 on error.
int dtkManager::add(dtkAugment *a)
{
  if(a->isInvalid())
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::add() failed: "
		 "dtkAugment is invalid.\n");
      return -1;
    } 

  if(check_augment(a->getName()))
    {
      dtkMsg.add(DTKMSG_WARN,
		 "dtkManager::add() failed:\n"
		 "  a dtkAugment with name \"%s\" is"
		 " already loaded.\n", a->getName());
      return -1;
    }

  { // add to dtkAugment_list
    struct dtkAugment_list *l = (struct dtkAugment_list *)
      dtk_malloc(sizeof(struct dtkAugment_list));
    l->augment = a;
    l->next = NULL;
    l->loaderFunc = NULL;
    l->flag = ALL_CALLBACKS;

    if(a->getType() == DTKDISPLAY_TYPE)
      l->flag |= DTK_ISDISPLAY;

    if(!augment_list_last)
      {
	augment_list_first = l;
	l->prev = NULL;
      }
    else
      {
	l->prev = augment_list_last;
	augment_list_last->next = l;
      }

    augment_list_last = l;

    addCallback(l->augment, DTK_PREFRAME);
    addCallback(l->augment, DTK_POSTFRAME);
  }

  // add to display callback list

  if(a->getType() == DTKDISPLAY_TYPE)
    { // display_list

      struct dtkDisplay_list *l = (struct dtkDisplay_list *)
	dtk_malloc(sizeof(struct dtkDisplay_list));
      l->display = (dtkDisplay *) a;
      l->next = NULL;
      l->flag = ALL_CALLBACKS;
      if(display_list)
	{
	  struct dtkDisplay_list *c = display_list;
	  while(c->next) c = c->next;
	  c->next = l;
	}
      else
	display_list = l;
    }

  dtkMsg.add(DTKMSG_INFO, "Added dtkAugment object "
	     "named \"%s%s%s\" at address %p"
	     ":\n  %s\n",
	     dtkMsg.color.grn, a->getName(),
	     dtkMsg.color.end, a,
	     a->getDescription());

  return 0;
}

// return 0 if it was removed
// return -1 if it was not found.
int dtkManager::remove(const char *name)
{
  struct dtkAugment_list *l = augment_list_first;
  for(;l;l=l->next)
    if(!strcmp(l->augment->getName(),name))
      return remove(l);
  return -1;
}

// return 0 if it was removed
// return -1 if it was not found.
int dtkManager::remove(dtkAugment *a)
{
  struct dtkAugment_list *l = augment_list_first;
  for(;l;l=l->next)
    if(l->augment == a)
      return remove(l);
  return -1;
}

int dtkManager::remove(struct dtkAugment_list *r)
{
  dtkMsg.add(DTKMSG_DEBUG, "Removing dtkAugment object "
	     "named \"%s%s%s\" at address %p"
	     ":\n  %s\n",
	     dtkMsg.color.tur, r->augment->getName(),
	     dtkMsg.color.end, r->augment,
	     r->augment->getDescription());

  if(r->flag & DTK_PREFRAME)
    removeCallback(r->augment, DTK_PREFRAME);
  if(r->flag & DTK_POSTFRAME)
    removeCallback(r->augment, DTK_POSTFRAME);

  if(r->next) (r->next)->prev = r->prev;
  else augment_list_last = r->prev;
  if(r->prev) (r->prev)->next = r->next;
  else augment_list_first = r->next;

  // remove from display_list
  if(display_list && 
     r->augment->getType() == DTKDISPLAY_TYPE)
    {
      struct dtkDisplay_list *c = display_list;
      if((dtkDisplay *) (r->augment) == c->display)
	{
	  display_list = c->next;
	  free(c);
	}
      else
	{
	  struct dtkDisplay_list *b = c;
	  for(c = b->next;c; c = b->next)
	    {
	      if((dtkDisplay *) (r->augment) == c->display)
		{
		  b->next = c->next;
		  free(c);
		  break;
		}
	      b = c;
	    }
	}
    }

  if(r->augment->getType() == DUMMYAUGMENT_TYPE)
    {
      // If this manager code made a dummy dtkAugment object than
      // delete it here and return what was returned by
      // dtkDSO_loader() to the value of r->augment.
      delete r->augment;
      r->augment = DTKDSO_LOAD_CONTINUE;
    }

  // unload the dtkAugment after fixing the list.
  if(r->loaderFunc)
    _unload(r->augment, r->loaderFunc);

  free(r);
  return 0;
}
