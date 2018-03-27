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

/*
******** gageContextNew()
**
** doesn't use biff
*/
gageContext *
gageContextNew () {
  gageContext *ctx;
  int i;
  
  ctx = (gageContext*)calloc(1, sizeof(gageContext));
  if (ctx) {
    ctx->verbose = gageDefVerbose;
    ctx->thisIsACopy = AIR_FALSE;
    gageParmReset(&ctx->parm);
    gageKernelReset(ctx);
    for (i=0; i<GAGE_PERVOLUME_NUM; i++)
      ctx->pvl[i] = NULL;
    ctx->numPvl = 0;
    ctx->shape = gageShapeNew();
    for (i=0; i<GAGE_CTX_FLAG_NUM; i++) {
      ctx->flag[i] = AIR_FALSE;
    }
    ctx->needD[0] = ctx->needD[1] = ctx->needD[2] = AIR_FALSE;
    for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      ctx->needK[i] = AIR_FALSE;
    }
    ctx->needPad = ctx->havePad = -1;
    ctx->fsl = ctx->fw = NULL;
    ctx->off = NULL;
    gagePointReset(&ctx->point);
  }
  return ctx;
}

/*
******** gageContextNix()
**
** responsible for freeing and clearing up everything hanging off a 
** context so that things can be returned to the way they were prior
** to gageContextNew().
**
** does not use biff
*/
gageContext *
gageContextNix (gageContext *ctx) {
  int i;

  if (ctx) {
    for (i=0; i<GAGE_KERNEL_NUM; i++) {
      nrrdKernelSpecNix(ctx->ksp[i]);
    }
    for (i=0; i<ctx->numPvl; i++) {
      gagePerVolumeNix(ctx->pvl[i]);
      /* no point in doing a detach, the whole context is going bye-bye */
    }
    ctx->shape = gageShapeNix(ctx->shape);
    AIR_FREE(ctx->fw);
    AIR_FREE(ctx->fsl);
    AIR_FREE(ctx->off);
  }
  AIR_FREE(ctx);
  return NULL;
}

/*
******** gageContextCopy()
**
** the semantics and utility of this are purposefully limited: given a context
** on which gageUpdate() has just been successfully called, this will create
** a new context and new attached pervolumes so that you can call gageProbe()
** on the new context by probing the same volumes with the same kernels
** and the same queries.  And that's all you can do- you can't change any
** state in either the original or any of the copy contexts.  This is only
** intended as a simple way to supported multi-threaded usages which want
** to do the exact same thing in many different threads.  If you want to 
** change state, then gageContextNix() all the copy contexts, gage...Set(),
** gageUpdate(), and gageContextCopy() again.
*/
gageContext *
gageContextCopy (gageContext *ctx) {
  char me[]="gageContextCopy", err[AIR_STRLEN_MED];
  gageContext *ntx;
  int fd, i;

  ntx = (gageContext*)calloc(1, sizeof(gageContext));
  if (!ntx) {
    sprintf(err, "%s: couldn't make a gageContext", me);
    biffAdd(GAGE, err); return NULL;
  }
  /* we should probably restrict ourselves to gage API calls, but given the
     constant state of gage construction, this seems much simpler.
     Pointers are fixed below */
  memcpy(ntx, ctx, sizeof(gageContext));

  for (i=0; i<ntx->numPvl; i++) {
    ntx->pvl[i] = _gagePerVolumeCopy(ctx->pvl[i], GAGE_FD(ctx));
    if (!ntx->pvl[i]) {
      sprintf(err, "%s: trouble copying pervolume %d", me, i);
      biffAdd(GAGE, err); return NULL;
    }
  }
  ntx->thisIsACopy = AIR_TRUE;
  fd = GAGE_FD(ntx);
  ntx->fsl = (gage_t *)calloc(fd*3, sizeof(gage_t));
  ntx->fw = (gage_t *)calloc(fd*3*GAGE_KERNEL_NUM, sizeof(gage_t));
  ntx->off = (unsigned int *)calloc(fd*fd*fd, sizeof(unsigned int));
  if (!( ntx->fsl && ntx->fw && ntx->off )) {
    sprintf(err, "%s: couldn't allocate new filter caches for fd=%d",
	    me, fd);
    biffAdd(GAGE, err); return NULL;
  }
  /* the content of the offset array needs to be copied because
     it won't be refilled simply by calls to gageProbe() */
  memcpy(ntx->off, ctx->off, fd*fd*fd*sizeof(unsigned int));

  /* make sure gageProbe() has to refill caches */
  gagePointReset(&ntx->point);

  return ntx;
}

