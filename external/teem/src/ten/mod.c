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


#include "ten.h"
#include "tenPrivate.h"

int
tenSizeNormalize(Nrrd *nout, Nrrd *nin, float _weight[3],
		 float amount, float target) {
  char me[]="tenSizeNormalize", err[AIR_STRLEN_MED];
  float *tin, *tout, eval[3], evec[9], size, weight[3];
  size_t N, I;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdCopy(nout, nin)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  ELL_3V_COPY(weight, _weight);
  size = weight[0] + weight[1] + weight[2];
  if (!size) {
    sprintf(err, "%s: some of eigenvalue weights is zero", me);
    biffAdd(TEN, err); return 1;
  }
  weight[0] /= size;
  weight[1] /= size;
  weight[2] /= size;
  /*
  fprintf(stderr, "!%s: real weight = %g %g %g; amount = %g\n",
	  me, weight[0], weight[1], weight[2], amount);
  */
  tin = (float*)(nin->data);
  tout = (float*)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<=N-1; I++) {
    tenEigensolve(eval, evec, tin);
    size = (weight[0]*AIR_ABS(eval[0])
	    + weight[1]*AIR_ABS(eval[1])
	    + weight[2]*AIR_ABS(eval[2]));
    /*
    fprintf(stderr, "!%s: eval = %g %g %g --> size = %g --> ",
	    me, eval[0], eval[1], eval[2], size);
    */
    eval[0] = AIR_AFFINE(0, amount, 1, eval[0], eval[0]/size);
    eval[1] = AIR_AFFINE(0, amount, 1, eval[1], eval[1]/size);
    eval[2] = AIR_AFFINE(0, amount, 1, eval[2], eval[2]/size);
    /*
    fprintf(stderr, "%g %g %g\n", eval[0], eval[1], eval[2]);
    */
    tenMakeOne(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  
  return 0;
}

int
tenSizeScale(Nrrd *nout, Nrrd *nin, float amount) {
  char me[]="tenSizeScale", err[AIR_STRLEN_MED];
  size_t I, N;
  float *tin, *tout;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tout[0] = tin[0];
    tout[1] = amount*tin[1];
    tout[2] = amount*tin[2];
    tout[3] = amount*tin[3];
    tout[4] = amount*tin[4];
    tout[5] = amount*tin[5];
    tout[6] = amount*tin[6];
    tin += 7;
    tout += 7;
  }
  return 0;
}


/*
******** tenAnisoScale
**
** scales the "deviatoric" part of a tensor up or down
*/
int
tenAnisoScale(Nrrd *nout, Nrrd *nin, float scale,
	      int fixDet, int makePositive) {
  char me[]="tenAnisoScale", err[AIR_STRLEN_MED];
  size_t I, N;
  float *tin, *tout, mean, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve(eval, evec, tin);
    if (fixDet) {
      eval[0] = AIR_MAX(eval[0], 0.00001);
      eval[1] = AIR_MAX(eval[1], 0.00001);
      eval[2] = AIR_MAX(eval[2], 0.00001);
      eval[0] = log(eval[0]);
      eval[1] = log(eval[1]);
      eval[2] = log(eval[2]);
    }
    mean = (eval[0] + eval[1] + eval[2])/3.0;
    eval[0] = AIR_LERP(scale, mean, eval[0]);
    eval[1] = AIR_LERP(scale, mean, eval[1]);
    eval[2] = AIR_LERP(scale, mean, eval[2]);
    if (fixDet) {
      eval[0] = exp(eval[0]);
      eval[1] = exp(eval[1]);
      eval[2] = exp(eval[2]);
    }
    if (eval[2] < 0 && makePositive) {
      eval[0] = AIR_MAX(eval[0], 0.0);
      eval[1] = AIR_MAX(eval[1], 0.0);
      eval[2] = AIR_MAX(eval[2], 0.0);
    }
    tenMakeOne(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  return 0;
}

/*
******** tenEigenvalueClamp
**
** enstates the given value as the lowest eigenvalue
*/
int
tenEigenvalueClamp(Nrrd *nout, Nrrd *nin, float min, float max) {
  char me[]="tenEigenvalueClamp", err[AIR_STRLEN_MED];
  size_t I, N;
  float *tin, *tout, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve(eval, evec, tin);
    if (AIR_EXISTS(min)) {
      eval[0] = AIR_MAX(eval[0], min);
      eval[1] = AIR_MAX(eval[1], min);
      eval[2] = AIR_MAX(eval[2], min);
    }
    if (AIR_EXISTS(max)) {
      eval[0] = AIR_MIN(eval[0], max);
      eval[1] = AIR_MIN(eval[1], max);
      eval[2] = AIR_MIN(eval[2], max);
    }
    tenMakeOne(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  return 0;
}

/*
******** tenEigenvaluePower
**
** raises the eigenvalues to some power
*/
int
tenEigenvaluePower(Nrrd *nout, Nrrd *nin, float expo) {
  char me[]="tenEigenvaluePower", err[AIR_STRLEN_MED];
  size_t I, N;
  float *tin, *tout, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve(eval, evec, tin);
    eval[0] = pow(eval[0], expo);
    eval[1] = pow(eval[1], expo);
    eval[2] = pow(eval[2], expo);
    tenMakeOne(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  return 0;
}

/*
******** tenEigenvalueAdd
**
** adds something to all eigenvalues
*/
int
tenEigenvalueAdd(Nrrd *nout, Nrrd *nin, float val) {
  char me[]="tenEigenvalueAdd", err[AIR_STRLEN_MED];
  size_t I, N;
  float *tin, *tout, eval[3], evec[9];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s: couldn't allocate output", me);
      biffMove(TEN, err, NRRD); return 1;
    }
  }
  tin = (float *)(nin->data);
  tout = (float *)(nout->data);
  N = nrrdElementNumber(nin)/7;
  for (I=0; I<N; I++) {
    tenEigensolve(eval, evec, tin);
    eval[0] += val;
    eval[1] += val;
    eval[2] += val;
    tenMakeOne(tout, tin[0], eval, evec);
    tin += 7;
    tout += 7;
  }
  return 0;
}
