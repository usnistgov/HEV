/*
  teem: Gordon Kindlmann's research software
  Copyright (C) 2003, 2002, 2001, 2000, 1999, 1998 University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "mite.h"
#include "privateMite.h"

/*
******** miteThreadBegin()
**
** this has the body of what would be miteThreadInit
*/
int 
miteThreadBegin(miteThread **mttP, miteRender *mrr,
		miteUser *muu, int whichThread) {
  char me[]="miteThreadBegin", err[AIR_STRLEN_MED];
  int msi;
  
  (*mttP) = mrr->tt[whichThread];
  if (!whichThread) {
    /* this is the first thread- it just points to the parent gageContext */
    (*mttP)->gctx = muu->gctx0;
  } else {
    /* we have to generate a new gageContext */
    (*mttP)->gctx = gageContextCopy(muu->gctx0);
    if (!(*mttP)->gctx) {
      sprintf(err, "%s: couldn't set up thread %d", me, whichThread);
      biffMove(MITE, err, GAGE); return 1;
    }
  }
  (*mttP)->ans = (*mttP)->gctx->pvl[0]->ans;
  (*mttP)->norm = (*mttP)->ans + gageKindScl->ansOffset[gageSclNormal];
  (*mttP)->nPerp = (*mttP)->ans + gageKindScl->ansOffset[gageSclNPerp];
  (*mttP)->gten = (*mttP)->ans + gageKindScl->ansOffset[gageSclGeomTens];
  for (msi=0; msi<=MITE_SCL_MAX; msi++) {
    (*mttP)->mscl[msi] = 0.0;
  }
  (*mttP)->thrid = whichThread;
  (*mttP)->samples = 0;
  (*mttP)->verbose = 0;
  _miteStageSet(*mttP, mrr, gageKindScl);
  return 0;
}

int 
miteThreadEnd(miteThread *mtt, miteRender *mrr,
	      miteUser *muu) {

  return 0;
}

