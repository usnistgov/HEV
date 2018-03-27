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

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkAugment.h"
#include "dtkRecord.h"
#include "dtkDisplay.h"
#include "dtkManager.h"
#include "dtkVec3.h"
#include "dtkCoord.h"
#include "dtkMatrix.h"
#include "dtkNav.h"
#include "dtkNavList.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "utils.h"


dtkNavList::dtkNavList(void) : dtkAugment("dtkNavList")
{
  setType(DTKNAVLIST_TYPE);
  navList = NULL;
  currentNav = NULL;
  validate();
}

dtkNavList::~dtkNavList(void)
{
  while(navList)
    {
      struct dtkNavList_navList *l = navList;
      navList = navList->next;
      free(l);
    }
  currentNav = NULL;
}

// Add a nav to the list and make it the current navigation.
int dtkNavList::add(dtkNav *nav)
{
  if(!nav || !nav->isValid(DTKNAV_TYPE))
    return dtkMsg.add(DTKMSG_ERROR, 0, -1, 
		      "dtkNavList::add(%p) failed: "
		      "pointer %p points to an "
		      "invalid dtkNav object.\n", nav, nav);

  struct dtkNavList_navList *l = navList;
  for(;l; l = l->next)
    if(l->nav == nav) // already in list
      return 0;

  struct dtkNavList_navList *newList = (struct dtkNavList_navList *)
    dtk_malloc(sizeof(struct dtkNavList_navList));
  newList->next = navList;
  newList->nav = nav;
  navList = newList;

  return 0;
}

int dtkNavList::remove(dtkNav *nav)
{
  if(!nav || !nav->isValid(DTKNAV_TYPE))
    return
      dtkMsg.add(DTKMSG_ERROR, 0, -1,
		 "dtkNavList::remove(%p) failed: "
		 "pointer %p points to an "
		 "invalid dtkNav object.\n", nav, nav);

  struct dtkNavList_navList *l = navList;
  struct dtkNavList_navList *prev = NULL;
  for(; l; l = l->next)
    {
      if(l->nav == nav)
	break;
      prev = l;
    }

  if(!l)
    return dtkMsg.add(DTKMSG_WARN, 0, -1,
		      "dtkNavList::remove(%p) failed: "
		      "cannot find dtkNav object,"
		      "with address %p, in the navList.\n",
		      nav, nav);

  // connect the NULL navigation if it was the currentNav
  if(currentNav == nav)
    currentNav = NULL;

  // fix the navigation list
  if(prev)
    prev->next = l->next;
  else
    navList = l->next;

  free(l);

  return 0; // success
}

int dtkNavList::deactivate(void)
{
  if(currentNav)
    {
      currentNav->deactivate();
      return 0;
    }

  dtkMsg.add(DTKMSG_WARN,
	     "dtkNavList::deactivate() "
	     "failed: the current navigation is not set.\n");
  return -1;
}

int dtkNavList::activate(void)
{
  if(currentNav)
    {
      currentNav->activate();
      return 0;
    }
  
  dtkMsg.add(DTKMSG_WARN,
	     "dtkNavList::activate() "
	     "failed: no navigations available.\n");
  return -1;
}


int dtkNavList::switch_(dtkNav *nav)
{
  if(nav && !nav->isValid(DTKNAV_TYPE))
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkNavList::switch_(%p) failed: "
		 "pointer %p points to an"
		 "invalid dtkNav object.\n", nav, nav);
      return -1;
    }

  // case: nav the same as current
  if(currentNav == nav && nav)
    {
      nav->activate();
      return 0;
    }

  // disable the current (dtkNav) navigation
  if(currentNav && currentNav != nav)
    {
      currentNav->deactivate();
      currentNav = NULL;
    }

  if(nav)
    {
      // case: nav is not NULL and not current
      // find it in the list.
      struct dtkNavList_navList *l = navList;
      for(;l && l->nav != nav;
	  l = l->next);
      
      // got it in the List
      if(l->nav == nav)
	{
	  // add this nav
	  currentNav = nav;
	  nav->activate();
	  return 0;
	}
    }
  else // nav == NULL == enable NULL nav
    {
      return 0;
    }

  dtkMsg.add(DTKMSG_WARN,
	     "dtkNavList::switch_(%p) failed:"
	     " dtkNav with address %p not "
	     "found in the navList. Use dtkNavList"
	     "::addNav() to add it.\n", nav, nav);
  return -1;
}


dtkNav *dtkNavList::next(dtkNav *nav)
{
  if(navList)
    {
      struct dtkNavList_navList *l = navList;
      for(;l && l->nav != nav; l = l->next);
      if(l && (l=l->next))
	return l->nav;
    }

  return NULL; // end of list or not in list
}
