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

miteRender *
_miteRenderNew(void) {
  miteRender *mrr;

  mrr = (miteRender *)calloc(1, sizeof(miteRender));
  if (mrr) {
    mrr->rmop = airMopNew();
    if (!mrr->rmop) {
      AIR_FREE(mrr);
    }
  }
  return mrr;
}

miteRender *
_miteRenderNix(miteRender *mrr) {
  
  if (mrr) {
    airMopOkay(mrr->rmop);
    AIR_FREE(mrr);
  }
  return NULL;
}

int 
miteRenderBegin(miteRender **mrrP, miteUser *muu) {
  char me[]="miteRenderBegin", err[AIR_STRLEN_MED];
  gagePerVolume *pvl;
  int E, T, thr;
  unsigned int query;
 
  if (!(mrrP && muu)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(MITE, err); return 1;
  }
  if (_miteUserCheck(muu)) {
    sprintf(err, "%s: problem with user-set parameters", me);
    biffAdd(MITE, err); return 1;
  }
  if (!( *mrrP = _miteRenderNew() )) {
    sprintf(err, "%s: couldn't alloc miteRender", me);
    biffAdd(MITE, err); return 1;
  }
  if (_miteNtxfAlphaAdjust(*mrrP, muu)) {
    sprintf(err, "%s: trouble copying and alpha-adjusting txfs", me);
    biffAdd(MITE, err); return 1;
  }

  query = 0;
  for (T=0; T<muu->ntxfNum; T++) {
    query |= _miteNtxfQuery(muu->ntxf[T], gageKindScl);
  }
  if (!muu->noDirLight) {
    query |= 1<<gageSclNormal;
  }
  fprintf(stderr, "!%s: gageScl query: \n", me);
  gageQueryPrint(stderr, gageKindScl, query);

  E = 0;
  if (!E) E |= !(pvl = gagePerVolumeNew(muu->gctx0, muu->nin, gageKindScl));
  if (!E) E |= gagePerVolumeAttach(muu->gctx0, pvl);
  if (!E) E |= gageKernelSet(muu->gctx0, gageKernel00,
			     muu->ksp[gageKernel00]->kernel,
			     muu->ksp[gageKernel00]->parm);
  if (!E) E |= gageKernelSet(muu->gctx0, gageKernel11,
			     muu->ksp[gageKernel11]->kernel,
			     muu->ksp[gageKernel11]->parm);
  if (!E) E |= gageKernelSet(muu->gctx0, gageKernel22,
			     muu->ksp[gageKernel22]->kernel,
			     muu->ksp[gageKernel22]->parm);
  if (!E) E |= gageQuerySet(muu->gctx0, pvl, query);
  if (!E) E |= gageUpdate(muu->gctx0);
  if (E) {
    sprintf(err, "%s: gage trouble", me);
    biffMove(MITE, err, GAGE); return 1;
  }
  fprintf(stderr, "!%s: kernel support = %d^3 samples\n",
	  me, GAGE_FD(muu->gctx0));
  
  if (nrrdMaybeAlloc(muu->nout, mite_nt, 3, 4,
		     muu->hctx->imgSize[0], muu->hctx->imgSize[1])) {
    sprintf(err, "%s: nrrd trouble", me);
    biffMove(MITE, err, NRRD);
    return 1;
  }
  muu->nout->axis[1].center = nrrdCenterCell;
  muu->nout->axis[1].min = muu->hctx->cam->uRange[0];
  muu->nout->axis[1].max = muu->hctx->cam->uRange[1];
  muu->nout->axis[2].center = nrrdCenterCell;
  muu->nout->axis[2].min = muu->hctx->cam->vRange[0];
  muu->nout->axis[2].max = muu->hctx->cam->vRange[1];

  for (thr=0; thr<muu->hctx->numThreads; thr++) {
    (*mrrP)->tt[thr] = (miteThread *)calloc(1, sizeof(miteThread));
    airMopAdd((*mrrP)->rmop, (*mrrP)->tt[thr], airFree, airMopAlways);
  }

  (*mrrP)->time0 = airTime();
  return 0;
}

int
miteRenderEnd(miteRender *mrr, miteUser *muu) {
  int thr;
  double samples;

  muu->rendTime = airTime() - mrr->time0;
  samples = 0;
  for (thr=0; thr<muu->hctx->numThreads; thr++) {
    samples += mrr->tt[thr]->samples;
  }
  muu->sampRate = samples/(1000.0*muu->rendTime);
  _miteRenderNix(mrr);
  return 0;
}
