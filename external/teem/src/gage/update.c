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

#include "gage.h"
#include "privateGage.h"

int
_gagePvlFlagCheck (gageContext *ctx, int pvlFlag) {
  int i, ret;
  
  ret = AIR_FALSE;
  for (i=0; i<ctx->numPvl; i++) {
    ret |= ctx->pvl[i]->flag[pvlFlag];
  }
  return ret;
}

void
_gagePvlFlagDown (gageContext *ctx, int pvlFlag) {
  int i;
  
  for (i=0; i<ctx->numPvl; i++) {
    ctx->pvl[i]->flag[pvlFlag] = AIR_FALSE;
  }
}

/* 
** One could go from all the pvls' queries to the context's needD in
** one shot, but doing it in two steps (as below) seems a little clearer,
** and it means that pvl->needD isn't needlessly re-computed for 
** pvl's whose query hasn't changed.
*/

/*
** for each pvl: pvl's query --> pvl's needD
*/
void
_gagePvlNeedDUpdate (gageContext *ctx) {
  char me[]="_gagePvlNeedDUpdate";
  gagePerVolume *pvl;
  int i, q, d, needD[3];

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (i=0; i<ctx->numPvl; i++) {
    pvl = ctx->pvl[i];
    if (pvl->flag[gagePvlFlagQuery]) {
      ELL_3V_SET(needD, 0, 0, 0);
      q = pvl->kind->queryMax+1;
      do {
	q--;
	if (pvl->query & (1 << q)) {
	  for (d=0; d<=2; d++) {
	    needD[d]  |= (pvl->kind->needDeriv[q] & (1 << d));
	  }
	}
      } while (q);
      if (!ELL_3V_EQUAL(needD, pvl->needD)) {
	if (ctx->verbose) {
	  fprintf(stderr, "%s: updating pvl[%d]'s needD to (%d,%d,%d)\n",
		  me, i, needD[0], needD[1], needD[2]);
	}
	ELL_3V_COPY(pvl->needD, needD);
	pvl->flag[gagePvlFlagNeedD] = AIR_TRUE;
      }
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** all pvls' needD --> ctx's needD
*/
void
_gageNeedDUpdate (gageContext *ctx) {
  char me[]="_gageNeedDUpdate";
  gagePerVolume *pvl;
  int i, needD[3];

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  ELL_3V_SET(needD, 0, 0, 0);
  for (i=0; i<ctx->numPvl; i++) {
    pvl = ctx->pvl[i];
    needD[0] |= pvl->needD[0];
    needD[1] |= pvl->needD[1];
    needD[2] |= pvl->needD[2];
  }
  if (!ELL_3V_EQUAL(needD, ctx->needD)) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: updating ctx's needD to (%d,%d,%d)\n",
	      me, needD[0], needD[1], needD[2]);
    }
    ELL_3V_COPY(ctx->needD, needD);
    ctx->flag[gageCtxFlagNeedD] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** ctx's needD & k3pack --> needK
*/
void
_gageNeedKUpdate (gageContext *ctx) {
  char me[]="_gageNeedKUpdate";
  int k, needK[GAGE_KERNEL_NUM], change;
  
  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (k=0; k<GAGE_KERNEL_NUM; k++) {
    needK[k] = AIR_FALSE;
  }
  if (ctx->needD[0]) {
    needK[gageKernel00] = AIR_TRUE;
  }
  if (ctx->needD[1]) {
    needK[gageKernel11] = AIR_TRUE;
    if (ctx->parm.k3pack) {
      needK[gageKernel00] = AIR_TRUE;
    } else {
      needK[gageKernel10] = AIR_TRUE;
    }  
  }
  if (ctx->needD[2]) {
    needK[gageKernel22] = AIR_TRUE;
    if (ctx->parm.k3pack) {
      needK[gageKernel00] = AIR_TRUE;
      needK[gageKernel11] = AIR_TRUE;
    } else {
      needK[gageKernel20] = AIR_TRUE;
      needK[gageKernel21] = AIR_TRUE;
    }  
  }
  change = AIR_FALSE;
  for (k=0; k<GAGE_KERNEL_NUM; k++) {
    change |= (needK[k] != ctx->needK[k]);
  }
  if (change) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: changing needK to (%d,%d,%d,%d,%d,%d)\n",
	      me, needK[0], needK[1], needK[2], needK[3], needK[4], needK[5]);
    }
    for (k=0; k<GAGE_KERNEL_NUM; k++) {
      ctx->needK[k] = needK[k];
    }
    ctx->flag[gageCtxFlagNeedK] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return;
}