/*
******** gageKernelSet()
**
** sets one kernel in a gageContext; but the value of this function
** is all the error checking it does.
**
** Refers to ctx->checkIntegrals and acts appropriately.
**
** Does use biff.
**
** Sets: ctx->k[which], ctx->kparm[which]
*/
int
gageKernelSet (gageContext *ctx, 
	       int which, NrrdKernel *k, double *kparm) {
  char me[]="gageKernelSet", err[AIR_STRLEN_MED];
  int numParm;
  double support, integral;

  if (!(ctx && k && kparm)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->thisIsACopy) {
    sprintf(err, "%s: can't operate on a context copy", me);
    biffAdd(GAGE, err); return 1;
  }
  if (airEnumValCheck(gageKernel, which)) {
    sprintf(err, "%s: \"which\" (%d) not in range [%d,%d]", me,
	    which, gageKernelUnknown+1, gageKernelLast-1);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->verbose) {
    fprintf(stderr, "%s: which = %d -> %s\n", me, which,
	    airEnumStr(gageKernel, which));
  }
  numParm = k->numParm;
  if (!(AIR_IN_CL(0, numParm, NRRD_KERNEL_PARMS_NUM))) {
    sprintf(err, "%s: kernel's numParm (%d) not in range [%d,%d]",
	    me, numParm, 0, NRRD_KERNEL_PARMS_NUM);
    biffAdd(GAGE, err); return 1;
  }
  support = k->support(kparm);
  if (!( support > 0 )) {
    sprintf(err, "%s: kernel's support (%g) not > 0", me, support);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->parm.checkIntegrals) {
    integral = k->integral(kparm);
    if (gageKernel00 == which ||
	gageKernel10 == which ||
	gageKernel20 == which) {
      if (!( integral > 0 )) {
	sprintf(err, "%s: reconstruction kernel's integral (%g) not > 0.0",
		me, integral);
	biffAdd(GAGE, err); return 1;
      }
    } else {
      /* its a derivative, so integral must be near zero */
      if (!( AIR_ABS(integral) <= ctx->parm.kernelIntegralNearZero )) {
	sprintf(err, "%s: derivative kernel's integral (%g) not within "
		"%g of 0.0",
		me, integral, ctx->parm.kernelIntegralNearZero);
	biffAdd(GAGE, err); return 1;
      }
    }
  }

  /* okay enough enough, go set the kernel */
  if (!ctx->ksp[which]) {
    ctx->ksp[which] = nrrdKernelSpecNew();
  }
  nrrdKernelSpecSet(ctx->ksp[which], k, kparm);
  ctx->flag[gageCtxFlagKernel] = AIR_TRUE;

  return 0;
}

/*
******** gageKernelReset()
**
** reset kernels and parameters.
*/
void
gageKernelReset (gageContext *ctx) {
  char me[]="gageKernelReset";
  int i;

  if (ctx) {
    if (ctx->thisIsACopy) {
      fprintf(stderr, "\n%s: can't operate on a context copy!\n\n", me);
      return;
    }
    for(i=gageKernelUnknown+1; i<gageKernelLast; i++)
      ctx->ksp[i] = nrrdKernelSpecNix(ctx->ksp[i]);
    ctx->flag[gageCtxFlagKernel] = AIR_TRUE;
  }
  return;
}

