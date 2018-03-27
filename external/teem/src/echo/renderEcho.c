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

#include "echo.h"
#include "privateEcho.h"

int
echoThreadStateInit(echoThreadState *tstate,
		    echoRTParm *parm, echoGlobalState *gstate) {
  char me[]="echoThreadStateInit", err[AIR_STRLEN_MED];

  if (!(tstate && parm && gstate)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(ECHO, err); return 1;
  }

  /* this will probably be over-written */
  tstate->verbose = gstate->verbose;

  if (nrrdMaybeAlloc(tstate->nperm, nrrdTypeInt, 2,
		     ECHO_JITTABLE_NUM, parm->numSamples)) {
    sprintf(err, "%s: couldn't allocate jitter permutation array", me);
    biffMove(ECHO, err, NRRD); return 1;
  }
  nrrdAxisInfoSet(tstate->nperm, nrrdAxisInfoLabel,
		  "jittable", "sample");

  if (nrrdMaybeAlloc(tstate->njitt, echoPos_nt, 3,
		     2, ECHO_JITTABLE_NUM, parm->numSamples)) {
    sprintf(err, "%s: couldn't allocate jitter array", me);
    biffMove(ECHO, err, NRRD); return 1;
  }
  nrrdAxisInfoSet(tstate->njitt, nrrdAxisInfoLabel,
		  "x,y", "jittable", "sample");

  AIR_FREE(tstate->permBuff);
  if (!( tstate->permBuff = (int*)calloc(parm->numSamples, sizeof(int)) )) {
    sprintf(err, "%s: couldn't allocate permutation buffer", me);
    biffAdd(ECHO, err); return 1;
  }

  AIR_FREE(tstate->chanBuff);
  if (!( tstate->chanBuff =
	 (echoCol_t*)calloc(ECHO_IMG_CHANNELS * parm->numSamples,
			    sizeof(echoCol_t)) )) {
    sprintf(err, "%s: couldn't allocate img channel sample buffer", me);
    biffAdd(ECHO, err); return 1;
  }
  
  return 0;
}

/*
******** echoJitterCompute()
**
**
*/
void
echoJitterCompute(echoRTParm *parm, echoThreadState *tstate) {
  echoPos_t *jitt, w;
  int s, i, j, xi, yi, n, N, *perm;

  N = parm->numSamples;
  n = sqrt(N);
  w = 1.0/n;
  /* each row in perm[] is for one sample, for going through all jittables;
     each column is a different permutation of [0..parm->numSamples-1] */
  perm = (int *)tstate->nperm->data;
  for (j=0; j<ECHO_JITTABLE_NUM; j++) {
    airShuffle(tstate->permBuff, parm->numSamples, parm->permuteJitter);
    for (s=0; s<N; s++) {
      perm[j + ECHO_JITTABLE_NUM*s] = tstate->permBuff[s];
    }
  }
  jitt = (echoPos_t *)tstate->njitt->data;
  for (s=0; s<N; s++) {
    for (j=0; j<ECHO_JITTABLE_NUM; j++) {
      i = perm[j + ECHO_JITTABLE_NUM*s];
      xi = i % n;
      yi = i / n;
      switch(parm->jitterType) {
      case echoJitterNone:
	jitt[0 + 2*j] = 0.0;
	jitt[1 + 2*j] = 0.0;
	break;
      case echoJitterGrid:
	jitt[0 + 2*j] = NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, xi);
	jitt[1 + 2*j] = NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, yi);
	break;
      case echoJitterJitter:
	jitt[0 + 2*j] = (NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, xi)
			 + AIR_AFFINE(0.0, airRand(), 1.0, -w/2, w/2));
	jitt[1 + 2*j] = (NRRD_POS(nrrdCenterCell, -0.5, 0.5, n, yi)
			 + AIR_AFFINE(0.0, airRand(), 1.0, -w/2, w/2));
	break;
      case echoJitterRandom:
	jitt[0 + 2*j] = airRand() - 0.5;
	jitt[1 + 2*j] = airRand() - 0.5;
	break;
      }
    }
    jitt += 2*ECHO_JITTABLE_NUM;
  }

  return;
}

