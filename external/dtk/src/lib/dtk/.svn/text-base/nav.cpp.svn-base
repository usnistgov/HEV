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


dtkNav::dtkNav(dtkManager *manager, const char *name) : dtkAugment(name)
{
  m = manager ;
  setType(DTKNAV_TYPE);
  
  _response = 1.f ;
  _userMatrix = NULL ;
  defVelocity = new dtkCoord ; // zero by default
  _velocity = defVelocity ;
  defPivot = new dtkCoord(0.f, 0.5f, 0.f, 0.f, 0.f, 0.f) ;
  _pivot = defPivot ;
  scale = old_scale = 1.f ;
  
  validate();
}

dtkNav::~dtkNav(void) {
  dtkNavList *nl = (dtkNavList *) m->check("dtkNavList", DTKNAVLIST_TYPE) ;
  if (nl) {
    nl->remove(this) ;
  }

  if (defPivot) {
    delete defPivot ;
    defPivot = NULL ;
  }

  if (defVelocity) {
    delete defVelocity ;
    defVelocity = NULL ;
  }

}

//Register the nav object with the dtkNavList object
int dtkNav::postConfig(void) {

  dtkNavList *nl = (dtkNavList *) m->check("dtkNavList", DTKNAVLIST_TYPE) ;
  if(!nl)
    {
      nl = new dtkNavList;
      if(m->add(nl))
	{
	  delete nl;
	  return ERROR_;
	}
    }

  //printf("~~~~~~~%s:%d\n",__FILE__,__LINE__) ;
  if(nl->add(this))
    return ERROR_;
  
  if(nl->switch_(this))
    return ERROR_;
  
  //printf("~~~~~~~%s:%d\n",__FILE__,__LINE__) ;
  return REMOVE_CALLBACK ;
}

//The dtkMatrix is calculated once per loop, after Frame
int dtkNav::postFrame(void) {
  //printf("~~~~~~~%s:%d\n",__FILE__,__LINE__) ;
  if (!_userMatrix) {
                      if ((!location.equals(&old_location)) || (scale != old_scale)) {
      _matrix.identity() ;
      _matrix.scale(scale) ;
      _matrix.coord(location) ;
      // the world moves, we don't
      //_matrix.invert() ;
      old_location = location ;
      old_scale = scale ;
    }
  } else
    _matrix.copy(_userMatrix) ;
  return CONTINUE;
}

// Set/return the navigation speed (or responsiveness).
int dtkNav::response(float r) {
  _response = r ;
  return 0;
}

float dtkNav::response(void) {
  return _response;
}

// velocity is xyz/hpr translational and angular velocites, using a
// pointer to a dtkCoord
dtkCoord *dtkNav::velocity(void) {
  return _velocity ;
}

// you can set a pointer to your own array of floats, in which case the it
// is used instead of the default velocity.  Setting it to NULL restores
// the default velocity.  Setting it if *velocity() is NULL returns 1, an
// error.  Otherwise 0 is returned.
void dtkNav::velocity(dtkCoord *c) {
  if (c)
    _velocity = c ;
  else
    _velocity = defVelocity ;
  
  return ;
}


// some navigations need a pivot around which to rotate.  You get pointers
// to the pivot so that the application programmer may vary them all
// through the running of the application.  The particular dtkNav should
// document what it does with pivot, if anything
dtkCoord *dtkNav::pivot(void) {
  return _pivot ;
}


// you can set a pointer to your own dtkCoord, in which case it is used
// instead of the default pivot behavior.  Setting it to NULL restores the
// default behavior. Setting it if *pivot() is NULL returns 1, an
// error.  Otherwise 0 is returned.
void dtkNav::pivot(dtkCoord *c) {
  // if NULL is passed, then use default pivot
  if (!c) {
    _pivot = defPivot ;
    //printf("default pivot\n") ;
  } else {
    // otherwise, a new pivot
    _pivot = c ;
  }
  return ;
}


// turn on and off the actions (function) of this object by
// adding/removing the callback from thr manager's list
int dtkNav::activate(void) {
  return (m->addCallback(this, DTK_POSTFRAME)) ;
}

int dtkNav::deactivate(void) {
  return (m->removeCallback(this, DTK_POSTFRAME)) ;
}


// provide a matrix that contains the navigation- if this is set then the
// usual calculation with location, scale and so forth are bypassed
int dtkNav::userMatrix(dtkMatrix *m) {
  _userMatrix = m ;
  return 0 ;
}

dtkMatrix *dtkNav::userMatrix(void) {
  return _userMatrix ;
}


// returns a pointer to a dtkMatrix containing the navigation
dtkMatrix *dtkNav::matrix(void) {
  return &_matrix ;
}
