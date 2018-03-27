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


// Adds the call-back to the list so that it gets called in the next
// cycle.
int dtkManager::addCallback(const dtkAugment *a, unsigned int when)
{
  if(when > ALL_CALLBACKS)
    return dtkMsg.add(DTKMSG_WARN,0,1,
		      "dtkManager::addCallback(%p,0%o) failed"
		      " 0%o is not a valid callback type.\n",
		      a, when, when);

  struct dtkAugment_list *l = augment_list_first;
  for(;l;l=l->next)
    if(a == l->augment)
      {
	l->flag |= when;

	if(when & DTK_PREFRAME)
	  _addCallback(l, &preFrame_list);
	if(when & DTK_POSTFRAME)
	  _addCallback(l, &postFrame_list);
	return 0;
      }

  dtkMsg.add(DTKMSG_WARN, "dtkManager::addCallback(%p,0%o) failed"
	     " to find %p in the loaded augments.\n",
	     a, when, a);
  return -1;
}

int dtkManager::removeCallback(const dtkAugment *a, unsigned int when)
{
  if(when > ALL_CALLBACKS)
    return dtkMsg.add(DTKMSG_WARN,0,1,
		      "dtkManager::removeCallback(%p,0%o) failed"
		      " 0%o is not a valid callback type.\n",
		      a, when, when);

  struct dtkAugment_list *l = augment_list_first;
  for(;l;l=l->next)
    if(a == l->augment)
      {
	l->flag &= ~when;

	if(when & DTK_PREFRAME)
	  _removeCallback(a, &preFrame_list);

	if(when & DTK_POSTFRAME)
	  _removeCallback(a, &postFrame_list);

	return 0;
      }

  dtkMsg.add(DTKMSG_WARN, "dtkManager::removeCallback(%p,0%o) failed"
	     " to find %p in the loaded augments.\n",
	     a, when, a);
  return -1;
}


// The calling the loaded dtkManager callbacks Returns the number of
// callback errors or -1 on other error.
int dtkManager::preConfig(void)
{
  // Stop any recursion.
  if(_isPreConfiged < 0)
    return
      dtkMsg.add(DTKMSG_INFO,0,0,
		 "dtkManager::preConfig() was called "
		 "from within dtkManager::preConfig(): "
		 "not recursing.\n");

  if(_isPreConfiged)
    return
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "Calling dtkManager::preConfig() "
		 "more then once per cycle is not allowed.\n");

  // marked as calling preConfig()
  _isPreConfiged = -1;

  dtkMsg.add(DTKMSG_DEBUG,
	     "%s-------------- dtkManager::preConfig() --------------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 

  int returnVal = 0;

  /* calling the dtkDisplay::preConfig() before the regular
   * dtkAugment::preConfig().
   */
  struct dtkDisplay_list *dl = display_list;
  while(dl)
    {
      if(dl->flag & DTK_PRECONFIG)
	{
	  struct dtkDisplay_list *k = dl;
	  dl = dl->next;

	  dtkDisplay *a = k->display;

	  dtkMsg.add(DTKMSG_DEBUG, "calling dtkAugment::"
		     "preConfig() for dtkDisplay \"%s%s%s\".\n",
		     dtkMsg.color.tur, a->getName(),
		     dtkMsg.color.end);
	  int i = a->preConfig();
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {
	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkDisplay \"%s\" "
			     "preConfig(): removing it from dtkManager.\n",
			     a->getName());
		  returnVal++;
		}
	      remove(a);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_PRECONFIG;
	}
      else
	dl = dl->next;
    }

  struct dtkAugment_list *l = augment_list_first;
  while(l)
    {
      if(l->flag & DTK_PRECONFIG && !(l->flag & DTK_ISDISPLAY))
	{
	  struct dtkAugment_list *k = l;
	  l = l->next;
	  dtkAugment *a = k->augment;

	  dtkMsg.add(DTKMSG_DEBUG, "calling dtkAugment::"
		     "preConfig() for \"%s%s%s\".\n",
		     dtkMsg.color.tur, a->getName(),
		     dtkMsg.color.end);
	  int i = a->preConfig();
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {
	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkAugment \"%s\" "
			     "preConfig(): removing it from dtkManager.\n",
			     a->getName());
		  returnVal++;
		}
	      remove(k);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_PRECONFIG;
	}
      else
	l = l->next;
    }

  // marked as called preConfig()
  _isPreConfiged = 1;

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::preConfig() had "
	       "%d callback error(s).\n",
	       returnVal);

  // reset so that the start of a callback cycle may by done again.
  _isConfiged = 0;

  return returnVal;
}

