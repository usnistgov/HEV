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

int tenVerbose = 0;

/*
******** tenTensorCheck()
**
** describes if the given nrrd could be a diffusion tensor dataset,
** either the measured DWI data or the calculated tensor data.
**
** We've been using 7 floats for BOTH kinds of tensor data- both the
** measured DWI and the calculated tensor matrices.  The measured data
** comes as one anatomical image and 6 DWIs.  For the calculated tensors,
** in addition to the 6 matrix components, we keep a "threshold" value
** which is based on the sum of all the DWIs, which describes if the
** calculated tensor means anything or not.
** 
** useBiff controls if biff is used to describe the problem
*/
int
tenTensorCheck(Nrrd *nin, int wantType, int want4D, int useBiff) {
  char me[]="tenTensorCheck", err[256];
  
  if (!nin) {
    sprintf(err, "%s: got NULL pointer", me);
    if (useBiff) biffAdd(TEN, err); return 1;
  }
  if (wantType) {
    if (nin->type != wantType) {
      sprintf(err, "%s: wanted type %s, got type %s", me,
	      airEnumStr(nrrdType, wantType),
	      airEnumStr(nrrdType, nin->type));
      if (useBiff) biffAdd(TEN, err); return 1;
    }
  }
  else {
    if (!(nin->type == nrrdTypeFloat || nin->type == nrrdTypeShort)) {
      sprintf(err, "%s: need data of type float or short", me);
      if (useBiff) biffAdd(TEN, err); return 1;
    }
  }
  if (want4D && !(4 == nin->dim)) {
    sprintf(err, "%s: given dimension is %d, not 4", me, nin->dim);
    if (useBiff) biffAdd(TEN, err); return 1;
  }
  if (!(7 == nin->axis[0].size)) {
    sprintf(err, "%s: axis 0 has size %d, not 7", me, nin->axis[0].size);
    if (useBiff) biffAdd(TEN, err); return 1;
  }
  return 0;
}