/*
** ctx's ksp[] & needK --> needPad & havePad
**
** the combined setting of needPad and havePad is based on the fact
** that besides needPad, there is no other state which, when changed
** triggers a change in havePad.  Changing noRepadWhenSmaller only
** changes future behavior; we won't repad an existing and usable volume
** just because there's a change in policy on how to repad based on 
** future changes to the needed padding.
*/
int
_gageHavePadUpdate (gageContext *ctx) {
  char me[]="_gageHavePadUpdate", err[AIR_STRLEN_MED];
  int k, fr, needPad;
  double maxRad, rad;
  NrrdKernelSpec *ksp;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  maxRad = 0;
  for (k=0; k<GAGE_KERNEL_NUM; k++) {
    if (ctx->needK[k]) {
      ksp = ctx->ksp[k];
      if (!ksp) {
	sprintf(err, "%s: need kernel %s but it hasn't been set", 
		me, airEnumStr(gageKernel, k));
	biffAdd(GAGE, err); return 1;
      }
      rad = ksp->kernel->support(ksp->parm);
      maxRad = AIR_MAX(maxRad, rad);
      if (ctx->verbose) {
	fprintf(stderr, "%s: k[%s]=%s -> rad = %g -> maxRad = %g\n", me,
		airEnumStr(gageKernel, k), ksp->kernel->name,
		rad, maxRad);
      }
    }
  }
  fr = AIR_ROUNDUP(maxRad);
  /* In case either kernels have tiny supports (less than 0.5), or if
     we in fact don't need any kernels, then we need to do this to 
     ensure that we generate a valid (trivial) padding */
  fr = AIR_MAX(fr, 1);
  needPad = fr - 1 + (nrrdCenterCell == ctx->shape->center);
  if (needPad != ctx->needPad) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: fr = %d, %s-(%d)-centering -> needPad=%d\n", me,
	      fr, airEnumStr(nrrdCenter, ctx->shape->center), 
	      ctx->shape->center, needPad);
      fprintf(stderr, "%s: changing needPad from %d to %d\n",
	      me, ctx->needPad, needPad);
    }
    ctx->needPad = needPad;
  }
  if (ctx->havePad < needPad
      || (ctx->havePad > needPad && !ctx->parm.noRepadWhenSmaller)) {
    if (ctx->verbose) {
      fprintf(stderr, "%s: changing havePad from %d to %d\n",
	      me, ctx->havePad, needPad);
    }
    ctx->havePad = needPad;
    ctx->flag[gageCtxFlagHavePad] = AIR_TRUE;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  return 0;
}