// just for dtkDisplays
int dtkManager::config(void)
{
  // Stop any recursion.
  if(_isConfiged < 0)
    return
      dtkMsg.add(DTKMSG_INFO,0,0,
		 "dtkManager::config() was called "
		 "from within dtkManager::config(): "
		 "not recursing.\n");

  // Stop any callback order problems.
  if(_isPreConfiged < 0)
    return
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "dtkManager::config() was called "
		 "from within dtkManager::preConfig().\n");

  if(_isConfiged)
    return
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "Calling dtkManager::config() "
		 "more than once per cycle is not allowed.\n");

  int returnVal = 0;

  if(!_isPreConfiged)
    returnVal += preConfig();

  // marked as calling config()
  _isConfiged = -1;

  dtkMsg.add(DTKMSG_DEBUG,
	     "%s---------------- dtkManager::config() ----------------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 

  struct dtkDisplay_list *l = display_list;
  while(l)
    {
      if(l->flag & DTK_CONFIG)
	{
	  struct dtkDisplay_list *k = l;
	  l = l->next;
	  dtkDisplay *d = k->display;

	  dtkMsg.add(DTKMSG_DEBUG, "calling dtkDisplay::"
		     "config() for \"%s%s%s\".\n",
		     dtkMsg.color.tur, d->getName(),
		     dtkMsg.color.end);
	  int i = d->config();
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {
	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkDisplay \"%s\" "
			     "config(): removing it from dtkManager.\n",
			     d->getName());
		  returnVal++;
		}
	      remove(d);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_CONFIG;
	}
      else
	l = l->next;
    }

  // marked as called config()
  _isConfiged = 1;

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::config() had "
	       "%d callback error(s).\n",
	       returnVal);

  // reset so that the start of a callback cycle may by done again.
  _isPostConfiged = 0;

  return returnVal;
} 

int dtkManager::postConfig(void)
{
  // Stop any recursion.
  if(_isPostConfiged < 0)
    return
      dtkMsg.add(DTKMSG_INFO,0,0,
		 "dtkManager::postConfig() was called "
		 "from within dtkManager::postConfig(): "
		 "not recursing.\n");

  // Stop any callback order problems.
  if(_isPreConfiged < 0)
    return
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "dtkManager::postConfig() was called "
		 "from within dtkManager::preConfig().\n");
  if(_isConfiged < 0)
    return
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "dtkManager::postConfig() was called "
		 "from within dtkManager::config().\n");

  if(_isPostConfiged)
    return
      dtkMsg.add(DTKMSG_WARN,0,-1,
		 "Calling dtkManager::postConfig() "
		 "more than once per cycle is not allowed.\n");

  int returnVal = 0;

  if(!_isConfiged)
    returnVal += config();

  // mark as calling postConfig()
  _isPostConfiged = -1;

  dtkMsg.add(DTKMSG_DEBUG,
	     "%s-------------- dtkManager::postConfig() --------------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 

  /* calling the dtkDisplay::postConfig() before the regular
   * dtkAugment::postConfig().
   */

  struct dtkDisplay_list *dl = display_list;
  while(dl)
    {
      if(dl->flag & DTK_POSTCONFIG)
	{
	  struct dtkDisplay_list *k = dl;
	  dl = dl->next;

	  dtkDisplay *a = k->display;

	  dtkMsg.add(DTKMSG_DEBUG, "calling dtkAugment::"
		     "postConfig() for dtkDisplay \"%s%s%s\".\n",
		     dtkMsg.color.tur, a->getName(),
		     dtkMsg.color.end);
	  int i = a->postConfig();
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {
	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkDisplay \"%s\" "
			     "postConfig(): removing it from dtkManager.\n",
			     a->getName());
		  returnVal++;
		}
	      remove(a);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_POSTCONFIG;
	}
      else
	dl = dl->next;
    }

  struct dtkAugment_list *l = augment_list_first;
  while(l)
    {
      if(l->flag & DTK_POSTCONFIG && !(l->flag & DTK_ISDISPLAY))
	{
	  struct dtkAugment_list *k = l;
	  l = l->next;

	  dtkAugment *a = k->augment;
	  
	  dtkMsg.add(DTKMSG_DEBUG, "calling dtkAugment::"
		     "postConfig() for \"%s%s%s\".\n",
		     dtkMsg.color.tur, a->getName(),
		     dtkMsg.color.end);
	  int i = a->postConfig();
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {
	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkAugment \"%s\" "
			     "postConfig(): removing it from dtkManager.\n",
			     a->getName());
		  returnVal++;
		}
	      remove(k);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_POSTCONFIG;
	}
      else
	l = l->next;
    }

  // mark as called postConfig()
  _isPostConfiged = 1;

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::postConfig() had "
	       "%d callback error(s).\n",
	       returnVal);

  dtkMsg.add(DTKMSG_DEBUG,
	     "%s-------- dtkManager::postConfig() finished -----------%s"
	     "\n", dtkMsg.color.yel, dtkMsg.color.end); 

  // reset so that the start of a callback cycle may by done again.
  _isPreConfiged = 0;

  return returnVal;
}