/*
******** gageParmSet()
**
** for setting the boolean-ish flags in the context in a safe and
** intelligent manner, since changing some of them can have many
** consequences
*/
void
gageParmSet (gageContext *ctx, int which, gage_t val) {
  char me[]="gageParmSet";
  int p;
  
  if (ctx->thisIsACopy) {
    fprintf(stderr, "\n%s: can't operate on a copy of a context!\n\n", me);
    return;
  }
  switch (which) {
  case gageParmVerbose:
    ctx->verbose = val;
    for (p=0; p<ctx->numPvl; p++) {
      ctx->pvl[p]->verbose = val;
    }
    break;
  case gageParmRenormalize:
    ctx->parm.renormalize = val ? AIR_TRUE : AIR_FALSE;
    /* we have to make sure that any existing filter weights
       are not re-used; because gageUpdage() is not called mid-probing,
       we don't use the flag machinery.  Instead we just invalidate
       the last known fractional probe locations */
    gagePointReset(&ctx->point);
    break;
  case gageParmCheckIntegrals:
    ctx->parm.checkIntegrals = val ? AIR_TRUE : AIR_FALSE;
    /* no flags to set, simply affects future calls to gageKernelSet() */
    break;
  case gageParmNoRepadWhenSmaller:
    ctx->parm.noRepadWhenSmaller = AIR_TRUE;
    /* no flag to set, but does affect future calls to _gageHavePadUpdate() */
    break;
  case gageParmK3Pack:
    ctx->parm.k3pack = val ? AIR_TRUE : AIR_FALSE;
    ctx->flag[gageCtxFlagK3Pack] = AIR_TRUE;
    break;
  case gageParmGradMagMin:
    ctx->parm.gradMagMin = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmGradMagCurvMin:
    ctx->parm.gradMagCurvMin = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmDefaultSpacing:
    ctx->parm.defaultSpacing = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmCurvNormalSide:
    ctx->parm.curvNormalSide = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmKernelIntegralNearZero:
    ctx->parm.kernelIntegralNearZero = val;
    /* no flag to set, simply affects future calls to gageKernelSet() */
    break;
  case gageParmRequireAllSpacings:
    ctx->parm.requireAllSpacings = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmRequireEqualCenters:
    ctx->parm.requireEqualCenters = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  case gageParmDefaultCenter:
    ctx->parm.defaultCenter = val;
    /* no flag to set, simply affects future calls to gageProbe() */
    break;
  default:
    fprintf(stderr, "\n%s: which = %d not valid!!\n\n", me, which);
    break;
  }
  return;
}

/*
******** gagePerVolumeIsAttached()
**
*/
int
gagePerVolumeIsAttached (gageContext *ctx, gagePerVolume *pvl) {
  int i, ret;

  ret = AIR_FALSE;
  for (i=0; i<ctx->numPvl; i++) {
    if (pvl == ctx->pvl[i]) {
      ret = AIR_TRUE;
    }
  }
  return ret;
}

