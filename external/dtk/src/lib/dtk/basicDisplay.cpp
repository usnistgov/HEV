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
#include "dtkConfigure.h"

#include <sys/types.h>
#include <stdarg.h>


#ifdef DTK_ARCH_WIN32_VCPP
#else
# include <sys/time.h>
#endif

#include <stdio.h>

#include "types.h"
#include "dtkBase.h"
#include "dtkColor.h"
#include "dtkMessage.h"
#include "dtkAugment.h"
#include "dtkRecord.h"
#include "dtkInput.h"
#include "dtkInButton.h"
#include "dtkInValuator.h"
#include "dtkInLocator.h"
#include "dtkDisplay.h"
#include "dtkManager.h"
#include "dtkBasicDisplay.h"
#include "utils.h"


#define X 0
#define Y 0
#define W 400
#define H 400


dtkBasicDisplay::~dtkBasicDisplay(void)
{
  struct dtkBasicDisplay_drawList *l = drawList;
  for(;l;l=drawList)
    {
      drawList = l->next;
      dtk_free(l);
    }
}


dtkBasicDisplay::
dtkBasicDisplay(dtkManager *m, const char *name)
  : dtkDisplay(name), manager(m)
{
  drawList = NULL;
}


void dtkBasicDisplay::add(int (*draw)(void))
{
  struct dtkBasicDisplay_drawList *l =
    (struct dtkBasicDisplay_drawList *)
    dtk_malloc(sizeof(struct dtkBasicDisplay_drawList));
  l->next = NULL;
  l->draw = draw;

  if(drawList)
    {
      struct dtkBasicDisplay_drawList *m = drawList;
      for(;m->next;m=m->next);
      m->next = l;
    }
  else
    drawList = l;
}


void dtkBasicDisplay::remove(int (*draw)(void))
{
  if(!draw) // remove all draw callback functions.
    {
      struct dtkBasicDisplay_drawList *l = drawList;
      for(;l;l=drawList)
	{
	  drawList = l->next;
	  dtk_free(l);
	}
      return;
    }
  else
    {
      struct dtkBasicDisplay_drawList *l = drawList;
      if(drawList->draw == draw)
	{
	  drawList = drawList->next;
	  dtk_free(l);
	  return;
	}
      struct dtkBasicDisplay_drawList *m = drawList->next;
      for(;m;m=m->next)
	{
	  if(m->draw == draw)
	    {
	      l->next = m->next;
	      dtk_free(m);
	      return;
	    }
	  l = m;
	}
    }
}