// just for dtkDisplays
int dtkManager::sync(void)
{
  int returnVal = 0;

  if(!_isPostConfiged)
    returnVal += postConfig();

  struct dtkDisplay_list *l = display_list;
  while(l)
    {
      if(l->flag & DTK_SYNC)
	{
	  struct dtkDisplay_list *k = l;
	  l = l->next;
	  dtkDisplay *d = k->display;
	  
	  int i = d->sync();
	  
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {

	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkDisplay \"%s\" "
			     "sync(): removing it from dtkManager.\n",
			     d->getName());
		  returnVal++;
		}
	      remove(d);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_SYNC;
	}
      else
	l = l->next;
    }

  _isSynced = 1;

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::sync() had "
	       "%d callback error(s).\n",
	       returnVal);

  return returnVal;
}

int dtkManager::preFrame(void)
{
  int returnVal = 0;

  // We put preFrame() callbacks in a seperate list from the general
  // augment list, so that it's faster.
  struct dtkAugment_callback *l = preFrame_list;
  while(l)
    {
      dtkAugment *a = l->augment;
      l = l->next;

      int i = a->preFrame();

      if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	{
	  if(i == dtkAugment::ERROR_)
	    {
	      dtkMsg.add(DTKMSG_WARN,
			 "Error calling dtkAugment \"%s\" "
			 "preFrame(): removing it from dtkManager.\n",
			 a->getName());
	      returnVal++;
	    }
	  remove(a);
	}
      else if(i == dtkAugment::REMOVE_CALLBACK)
	removeCallback(a, DTK_PREFRAME);
    }

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::preFrame() had "
	       "%d callback error(s).\n",
	       returnVal);

  return returnVal;
}

// just for dtkDisplays
int dtkManager::frame(void)
{
  int returnVal = 0;

  if(!_isPostConfiged)
    returnVal += postConfig();

  // Force the sync() call only if sync() was not called
  // before the last time this was called.
  if(_forceSync)
    {
      if(!_isSynced)
	returnVal += sync();
      _isSynced = 0;
    }

  if(!_userCallsPrePostFrame)
    returnVal += preFrame();

  struct dtkDisplay_list *l = display_list;
  while(l)
    {
      if(l->flag & DTK_FRAME)
	{
	  struct dtkDisplay_list *k = l;
	  l = l->next;

	  dtkDisplay *d = k->display;
	  
	  int i = d->frame();
	  
	  if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	    {
	      if(i == dtkAugment::ERROR_)
		{
		  dtkMsg.add(DTKMSG_WARN,
			     "Error calling dtkDisplay \"%s\" "
			     "frame(): removing it from dtkManager.\n",
			     d->getName());
		  returnVal++;
		}
	      remove(d);
	    }
	  else if(i == dtkAugment::REMOVE_CALLBACK)
	    k->flag &= ~DTK_FRAME;
	}
      else
	l = l->next;
    }

  _isFramed = 1;

  if(!_userCallsPrePostFrame)
    returnVal += postFrame();

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::frame() had "
	       "%d callback error(s).\n"
	       "Some errors may be due to sync(), "
	       "preFrame(), and/or postFrame().\n",
	       returnVal);

  return returnVal;
}

