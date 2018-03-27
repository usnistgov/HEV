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


#include "limn.h"

/*
******** limnCameraUpdate()
**
** sets in cam: W2V matrix, vspNeer, vspFaar, vspDist
**
** This does use biff to describe problems with camera settings
*/
int
limnCameraUpdate(limnCamera *cam) {
  char me[] = "limnCameraUpdate", err[129];
  double len, l[4], u[4], v[4], n[4], T[16], R[16];

  ELL_4V_SET(u, 0, 0, 0, 0);
  ELL_4V_SET(v, 0, 0, 0, 0);
  ELL_4V_SET(n, 0, 0, 0, 0);
  ELL_4V_SET(l, 0, 0, 0, 1);

  ELL_3V_SUB(n, cam->at, cam->from);
  len = ELL_3V_LEN(n);
  if (!len) {
    sprintf(err, "%s: cam->at (%g,%g,%g) == cam->from (%g,%g,%g)\n", me,
	    cam->at[0], cam->at[1], cam->at[2], 
	    cam->from[0], cam->from[1], cam->from[2]);
    biffAdd(LIMN, err); return 1;
  }
  if (cam->atRelative) {
    /* ctx->cam->{neer,dist} are "at" relative */
    cam->vspNeer = cam->neer + len;
    cam->vspFaar = cam->faar + len;
    cam->vspDist = cam->dist + len;
  }
  else {
    /* ctx->cam->{neer,dist} are eye relative */
    cam->vspNeer = cam->neer;
    cam->vspFaar = cam->faar;
    cam->vspDist = cam->dist;
  }
  if (!(cam->vspNeer > 0 && cam->vspDist > 0 && cam->vspFaar > 0)) {
    sprintf(err, "%s: eye-relative near (%g), dist (%g), or far (%g) <= 0\n",
	    me, cam->vspNeer, cam->vspDist, cam->vspFaar);
    biffAdd(LIMN, err); return 1;
  }
  if (!(cam->vspNeer <= cam->vspFaar)) {
    sprintf(err, "%s: eye-relative near (%g) further than far (%g)\n",
	    me, cam->vspNeer, cam->vspFaar);
    biffAdd(LIMN, err); return 1 ;
  }
  ELL_3V_SCALE(n, 1.0/len, n);
  ELL_3V_CROSS(u, n, cam->up);
  len = ELL_3V_LEN(u);
  if (!len) {
    sprintf(err, "%s: cam->up is co-linear with view direction\n", me);
    biffAdd(LIMN, err); return 1 ;
  }
  ELL_3V_SCALE(u, 1.0/len, u);

  if (cam->rightHanded) {
    ELL_3V_CROSS(v, n, u);
  }
  else {
    ELL_3V_CROSS(v, u, n);
  }

  ELL_4V_COPY(cam->U, u);
  ELL_4V_COPY(cam->V, v);
  ELL_4V_COPY(cam->N, n);
  ELL_4M_TRANSLATE_SET(T, -cam->from[0], -cam->from[1], -cam->from[2]);
  ELL_4M_ROWS_SET(R, u, v, n, l);
  ELL_4M_MUL(cam->W2V, R, T);
  ELL_4M_COPY(T, cam->W2V);
  ell_4m_inv_d(cam->V2W, T);

  return 0;
}

