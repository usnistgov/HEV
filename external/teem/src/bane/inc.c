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

Nrrd *
_baneInc_EmptyHistNew(double *incParm) {

  return nrrdNew();
}

Nrrd *
_baneInc_HistNew(double *incParm) {
  char me[]="_baneInc_HistNew", err[AIR_STRLEN_MED];
  Nrrd *nhist;
  
  if (nrrdMaybeAlloc(nhist=nrrdNew(), nrrdTypeInt, 1, (int)(incParm[0]))) {
    sprintf(err, "%s: ", me);
    biffMove(BANE, err, NRRD); return NULL;
  }
  return nhist;
}

/*
** _baneInc_LearnMinMax() and _baneInc_HistFill() are 
** candidates for the baneIncPass *passA and *passB
** that are in the baneInc struct
*/

void
_baneInc_LearnMinMax(Nrrd *n, double val, double *incParm) {

  if (AIR_EXISTS(n->axis[0].min)) {
    /* then both min and max have seen at least one valid value */
    n->axis[0].min = AIR_MIN(n->axis[0].min, val);
    n->axis[0].max = AIR_MAX(n->axis[0].max, val);
  } else {
    n->axis[0].min = n->axis[0].max = val;
  }
  /*
  fprintf(stderr, "## _baneInc_LearnMinMax: (%g,%g)\n",
	  n->axis[0].min, n->axis[0].max);
  */
}

void
_baneInc_HistFill(Nrrd *n, double val, double *incParm) {
  int idx, *hist;

  AIR_INDEX(n->axis[0].min, val, n->axis[0].max, n->axis[0].size, idx);
  /*
  fprintf(stderr, "## _baneInc_HistFill: (%g,%g,%g) %d ---> %d\n",
	  n->axis[0].min, val, n->axis[0].max, n->axis[0].size, idx);
  */
  if (AIR_IN_CL(0, idx, n->axis[0].size-1)) {
    hist = (int*)n->data;
    hist[idx]++;
  }
}

/* ----------------- baneIncUnknown -------------------- */

int
_baneIncUnknown_Ans(double *minP, double *maxP,
		     Nrrd *hist, double *incParm,
		     baneRange *range) {
  char me[]="_baneIncUnknown_Ans", err[AIR_STRLEN_MED];

  sprintf(err, "%s: a baneInc is unset", me);
  biffAdd(BANE, err); 
  return 1;
}

baneInc
_baneIncUnknown = {
  "unknown",
  baneIncUnknown_e,
  0,
  _baneInc_EmptyHistNew,
  NULL,
  NULL,
  _baneIncUnknown_Ans
};
baneInc *
baneIncUnknown = &_baneIncUnknown;
  
/* ----------------- baneIncAbsolute -------------------- */

/*
** _baneIncAbsolute_Ans
**
** incParm[0]: new min
** incParm[1]: new max
*/
int
_baneIncAbsolute_Ans(double *minP, double *maxP,
		     Nrrd *hist, double *incParm,
		     baneRange *range) {
  *minP = incParm[0];
  *maxP = incParm[1];
  return 0;
}

baneInc
_baneIncAbsolute = {
  "absolute",
  baneIncAbsolute_e,
  2,
  _baneInc_EmptyHistNew,
  NULL,
  NULL,
  _baneIncAbsolute_Ans
};
baneInc *
baneIncAbsolute = &_baneIncAbsolute;
  
/* ----------------- baneIncRangeRatio -------------------- */

/*
** _baneIncRangeRatio_Ans
**
** incParm[0]: scales the size of the range after it has been
** sent through the associated range function.
** incParm[1]: (optional) for baneRangeFloat: midpoint of scaling;
** if doesn't exist, average of min and max is used.  For all other
** range types, 0 is always used.
*/
int
_baneIncRangeRatio_Ans(double *minP, double *maxP, 
		       Nrrd *hist, double *incParm,
		       baneRange *range) {
  char me[]="_baneIncRangeRatio_Ans", err[AIR_STRLEN_MED];
  double mid;
  
  if (range->ans(minP, maxP, hist->axis[0].min, hist->axis[0].max)) {
    sprintf(err, "%s:", me); biffAdd(BANE, err); return 1;
  }

  if (baneRangeFloat_e == range->which) {
    mid = AIR_EXISTS(incParm[1]) ? incParm[1] : (*minP + *maxP)/2;
    *minP = AIR_AFFINE(-1, -incParm[0], 0, *minP, mid);
    *maxP = AIR_AFFINE(0, incParm[0], 1, mid, *maxP);
  } else {
    *minP *= incParm[0];
    *maxP *= incParm[0];
  }
  return 0;
}

