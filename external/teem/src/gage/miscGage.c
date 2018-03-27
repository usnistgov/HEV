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

char
gageErrStr[AIR_STRLEN_LARGE]="";

int
gageErrNum=-1;

/*
******** gageZeroNormal[]
**
** this is the vector to supply when someone wants the normalized
** version of a vector with zero length.  We could be nasty and
** set this to {AIR_NAN, AIR_NAN, AIR_NAN}, but simply passing
** NANs around can make things fantastically slow ...
*/
gage_t
gageZeroNormal[3] = {1,0,0};

char
_gageKernelStr[][AIR_STRLEN_SMALL] = {
  "(unknown_kernel)",
  "00",
  "10",
  "11",
  "20",
  "21",
  "22"
};

char
_gageKernelDesc[][AIR_STRLEN_MED] = {
  "unknown kernel",
  "kernel for reconstructing values",
  "kernel for reconstruction values when doing 1st derivatives",
  "kernel for measuring 1st derivatives when doing 1st derivatives",
  "kernel for reconstruction values when doing 2nd derivatives",
  "kernel for measuring 1st derivatives when doing 2nd derivatives",
  "kernel for measuring 2nd derivatives when doing 2nd derivatives"
};

int
_gageKernelVal[] = {
  gageKernelUnknown,
  gageKernel00,
  gageKernel10,
  gageKernel11,
  gageKernel20,
  gageKernel21,
  gageKernel22
};

char
_gageKernelStrEqv[][AIR_STRLEN_SMALL] = {
  "00", "k00",
  "10", "k10",
  "11", "k11",
  "20", "k20",
  "21", "k21",
  "22", "k22",
  ""
};

int
_gageKernelValEqv[] = {
  gageKernel00, gageKernel00,
  gageKernel10, gageKernel10,
  gageKernel11, gageKernel11,
  gageKernel20, gageKernel20,
  gageKernel21, gageKernel21,
  gageKernel22, gageKernel22
};

airEnum
_gageKernel_enum = {
  "kernel",
  GAGE_KERNEL_NUM,
  _gageKernelStr, _gageKernelVal,
  _gageKernelDesc,
  _gageKernelStrEqv, _gageKernelValEqv,
  AIR_FALSE
};
airEnum *
gageKernel = &_gageKernel_enum;

/*
** _gageStandardPadder()
**
** the usual/default padder used in gage, basically just a simple
** wrapper around nrrdPad(), but similar in spirit to nrrdSimplePad().
**
** The "kind" is needed to learn the baseDim for this kind of volume.
** The "_info" pointer is ignored.
**
** Any padder used in gage must, if it generates biff errors, 
** accumulate them under the GAGE key.  This padder is a shining
** example of this.
*/
Nrrd *
_gageStandardPadder (Nrrd *nin, gageKind *kind, 
		     int padding, gagePerVolume *pvl) {
  Nrrd *npad;
  char me[]="_gageStandardPadder", err[AIR_STRLEN_MED];
  int i, min[NRRD_DIM_MAX], max[NRRD_DIM_MAX], baseDim;

  if (!(nin && kind)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err); return NULL;
  }

  baseDim = kind->baseDim;
  if (!( padding >= 0 )) {
    sprintf(err, "%s: given padding %d invalid", me, padding);
    biffAdd(GAGE, err); return NULL;
  }
  if (0 == padding) {
    /* we can use the original volume if we promise not to blow it away,
       see _gageStandardNixer() */
    return nin;
  } else {
    npad = nrrdNew();
    for (i=0; i<baseDim; i++) {
      min[i] = 0;
      max[i] = nin->axis[i].size - 1;
    }
    min[0 + baseDim] = -padding;
    min[1 + baseDim] = -padding;
    min[2 + baseDim] = -padding;
    max[0 + baseDim] = nin->axis[0 + baseDim].size - 1 + padding;
    max[1 + baseDim] = nin->axis[1 + baseDim].size - 1 + padding;
    max[2 + baseDim] = nin->axis[2 + baseDim].size - 1 + padding;
    if (nrrdPad(npad, nin, min, max, nrrdBoundaryBleed)) {
      sprintf(err, "%s: trouble padding input volume", me);
      biffMove(GAGE, err, NRRD); return NULL;
    }
  }
  return npad;
}

/*
** _gageStandardNixer()
**
** the usual/default nixer used in gage, which is a simple wrapper
** around nrrdNuke at this point, although other things could be
** implemented later ...
**
** The "kind" and "_info" pointers are ignored.
**
** Nixers must gracefully handle being handed a NULL npad.
**
** The intention is that nixers cannot generate errors, so the return
** is void, and no nixer should accumulate errors in biff.  This nixer
** is a shining example of this.
*/
void
_gageStandardNixer (Nrrd *npad, gageKind *kind, gagePerVolume *pvl) {

  if (npad != pvl->nin) {
    nrrdNuke(npad);
  }
}

void
gageParmReset (gageParm *parm) {

  if (parm) {
    parm->renormalize = gageDefRenormalize;
    parm->checkIntegrals = gageDefCheckIntegrals;
    parm->noRepadWhenSmaller = gageDefNoRepadWhenSmaller;
    parm->k3pack = gageDefK3Pack;
    parm->gradMagMin = gageDefGradMagMin;
    parm->gradMagCurvMin = gageDefGradMagCurvMin;
    parm->defaultSpacing = gageDefDefaultSpacing;
    parm->curvNormalSide = gageDefCurvNormalSide;
    parm->kernelIntegralNearZero = gageDefKernelIntegralNearZero;
    parm->requireAllSpacings = gageDefRequireAllSpacings;
    parm->requireEqualCenters = gageDefRequireEqualCenters;
    parm->defaultCenter = gageDefDefaultCenter;
  }
  return;
}

void
gagePointReset (gagePoint *point) {

  if (point) {
    /* learned: can't initialize the floating point to AIR_NAN, 
       non-dot-net windows compilers proclaim that QNAN == x
       for any existant x!!!  For some reason though, infinity
       is handled correctly */
    point->xf = point->yf = point->zf = AIR_POS_INF;
    point->xi = point->yi = point->zi = -1;
  }
  return;
}
