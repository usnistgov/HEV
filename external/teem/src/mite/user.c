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

miteUser *
miteUserNew() {
  miteUser *muu;
  int i;

  muu = (miteUser *)calloc(1, sizeof(miteUser));
  if (!muu)
    return NULL;

  muu->umop = airMopNew();
  muu->nin = NULL;
  muu->ntxf = NULL;              /* managed by user (with miter: hest) */
  muu->nout = NULL;              /* managed by user (with miter: hest) */
  muu->ntxfNum = 0;
  for (i=0; i<MITE_RANGE_NUM; i++) {
    muu->rangeInit[i] = 1.0;
  }
  muu->normalSide = miteDefNormalSide;
  muu->refStep = miteDefRefStep;
  muu->rayStep = AIR_NAN;
  muu->near1 = miteDefNear1;
  muu->hctx = hooverContextNew();
  airMopAdd(muu->umop, muu->hctx, (airMopper)hooverContextNix, airMopAlways);
  for (i=0; i<GAGE_KERNEL_NUM; i++) {
    muu->ksp[i] = NULL;
  }
  muu->gctx0 = gageContextNew();
  airMopAdd(muu->umop, muu->gctx0, (airMopper)gageContextNix, airMopAlways);
  gageParmSet(muu->gctx0, gageParmRequireAllSpacings, AIR_FALSE);
  muu->lit = limnLightNew();
  airMopAdd(muu->umop, muu->lit, (airMopper)limnLightNix, airMopAlways);
  muu->justSum = AIR_FALSE;
  muu->noDirLight = AIR_FALSE;
  muu->rendTime = 0;
  muu->sampRate = 0;
  return muu;
}

miteUser *
miteUserNix(miteUser *muu) {

  if (muu) {
    airMopOkay(muu->umop);
    AIR_FREE(muu);
  }
  return NULL;
}

int
_miteUserCheck(miteUser *muu) {
  char me[]="miteUserCheck", err[AIR_STRLEN_MED];
  int T, gotOpac;
  
  if (!muu) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(MITE, err); return 1;
  }
  if (!( muu->ntxfNum >= 1 )) {
    sprintf(err, "%s: need at least one transfer function", me);
    biffAdd(MITE, err); return 1;
  }
  gotOpac = AIR_FALSE;
  for (T=0; T<muu->ntxfNum; T++) {
    if (miteNtxfCheck(muu->ntxf[T], gageKindScl)) {
      sprintf(err, "%s: ntxf[%d] (%d of %d) can't be used as a txf",
	      me, T, T+1, muu->ntxfNum);
      biffAdd(MITE, err); return 1;
    }
    gotOpac |= !!strchr(muu->ntxf[T]->axis[0].label, 'A');
  }
  if (!gotOpac) {
    fprintf(stderr, "\n\n%s: !!! WARNING !!! opacity (\"A\") not set "
	    "by any transfer function\n\n", me);
  }
  if (!muu->nout) {
    sprintf(err, "%s: rendered image nrrd is NULL", me);
    biffAdd(MITE, err); return 1;
  }
  if (gageVolumeCheck(muu->gctx0, muu->nin, gageKindScl)) {
    sprintf(err, "%s: trouble with input volume", me);
    biffMove(MITE, err, GAGE); return 1;
  }

  return 0;
}
