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


#include "bane.h"
#include "privateBane.h"

/* ----------------- baneClipUnknown -------------------- */

int
_baneClipUnknown_Ans(Nrrd *hvol, double *clipParm) {
  char me[]="_baneClipUnknown_Ans";
  fprintf(stderr, "%s: a baneClip is unset somewhere ...\n", me);
  return -1;
}

baneClip
_baneClipUnknown = {
  "unknown",
  baneClipUnknown_e,
  0,
  _baneClipUnknown_Ans
};
baneClip *
baneClipUnknown = &_baneClipUnknown;

/* ----------------- baneClipAbsolute -------------------- */

int
_baneClipAbsolute_Ans(Nrrd *hvol, double *clipParm) {

  return clipParm[0];
}

baneClip
_baneClipAbsolute = {
  "absolute",
  baneClipAbsolute_e,
  1,
  _baneClipAbsolute_Ans
};
baneClip *
baneClipAbsolute = &_baneClipAbsolute;

/* ----------------- baneClipPeakRatio -------------------- */

int
_baneClipPeakRatio_Ans(Nrrd *hvol, double *clipParm) {
  int *hits, maxhits;
  size_t idx, num;
  
  hits = hvol->data;
  maxhits = 0;
  num = nrrdElementNumber(hvol);
  for (idx=0; idx<num; idx++) {
    maxhits = AIR_MAX(maxhits, hits[idx]);
  }

  return maxhits*clipParm[0];
}

baneClip
_baneClipPeakRatio = {
  "peak-ratio",
  baneClipPeakRatio_e,
  1,
  _baneClipPeakRatio_Ans
};
baneClip *
baneClipPeakRatio = &_baneClipPeakRatio;

/* ----------------- baneClipPercentile -------------------- */

int
_baneClipPercentile_Ans(Nrrd *hvol, double *clipParm) {
  char me[]="_baneClipPercentile", err[AIR_STRLEN_MED];
  Nrrd *copy;
  int *hits, clip;
  size_t num, sum, out, outsofar, hi;

  if (nrrdCopy(copy=nrrdNew(), hvol)) {
    sprintf(err, "%s: couldn't create copy of histovol", me);
    biffMove(BANE, err, NRRD); return -1;
  }
  hits = copy->data;
  num = nrrdElementNumber(copy);
  qsort(hits, num, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  sum = 0;
  for (hi=0; hi<num; hi++) {
    sum += hits[hi];
  }
  out = sum*clipParm[0]/100;
  outsofar = 0;
  hi = num-1;
  do {
    outsofar += hits[hi--];
  } while (outsofar < out);
  clip = hits[hi];
  nrrdNuke(copy);

  return clip;
}

baneClip
_baneClipPercentile = {
  "percentile",
  baneClipPercentile_e,
  1,
  _baneClipPercentile_Ans
};
baneClip *
baneClipPercentile = &_baneClipPercentile;

/* ----------------- baneClipTopN -------------------- */

int
_baneClipTopN_Ans(Nrrd *hvol, double *clipParm) {
  char me[]="_baneClipTopN", err[AIR_STRLEN_MED];
  Nrrd *copy;
  int *hits, clip;
  size_t num;

  if (nrrdCopy(copy=nrrdNew(), hvol)) {
    sprintf(err, "%s: couldn't create copy of histovol", me);
    biffMove(BANE, err, NRRD); return -1;
  }
  hits = copy->data;
  num = nrrdElementNumber(copy);
  qsort(hits, num, sizeof(int), nrrdValCompare[nrrdTypeInt]);
  clipParm[0] = AIR_CLAMP(0, (int)clipParm[0], num-1);
  clip = hits[num-(int)clipParm[0]-1];
  nrrdNuke(copy);

  return clip;
}

baneClip
_baneClipTopN = {
  "top-N",
  baneClipTopN_e,
  1,
  _baneClipTopN_Ans
};
baneClip *
baneClipTopN = &_baneClipTopN;

/* --------------------------------------------------- */

baneClip *
baneClipArray[BANE_CLIP_MAX+1] = {
  &_baneClipUnknown,
  &_baneClipAbsolute,
  &_baneClipPeakRatio,
  &_baneClipPercentile,
  &_baneClipTopN
};

