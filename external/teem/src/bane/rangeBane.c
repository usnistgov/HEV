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

/* ----------------- baneRangeUnknown -------------------- */

int
_baneRangeUnknown_Ans(double *ominP, double *omaxP,
		      double imin, double imax) {
  char me[]="_baneRangeUnknown_Ans", err[AIR_STRLEN_MED];

  sprintf(err, "%s: a baneRange is unset", me);
  biffAdd(BANE, err);
  return 1;
}

baneRange
_baneRangeUnknown = {
  "unknown",
  baneRangeUnknown_e,
  _baneRangeUnknown_Ans
};
baneRange *
baneRangeUnknown = &_baneRangeUnknown;

/* ----------------- baneRangePos -------------------- */

int
_baneRangePos_Ans(double *ominP, double *omaxP,
		  double imin, double imax) {
  char me[]="_baneRangePos_Ans", err[AIR_STRLEN_MED];

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    sprintf(err, "%s: imin and imax don't both exist", me);
    biffAdd(BANE, err); return 1;
  }
  *ominP = 0;
  *omaxP = imax;
  return 0;
}

baneRange
_baneRangePos = {
  "positive",
  baneRangePos_e,
  _baneRangePos_Ans
};
baneRange *
baneRangePos = &_baneRangePos;

/* ----------------- baneRangeNeg -------------------- */

int
_baneRangeNeg_Ans(double *ominP, double *omaxP,
		  double imin, double imax) {
  char me[]="_baneRangeNeg_Ans", err[AIR_STRLEN_MED];
  
  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    sprintf(err, "%s: imin and imax don't both exist", me);
    biffAdd(BANE, err); return 1;
  }
  *ominP = imin;
  *omaxP = 0;
  return 0;
}

baneRange
_baneRangeNeg = {
  "negative",
  baneRangeNeg_e,
  _baneRangeNeg_Ans
};
baneRange *
baneRangeNeg = &_baneRangeNeg;

/* ----------------- baneRangeCent -------------------- */

/*
** _baneRangeZeroCent_Ans
**
** Unlike the last version of this function, this is conservative: we
** choose the smallest zero-centered range that includes the original
** min and max.  Previously the average of the min and max magnitude
** were used.
*/
int
_baneRangeZeroCent_Ans(double *ominP, double *omaxP,
		       double imin, double imax) {
  char me[]="_baneRangeZeroCent_Ans", err[AIR_STRLEN_MED];

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    sprintf(err, "%s: imin and imax don't both exist", me);
    biffAdd(BANE, err); return 1;
  }
  imin = AIR_MIN(imin, 0);
  imax = AIR_MAX(imax, 0);
  /* now the signs of imin and imax aren't wrong */
  *ominP = AIR_MIN(-imax, imin);
  *omaxP = AIR_MAX(imax, -imin);
  return 0;
}

baneRange
_baneRangeZeroCent = {
  "zero-center",
  baneRangeZeroCent_e,
  _baneRangeZeroCent_Ans
};
baneRange *
baneRangeZeroCent = &_baneRangeZeroCent;

/* ----------------- baneRangeFloat -------------------- */

int
_baneRangeFloat_Ans(double *ominP, double *omaxP,
		    double imin, double imax) {
  char me[]="_baneRangeFloat_Ans", err[AIR_STRLEN_MED];

  if (!( AIR_EXISTS(imin) && AIR_EXISTS(imax) )) {
    sprintf(err, "%s: imin and imax don't both exist", me);
    biffAdd(BANE, err); return 1;
  }
  *ominP = imin;
  *omaxP = imax;
  return 0;
}

baneRange
_baneRangeFloat = {
  "float",
  baneRangeFloat_e,
  _baneRangeFloat_Ans
};
baneRange *
baneRangeFloat = &_baneRangeFloat;

/* ----------------------------------------------------- */

baneRange *
baneRangeArray[BANE_RANGE_MAX+1] = {
  &_baneRangeUnknown,
  &_baneRangePos,
  &_baneRangeNeg,
  &_baneRangeZeroCent,
  &_baneRangeFloat
};