/*
******** limnCameraPathMake
**
** the atRelative, orthographic, and rightHanded fields are used from
** keycam[0], and those fields are ignored for all other keycam[i]
*/
int
limnCameraPathMake(limnCamera *cam, int numFrames,
		   limnCamera *keycam, double *time,
		   int numKeys, int trackFrom, 
		   limnSplineTypeSpec *quatType,
		   limnSplineTypeSpec *posType,
		   limnSplineTypeSpec *distType,
		   limnSplineTypeSpec *uvType) {
  char me[]="limnCameraPathMake", err[AIR_STRLEN_MED];
  char which[AIR_STRLEN_MED];
  airArray *mop;
  Nrrd *nquat, *npos, *ndist, *nuv, *ntime, *nsample;
  double fratVec[3], *quat, *pos, *dist, *uv, W2V[9], N[3], fratDist;
  limnSpline *timeSpline, *quatSpline, *posSpline, *distSpline, *uvSpline;
  limnSplineTypeSpec *timeType;
  int ii;
  
  if (!( cam && keycam && time
	 && quatType && posType && distType && uvType )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }

  /* create and allocate nrrds */
  mop = airMopNew();
  airMopAdd(mop, nquat = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, npos = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ndist = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nuv = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntime = nrrdNew(), (airMopper)nrrdNix, airMopAlways);
  if (nrrdWrap(ntime, time, nrrdTypeDouble, 1, numKeys)) {
    sprintf(err, "%s: trouble wrapping time values", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
  }
  airMopAdd(mop, nsample = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  timeType = limnSplineTypeSpecNew(limnSplineTypeTimeWarp);
  airMopAdd(mop, timeType, (airMopper)limnSplineTypeSpecNix, airMopAlways);
  if (nrrdMaybeAlloc(nquat, nrrdTypeDouble, 2, 4, numKeys)
      || nrrdMaybeAlloc(npos, nrrdTypeDouble, 2, 3, numKeys)
      || nrrdMaybeAlloc(ndist, nrrdTypeDouble, 2, 4, numKeys)
      || nrrdMaybeAlloc(nuv, nrrdTypeDouble, 2, 4, numKeys)) {
    sprintf(err, "%s: couldn't allocate buffer nrrds", me);
    biffMove(LIMN, err, NRRD); airMopError(mop); return 1;
  }
  quat = (double*)(nquat->data);
  pos = (double*)(npos->data);
  dist = (double*)(ndist->data);
  uv = (double*)(nuv->data);
  
  /* check cameras, and put camera information into nrrds */
  for (ii=0; ii<numKeys; ii++) {
    if (limnCameraUpdate(keycam + ii)) {
      sprintf(err, "%s: trouble with camera at keyframe %d\n", me, ii);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    ell_4m_to_q_d(quat + 4*ii, keycam[ii].W2V);
    if (ii) {
      if (0 > ELL_4V_DOT(quat + 4*ii, quat + 4*(ii-1))) {
	ELL_4V_SCALE(quat + 4*ii, -1, quat + 4*ii);
      }
    }
    if (trackFrom) {
      ELL_3V_COPY(pos + 3*ii, keycam[ii].from);
    } else {
      ELL_3V_COPY(pos + 3*ii, keycam[ii].at);
    }
    ELL_3V_SUB(fratVec, keycam[ii].from, keycam[ii].at);
    ELL_4V_SET(dist + 4*ii,
	       ELL_3V_LEN(fratVec),
	       keycam[ii].neer, keycam[ii].dist, keycam[ii].faar);
    ELL_4V_SET(uv + 4*ii,
	       keycam[ii].uRange[0], keycam[ii].uRange[1],
	       keycam[ii].vRange[0], keycam[ii].vRange[1]);
  }

  /* create splines from nrrds */
  if (!( (strcpy(which, "quaternion"), quatSpline = 
	  limnSplineCleverNew(nquat, limnSplineInfoQuaternion, quatType))
	 && (strcpy(which, "position"), posSpline = 
	     limnSplineCleverNew(npos, limnSplineInfo3Vector, posType))
	 && (strcpy(which, "distance"), distSpline = 
	     limnSplineCleverNew(ndist, limnSplineInfo4Vector, distType))
	 && (strcpy(which, "uvRange"), uvSpline =
	     limnSplineCleverNew(nuv, limnSplineInfo4Vector, uvType))
	 && (strcpy(which, "time warp"), timeSpline = 
	     limnSplineCleverNew(ntime, limnSplineInfoScalar, timeType)) )) {
    sprintf(err, "%s: trouble creating %s spline", me, which);
    biffAdd(LIMN, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, quatSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, posSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, distSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, quatSpline, (airMopper)limnSplineNix, airMopAlways);
  airMopAdd(mop, timeSpline, (airMopper)limnSplineNix, airMopAlways);

  /* evaluate splines */
  if (limnSplineSample(nsample, timeSpline, limnSplineMinT(timeSpline), 
		       numFrames, limnSplineMaxT(timeSpline))
      || limnSplineNrrdEvaluate(nquat, quatSpline, nsample)
      || limnSplineNrrdEvaluate(npos, posSpline, nsample)
      || limnSplineNrrdEvaluate(ndist, distSpline, nsample)
      || limnSplineNrrdEvaluate(nuv, uvSpline, nsample)) {
    sprintf(err, "%s: trouble evaluating splines", me);
    biffAdd(LIMN, err); airMopError(mop); return 1;
  }
  quat = (double*)(nquat->data);
  pos = (double*)(npos->data);
  dist = (double*)(ndist->data);
  uv = (double*)(nuv->data);

  /* copy information from nrrds back into cameras */
  for (ii=0; ii<numFrames; ii++) {
    cam[ii].atRelative = keycam[0].atRelative;
    cam[ii].orthographic = keycam[0].orthographic;
    cam[ii].rightHanded = keycam[0].rightHanded;
    ell_q_to_3m_d(W2V, quat + 4*ii);
    ELL_3MV_ROW1_GET(cam[ii].up, W2V);
    if (cam[ii].rightHanded) {
      ELL_3V_SCALE(cam[ii].up, -1, cam[ii].up);
    }
    ELL_3MV_ROW2_GET(N, W2V);
    ELL_4V_GET(fratDist, cam[ii].neer, cam[ii].dist, cam[ii].faar,
	       dist + 4*ii);
    if (trackFrom) {
      ELL_3V_COPY(cam[ii].from, pos + 3*ii);
      ELL_3V_SCALE_ADD2(cam[ii].at, 1.0, cam[ii].from, fratDist, N);
    } else {
      ELL_3V_COPY(cam[ii].at, pos + 3*ii);
      ELL_3V_SCALE_ADD2(cam[ii].from, 1.0, cam[ii].at, -fratDist, N);
    }
    ELL_4V_GET(cam[ii].uRange[0], cam[ii].uRange[1],
	       cam[ii].vRange[0], cam[ii].vRange[1], uv + 4*ii);
    if (limnCameraUpdate(cam + ii)) {
      sprintf(err, "%s: trouble with output camera %d\n", me, ii);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
  }
  
  airMopOkay(mop);
  return 0;
}
