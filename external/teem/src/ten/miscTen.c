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
tenEvecRGB(Nrrd *nout, Nrrd *nin, int which, int aniso,
	   float gamma, float bgGray, float isoGray) {
  char me[]="tenEvecRGB", err[AIR_STRLEN_MED];
  int size[NRRD_DIM_MAX];
  float *tdata, *cdata, eval[3], evec[9], R, G, B, an[TEN_ANISO_MAX+1], conf;
  size_t II, NN;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_IN_CL(0, which, 2))) {
    sprintf(err, "%s: eigenvector index %d not in range [0..2]", me, which);
    biffAdd(TEN, err); return 1;
  }
  if (airEnumValCheck(tenAniso, aniso)) {
    sprintf(err, "%s: anisotropy metric %d not valid", me, aniso);
    biffAdd(TEN, err); return 1;
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_FALSE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a valid DT volume", me);
    biffAdd(TEN, err); return 1;
  }

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, size);
  size[0] = 3;
  if (nrrdMaybeAlloc_nva(nout, nrrdTypeFloat, nin->dim, size)) {
    sprintf(err, "%s: couldn't alloc output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  NN = nrrdElementNumber(nin)/7;
  cdata = nout->data;
  tdata = nin->data;
  for (II=0; II<NN; II++) {
    /* tenVerbose = (II == (50 + 64*(32 + 64*0))); */
    tenEigensolve(eval, evec, tdata);
    tenAnisoCalc(an, eval);
    R = AIR_ABS(evec[0 + 3*which]);
    G = AIR_ABS(evec[1 + 3*which]);
    B = AIR_ABS(evec[2 + 3*which]);
    /*
    if (tenVerbose) {
      fprintf(stderr, "!%s: --> RGB = %g %g %g\n", me, R, G, B);
    }
    */
    R = pow(R, 1.0/gamma);
    G = pow(G, 1.0/gamma);
    B = pow(B, 1.0/gamma);
    R = AIR_LERP(an[aniso], isoGray, R);
    G = AIR_LERP(an[aniso], isoGray, G);
    B = AIR_LERP(an[aniso], isoGray, B);
    conf = AIR_CLAMP(0, tdata[0], 1);
    R = AIR_LERP(conf, bgGray, R);
    G = AIR_LERP(conf, bgGray, G);
    B = AIR_LERP(conf, bgGray, B);
    ELL_3V_SET(cdata, R, G, B);
    cdata += 3;
    tdata += 7;
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: couldn't copy axis info", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  
  return 0;
}

#define SQR(i) ((i)*(i))

short
tenEvqOne(float vec[3], float scl) {
  char me[]="tenEvqOne";
  float tmp, L1;
  int mi, bins, base, vi, ui;
  short ret;

  ELL_3V_NORM(vec, vec, tmp);
  L1 = AIR_ABS(vec[0]) + AIR_ABS(vec[1]) + AIR_ABS(vec[2]);
  ELL_3V_SCALE(vec, 1/L1, vec);
  scl = AIR_CLAMP(0.0, scl, 1.0);
  scl = pow(scl, 0.75);
  AIR_INDEX(0.0, scl, 1.0, 6, mi);
  if (mi) {
    switch (mi) {
    case 1: bins = 16; base = 1;                                 break;
    case 2: bins = 32; base = 1+SQR(16);                         break;
    case 3: bins = 48; base = 1+SQR(16)+SQR(32);                 break;
    case 4: bins = 64; base = 1+SQR(16)+SQR(32)+SQR(48);         break;
    case 5: bins = 80; base = 1+SQR(16)+SQR(32)+SQR(48)+SQR(64); break;
    default:
      fprintf(stderr, "%s: PANIC: mi = %d\n", me, mi);
      exit(0);
    }
    AIR_INDEX(-1, vec[0]+vec[1], 1, bins, vi);
    AIR_INDEX(-1, vec[0]-vec[1], 1, bins, ui);
    ret = vi*bins + ui + base;
  }
  else {
    ret = 0;
  }
  return ret;
}

int
tenEvqVolume(Nrrd *nout, Nrrd *nin, int which, int aniso, int scaleByAniso) {
  char me[]="tenEvqVolume", err[AIR_STRLEN_MED];
  int sx, sy, sz, map[3];
  short *qdata;
  float *tdata, eval[3], evec[9], c[TEN_ANISO_MAX+1], an;
  size_t N, I;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!(AIR_IN_CL(0, which, 2))) {
    sprintf(err, "%s: eigenvector index %d not in range [0..2]", me, which);
    biffAdd(TEN, err); return 1;
  }
  if (scaleByAniso) {
    if (airEnumValCheck(tenAniso, aniso)) {
      sprintf(err, "%s: anisotropy metric %d not valid", me, aniso);
      biffAdd(TEN, err); return 1;
    }
  }
  if (tenTensorCheck(nin, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: didn't get a valid DT volume", me);
    biffAdd(TEN, err); return 1;
  }
  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  if (nrrdMaybeAlloc(nout, nrrdTypeShort, 3, sx, sy, sz)) {
    sprintf(err, "%s: can't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  N = sx*sy*sz;
  tdata = nin->data;
  qdata = nout->data;
  for (I=0; I<N; I++) {
    tenEigensolve(eval, evec, tdata);
    if (scaleByAniso) {
      tenAnisoCalc(c, eval);
      an = c[aniso];
    } else {
      an = 1.0;
    }
    qdata[I] = tenEvqOne(evec+ 3*which, an);
    tdata += 7;
  }
  ELL_3V_SET(map, 1, 2, 3);
  if (nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_SIZE_BIT)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  
  return 0;
}

int
tenGradCheck(Nrrd *ngrad) {
  char me[]="tenGradCheck", err[AIR_STRLEN_MED];

  if (nrrdCheck(ngrad)) {
    sprintf(err, "%s: basic validity check failed", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 3 == ngrad->axis[0].size && 2 == ngrad->dim )) {
    sprintf(err, "%s: need a 3xN 2-D array (not a %dxN %d-D array)",
	    me, ngrad->axis[0].size, ngrad->dim);
    biffAdd(TEN, err); return 1;
  }
  if (!( 6 <= ngrad->axis[1].size )) {
    sprintf(err, "%s: have only %d gradients, need at least 6",
	    me, ngrad->axis[1].size);
    biffAdd(TEN, err); return 1;
  }

  return 0;
}

int
tenBMatrixCheck(Nrrd *nbmat) {
  char me[]="tenBMatrixCheck", err[AIR_STRLEN_MED];

  if (nrrdCheck(nbmat)) {
    sprintf(err, "%s: basic validity check failed", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (!( 6 == nbmat->axis[0].size && 2 == nbmat->dim )) {
    sprintf(err, "%s: need a 6xN 2-D array (not a %dxN %d-D array)",
	    me, nbmat->axis[0].size, nbmat->dim);
    biffAdd(TEN, err); return 1;
  }
  if (!( 6 <= nbmat->axis[1].size )) {
    sprintf(err, "%s: have only %d rows, need at least 6",
	    me, nbmat->axis[1].size);
    biffAdd(TEN, err); return 1;
  }

  return 0;
}

/*
******** _tenFindValley
**
** This is not a general purpose function, and it will take some
** work to make it that way.
**
** the tweak argument implements a cheesy heuristic: threshold should be
** on low side of histogram valley, since stdev for background is much
** narrower then stdev for brain
*/
int
_tenFindValley(float *valP, Nrrd *nhist, float tweak) {
  char me[]="_tenFindValley", err[AIR_STRLEN_MED];
  double gparm[NRRD_KERNEL_PARMS_NUM], dparm[NRRD_KERNEL_PARMS_NUM];
  Nrrd *ntmpA, *ntmpB, *nhistD, *nhistDD;
  float *hist, *histD, *histDD;
  airArray *mop;
  int maxbb, bb, bins;
  NrrdRange *range;

  /*
  tenEMBimodalParm *biparm;
  biparm = tenEMBimodalParmNew();
  tenEMBimodal(biparm, nhist);
  biparm = tenEMBimodalParmNix(biparm);
  */
  
  mop = airMopNew();
  airMopAdd(mop, ntmpA=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, ntmpB=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nhistD=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  airMopAdd(mop, nhistDD=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);

  bins = nhist->axis[0].size;
  gparm[0] = bins/50;  /* wacky heuristic for gaussian stdev */
  gparm[1] = 3;        /* how many stdevs to cut-off at */
  dparm[0] = 1.0;      /* unit spacing */
  dparm[1] = 1.0;      /* B-Spline kernel */
  dparm[2] = 0.0;
  if (nrrdCheapMedian(ntmpA, nhist, AIR_FALSE, 2, 1.0, 1024)
      || nrrdSimpleResample(ntmpB, ntmpA,
			    nrrdKernelGaussian, gparm, &bins, NULL)
      || nrrdSimpleResample(nhistD, ntmpB,
			    nrrdKernelBCCubicD, dparm, &bins, NULL)
      || nrrdSimpleResample(nhistDD, ntmpB,
			    nrrdKernelBCCubicDD, dparm, &bins, NULL)) {
    sprintf(err, "%s: trouble processing histogram", me);
    biffMove(TEN, err, NRRD), airMopError(mop); return 1;
  }
  hist = (float*)(ntmpB->data);
  histD = (float*)(nhistD->data);
  histDD = (float*)(nhistDD->data);
  range = nrrdRangeNewSet(ntmpB, nrrdBlind8BitRangeState);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  for (bb=0; bb<bins-1; bb++) {
    if (hist[bb] == range->max) {
      /* first seek to max in histogram */
      break;
    }
  }
  maxbb = bb;
  for (; bb<bins-1; bb++) {
    if (histD[bb]*histD[bb+1] < 0 && histDD[bb] > 0) {
      /* zero-crossing in 1st deriv, positive 2nd deriv */
      break;
    }
  }
  if (bb == bins-1) {
    sprintf(err, "%s: never saw a satisfactory zero crossing", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }

  *valP = nrrdAxisPos(nhist, 0, AIR_AFFINE(0, tweak, 1, maxbb, bb));

  airMopOkay(mop);
  return 0;
}
