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

/*
******** tenBMatrixCalc
**
** given a list of gradient directions (arbitrary type), contructs the
** B-matrix that records how each coefficient of the diffusion tensor
** is weighted in the diffusion weighted images.  Matrix will be a
** 6-by-N 2D array of doubles.
**
** NOTE 1: The ordering of the elements in each row is (like the ordering
** of the tensor elements in all of ten):
**
**    Bxx  Bxy  Bxz   Byy  Byz   Bzz
** 
** NOTE 2: The off-diagonal elements are NOT pre-multiplied by two.
*/
int
tenBMatrixCalc(Nrrd *nbmat, Nrrd *_ngrad) {
  char me[]="tenBMatrixCalc", err[AIR_STRLEN_MED];
  Nrrd *ngrad;
  double *bmat, *G;
  int DD, dd;
  airArray *mop;

  if (!(nbmat && _ngrad && !tenGradCheck(_ngrad))) {
    sprintf(err, "%s: got NULL pointer or invalid arg", me);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, ngrad=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(ngrad, _ngrad, nrrdTypeDouble)
      || nrrdMaybeAlloc(nbmat, nrrdTypeDouble, 2, 6, ngrad->axis[1].size)) {
    sprintf(err, "%s: trouble", me);
    biffMove(TEN, err, NRRD); return 1;
  }

  DD = ngrad->axis[1].size;
  bmat = (double*)(nbmat->data);
  for (dd=0; dd<DD; dd++) {
    G = (double*)(ngrad->data) + 3*dd;
    ELL_6V_SET(bmat,
	       G[0]*G[0], G[0]*G[1], G[0]*G[2],
	       G[1]*G[1], G[1]*G[2],
	       G[2]*G[2]);
    bmat += 6;
  }
  
  return 0;
}