int
tenExpand(Nrrd *nout, Nrrd *nin, float scale, float thresh) {
  char me[]="tenExpand", err[AIR_STRLEN_MED];
  size_t N, I;
  int sx, sy, sz;
  float *seven, *nine;

  if (!( nout && nin && AIR_EXISTS(thresh) )) {
    sprintf(err, "%s: got NULL pointer or non-existant threshold", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: ", me);
    biffAdd(TEN, err); return 1;
  }

  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  N = sx*sy*sz;
  if (nrrdMaybeAlloc(nout, nrrdTypeFloat, 4, 9, sx, sy, sz)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  for (I=0; I<=N-1; I++) {
    seven = (float*)(nin->data) + I*7;
    nine = (float*)(nout->data) + I*9;
    if (seven[0] < thresh) {
      ELL_3M_ZERO_SET(nine);
      continue;
    }
    TEN_LIST2MAT(nine, seven);
    ELL_3M_SCALE(nine, scale, nine);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  AIR_FREE(nout->axis[0].label);
  nout->axis[0].label = airStrdup("matrix");

  return 0;
}

int
tenShrink(Nrrd *tseven, Nrrd *nconf, Nrrd *tnine) {
  char me[]="tenShrink", err[AIR_STRLEN_MED];
  int sx, sy, sz;
  float *seven, *conf, *nine;
  size_t I, N;
  
  if (!(tseven && tnine)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( nrrdTypeFloat == tnine->type &&
	 4 == tnine->dim &&
	 9 == tnine->axis[0].size )) {
    sprintf(err, "%s: type not %s (was %s) or dim not 4 (was %d) "
	    "or first axis size not 9 (was %d)", me,
	    airEnumStr(nrrdType, nrrdTypeFloat),
	    airEnumStr(nrrdType, tnine->type),
	    tnine->dim, tnine->axis[0].size);
    biffAdd(TEN, err); return 1;
  }
  sx = tnine->axis[1].size;
  sy = tnine->axis[2].size;
  sz = tnine->axis[3].size;
  if (nconf) {
    if (!( nrrdTypeFloat == nconf->type &&
	   3 == nconf->dim &&
	   sx == nconf->axis[0].size &&
	   sy == nconf->axis[1].size &&
	   sz == nconf->axis[2].size )) {
      sprintf(err, "%s: confidence type not %s (was %s) or dim not 3 (was %d) "
	      "or dimensions didn't match tensor volume", me,
	      airEnumStr(nrrdType, nrrdTypeFloat),
	      airEnumStr(nrrdType, nconf->type),
	      nconf->dim);
      biffAdd(TEN, err); return 1;
    }
  }
  if (nrrdMaybeAlloc(tseven, nrrdTypeFloat, 4, 7, sx, sy, sz)) {
    sprintf(err, "%s: trouble allocating output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  seven = tseven->data;
  conf = nconf ? nconf->data : NULL;
  nine = tnine->data;
  N = sx*sy*sz;
  for (I=0; I<N; I++) {
    TEN_MAT2LIST(seven, nine);
    seven[0] = conf ? conf[I] : 1.0;
    seven += 7;
    nine += 9;
  }
  if (nrrdAxisInfoCopy(tseven, tnine, NULL, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  AIR_FREE(tseven->axis[0].label);
  tseven->axis[0].label = airStrdup("tensor");

  return 0;
}

/*
******** tenEigensolve
**
** uses ell_3m_eigensolve_d to get the eigensystem of a single tensor
** disregards the confidence value t[0]
**
** return is same as ell_3m_eigensolve_d, which is same as ell_cubic
**
** This does NOT use biff
*/
int
tenEigensolve(float _eval[3], float _evec[9], float t[7]) {
  double m[9], eval[3], evec[9], trc, iso[9];
  int ret;
  
  TEN_LIST2MAT(m, t);
  trc = ELL_3M_TRACE(m)/3.0;
  ELL_3M_IDENTITY_SET(iso);
  ELL_3M_SCALE(iso, trc, iso);
  ELL_3M_SUB(m, m, iso);
  if (_evec) {
    ret = ell_3m_eigensolve_d(eval, evec, m, AIR_TRUE);
    if (ell_cubic_root_three != ret && tenVerbose) {
      fprintf(stderr, "---- cubic ret = %d\n", ret);
      fprintf(stderr, "tensor = {\n");
      fprintf(stderr, "    % 15.7f,\n", t[1]);
      fprintf(stderr, "    % 15.7f,\n", t[2]);
      fprintf(stderr, "    % 15.7f,\n", t[3]);
      fprintf(stderr, "    % 15.7f,\n", t[4]);
      fprintf(stderr, "    % 15.7f,\n", t[5]);
      fprintf(stderr, "    % 15.7f}\n", t[6]);
      fprintf(stderr, "roots = \n");
      fprintf(stderr, "    % 31.15f\n", trc + eval[0]);
      fprintf(stderr, "    % 31.15f\n", trc + eval[1]);
      fprintf(stderr, "    % 31.15f\n", trc + eval[2]);
    }
    ELL_3V_SET(_eval, eval[0] + trc, eval[1] + trc, eval[2] + trc);
    ELL_3M_COPY(_evec, evec);
    if (ell_cubic_root_single_double == ret) {
      /* this was added to fix a stupid problem with very nearly
	 isotropic glyphs, used for demonstration figures */
      if (eval[0] == eval[1]) {
	ELL_3V_CROSS(_evec+6, _evec+0, _evec+3);
      } else {
	ELL_3V_CROSS(_evec+0, _evec+3, _evec+6);
      }
    }
    if (tenVerbose && _eval[2] < 0) {
      fprintf(stderr, "tenEigensolve -------------\n");
      fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", 
	      t[1], t[2], t[3]);
      fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", 
	      t[2], t[4], t[5]);
      fprintf(stderr, "% 15.7f % 15.7f % 15.7f\n", 
	      t[3], t[5], t[6]);
      fprintf(stderr, " --> % 15.7f % 15.7f % 15.7f\n",
	      _eval[0], _eval[1], _eval[2]);
    }
  } else {
    /* caller only wants eigenvalues */
    ret = ell_3m_eigenvalues_d(eval, m, AIR_TRUE);
    ELL_3V_SET(_eval, eval[0] + trc, eval[1] + trc, eval[2] + trc);
  }    
  return ret;
}

/*  lop A
    fprintf(stderr, "###################################  I = %d\n", (int)I);
    tenEigensolve(teval, tevec, out);
    fprintf(stderr, "evals: (%g %g %g) %g %g %g --> %g %g %g\n", 
	    AIR_ABS(eval[0] - teval[0]) + 1,
	    AIR_ABS(eval[1] - teval[1]) + 1,
	    AIR_ABS(eval[2] - teval[2]) + 1,
	    eval[0], eval[1], eval[2], 
	    teval[0], teval[1], teval[2]);
    fprintf(stderr, "   tevec lens: %g %g %g\n", ELL_3V_LEN(tevec+3*0),
	    ELL_3V_LEN(tevec+3*1), ELL_3V_LEN(tevec+3*2));
    ELL_3V_CROSS(tmp1, evec+3*0, evec+3*1); tmp2[0] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, evec+3*0, evec+3*2); tmp2[1] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, evec+3*1, evec+3*2); tmp2[2] = ELL_3V_LEN(tmp1);
    fprintf(stderr, "   evec[0] = %g %g %g\n", 
	    (evec+3*0)[0], (evec+3*0)[1], (evec+3*0)[2]);
    fprintf(stderr, "   evec[1] = %g %g %g\n",
	    (evec+3*1)[0], (evec+3*1)[1], (evec+3*1)[2]);
    fprintf(stderr, "   evec[2] = %g %g %g\n",
	    (evec+3*2)[0], (evec+3*2)[1], (evec+3*2)[2]);
    fprintf(stderr, "   evec crosses: %g %g %g\n",
	    tmp2[0], tmp2[1], tmp2[2]);
    ELL_3V_CROSS(tmp1, tevec+3*0, tevec+3*1); tmp2[0] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, tevec+3*0, tevec+3*2); tmp2[1] = ELL_3V_LEN(tmp1);
    ELL_3V_CROSS(tmp1, tevec+3*1, tevec+3*2); tmp2[2] = ELL_3V_LEN(tmp1);
    fprintf(stderr, "   tevec[0] = %g %g %g\n", 
	    (tevec+3*0)[0], (tevec+3*0)[1], (tevec+3*0)[2]);
    fprintf(stderr, "   tevec[1] = %g %g %g\n",
	    (tevec+3*1)[0], (tevec+3*1)[1], (tevec+3*1)[2]);
    fprintf(stderr, "   tevec[2] = %g %g %g\n",
	    (tevec+3*2)[0], (tevec+3*2)[1], (tevec+3*2)[2]);
    fprintf(stderr, "   tevec crosses: %g %g %g\n",
	    tmp2[0], tmp2[1], tmp2[2]);
    if (tmp2[1] < 0.5) {
      fprintf(stderr, "(panic)\n");
      exit(0);
    }
*/

void
tenMakeOne(float ten[7], float conf, float eval[3], float evec[9]) {
  double tmpMat1[9], tmpMat2[9], diag[9], evecT[9];

  ELL_3M_ZERO_SET(diag);
  ELL_3M_DIAG_SET(diag, eval[0], eval[1], eval[2]);
  ELL_3M_TRAN(evecT, evec);
  ELL_3M_MUL(tmpMat1, diag, evecT);
  ELL_3M_MUL(tmpMat2, evec, tmpMat1);
  ten[0] = conf;
  TEN_MAT2LIST(ten, tmpMat2);
  return;
}

/*
******** tenMake
**
** create a tensor nrrd from nrrds of confidence, eigenvalues, and
** eigenvectors
*/
int
tenMake(Nrrd *nout, Nrrd *nconf, Nrrd *neval, Nrrd *nevec) {
  char me[]="tenTensorMake", err[AIR_STRLEN_MED];
  int sx, sy, sz;
  size_t I, N;
  float *out, *conf, *eval, *evec;
  int map[4];
  /* float teval[3], tevec[9], tmp1[3], tmp2[3]; */

  if (!(nout && nconf && neval && nevec)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdCheck(nconf) || nrrdCheck(neval) || nrrdCheck(nevec)) {
    sprintf(err, "%s: didn't get three valid nrrds", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 3 == nconf->dim && nrrdTypeFloat == nconf->type )) {
    sprintf(err, "%s: first nrrd not a confidence volume "
	    "(dim = %d, not 3; type = %s, not %s)", me,
	    nconf->dim, airEnumStr(nrrdType, nconf->type),
	    airEnumStr(nrrdType, nrrdTypeFloat));
    biffAdd(TEN, err); return 1;
  }
  sx = nconf->axis[0].size;
  sy = nconf->axis[1].size;
  sz = nconf->axis[2].size;
  if (!( 4 == neval->dim && 4 == nevec->dim &&
	 nrrdTypeFloat == neval->type &&
	 nrrdTypeFloat == nevec->type )) {
    sprintf(err, "%s: second and third nrrd aren't both 4-D (%d and %d) "
	    "and type %s (%s and %s)",
	    me, neval->dim, nevec->dim,
	    airEnumStr(nrrdType, nrrdTypeFloat),
	    airEnumStr(nrrdType, neval->type),
	    airEnumStr(nrrdType, nevec->type));
    biffAdd(TEN, err); return 1;
  }
  if (!( 3 == neval->axis[0].size &&
	 sx == neval->axis[1].size &&
	 sy == neval->axis[2].size &&
	 sz == neval->axis[3].size )) {
    sprintf(err, "%s: second nrrd sizes wrong: (%d,%d,%d,%d) not (3,%d,%d,%d)",
	    me, neval->axis[0].size, neval->axis[1].size,
	    neval->axis[2].size, neval->axis[3].size,
	    sx, sy, sz);
    biffAdd(TEN, err); return 1;
  }
  if (!( 9 == nevec->axis[0].size &&
	 sx == nevec->axis[1].size &&
	 sy == nevec->axis[2].size &&
	 sz == nevec->axis[3].size )) {
    sprintf(err, "%s: third nrrd sizes wrong: (%d,%d,%d,%d) not (9,%d,%d,%d)",
	    me, nevec->axis[0].size, nevec->axis[1].size,
	    nevec->axis[2].size, nevec->axis[3].size,
	    sx, sy, sz);
    biffAdd(TEN, err); return 1;
  }

  /* finally */
  if (nrrdMaybeAlloc(nout, nrrdTypeFloat, 4, 7, sx, sy, sz)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  N = sx*sy*sz;
  conf = (float *)(nconf->data);
  eval = (float *)neval->data;
  evec = (float *)nevec->data;
  out = (float *)nout->data;
  for (I=0; I<N; I++) {
    tenMakeOne(out, conf[I], eval, evec);
    /* lop A */
    out += 7;
    eval += 3;
    evec += 9;
  }
  ELL_4V_SET(map, -1, 0, 1, 2);
  if (nrrdAxisInfoCopy(nout, nconf, map, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  AIR_FREE(nout->axis[0].label);
  nout->axis[0].label = airStrdup("tensor");

  return 0;
}

int
tenSlice(Nrrd *nout, Nrrd *nten, int axis, int pos, int dim) {
  Nrrd *nslice, *ncoeff[4];
  int ci[4];
  char me[]="tenSlice", err[AIR_STRLEN_MED];
  airArray *mop;

  if (!(nout && nten)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nten, nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a valid tensor field", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(2 == dim || 3 == dim)) {
    sprintf(err, "%s: given dim (%d) not 2 or 3", me, dim);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_IN_CL(0, axis, 2))) {
    sprintf(err, "%s: axis %d not in valid range [0,1,2]", me, axis);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_IN_CL(0, pos, nten->axis[1+axis].size-1))) {
    sprintf(err, "%s: slice position %d not in valid range [0..%d]", me,
	    pos, nten->axis[1+axis].size-1);
    biffAdd(TEN, err); return 1;
  }

  /*
  ** threshold        0
  ** Dxx Dxy Dxz      1   2   3
  ** Dxy Dyy Dyz  =  (2)  4   5
  ** Dxz Dyz Dzz     (3) (5)  6 
  */
  mop = airMopNew();
  airMopAdd(mop, nslice=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (3 == dim) {
    if (nrrdSlice(nslice, nten, axis+1, pos)
	|| nrrdAxesInsert(nout, nslice, axis+1)) {
      sprintf(err, "%s: trouble making slice", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  } else {
    airMopAdd(mop, ncoeff[0]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, ncoeff[1]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, ncoeff[2]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, ncoeff[3]=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    switch(axis) {
    case 0:
      ELL_4V_SET(ci, 0, 4, 5, 6);
      break;
    case 1:
      ELL_4V_SET(ci, 0, 1, 3, 6);
      break;
    case 2:
      ELL_4V_SET(ci, 0, 1, 2, 4);
      break;
    default:
      sprintf(err, "%s: axis %d bogus", me, axis);
      biffAdd(TEN, err); airMopError(mop); return 1;
      break;
    }
    if (nrrdSlice(nslice, nten, axis+1, pos)
	|| nrrdSlice(ncoeff[0], nslice, 0, ci[0])
	|| nrrdSlice(ncoeff[1], nslice, 0, ci[1])
	|| nrrdSlice(ncoeff[2], nslice, 0, ci[2])
	|| nrrdSlice(ncoeff[3], nslice, 0, ci[3])
	|| nrrdJoin(nout, (const Nrrd **)ncoeff, 4, 0, AIR_TRUE)) {
      sprintf(err, "%s: trouble collecting coefficients", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}