int dtkManager::postFrame(void)
{
  int returnVal = 0;

  // We put postFrame() callbacks in a seperate list from the general
  // augment list, so that it's faster.
  struct dtkAugment_callback *l = postFrame_list;
  while(l)
    {
      dtkAugment *a = l->augment;
      l = l->next;

      int i = a->postFrame();

      if(i == dtkAugment::REMOVE_OBJECT || i == dtkAugment::ERROR_)
	{
	  if(i == dtkAugment::ERROR_)
	    {
	      dtkMsg.add(DTKMSG_WARN,
			 "Error calling dtkAugment \"%s\" "
			 "postFrame(): removing it from dtkManager.\n",
			 a->getName());
	      returnVal++;
	    }
	  remove(a);
	}
      else if(i == dtkAugment::REMOVE_CALLBACK)
	removeCallback(a, DTK_POSTFRAME);
    }

  if(returnVal)
    dtkMsg.add(DTKMSG_WARN,
	       "dtkManager::postFrame() had "
	       "%d callback error(s).\n",
	       returnVal);

  return returnVal;
}

// ckeck if name is in the augment list
// return 1 if it is else return 0
int dtkManager::check_augment(const char *name)
{
  struct dtkAugment_list *l = augment_list_first;
  for(;l;l = l->next)
    if(!strcmp(l->augment->getName(), name))
      return 1;
  return 0;
}


int dtkManager::preFrame(unsigned int magic_num)
{
  return ((_userCallsPrePostFrame &&
	   magic_num == __DTKMANAGER_PREFRAME_MAGIC_) ?
	  preFrame(): -1);
}

int dtkManager::postFrame(unsigned int magic_num)
{
  return ((_userCallsPrePostFrame &&
	   magic_num == __DTKMANAGER_POSTFRAME_MAGIC_) ?
	  postFrame(): -1);
}

void dtkManager::setPublicPrePostFrames(unsigned int magic_num)
{
  if(magic_num == __DTKMANAGER_PREPOSTFRAME_MAGIC_)
    _userCallsPrePostFrame = 1;
}

void dtkManager::setNoSyncForcing(unsigned int magic_num)
{
  if(magic_num == __DTKMANAGER_NOSYNCFORCING_)
    _forceSync = 0;
}

void dtkManager::_addCallback(struct dtkAugment_list *al,
			      struct dtkAugment_callback **cb_list)
{
  // Make sure it's not in the list already.
  struct dtkAugment_callback *pl = *cb_list;
  for(;pl;pl=pl->next)
    if(pl->augment == al->augment)
      return;

  // We need to keep the order of callbacks the same as in the
  // dtkAugment_list.
  struct dtkAugment_callback *prev = NULL;
  // prev will be the previous augment in the callback list.
  struct dtkAugment_list *l = augment_list_first;
  for(;l != al;l=l->next)
    for(pl = *cb_list;pl;pl=pl->next)
      if(pl->augment == l->augment)
	{
	  prev = pl;
	  break;
	}

  pl = (struct dtkAugment_callback *)
    dtk_malloc(sizeof(struct dtkAugment_callback));
  pl->augment = al->augment;

  if(prev)
    {
      pl->next = prev->next;
      prev->next = pl;
    }
  else
    {
      pl->next = *cb_list;
      *cb_list = pl;
    }
}


void dtkManager::
_removeCallback(const dtkAugment *a,
		struct dtkAugment_callback **cb_list)
{
  struct dtkAugment_callback *pl = *cb_list;
  struct dtkAugment_callback *prev = NULL;
  for(;pl;pl=pl->next)
    {
      if(pl->augment == a)
	break;
      prev = pl;
    }
  if(!pl) return;

  if(prev)
    prev->next = pl->next;
  else
    *cb_list = pl->next;

  free(pl);
}