/*
******** echoRTRenderCheck
**
** does all the error checking required of echoRTRender and
** everything that it calls
*/
int
echoRTRenderCheck(Nrrd *nraw, limnCamera *cam, echoScene *scene,
		  echoRTParm *parm, echoGlobalState *gstate) {
  char me[]="echoRTRenderCheck", err[AIR_STRLEN_MED];
  int tmp;

  if (!(nraw && cam && scene && parm && gstate)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(ECHO, err); return 1;
  }
  if (limnCameraUpdate(cam)) {
    sprintf(err, "%s: camera trouble", me);
    biffMove(ECHO, err, LIMN); return 1;
  }
  if (scene->envmap) {
    if (limnEnvMapCheck(scene->envmap)) {
      sprintf(err, "%s: environment map not valid", me);
      biffMove(ECHO, err, LIMN); return 1;
    }
  }
  if (airEnumValCheck(echoJitter, parm->jitterType)) {
    sprintf(err, "%s: jitter method (%d) invalid", me, parm->jitterType);
    biffAdd(ECHO, err); return 1;
  }
  if (!(parm->numSamples > 0)) {
    sprintf(err, "%s: # samples (%d) invalid", me, parm->numSamples);
    biffAdd(ECHO, err); return 1;
  }
  if (!(parm->imgResU > 0 && parm->imgResV)) {
    sprintf(err, "%s: image dimensions (%dx%d) invalid", me,
	    parm->imgResU, parm->imgResV);
    biffAdd(ECHO, err); return 1;
  }
  if (!AIR_EXISTS(parm->aperture)) {
    sprintf(err, "%s: aperture doesn't exist", me);
    biffAdd(ECHO, err); return 1;
  }
  
  switch (parm->jitterType) {
  case echoJitterNone:
  case echoJitterRandom:
    break;
  case echoJitterGrid:
  case echoJitterJitter:
    tmp = sqrt(parm->numSamples);
    if (tmp*tmp != parm->numSamples) {
      sprintf(err, "%s: need a square # samples for %s jitter method (not %d)",
	      me, airEnumStr(echoJitter, parm->jitterType), parm->numSamples);
      biffAdd(ECHO, err); return 1;
    }
    break;
  }

  /* for the time being things are hard-coded to be r,g,b,a,time */
  if (ECHO_IMG_CHANNELS != 5) {
    sprintf(err, "%s: ECHO_IMG_CHANNELS != 5", me);
    biffAdd(ECHO, err); return 1;
  }
  
  /* all is well */
  return 0;
}

void
echoChannelAverage(echoCol_t *img,
		   echoRTParm *parm, echoThreadState *tstate) {
  int s;
  echoCol_t R, G, B, A, T;
  
  R = G = B = A = T = 0;
  for (s=0; s<parm->numSamples; s++) {
    R += tstate->chanBuff[0 + ECHO_IMG_CHANNELS*s];
    G += tstate->chanBuff[1 + ECHO_IMG_CHANNELS*s];
    B += tstate->chanBuff[2 + ECHO_IMG_CHANNELS*s];
    A += tstate->chanBuff[3 + ECHO_IMG_CHANNELS*s];
    T += tstate->chanBuff[4 + ECHO_IMG_CHANNELS*s];
  }
  img[0] = R / parm->numSamples;
  img[1] = G / parm->numSamples;
  img[2] = B / parm->numSamples;
  img[3] = A / parm->numSamples;
  img[4] = T;
  
  return;
}