/*
******** tenEMatrixCalc
**
*/
int
tenEMatrixCalc(Nrrd *nemat, Nrrd *_nbmat, int knownB0) {
  char me[]="tenEMatrixCalc", err[AIR_STRLEN_MED];
  Nrrd *nbmat, *ntmp;
  airArray *mop;
  int ri, padmin[2], padmax[2];
  double *bmat;
  
  if (!(nemat && _nbmat)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenBMatrixCheck(_nbmat)) {
    sprintf(err, "%s: problem with B matrix", me);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, nbmat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (knownB0) {
    if (nrrdConvert(nbmat, _nbmat, nrrdTypeDouble)) {
      sprintf(err, "%s: couldn't convert given bmat to doubles", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  } else {
    airMopAdd(mop, ntmp=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    if (nrrdConvert(ntmp, _nbmat, nrrdTypeDouble)) {
      sprintf(err, "%s: couldn't convert given bmat to doubles", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    ELL_2V_SET(padmin, 0, 0);
    ELL_2V_SET(padmax, 6, _nbmat->axis[1].size-1);
    if (nrrdPad_nva(nbmat, ntmp, padmin, padmax, nrrdBoundaryPad, -1)) {
      sprintf(err, "%s: couldn't pad given bmat", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
  }
  bmat = (double*)(nbmat->data);
  /* HERE is where the off-diagonal elements get multiplied by 2 */
  for (ri=0; ri<nbmat->axis[1].size; ri++) {
    bmat[1] *= 2;
    bmat[2] *= 2;
    bmat[4] *= 2;
    bmat += nbmat->axis[0].size;
  }
  if (ell_Nm_pseudo_inv(nemat, nbmat)) {
    sprintf(err, "%s: trouble pseudo-inverting B-matrix", me);
    biffMove(TEN, err, ELL); airMopError(mop); return 1;
  }
  airMopOkay(mop);
  return 0;
}

#define _TEN_MAX_DWI_NUM 128

/*
******** tenEstimateSingle
**
** estimate one single tensor
**
** -------------- IF knownB0 -------------------------
** input:
** dwi[0] is the B0 image, dwi[1]..dwi[DD-1] are the (DD-1) DWI values,
** emat is the (DD-1)-by-6 estimation matrix, which is the pseudo-inverse
** of the B-matrix (after the off-diagonals have been multiplied by 2)
**
** output:
** ten[0]..ten[6] will be the confidence value followed by the tensor
** -------------- !IF knownB0 -------------------------
** input:
** dwi[0]..dwi[DD-1] are the DD DWI values,
** emat is the DD-by-7 estimation matrix.  The 7th row is for estimating
** the B0 image
**
** output:
** ten[0]..ten[6] will be the confidence value followed by the tensor
** if B0P, then *B0P is set to estimated B0 value.
** ----------------------------------------------------
*/
void
tenEstimateLinearSingle(float *ten, float *B0P, float *dwi, double *emat,
			int DD, int knownB0,
			float thresh, float soft, float b) {
  double logB0, v[_TEN_MAX_DWI_NUM], tmp, mean;
  int i, j;
  
  if (knownB0) {
    logB0 = log(AIR_MAX(dwi[0], 1));
    mean = 0;
    for (i=1; i<DD; i++) {
      tmp = AIR_MAX(dwi[i], 1);
      mean += tmp;
      v[i-1] = (logB0 - log(tmp))/b;
      if (tenVerbose)
	fprintf(stderr, "v[%d] = %f\n", i-1, v[i-1]);
    }
    mean /= DD-1;
    ten[0] = AIR_AFFINE(-1, airErf((mean - thresh)/(soft + 0.000001)), 1,
			0, 1);
    for (j=0; j<6; j++) {
      tmp = 0;
      for (i=0; i<DD-1; i++) {
	tmp += emat[i + (DD-1)*j]*v[i];
      }
      ten[j+1] = tmp;
    }
  } else {
    mean = 0;
    for (i=0; i<DD; i++) {
      tmp = AIR_MAX(dwi[i], 1);
      mean += tmp;
      v[i] = -log(tmp)/b;
    }
    mean /= DD;
    ten[0] = AIR_AFFINE(-1, airErf((mean - thresh)/(soft + 0.000001)), 1,
			0, 1);
    for (j=0; j<=6; j++) {
      tmp = 0;
      for (i=0; i<DD; i++) {
	tmp += emat[i + DD*j]*v[i];
      }
      if (j < 6) {
	ten[j+1] = tmp;
      } else {
	if (B0P) {
	  *B0P = tmp;
	}
      }
    }
  }
  return;
}

/*
******** tenEstimateLinear3D
**
** takes an array of DWIs (starting with the B=0 image), joins them up,
** and passes it all off to tenEstimateLinear4D
**
** Note: this will copy per-axis peripheral information from _ndwi[0]
*/
int
tenEstimateLinear3D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
		    Nrrd **_ndwi, int dwiLen, 
		    Nrrd *_nbmat, int knownB0, 
		    float thresh, float soft, float b) {
  char me[]="tenEstimateLinear3D", err[AIR_STRLEN_MED];
  Nrrd *ndwi;
  airArray *mop;
  int amap[4] = {-1, 0, 1, 2};

  if (!(_ndwi)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  ndwi = nrrdNew();
  airMopAdd(mop, ndwi, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdJoin(ndwi, (const Nrrd**)_ndwi, dwiLen, 0, AIR_TRUE)) {
    sprintf(err, "%s: trouble joining inputs", me);
    biffMove(TEN, err, NRRD); airMopError(mop); return 1;
  }

  nrrdAxisInfoCopy(ndwi, _ndwi[0], amap, NRRD_AXIS_INFO_NONE);
  if (tenEstimateLinear4D(nten, nterrP, nB0P, 
			  ndwi, _nbmat, knownB0, thresh, soft, b)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(TEN, err); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

/*
******** tenEstimateLinear4D
**
** given a stack of DWI volumes (ndwi) and the imaging B-matrix used 
** for acquisiton (_nbmat), computes and stores diffusion tensors in
** nten.
**
** The mean of the diffusion-weighted images is thresholded at "thresh" with
** softness parameter "soft".
**
** This takes the B-matrix (weighting matrix), such as formed by tenBMatrix,
** or from a more complete account of the gradients present in an imaging
** sequence, and then does the pseudo inverse to get the estimation matrix
*/
int
tenEstimateLinear4D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
		    Nrrd *ndwi, Nrrd *_nbmat, int knownB0,
		    float thresh, float soft, float b) {
  char me[]="tenEstimateLinear4D", err[AIR_STRLEN_MED];
  Nrrd *nemat, *ncrop, *nhist;
  airArray *mop;
  int E, DD, d, II, sx, sy, sz, cmin[4], cmax[4], amap[4] = {-1,1,2,3};
  float *ten, dwi1[_TEN_MAX_DWI_NUM], *terr=NULL, 
    *B0=NULL, (*lup)(const void *, size_t);
  double *emat;
  NrrdRange *range;

  /*
  HEY: uncomment with tenSimulate stuff is working for unknown B0 case
  Nrrd *nbmat;
  double *bmat;
  const char *bk;
  float dwi2[_TEN_MAX_DWI_NUM], te, d1, d2;
  */

  if (!(nten && ndwi && _nbmat)) {
    /* nerrP can be NULL */
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!knownB0 && !nB0P) {
    /* For the time being, this needless restriction helps simplify things */
    sprintf(err, "%s: sorry, don't know (and hence will estimate) B=0 image, "
	    "but didn't get a Nrrd* to put it in", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( 4 == ndwi->dim && 7 <= ndwi->axis[0].size )) {
    sprintf(err, "%s: dwi should be 4-D array with axis 0 size >= 7", me);
    biffAdd(TEN, err); return 1;
  }
  if (tenBMatrixCheck(_nbmat)) {
    sprintf(err, "%s: problem with B matrix", me);
    biffAdd(TEN, err); return 1;
  }
  if (knownB0) {
    if (!( ndwi->axis[0].size == 1 + _nbmat->axis[1].size )) {
      sprintf(err, "%s: (knownB0 == true) # input images (%d) "
	      "!= 1 + # B matrix rows (1+%d)",
	      me, ndwi->axis[0].size, _nbmat->axis[1].size);
      biffAdd(TEN, err); return 1;
    }
  } else {
    if (!( ndwi->axis[0].size == _nbmat->axis[1].size )) {
      sprintf(err, "%s: (knownB0 == false) # dwi (%d) != "
	      "# B matrix rows (%d)",
	      me, ndwi->axis[0].size, _nbmat->axis[1].size);
      biffAdd(TEN, err); return 1;
    }
  }
  
  mop = airMopNew();
  /* HEY, right now, nbmat is not used for anything (not tenSimulate stuff)
     because tenSimulate currently can't handle the unknown B0 case */
  /* airMopAdd(mop, nbmat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways); */
  airMopAdd(mop, nemat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (tenEMatrixCalc(nemat, _nbmat, knownB0)) {
    sprintf(err, "%s: trouble computing estimation matrix", me);
    biffAdd(TEN, err); return 1;
  }
  
  DD = ndwi->axis[0].size;
  sx = ndwi->axis[1].size;
  sy = ndwi->axis[2].size;
  sz = ndwi->axis[3].size;
  if (!AIR_EXISTS(thresh)) {
    airMopAdd(mop, ncrop=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    airMopAdd(mop, nhist=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
    ELL_4V_SET(cmin, knownB0 ? 1 : 0, 0, 0, 0);
    ELL_4V_SET(cmax, DD-1, sx-1, sy-1, sz-1);
    E = 0;
    if (!E) E |= nrrdCrop(ncrop, ndwi, cmin, cmax);
    if (!E) range = nrrdRangeNewSet(ncrop, nrrdBlind8BitRangeState);
    if (!E) airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
    if (!E) E |= nrrdHisto(nhist, ncrop, range, NULL,
			   (int)AIR_MIN(1024, range->max - range->min + 1),
			   nrrdTypeFloat);
    if (E) {
      sprintf(err, "%s: trouble histograming to find DW threshold", me);
      biffMove(TEN, err, NRRD); airMopError(mop); return 1;
    }
    if (_tenFindValley(&thresh, nhist, 0.85)) {
      sprintf(err, "%s: problem finding DW histogram valley", me);
      biffAdd(TEN, err); airMopError(mop); return 1;
    }
    fprintf(stderr, "%s: using %g for DW confidence threshold\n", me, thresh);
  }
  if (nrrdMaybeAlloc(nten, nrrdTypeFloat, 4, 7, sx, sy, sz)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  if (nterrP) {

    /* HEY HEY HEY HEY */
    sprintf(err, "%s: sorry, this functionality currently disabled", me);
    biffAdd(TEN, err); return 1;
    /* HEY HEY HEY HEY */

    if (!(*nterrP)) {
      *nterrP = nrrdNew();
    }
    if (nrrdMaybeAlloc(*nterrP, nrrdTypeFloat, 3, sx, sy, sz)) {
      sprintf(err, "%s: couldn't allocate error output", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    airMopAdd(mop, nterrP, (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, *nterrP, (airMopper)nrrdNuke, airMopOnError);
    terr = (float*)((*nterrP)->data);
  }
  if (nB0P) {
    if (!(*nB0P)) {
      *nB0P = nrrdNew();
    }
    if (nrrdMaybeAlloc(*nB0P, nrrdTypeFloat, 3, sx, sy, sz)) {
      sprintf(err, "%s: couldn't allocate error output", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
    airMopAdd(mop, nB0P, (airMopper)airSetNull, airMopOnError);
    airMopAdd(mop, *nB0P, (airMopper)nrrdNuke, airMopOnError);
    B0 = (float*)((*nB0P)->data);
  }
  /* bmat = (double*)(nbmat->data); */
  emat = (double*)(nemat->data);
  ten = (float*)(nten->data);
  lup = nrrdFLookup[ndwi->type];
  for (II=0; II<sx*sy*sz; II++) {
    /* tenVerbose = (II == 42 + 190*(96 + 196*0)); */
    for (d=0; d<DD; d++) {
      dwi1[d] = lup(ndwi->data, d + DD*II);
      if (tenVerbose)
	fprintf(stderr, "%s: input dwi1[%d] = %g\n", me, d, dwi1[d]);
    }
    tenEstimateLinearSingle(ten, B0, dwi1, emat, DD, knownB0, thresh, soft, b);
    if (tenVerbose) 
      fprintf(stderr, "%s: output ten = (%g) %g,%g,%g  %g,%g  %g\n", me,
	      ten[0], ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);
    if (nterrP) {
      /*
      tenSimulateOne(dwi2, dwi1[0], ten, bmat, DD, b);
      te = 0;
      for (d=0; d<DD; d++) {
	d1 = AIR_MAX(dwi1[d], 1);
	d2 = AIR_MAX(dwi2[d], 1);
	te += (d1 - d2)*(d1 - d2);
	if (tenVerbose)
	  fprintf(stderr, "%s: dwi1,2[%d] = %g,%g --> %g\n",
		  me, d, dwi1[d], dwi2[d], te);
      }
      *terr = sqrt(te);
      */
      terr += 1;
    }
    ten += 7;
    if (B0) {
      B0 += 1;
    }
  }
  /* tenEigenvalueClamp(nten, nten, 0, AIR_NAN); */

  nrrdAxisInfoCopy(nten, ndwi, amap, NRRD_AXIS_INFO_NONE);

  airMopOkay(mop);
  return 0;
}

/*
******** tenSimulateOne
**
** simulate a diffusion weighted measurement
**
*/
void
tenSimulateOne(float *dwi, float B0, float *ten,
	       double *bmat, int DD, float b) {
  double v[_TEN_MAX_DWI_NUM];
  int i, j;
  
  dwi[0] = B0;
  if (tenVerbose) {
    fprintf(stderr, "ten = %g,%g,%g  %g,%g  %g\n", 
	    ten[1], ten[2], ten[3], ten[4], ten[5], ten[6]);
  }
  for (i=0; i<DD-1; i++) {
    v[i] = 0;
    for (j=0; j<6; j++) {
      v[i] += bmat[j + 6*i]*ten[j+1];
    }
    dwi[i+1] = AIR_MAX(B0, 1)*exp(-b*v[i]);
    if (tenVerbose) {
      fprintf(stderr, "v[%d] = %g --> dwi = %g\n", i, v[i], dwi[i+1]);
    }
  }
  
  return;
}

int
tenSimulate(Nrrd *ndwi, Nrrd *nT2, Nrrd *nten, Nrrd *_nbmat, float b) {
  char me[]="tenSimulate", err[AIR_STRLEN_MED];
  size_t II;
  Nrrd *nbmat;
  int DD, sx, sy, sz;
  airArray *mop;
  double *bmat;
  float *dwi, *ten, (*lup)(const void *, size_t I);
  
  if (!ndwi || !nT2 || !nten || !_nbmat
      || tenTensorCheck(nten, nrrdTypeFloat, AIR_TRUE, AIR_TRUE)
      || tenBMatrixCheck(_nbmat)) {
    sprintf(err, "%s: got NULL pointer or invalid args", me);
    biffAdd(TEN, err); return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, nbmat=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nbmat, _nbmat, nrrdTypeDouble)) {
    sprintf(err, "%s: couldn't convert B matrix", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  
  DD = nbmat->axis[1].size+1;
  sx = nT2->axis[0].size;
  sy = nT2->axis[1].size;
  sz = nT2->axis[2].size;
  if (!(3 == nT2->dim
	&& sx == nten->axis[1].size
	&& sy == nten->axis[2].size
	&& sz == nten->axis[3].size)) {
    sprintf(err, "%s: dimensions of T2 volume (%d,%d,%d) don't match "
	    "tensor volume (%d,%d,%d)", me, sx, sy, sz, nten->axis[1].size,
	    nten->axis[2].size, nten->axis[3].size);
    biffAdd(TEN, err); return 1;
  }
  if (nrrdMaybeAlloc(ndwi, nrrdTypeFloat, 4, DD, sx, sy, sz)) {
    sprintf(err, "%s: couldn't allocate output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  dwi = (float*)(ndwi->data);
  ten = (float*)(nten->data);
  bmat = (double*)(nbmat->data);
  lup = nrrdFLookup[nT2->type];
  for (II=0; II<sx*sy*sz; II++) {
    /* tenVerbose = (II == 42 + 190*(96 + 196*0)); */
    tenSimulateOne(dwi, lup(nT2->data, II), ten, bmat, DD, b);
    dwi += DD;
    ten += 7;
  }

  airMopOkay(mop);
  return 0;
}


















/* old stuff, prior to tenEstimationMatrix ... */


/*
******** tenCalcOneTensor1
**
** make one diffusion tensor from the measurements at one voxel, based
** on the gradient directions used by Andy Alexander
*/
void
tenCalcOneTensor1(float tens[7], float chan[7], 
		  float thresh, float slope, float b) {
  double c[7], sum, d1, d2, d3, d4, d5, d6;
  
  c[0] = AIR_MAX(chan[0], 1);
  c[1] = AIR_MAX(chan[1], 1);
  c[2] = AIR_MAX(chan[2], 1);
  c[3] = AIR_MAX(chan[3], 1);
  c[4] = AIR_MAX(chan[4], 1);
  c[5] = AIR_MAX(chan[5], 1);
  c[6] = AIR_MAX(chan[6], 1);
  sum = c[1] + c[2] + c[3] + c[4] + c[5] + c[6];
  tens[0] = (1 + airErf(slope*(sum - thresh)))/2.0;
  d1 = (log(c[0]) - log(c[1]))/b;
  d2 = (log(c[0]) - log(c[2]))/b;
  d3 = (log(c[0]) - log(c[3]))/b;
  d4 = (log(c[0]) - log(c[4]))/b;
  d5 = (log(c[0]) - log(c[5]))/b;
  d6 = (log(c[0]) - log(c[6]))/b;
  tens[1] =  d1 + d2 - d3 - d4 + d5 + d6;    /* Dxx */
  tens[2] =  d5 - d6;                        /* Dxy */
  tens[3] =  d1 - d2;                        /* Dxz */
  tens[4] = -d1 - d2 + d3 + d4 + d5 + d6;    /* Dyy */
  tens[5] =  d3 - d4;                        /* Dyz */
  tens[6] =  d1 + d2 + d3 + d4 - d5 - d6;    /* Dzz */
  return;
}

/*
******** tenCalcOneTensor2
**
** using gradient directions used by EK
*/
void
tenCalcOneTensor2(float tens[7], float chan[7], 
		  float thresh, float slope, float b) {
  double c[7], sum, d1, d2, d3, d4, d5, d6;
  
  c[0] = AIR_MAX(chan[0], 1);
  c[1] = AIR_MAX(chan[1], 1);
  c[2] = AIR_MAX(chan[2], 1);
  c[3] = AIR_MAX(chan[3], 1);
  c[4] = AIR_MAX(chan[4], 1);
  c[5] = AIR_MAX(chan[5], 1);
  c[6] = AIR_MAX(chan[6], 1);
  sum = c[1] + c[2] + c[3] + c[4] + c[5] + c[6];
  tens[0] = (1 + airErf(slope*(sum - thresh)))/2.0;
  d1 = (log(c[0]) - log(c[1]))/b;
  d2 = (log(c[0]) - log(c[2]))/b;
  d3 = (log(c[0]) - log(c[3]))/b;
  d4 = (log(c[0]) - log(c[4]))/b;
  d5 = (log(c[0]) - log(c[5]))/b;
  d6 = (log(c[0]) - log(c[6]))/b;
  tens[1] =  d1;                 /* Dxx */
  tens[2] =  d6 - (d1 + d2)/2;   /* Dxy */
  tens[3] =  d5 - (d1 + d3)/2;   /* Dxz */
  tens[4] =  d2;                 /* Dyy */
  tens[5] =  d4 - (d2 + d3)/2;   /* Dyz */
  tens[6] =  d3;                 /* Dzz */
  return;
}

/*
******** tenCalcTensor
**
** Calculate a volume of tensors from measured data
*/
int
tenCalcTensor(Nrrd *nout, Nrrd *nin, int version,
	      float thresh, float slope, float b) {
  char me[] = "tenCalcTensor", err[128], cmt[128];
  float *out, tens[7], chan[7];
  int sx, sy, sz;
  size_t I;
  void (*calcten)(float tens[7], float chan[7], 
		  float thresh, float slope, float b);
  
  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (!( 1 == version || 2 == version )) {
    sprintf(err, "%s: version should be 1 or 2, not %d", me, version);
    biffAdd(TEN, err); return 1;
  }
  switch (version) {
  case 1:
    calcten = tenCalcOneTensor1;
    break;
  case 2:
    calcten = tenCalcOneTensor2;
    break;
  default:
    sprintf(err, "%s: PANIC, version = %d not handled", me, version);
    biffAdd(TEN, err); return 1;
    break;
  }
  if (tenTensorCheck(nin, nrrdTypeDefault, AIR_TRUE, AIR_TRUE)) {
    sprintf(err, "%s: wasn't given valid tensor nrrd", me);
    biffAdd(TEN, err); return 1;
  }
  sx = nin->axis[1].size;
  sy = nin->axis[2].size;
  sz = nin->axis[3].size;
  if (nrrdMaybeAlloc(nout, nrrdTypeFloat, 4, 7, sx, sy, sz)) {
    sprintf(err, "%s: couldn't alloc output", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  nout->axis[0].label = airStrdup("c,Dxx,Dxy,Dxz,Dyy,Dyz,Dzz");
  nout->axis[1].label = airStrdup("x");
  nout->axis[2].label = airStrdup("y");
  nout->axis[3].label = airStrdup("z");
  nout->axis[0].spacing = AIR_NAN;
  if (AIR_EXISTS(nin->axis[1].spacing) && 
      AIR_EXISTS(nin->axis[2].spacing) &&
      AIR_EXISTS(nin->axis[3].spacing)) {
    nout->axis[1].spacing = nin->axis[1].spacing;
    nout->axis[2].spacing = nin->axis[2].spacing;
    nout->axis[3].spacing = nin->axis[3].spacing;
  }
  else {
    nout->axis[1].spacing = 1.0;
    nout->axis[2].spacing = 1.0;
    nout->axis[3].spacing = 1.0;
  }    
  sprintf(cmt, "%s: using thresh = %g, slope = %g, b = %g\n", 
	  me, thresh, slope, b);
  nrrdCommentAdd(nout, cmt);
  out = nout->data;
  for (I=0; I<=sx*sy*sz-1; I++) {
    if (tenVerbose && !(I % (sx*sy)))
      fprintf(stderr, "%s: z = %d of %d\n", me, ((int)I)/(sx*sy), sz-1);
    chan[0] = nrrdFLookup[nin->type](nin->data, 0 + 7*I);
    chan[1] = nrrdFLookup[nin->type](nin->data, 1 + 7*I);
    chan[2] = nrrdFLookup[nin->type](nin->data, 2 + 7*I);
    chan[3] = nrrdFLookup[nin->type](nin->data, 3 + 7*I);
    chan[4] = nrrdFLookup[nin->type](nin->data, 4 + 7*I);
    chan[5] = nrrdFLookup[nin->type](nin->data, 5 + 7*I);
    chan[6] = nrrdFLookup[nin->type](nin->data, 6 + 7*I);
    calcten(tens, chan, thresh, slope, b);
    out[0 + 7*I] = tens[0];
    out[1 + 7*I] = tens[1];
    out[2 + 7*I] = tens[2];
    out[3 + 7*I] = tens[3];
    out[4 + 7*I] = tens[4];
    out[5 + 7*I] = tens[5];
    out[6 + 7*I] = tens[6];
  }
  return 0;
}