baneInc
_baneIncRangeRatio = {
  "range-ratio",
  baneIncRangeRatio_e,
  1,  /* HEY: only one is required, two can be used */
  _baneInc_EmptyHistNew,
  NULL,
  _baneInc_LearnMinMax,
  _baneIncRangeRatio_Ans
};
baneInc *
baneIncRangeRatio = &_baneIncRangeRatio;


/* ----------------- baneIncPercentile -------------------- */

/*
** _baneIncPercentile_Ans
**
** incParm[0]: resolution of histogram generated
** incParm[1]: PERCENT of hits to throw away, by nibbling away at
** lower and upper ends of range, in a manner dependant on the 
** range type
** incParm[2]: (optional) for baneRangeFloat, center value
** that we nibble towards.  If it doesn't exist, we use the
** average of the min and max (though perhaps the mode value would
** be better).  For all other range types, we nibble towards 0.
*/
int
_baneIncPercentile_Ans(double *minP, double *maxP,
		       Nrrd *nhist, double *incParm,
		       baneRange *range) {
  char me[]="_baneIncPercentile_Ans", err[AIR_STRLEN_MED];
  int *hist, i, histSize, sum;
  float minIncr, maxIncr, out, outsofar, mid, minIdx, maxIdx;
  double min, max;

  /* integrate histogram and determine how many hits to exclude */
  sum = 0;
  hist = nhist->data;
  histSize = nhist->axis[0].size;
  for (i=0; i<histSize; i++) {
    sum += hist[i];
  }
  if (!sum) {
    sprintf(err, "%s: integral of histogram is zero", me);
    biffAdd(BANE, err); return 1;
  }
  /*
  sprintf(err, "%03d-histo.nrrd", baneHack); nrrdSave(err, nhist, NULL);
  baneHack++;
  */
  out = sum*incParm[1]/100.0;
  fprintf(stderr, "##%s: hist's size=%d, sum=%d --> out = %g\n", me,
	  histSize, sum, out);
  if (range->ans(&min, &max, nhist->axis[0].min, nhist->axis[0].max)) {
    sprintf(err, "%s:", me); biffAdd(BANE, err); return 1;
  }
  fprintf(stderr, "##%s: hist's min,max (%g,%g) ---%s---> %g, %g\n",
	  me, nhist->axis[0].min, nhist->axis[0].max,
	  range->name, min, max);
  if (baneRangeFloat_e == range->which) {
    mid = AIR_EXISTS(incParm[2]) ? incParm[2] : (min + max)/2;
  } else {
    mid = 0;
    /* yes, this is okay.  The "mid" is the value we march towards
       from both ends, but we control the rate of marching according
       to the distance to the ends.  So if min == mid == 0, then
       there is no marching up from below */
  }
  fprintf(stderr, "##%s: hist (%g,%g) --> min,max = (%g,%g) --> mid = %g\n",
	  me, nhist->axis[0].min, nhist->axis[0].max, min, max, mid);
  if (max-mid > mid-min) {
    /* the max is further from the mid than the min */
    maxIncr = 1;
    minIncr = (mid-min)/(max-mid);
  } else {
    /* the min is further */
    minIncr = 1;
    maxIncr = (max-mid)/(mid-min);
  }
  if (!( AIR_EXISTS(minIncr) && AIR_EXISTS(maxIncr) )) {
    sprintf(err, "%s: minIncr, maxIncr don't both exist", me);
    biffAdd(BANE, err); return 1;
  }
  fprintf(stderr, "##%s: --> {min,max}Incr = %g,%g\n", me, minIncr, maxIncr);
  minIdx = AIR_AFFINE(nhist->axis[0].min, min, nhist->axis[0].max,
		      0, histSize-1);
  maxIdx = AIR_AFFINE(nhist->axis[0].min, max, nhist->axis[0].max,
		      0, histSize-1);
  outsofar = 0;
  while (outsofar < out) {
    if (AIR_IN_CL(0, minIdx, histSize-1)) {
      outsofar += minIncr*hist[AIR_ROUNDUP(minIdx)];
    }
    if (AIR_IN_CL(0, maxIdx, histSize-1)) {
      outsofar += maxIncr*hist[AIR_ROUNDUP(maxIdx)];
    }
    minIdx += minIncr;
    maxIdx -= maxIncr;
    if (minIdx > maxIdx) {
      sprintf(err, "%s: minIdx (%g) passed maxIdx (%g) during "
	      "histogram traversal", me, minIdx, maxIdx);
      biffAdd(BANE, err); return 1;
    }
  }
  *minP = AIR_AFFINE(0, minIdx, histSize-1,
		     nhist->axis[0].min, nhist->axis[0].max);
  *maxP = AIR_AFFINE(0, maxIdx, histSize-1,
		     nhist->axis[0].min, nhist->axis[0].max);
  fprintf(stderr, "##%s: --> output min, max = %g, %g\n", me, *minP, *maxP);
  return 0;
}