/*
******** gagePerVolumeAttach()
**
** attaches a pervolume to a context, which actually involves 
** very little work
*/
int
gagePerVolumeAttach (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gagePerVolumeAttach", err[AIR_STRLEN_MED];
  gageShape *shape;

  if (!( ctx && pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->thisIsACopy) {
    sprintf(err, "%s: can't operate on a context copy", me);
    biffAdd(GAGE, err); return 1;
  }
  if (gagePerVolumeIsAttached(ctx, pvl)) {
    sprintf(err, "%s: given pervolume already attached", me);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->numPvl == GAGE_PERVOLUME_NUM) {
    sprintf(err, "%s: sorry, already have GAGE_PERVOLUME_NUM == %d "
	    "pervolumes attached", me, GAGE_PERVOLUME_NUM);
    biffAdd(GAGE, err); return 1;
  }

  if (0 == ctx->numPvl) {
    /* the volume "shape" is context state that we set now, because unlike 
       everything else (handled by gageUpdate()), it does not effect
       the kind or amount of padding done */
    if (_gageShapeSet(ctx, ctx->shape, pvl->nin, pvl->kind->baseDim)) {
      sprintf(err, "%s: trouble", me); 
      biffAdd(GAGE, err); return 1;
    }
    ctx->flag[gageCtxFlagShape] = AIR_TRUE;
  } else {
    /* have to check to that new pvl matches first one.  Since all
       attached pvls were at some point the "new" one, they all
       should match each other */
    shape = gageShapeNew();
    if (_gageShapeSet(ctx, shape, pvl->nin, pvl->kind->baseDim)) {
      sprintf(err, "%s: trouble", me); 
      biffAdd(GAGE, err); return 1;
    }
    if (!gageShapeEqual(ctx->shape, "existing context", shape, "new volume")) {
      sprintf(err, "%s: trouble", me);
      biffAdd(GAGE, err); gageShapeNix(shape); return 1;
    }
    gageShapeNix(shape); 
  }
  /* here we go */
  ctx->pvl[ctx->numPvl++] = pvl;
  pvl->verbose = ctx->verbose;

  return 0;
}

/*
******** gagePerVolumeDetach()
**
** detaches a pervolume from a context, but does nothing else
** with the pervolume; caller may want to call gagePerVolumeNix
** if this pervolume will no longer be used
*/
int
gagePerVolumeDetach (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gagePerVolumeDetach", err[AIR_STRLEN_MED];
  int i, idx=0;

  if (!( ctx && pvl )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return 1;
  }
  if (ctx->thisIsACopy) {
    sprintf(err, "%s: can't operate on a context copy", me);
    biffAdd(GAGE, err); return 1;
  }
  if (!gagePerVolumeIsAttached(ctx, pvl)) {
    sprintf(err, "%s: given pervolume not currently attached", me);
    biffAdd(GAGE, err); return 1;
  }
  for (i=0; i<ctx->numPvl; i++) {
    if (pvl == ctx->pvl[i]) {
      idx = i;
    }
  }
  for (i=idx+1; i<ctx->numPvl; i++) {
    ctx->pvl[i-1] = ctx->pvl[i];
  }
  ctx->pvl[ctx->numPvl--] = NULL;
  if (0 == ctx->numPvl) {
    /* leave things the way that they started */
    gageShapeReset(ctx->shape);
    ctx->flag[gageCtxFlagShape] = AIR_TRUE;
  }
  return 0;
}

/*
** gageIv3Fill()
**
** based on ctx's shape and havePad, and the (xi,yi,zi) determined from
** the probe location, fills the iv3 cache in the given pervolume
*/
void
gageIv3Fill (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="gageIv3Fill";
  int i, sx, sy, sz, fd, fddd, bidx, tup;
  void *here;

  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  sx = PADSIZE_X(ctx);
  sy = PADSIZE_Y(ctx);
  sz = PADSIZE_Z(ctx);
  fd = GAGE_FD(ctx);
  fddd = fd*fd*fd;
  /* we shouldn't have to worry about centering anymore, since it
     was taken into account in calculating havePad in _gageHavePadUpdate(),
     right ? */
  /* diff = -ctx->havePad + (nrrdCenterCell == ctx->shape.center); */
  bidx = (ctx->point.xi - ctx->havePad
	  + sx*(ctx->point.yi - ctx->havePad
		+ sy*(ctx->point.zi - ctx->havePad)));
  if (ctx->verbose) {
    fprintf(stderr, "%s: hello, valLen = %d, pvl->npad = %p, data = %p\n",
	    me, pvl->kind->valLen, pvl->npad, pvl->npad->data);
  }
  here = ((char*)(pvl->npad->data) + (bidx * pvl->kind->valLen * 
				      nrrdTypeSize[pvl->npad->type]));
  if (ctx->verbose) fprintf(stderr, "%s: hello\n", me);
  if (ctx->verbose) {
    fprintf(stderr, "%s: padded size = (%d,%d,%d);\n"
	    "    fd = %d; point (pad: %d) coord = (%d,%d,%d) --> bidx = %d\n",
	    me, sx, sy, sz,
	    fd, ctx->havePad, ctx->point.xi, ctx->point.yi, ctx->point.zi,
	    bidx);
    fprintf(stderr, "%s: here = %p; iv3 = %p; off[0] = %d\n",
	    me, here, pvl->iv3, ctx->off[0]);
  }
  switch(pvl->kind->valLen) {
  case 1:
    for (i=0; i<fddd; i++) {
      pvl->iv3[i] = pvl->lup(here, ctx->off[i]);
    }
    break;
    /* the tuple axis is being shifted from the fastest to
       the slowest axis, to anticipate component-wise filtering
       operations */
  case 3:
    for (i=0; i<fddd; i++) {
      pvl->iv3[i + fddd*0] = pvl->lup(here, 0 + 3*ctx->off[i]);
      pvl->iv3[i + fddd*1] = pvl->lup(here, 1 + 3*ctx->off[i]);
      pvl->iv3[i + fddd*2] = pvl->lup(here, 2 + 3*ctx->off[i]);
    }
    break;
  case 7:
    /* this might come in handy for tenGage ... */
    for (i=0; i<fddd; i++) {
      pvl->iv3[i + fddd*0] = pvl->lup(here, 0 + 7*ctx->off[i]);
      pvl->iv3[i + fddd*1] = pvl->lup(here, 1 + 7*ctx->off[i]);
      pvl->iv3[i + fddd*2] = pvl->lup(here, 2 + 7*ctx->off[i]);
      pvl->iv3[i + fddd*3] = pvl->lup(here, 3 + 7*ctx->off[i]);
      pvl->iv3[i + fddd*4] = pvl->lup(here, 4 + 7*ctx->off[i]);
      pvl->iv3[i + fddd*5] = pvl->lup(here, 5 + 7*ctx->off[i]);
      pvl->iv3[i + fddd*6] = pvl->lup(here, 6 + 7*ctx->off[i]);
    }
    break;
  default:
    for (i=0; i<fddd; i++) {
      for (tup=0; tup<pvl->kind->valLen; tup++) {
	pvl->iv3[i + fddd*tup] = 
	  pvl->lup(here, 0 + pvl->kind->valLen*ctx->off[i]);
      }
    }
    break;
  }
  if (ctx->verbose) fprintf(stderr, "%s: bye\n", me);
  return;
}

/*
******** gageProbe()
**
** how to do probing.  (x,y,z) position is in UNPADDED volume
**
** doesn't actually do much more than call callbacks in the gageKind
** structs of the attached pervolumes
*/
int
gageProbe (gageContext *ctx, gage_t x, gage_t y, gage_t z) {
  char me[]="gageProbe";
  int xi, yi, zi, i;
  
  /* fprintf(stderr, "##%s: bingo 0\n", me); */
  xi = ctx->point.xi;
  yi = ctx->point.yi;
  zi = ctx->point.zi;
  if (_gageLocationSet(ctx, x, y, z)) {
    /* we're outside the volume; leave gageErrStr and gageErrNum set
       (as they should be) */
    return 1;
  }
  
  /* fprintf(stderr, "##%s: bingo 1\n", me); */
  /* if necessary, refill the iv3 cache */
  if (!( xi == ctx->point.xi &&
	 yi == ctx->point.yi &&
	 zi == ctx->point.zi )) {
    for (i=0; i<ctx->numPvl; i++) {
      gageIv3Fill(ctx, ctx->pvl[i]);
    }
  }
  /* fprintf(stderr, "##%s: bingo 2\n", me); */
  for (i=0; i<ctx->numPvl; i++) {
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s: pvl[%d]'s value cache with (unpadded) "
	      "coords = %d,%d,%d:\n", me, i,
	      ctx->point.xi - ctx->havePad,
	      ctx->point.yi - ctx->havePad,
	      ctx->point.zi - ctx->havePad);
      ctx->pvl[i]->kind->iv3Print(stderr, ctx, ctx->pvl[i]);
    }
    ctx->pvl[i]->kind->filter(ctx, ctx->pvl[i]);
    ctx->pvl[i]->kind->answer(ctx, ctx->pvl[i]);
  }
  
  /* fprintf(stderr, "##%s: bingo 5\n", me); */
  return 0;
}