/*
******** echoRayColor
**
** This is called by echoRTRender and by the various color routines,
** following an intersection with non-phong non-light material (the
** things that require reflection or refraction rays).  As such, it is
** never called on shadow rays.
*/
void
echoRayColor(echoCol_t *chan, echoRay *ray,
	     echoScene *scene, echoRTParm *parm, echoThreadState *tstate) {
  char me[]="echoRayColor";
  echoCol_t rgba[4];
  echoIntx intx;
  
  tstate->depth++;
  if (tstate->depth > parm->maxRecDepth) {
    /* we've exceeded the recursion depth, so no more rays for you */
    ELL_4V_SET(chan, parm->maxRecCol[0], parm->maxRecCol[1],
	       parm->maxRecCol[2], 1.0);
    goto done;
  }

  intx.boxhits = 0;
  if (!echoRayIntx(&intx, ray, scene, parm, tstate)) {
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: (nothing was hit)\n",_echoDot(tstate->depth), me);
    }
    /* ray hits nothing in scene */
    ELL_4V_SET(chan, scene->bkgr[0], scene->bkgr[1], scene->bkgr[2],
	       (parm->renderBoxes
		? 1.0 - pow(1.0 - parm->boxOpac, intx.boxhits)
		: 1.0));
    goto done;
  }

  if (tstate->verbose) {
    fprintf(stderr, "%s%s: hit a %d (%p) at (%g,%g,%g)\n"
	    "%s    = %g along (%g,%g,%g)\n", _echoDot(tstate->depth), me,
	    intx.obj->type, intx.obj,
	    intx.pos[0], intx.pos[1], intx.pos[2], _echoDot(tstate->depth),
	    intx.t, ray->dir[0], ray->dir[1], ray->dir[2]);
  }
  echoIntxColor(rgba, &intx, scene, parm, tstate);
  ELL_4V_COPY(chan, rgba);
 done:
  tstate->depth--;
  return;
}

