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

/* ----------------- baneMeasrUnknown -------------------- */

float
_baneMeasrUnknown_Ans(gage_t *san, double *measrParm) {
  char me[]="_baneMeasrUnknown_Ans";
  fprintf(stderr, "%s: a baneMeasr is unset somewhere ...\n", me);
  return AIR_NAN;
}

baneMeasr
_baneMeasrUnknown = {
  "unknown",
  baneMeasrUnknown_e,
  0,
  0,
  &_baneRangePos,
  _baneMeasrUnknown_Ans
};
baneMeasr *
baneMeasrUnknown = &_baneMeasrUnknown;

/* ----------------- baneMeasrVal -------------------- */

float
_baneMeasrVal_Ans(gage_t *san, double *measrParm) {

  /* fprintf(stderr, "## _baneMeasrVal_Ans: %g\n", san->val[0]); */
  return san[gageKindScl->ansOffset[gageSclValue]];
}

/*
** I'm really asking for it here.  Because data value can have
** different ranges depending on the data, I want to have different
** baneMeasrs for them, even though all of them use the same
** ans() method.  This obviously breaks the one-to-one relationship
** between the members of the enum and the related structs which
** occurs with inclusions, measures, and clippings.
*/

baneMeasr
_baneMeasrValFloat = {
  "val(float)",
  baneMeasrVal_e,
  0,
  (1<<gageSclValue),
  &_baneRangeFloat,
  _baneMeasrVal_Ans
};
baneMeasr *
baneMeasrValFloat = &_baneMeasrValFloat;

baneMeasr
_baneMeasrValPos = {
  "val(pos)",
  baneMeasrVal_e,
  0,
  (1<<gageSclValue),
  &_baneRangePos,
  _baneMeasrVal_Ans
};
baneMeasr *
baneMeasrValPos = &_baneMeasrValPos;

baneMeasr
_baneMeasrValZeroCent = {
  "val(zerocent)",
  baneMeasrVal_e,
  0,
  (1<<gageSclValue),
  &_baneRangeZeroCent,
  _baneMeasrVal_Ans
};
baneMeasr *
baneMeasrValZeroCent = &_baneMeasrValZeroCent;

/*
** We're making baneMeasrVal point to _baneMeasrValFloat, with
** baneRangeFloat, because that is what was done in the last version
** of bane.
*/
baneMeasr *
baneMeasrVal = &_baneMeasrValFloat;


/* ----------------- baneMeasrGradMag -------------------- */

float
_baneMeasrGradMag_Ans(gage_t *san, double *measrParm) {

  /* fprintf(stderr, "## _baneMeasrGradMag_Ans: %g\n", san->gmag[0]); */
  return san[gageKindScl->ansOffset[gageSclGradMag]];
}

baneMeasr
_baneMeasrGradMag = {
  "gradmag",
  baneMeasrGradMag_e,
  0,
  (1<<gageSclGradMag),
  &_baneRangePos,
  _baneMeasrGradMag_Ans
};
baneMeasr *
baneMeasrGradMag = &_baneMeasrGradMag;

/* ----------------- baneMeasrLapl -------------------- */

float
_baneMeasrLapl_Ans(gage_t *san, double *measrParm) {

  return san[gageKindScl->ansOffset[gageSclLaplacian]];
}

baneMeasr
_baneMeasrLapl = {
  "laplacian",
  baneMeasrLapl_e,
  0,
  (1<<gageSclLaplacian),
  &_baneRangeZeroCent,
  _baneMeasrLapl_Ans
};
baneMeasr *
baneMeasrLapl = &_baneMeasrLapl;

/* ----------------- baneMeasrHess -------------------- */

float
_baneMeasrHess_Ans(gage_t *san, double *measrParm) {
  
  /* fprintf(stderr, "## _baneMeasrHess_Ans: %g\n", san->scnd[0]); */
  return san[gageKindScl->ansOffset[gageScl2ndDD]];
}

baneMeasr
_baneMeasrHess = {
  "hess-2nd",
  baneMeasrHess_e,
  0,
  (1<<gageScl2ndDD),  /* this is hessian BASED 2nd DD measure */
  &_baneRangeZeroCent,
  _baneMeasrHess_Ans
};
baneMeasr *
baneMeasrHess = &_baneMeasrHess;

/* ----------------- baneMeasrCurvedness -------------------- */

float
_baneMeasrCurvedness_Ans(gage_t *san, double *measrParm) {

  return san[gageKindScl->ansOffset[gageSclCurvedness]];
}

baneMeasr
_baneMeasrCurvedness = {
  "curvedness",
  baneMeasrCurvedness_e,
  0,
  (1<<gageSclCurvedness),
  &_baneRangePos,
  _baneMeasrCurvedness_Ans
};
baneMeasr *
baneMeasrCurvedness = &_baneMeasrCurvedness;

/* ----------------- baneMeasrShapeTrace -------------------- */

float
_baneMeasrShapeTrace_Ans(gage_t *san, double *measrParm) {

  return san[gageKindScl->ansOffset[gageSclShapeTrace]];
}

baneMeasr
_baneMeasrShapeTrace = {
  "shape-trace",
  baneMeasrShapeTrace_e,
  0,
  (1<<gageSclShapeTrace),
  &_baneRangeZeroCent,
  _baneMeasrShapeTrace_Ans
};
baneMeasr *
baneMeasrShapeTrace = &_baneMeasrShapeTrace;

/* ---------------------------------------------------------- */

baneMeasr *baneMeasrArray[BANE_MEASR_MAX+1] = {
  &_baneMeasrUnknown,
  /* HEY: this shouldn't be specific here */
  &_baneMeasrValFloat,
  &_baneMeasrGradMag,
  &_baneMeasrLapl,
  &_baneMeasrHess,
  &_baneMeasrCurvedness,
  &_baneMeasrShapeTrace
};
