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


dtkManager::dtkManager(void) : dtkBase(DTKMANAGER_TYPE)
{
  state = DTK_ISRUNNING;

  // These _is* flags are 1 if the method in question has finished
  // being called, 0 if the method in question has not been called,
  // and -1 if the method in question is being called now.
  _isPreConfiged = 0;
  _isConfiged = 0;
  _isPostConfiged = 0;
  _isFramed = 0;
  _isSynced = 1;
  _forceSync = 1;

  _userCallsPrePostFrame = 0;

  DSOPath = NULL;

  augment_list_first = NULL;
  augment_list_last = NULL;

  preFrame_list = NULL;
  postFrame_list = NULL;

  display_list = NULL;

  _record = NULL;
  validate();
  
  _argc = 0;
  _argv = NULL;

  dtkMsg.add(DTKMSG_DEBUG,
	     "%s--------- dtkManager::dtkManager() finished ----------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 
}

dtkManager::dtkManager( int argc, char** argv )
{
  state = DTK_ISRUNNING;

  // These _is* flags are 1 if the method in question has finished
  // being called, 0 if the method in question has not been called,
  // and -1 if the method in question is being called now.
  _isPreConfiged = 0;
  _isConfiged = 0;
  _isPostConfiged = 0;
  _isFramed = 0;
  _isSynced = 1;
  _forceSync = 1;

  _userCallsPrePostFrame = 0;

  DSOPath = NULL;

  augment_list_first = NULL;
  augment_list_last = NULL;

  preFrame_list = NULL;
  postFrame_list = NULL;

  display_list = NULL;

  _record = NULL;
  validate();

  _argc = argc;
  _argv = argv;

  dtkMsg.add(DTKMSG_DEBUG,
	     "%s--------- dtkManager::dtkManager(argc, argv) finished ----------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 
}


dtkManager::~dtkManager(void)
{
  dtkMsg.add(DTKMSG_DEBUG,
	     "%s--------- dtkManager::~dtkManager() -------------------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 
  clean_up();
  dtkMsg.add(DTKMSG_DEBUG,
	     "%s--------- dtkManager::~dtkManager() finished ----------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 
}

int dtkManager::argc()const
{
    return _argc;
}
char** dtkManager::argv()const
{
    return _argv;
}

dtkRecord *dtkManager::record(size_t queue_lenght)
{
  if(_record)
    return _record;

  _record = new dtkRecord(queue_lenght);
  if(!_record || _record->isInvalid())
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkManager::record() failed.\n");
      if(_record)
	{
	  delete _record;
	  _record = NULL;
	}
    }
  return _record;
}

void dtkManager::removeAll(void)
{
  while(augment_list_last)
    remove(augment_list_last);
}

void dtkManager::clean_up(void)
{
  invalidate();

  if(DSOPath)
    {
      free(DSOPath);
      DSOPath = NULL;
    }

  if(display_list)
    {
      struct dtkDisplay_list *l = display_list;
      while(l)
	{
	  struct dtkDisplay_list *m = l->next;
	  free(l);
	  l = m;
	}
      display_list = NULL;
    }

  while(augment_list_last)
    remove(augment_list_last);

  if(_record)
    {
      delete _record;
      _record = NULL;
    }
}


dtkAugment *dtkManager::getNext(u_int32_t class_type) const
{
  return getNext(NULL, class_type);
}

// Returns the next dtkAugment object from the list.
dtkAugment *dtkManager::getNext(const dtkAugment *augment,
				u_int32_t class_type) const
{
  struct dtkAugment_list *l = augment_list_first;
  if(!l) return NULL;
  if(!augment)
    {
      for(;l;l=l->next)
	{
	  dtkAugment *a = l->augment;
	  if(a->getType() == class_type ||
	     class_type == DTK_ANY_TYPE)
	    return a;
	}
      return NULL;
    }

  for(;l;l=l->next)
    if(l->augment == augment)
      break;

  for(l=l->next;l;l=l->next)
    {
      dtkAugment *a = l->augment;
      if(a->getType() == class_type ||
	 class_type == DTK_ANY_TYPE)
	return a;
    }

  return NULL;
}

// Returns a dtkAugment object from the list.
dtkAugment *dtkManager::get(const char *name,
			    u_int32_t class_type) const
{
  dtkAugment *a = check(name, class_type);

  if(!a)
    dtkMsg.add(DTKMSG_WARN, "dtkManager::get(\"%s\",%d) failed"
	       " to find augment named \"%s\".\n",
	       name, class_type, name);


  return a;
}

// This is the same as get() but will not spew if not found.
dtkAugment *dtkManager::check(const char *name,
			      u_int32_t class_type) const
{
  struct dtkAugment_list *l = augment_list_first;
  for(;l;l=l->next)
    if(!strcmp(l->augment->getName(), name) &&
       (l->augment->getType() == class_type ||
	class_type == DTK_ANY_TYPE)
       )
      return l->augment;

  return NULL;
}

// returns dtkAugment *a if found
// returns NULL is not found
dtkAugment *dtkManager::check(const dtkAugment *a) const
{
  struct dtkAugment_list *l = augment_list_first;
  for(;l;l=l->next)
    if(l->augment == a)
      return (dtkAugment *) a;
  return NULL;
}

// debug printing using file
void dtkManager::print(FILE *file) const
{
  if(!file) file = stdout;

  struct dtkAugment_list *l = augment_list_first;
  if(!l)
    {
      fprintf(file, 
	     "************ dtkManager STATE ***************\n"
	     "NO dtkAugments loaded\n");
      return;
    }

  fprintf(file,
	 "************ dtkManager STATE ***************\n"
	 "*********************************************\n"
	 "dtkAugment  %saddress%s type %sname%s / description\n\n",
	 dtkMsg.color.tur, dtkMsg.color.end,
	 dtkMsg.color.grn, dtkMsg.color.end);
  

  for(;l;l=l->next)
    {
      dtkAugment *a = l->augment;
      if(a->getType() != DTKDISPLAY_TYPE)
	fprintf(file,"Augment ");
      else
	fprintf(file,"Display ");
      fprintf(file,"%s%p%s %p %s%s%s:\n   %s\n\n",
	      dtkMsg.color.tur, a,  dtkMsg.color.end,
	      (void *) a->getType(),
	      dtkMsg.color.grn, a->getName(), dtkMsg.color.end,
	      a->getDescription());
    }

  fprintf(file,
	  "*********************************************\n");
}