/*
** for all pvls: pvl's padder & padInfo, ctx's havePad --> pvl's npad
*/
int
_gageNpadUpdate (gageContext *ctx) {
  char me[]="_gageNpadUpdate", err[AIR_STRLEN_MED];
  gagePerVolume *pvl;
  gageShape *shape1, *shape2;
  int i, osx, osy, osz;
  airArray *mop;

  mop = airMopNew();
  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  for (i=0; i<ctx->numPvl; i++) {
    pvl = ctx->pvl[i];
    if (pvl->thisIsACopy) {
      /* this should never happen */
      sprintf(err, "%s: can't operate on a pervolume copy", me);
      biffAdd(GAGE, err); return 1;
    }
    /* if we *don't* need to repad this volume, then continue */
    if (!( ctx->flag[gageCtxFlagHavePad] 
	   || pvl->flag[gagePvlFlagVolume]
	   || pvl->flag[gagePvlFlagPadder]
	   || pvl->flag[gagePvlFlagPadInfo])) {
      if (ctx->verbose) {
	fprintf(stderr, "%s: pvl[%d] left as is\n", me, i);
      }
      continue;
    }
    if (ctx->verbose) {
      fprintf(stderr, "%s: pvl[%d] being repadded ... \n", me, i);
    }
    if (pvl->nixer && pvl->npad) {
      pvl->nixer(pvl->npad, pvl->kind, pvl);
      pvl->npad = NULL;
    }
    if (pvl->padder) {
      pvl->npad = pvl->padder(pvl->nin, pvl->kind, ctx->havePad, pvl);
      if (!pvl->npad) {
	sprintf(err, "%s: trouble in padder callback", me);
	biffAdd(GAGE, err); return 1;
      }
      if (nrrdCheck(pvl->npad)) {
	sprintf(err, "%s: padder generated bad nrrd", me);
	biffMove(GAGE, err, NRRD); return 1;
      }
      if (pvl->nin->dim != pvl->npad->dim) {
	sprintf(err, "%s: whoa: padder made %d-dim out of %d-dim nrrd", 
		me, pvl->npad->dim, pvl->nin->dim);
	biffAdd(GAGE, err); return 1;
      }
      /* the only that should have changed was the size */
      shape1 = gageShapeNew();
      shape2 = gageShapeNew();
      airMopAdd(mop, shape1, (airMopper)gageShapeNix, airMopAlways);
      airMopAdd(mop, shape2, (airMopper)gageShapeNix, airMopAlways);
      if (_gageShapeSet(ctx, shape1, pvl->nin, pvl->kind->baseDim)) {
	sprintf(err, "%s: trouble", me); 
	biffAdd(GAGE, err); airMopError(mop); return 1;
      }
      osx = shape1->size[0];
      osy = shape1->size[1];
      osz = shape1->size[2];
      shape1->size[0] += 2*ctx->havePad;
      shape1->size[1] += 2*ctx->havePad;
      shape1->size[2] += 2*ctx->havePad;
      if (_gageShapeSet(ctx, shape2, pvl->npad, pvl->kind->baseDim)) {
	sprintf(err, "%s: trouble", me); 
	biffAdd(GAGE, err); airMopError(mop); return 1;
      }
      if (!gageShapeEqual(shape1, "padding target",
			  shape2, "padding result")) {
	sprintf(err, "%s: padder didn't comply with havePad = %d",
		me, ctx->havePad);
	biffAdd(GAGE, err); airMopError(mop); return 1;
      }
      /* it seems that padder did all the right things. Having done
	 the padding, there are no more flags to set here */
      if (ctx->verbose) {
	fprintf(stderr, "%s: padded (%d,%d,%d) --> (%d,%d,%d) okay\n", me,
		osx, osy, osz, shape1->size[0],
		shape1->size[1], shape1->size[2]);
	fprintf(stderr, "%s: npad data = %p\n", me, pvl->npad->data);
	if (ctx->verbose > 1) {
	  nrrdSave("npad.nrrd", pvl->npad, NULL);
	}
      }
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);

  airMopOkay(mop);
  return 0;
}

int
_gageCacheSizeUpdate (gageContext *ctx) {
  char me[]="_gageCacheSizeUpdate", err[AIR_STRLEN_MED];
  int i, fd;
  gagePerVolume *pvl;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  fd = GAGE_FD(ctx);
  AIR_FREE(ctx->fsl);
  AIR_FREE(ctx->fw);
  AIR_FREE(ctx->off);
  ctx->fsl = (gage_t *)calloc(fd*3, sizeof(gage_t));
  ctx->fw = (gage_t *)calloc(fd*3*GAGE_KERNEL_NUM, sizeof(gage_t));
  ctx->off = (unsigned int *)calloc(fd*fd*fd, sizeof(unsigned int));
  if (!(ctx->fsl && ctx->fw && ctx->off)) {
    sprintf(err, "%s: couldn't allocate filter caches for fd=%d", me, fd);
    biffAdd(GAGE, err); return 1;
  }
  for (i=0; i<ctx->numPvl; i++) {
    pvl = ctx->pvl[i];
    AIR_FREE(pvl->iv3);
    AIR_FREE(pvl->iv2);
    AIR_FREE(pvl->iv1);
    pvl->iv3 = (gage_t *)calloc(fd*fd*fd*pvl->kind->valLen, sizeof(gage_t));
    pvl->iv2 = (gage_t *)calloc(fd*fd*pvl->kind->valLen, sizeof(gage_t));
    pvl->iv1 = (gage_t *)calloc(fd*pvl->kind->valLen, sizeof(gage_t));
    if (!(pvl->iv3 && pvl->iv2 && pvl->iv1)) {
      sprintf(err, "%s: couldn't allocate pvl[%d]'s value caches for fd=%d",
	      me, i, fd);
      biffAdd(GAGE, err); return 1;
    }
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  
  return 0;
}

void
_gageOffValueUpdate (gageContext *ctx) {
  char me[]="_gageOffValueUpdate";
  int fd, i, j, k, sx, sy;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  sx = PADSIZE_X(ctx);
  sy = PADSIZE_Y(ctx);
  fd = GAGE_FD(ctx);
  /* HEY: look into special casing this for small fd */
  for (k=0; k<fd; k++)
    for (j=0; j<fd; j++)
      for (i=0; i<fd; i++)
	ctx->off[i + fd*(j + fd*k)] = i + sx*(j + sy*k);
  /* no flags to set for further action */
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  
  return;
}

/*
******** gageUpdate()
**
** call just before probing begins.
*/
int
gageUpdate (gageContext *ctx) {
  char me[]="gageUpdate", err[AIR_STRLEN_MED];
  int i;

  if (!( ctx )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->thisIsACopy) {
    sprintf(err, "%s: can't operate on a context copy", me);
    biffAdd(GAGE, err); return 1;
  }
  if (0 == ctx->numPvl) {
    sprintf(err, "%s: context has no attached pervolumes", me);
    biffAdd(GAGE, err); return 1;
  }

  /* start traversing the whole update graph ... */
  if (ctx->verbose) {
    fprintf(stderr, "%s: hello ____________________ \n", me);
    fprintf(stderr, "    context flags:");
    for (i=gageCtxFlagUnknown+1; i<gageCtxFlagLast; i++) {
      fprintf(stderr, " %d=%d", i, ctx->flag[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "    pvl flags:");
    for (i=gagePvlFlagUnknown+1; i<gagePvlFlagLast; i++) {
      fprintf(stderr, " %d=%d", i, _gagePvlFlagCheck(ctx, i));
    }
    fprintf(stderr, "\n");
  }
  if (_gagePvlFlagCheck(ctx, gagePvlFlagQuery)) {
    _gagePvlNeedDUpdate(ctx);
    _gagePvlFlagDown(ctx, gagePvlFlagQuery);
  }
  if (_gagePvlFlagCheck(ctx, gagePvlFlagNeedD)) {
    _gageNeedDUpdate(ctx);
    _gagePvlFlagDown(ctx, gagePvlFlagNeedD);
  }
  if (ctx->flag[gageCtxFlagNeedD] || ctx->flag[gageCtxFlagK3Pack]) {
    _gageNeedKUpdate(ctx);
    ctx->flag[gageCtxFlagNeedD] = AIR_FALSE;
    ctx->flag[gageCtxFlagK3Pack] = AIR_FALSE;
  }
  if (ctx->flag[gageCtxFlagKernel] || ctx->flag[gageCtxFlagNeedK]) {
    if (_gageHavePadUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
    ctx->flag[gageCtxFlagKernel] = AIR_FALSE;
    ctx->flag[gageCtxFlagNeedK] = AIR_FALSE;
  }
  if (ctx->flag[gageCtxFlagHavePad]
      || _gagePvlFlagCheck(ctx, gagePvlFlagVolume)
      || _gagePvlFlagCheck(ctx, gagePvlFlagPadder)
      || _gagePvlFlagCheck(ctx, gagePvlFlagPadInfo)) {
    if (_gageNpadUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
    _gagePvlFlagDown(ctx, gagePvlFlagPadder);
    _gagePvlFlagDown(ctx, gagePvlFlagPadInfo);
  }
  if (ctx->flag[gageCtxFlagHavePad]
      /* HEY HEY HEY: this is a total hack: right now its possible for a 
	 new pvl to have unallocated iv3,iv2,iv1, if it was attached to a
	 context which had already been probing, as was the case with
	 _tenRegisterDoit.  So, with this hack we reallocate ALL caches
	 just because a new pervolume was attached ... */
      || _gagePvlFlagCheck(ctx, gagePvlFlagVolume)) {
    if (_gageCacheSizeUpdate(ctx)) {
      sprintf(err, "%s: trouble", me); biffAdd(GAGE, err); return 1;
    }
  }
  if (ctx->flag[gageCtxFlagHavePad]
      || ctx->flag[gageCtxFlagShape]) {
    _gageOffValueUpdate(ctx);
    ctx->flag[gageCtxFlagShape] = AIR_FALSE;
  }
  ctx->flag[gageCtxFlagHavePad] = AIR_FALSE;
    
  /* chances are, something above has invalidated the state maintained
     during successive calls to gageProbe() */
  gagePointReset(&ctx->point);
  if (ctx->verbose) fprintf(stderr, "%s: bye ^^^^^^^^^^^^^^^^^^^ \n", me);

  return 0;
}