/*
******** echoRTRender
**
** top-level call to accomplish all (ray-tracing) rendering.  As much
** error checking as possible should be done here and not in the
** lower-level functions.
*/
int
echoRTRender(Nrrd *nraw, limnCamera *cam, echoScene *scene,
	     echoRTParm *parm, echoGlobalState *gstate) {
  char me[]="echoRTRender", err[AIR_STRLEN_MED], done[20];
  int imgUi, imgVi,         /* integral pixel indices */
    samp;                   /* which sample are we doing */
  echoPos_t tmp0, tmp1,
    eye[3],                 /* eye center before jittering */
    at[3],                  /* ray destination (pixel center post-jittering) */
    U[4], V[4], N[4],       /* view space basis (only first 3 elements used) */
    pixUsz, pixVsz,         /* U and V dimensions of a pixel */
    imgU, imgV,             /* floating point pixel center locations */
    imgOrig[3];             /* image origin */
  echoThreadState *tstate;  /* only one thread for now */
  echoCol_t *img, *chan;    /* current scanline of channel buffer array */
  echoRay ray;              /* (not a pointer) */
  double time0;

  if (echoRTRenderCheck(nraw, cam, scene, parm, gstate)) {
    sprintf(err, "%s: problem with input", me);
    biffAdd(ECHO, err); return 1;
  }
  if (nrrdMaybeAlloc(nraw, echoCol_nt, 3,
		     ECHO_IMG_CHANNELS, parm->imgResU, parm->imgResV)) {
    sprintf(err, "%s: couldn't allocate output image", me);
    biffMove(ECHO, err, NRRD); return 1;
  }
  nrrdAxisInfoSet(nraw, nrrdAxisInfoLabel,
		  "r,g,b,a,t", "x", "y");
  nrrdAxisInfoSet(nraw, nrrdAxisInfoMin,
		  AIR_NAN, cam->uRange[0], cam->vRange[0]);
  nrrdAxisInfoSet(nraw, nrrdAxisInfoMax,
		  AIR_NAN, cam->uRange[1], cam->vRange[1]);
  tstate = echoThreadStateNew();
  if (echoThreadStateInit(tstate, parm, gstate)) {
    sprintf(err, "%s:", me);
    biffAdd(ECHO, err); return 1;
  }

  gstate->time = airTime();
  if (parm->seedRand) {
    airSrand();
  }
  echoJitterCompute(parm, tstate);
  if (gstate->verbose > 2) {
    nrrdSave("jitt.nrrd", tstate->njitt, NULL);
  }
  
  /* set eye, U, V, N, imgOrig */
  ELL_3V_COPY(eye, cam->from);
  ELL_4MV_ROW0_GET(U, cam->W2V);
  ELL_4MV_ROW1_GET(V, cam->W2V);
  ELL_4MV_ROW2_GET(N, cam->W2V);
  ELL_3V_SCALE_ADD2(imgOrig, 1.0, eye, cam->vspDist, N);
  
  /* determine size of a single pixel (based on cell-centering) */
  pixUsz = (cam->uRange[1] - cam->uRange[0])/(parm->imgResU);
  pixVsz = (cam->vRange[1] - cam->vRange[0])/(parm->imgResV);

  tstate->depth = 0;
  ray.shadow = AIR_FALSE;
  img = (echoCol_t *)nraw->data;
  fprintf(stderr, "%s:       ", me);  /* prep for printing airDoneStr */
  tstate->verbose = AIR_FALSE;
  for (imgVi=0; imgVi<parm->imgResV; imgVi++) {
    imgV = NRRD_POS(nrrdCenterCell, cam->vRange[0], cam->vRange[1],
		    parm->imgResV, imgVi);
    if (!(imgVi % 5)) {
      fprintf(stderr, "%s", airDoneStr(0, imgVi, parm->imgResV-1, done));
      fflush(stderr);
    }
    for (imgUi=0; imgUi<parm->imgResU; imgUi++) {
      imgU = NRRD_POS(nrrdCenterCell, cam->uRange[0], cam->uRange[1],
		      parm->imgResU, imgUi);

      /* initialize things on first "scanline" */
      tstate->jitt = (echoPos_t *)tstate->njitt->data;
      chan = tstate->chanBuff;

      /* tstate->verbose = ( (160 == imgUi && 160 == imgVi) ); */
      
      if (tstate->verbose) {
	fprintf(stderr, "\n");
	fprintf(stderr, "-----------------------------------------------\n");
	fprintf(stderr, "----------------- (%3d, %3d) ------------------\n",
		imgUi, imgVi);
	fprintf(stderr, "-----------------------------------------------\n\n");
      }

      /* go through samples */
      for (samp=0; samp<parm->numSamples; samp++) {
	/* set ray.from[] */
	ELL_3V_COPY(ray.from, eye);
	if (parm->aperture) {
	  tmp0 = parm->aperture*(tstate->jitt[0 + 2*echoJittableLens]);
	  tmp1 = parm->aperture*(tstate->jitt[1 + 2*echoJittableLens]);
	  ELL_3V_SCALE_ADD3(ray.from, 1, ray.from, tmp0, U, tmp1, V);
	}
	
	/* set at[] */
	tmp0 = imgU + pixUsz*(tstate->jitt[0 + 2*echoJittablePixel]);
	tmp1 = imgV + pixVsz*(tstate->jitt[1 + 2*echoJittablePixel]);
	ELL_3V_SCALE_ADD3(at, 1, imgOrig, tmp0, U, tmp1, V);

	/* do it! */
	ELL_3V_SUB(ray.dir, at, ray.from);
	ELL_3V_NORM(ray.dir, ray.dir, tmp0);
	ray.neer = 0.0;
	ray.faar = ECHO_POS_MAX;
	time0 = airTime();
	if (0) {
	  memset(chan, 0, ECHO_IMG_CHANNELS*sizeof(echoCol_t));
	} else {
	  echoRayColor(chan, &ray, scene, parm, tstate);
	}
	chan[4] = airTime() - time0;
	
	/* move to next "scanlines" */
	tstate->jitt += 2*ECHO_JITTABLE_NUM;
	chan += ECHO_IMG_CHANNELS;
      }
      echoChannelAverage(img, parm, tstate);
      img += ECHO_IMG_CHANNELS;
      if (!parm->reuseJitter) {
	echoJitterCompute(parm, tstate);
      }
    }
  }
  gstate->time = airTime() - gstate->time;
  fprintf(stderr, "\n%s: time = %g\n", me, gstate->time);
  
  tstate = echoThreadStateNix(tstate);

  return 0;
}