baneInc
_baneIncPercentile = {
  "percentile",
  baneIncPercentile_e,
  2,  /* HEY: only 2 are required, three can be used */
  _baneInc_HistNew,
  _baneInc_LearnMinMax,
  _baneInc_HistFill,
  _baneIncPercentile_Ans,
};
baneInc *
baneIncPercentile = &_baneIncPercentile;

/* ----------------- baneIncStdv -------------------- */

Nrrd *
_baneIncStdv_EmptyHistNew(double *incParm) {
  Nrrd *hist;

  hist = nrrdNew();
  /* this is a total horrid sham and a hack: we don't need a histogram
     at all; but we're going to use axis[1].min to store the sum of
     all values, and axis[1].max to store the sum of all squared
     values, and axis[1].size to store the number of values.  It may
     be tempting to use incParm for this, but its only meant for
     input and we can't surprise anyone by over-writing values.

     The road to hell ...
  */
  hist->axis[1].min = 0.0;
  hist->axis[1].max = 0.0;
  hist->axis[1].size = 0;
  return hist;
}

void 
_baneIncStdv_Pass(Nrrd *hist, double val, double *incParm) {
  
  _baneInc_LearnMinMax(hist, val, incParm);
  hist->axis[1].min += val;
  hist->axis[1].max += val*val;
  hist->axis[1].size += 1;
}


/*
** _baneIncStdv_Ans()
**
** incParm[0]: range is standard deviation times this
** incParm[1]: (optional) for baneRangeFloat: if exists, the midpoint
** of the range, otherwise the mean is used.  For all other range
** types, the range is positioned in the logical way.
*/
int
_baneIncStdv_Ans(double *minP, double *maxP,
		 Nrrd *hist, double *incParm,
		 baneRange *range) {
  float SS, stdv, mid, mean, width;
  int count;

  count = hist->axis[1].size;
  mean = hist->axis[1].min/count;
  SS = hist->axis[1].max/count;
  stdv = sqrt(SS - mean*mean);
  width = incParm[0]*stdv;
  fprintf(stderr, "##%s: mean=%g, stdv=%g --> width=%g\n",
	  "_baneIncStdv_Ans", mean, stdv, width);
  switch (range->which) {
  case baneRangePos_e:
    *minP = 0;
    *maxP = width;
    break;
  case baneRangeNeg_e:
    *minP = -width;
    *maxP = 0;
    break;
  case baneRangeZeroCent_e:
    *minP = -width/2;
    *maxP = width/2;
    break;
  case baneRangeFloat_e:
    mid = AIR_EXISTS(incParm[1]) ? incParm[1] : mean;
    *minP = mid - width/2;
    *maxP = mid + width/2;
    break;
  default:
    *minP = *maxP = AIR_NAN;
    break;
  }
  return 0;
}

baneInc 
_baneIncStdv = {
  "stdv",
  baneIncStdv_e,
  1,  /* HEY: only one is required, two can be used */
  _baneIncStdv_EmptyHistNew,
  NULL,
  _baneIncStdv_Pass,
  _baneIncStdv_Ans
};
baneInc *
baneIncStdv = &_baneIncStdv;
     
/* -------------------------------------------------- */

baneInc *baneIncArray[BANE_INC_MAX+1] = {
  &_baneIncUnknown,
  &_baneIncAbsolute,
  &_baneIncRangeRatio,
  &_baneIncPercentile,
  &_baneIncStdv
};
