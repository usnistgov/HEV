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

#include "nrrd.h"
#include "privateNrrd.h"

/* ------------------------------------------------------------ */

void
_nrrdAxisInfoInit(NrrdAxis *axis) {
  
  if (axis) {
    axis->size = 0;
    axis->spacing = AIR_NAN;
    axis->min = axis->max = AIR_NAN;
    AIR_FREE(axis->label);
    AIR_FREE(axis->unit);
    axis->center = nrrdCenterUnknown;
  }
}

/*
nrrdAxisInfoNew(void) {
  nrrdAxisInfo *axis;

  axis = calloc(1, sizeof(NrrdAxis));
  axis->label = NULL;
  axis->unit = NULL;
  if (axis) {
    _nrrdAxisInfoInit(axis);
  }
  return axis;
}

NrrdAxis *
nrrdAxisInfoNix(NrrdAxis *axis) {

  if (axis) {
    AIR_FREE(axis->label);
    AIR_FREE(axis->unit);
    AIR_FREE(axis);
  }
  return NULL;
}
*/

/* ------------------------------------------------------------ */

void
_nrrdAxisInfoCopy(NrrdAxis *dest, const NrrdAxis *src, int bitflag) {

  if (!(NRRD_AXIS_INFO_SIZE_BIT & bitflag)) {
    dest->size = src->size;
  }
  if (!(NRRD_AXIS_INFO_SPACING_BIT & bitflag)) {
    dest->spacing = src->spacing;
  }
  if (!(NRRD_AXIS_INFO_MIN_BIT & bitflag)) {
    dest->min = src->min;
  }
  if (!(NRRD_AXIS_INFO_MAX_BIT & bitflag)) {
    dest->max = src->max;
  }
  if (!(NRRD_AXIS_INFO_CENTER_BIT & bitflag)) {
    dest->center = src->center;
  }
  if (!(NRRD_AXIS_INFO_LABEL_BIT & bitflag)) {
    if (dest->label != src->label) {
      AIR_FREE(dest->label);
      dest->label = airStrdup(src->label);
    }
  }
  if (!(NRRD_AXIS_INFO_UNIT_BIT & bitflag)) {
    if (dest->unit != src->unit) {
      AIR_FREE(dest->unit);
      dest->unit = airStrdup(src->unit);
    }
  }
  return;
}

/*
******** nrrdAxisInfoCopy()
**
** For copying all the per-axis peripheral information.  Takes a
** permutation "map"; map[d] tells from which axis in input should the
** output axis d copy its information.  The length of this permutation
** array is nout->dim.  If map is NULL, the identity permutation is
** assumed.  If map[i]==-1 for any i in [0,dim-1], then nothing is
** copied into axis i of output.  The "bitflag" field controls which
** per-axis fields will NOT be copied; if bitflag==0, then all fields
** are copied.  The value of bitflag should be |'s of NRRD_AXIS_INFO_*
** defines.
**
** Decided to Not use Biff, since many times map will be NULL, in
** which case the only error is getting a NULL nrrd, or an invalid map
** permutation, which will probably be unlikely given the contexts in
** which this is called.  For the paranoid, the integer return value
** indicates error.
*/
int
nrrdAxisInfoCopy(Nrrd *nout, const Nrrd *nin, const int *map, int bitflag) {
  int d, from;
  
  if (!(nout && nin)) {
    return 1;
  }
  if (nout == nin) {
    /* this is a problem because we don't want to have to deal with
       temp variables to re-arrange the axes */
    return 2;
  }
  if (map) {
    for (d=0; d<nout->dim; d++) {
      if (-1 == map[d]) {
	continue;
      }
      if (!AIR_IN_CL(0, map[d], nin->dim-1)) {
	return 3;
      }
    }
  }
  
  for (d=0; d<nout->dim; d++) {
    from = map ? map[d] : d;
    if (-1 == from) {
      /* for this axis, we don't touch a thing */
      continue;
    }
    _nrrdAxisInfoCopy(&(nout->axis[d]), &(nin->axis[from]), bitflag);
  }
  return 0;
}

/*
******** nrrdAxisInfoSet_nva()
**
** Simple means of setting fields of the axis array in the nrrd.
*/
void
nrrdAxisInfoSet_nva(Nrrd *nrrd, int axInfo, const void *_info) {
  _nrrdAxisInfoSetPtrs info;
  int d;
  
  if (!( nrrd 
	 && AIR_IN_CL(1, nrrd->dim, NRRD_DIM_MAX) 
	 && AIR_IN_OP(nrrdAxisInfoUnknown, axInfo, nrrdAxisInfoLast) 
	 && _info )) {
    return;
  }
  info.P = _info;

  for (d=0; d<nrrd->dim; d++) {
    switch (axInfo) {
    case nrrdAxisInfoSize:
      nrrd->axis[d].size = info.I[d];
      break;
    case nrrdAxisInfoSpacing:
      nrrd->axis[d].spacing = info.D[d];
      break;
    case nrrdAxisInfoMin:
      nrrd->axis[d].min = info.D[d];
      break;
    case nrrdAxisInfoMax:
      nrrd->axis[d].max = info.D[d];
      break;
    case nrrdAxisInfoCenter:
      nrrd->axis[d].center = info.I[d];
      break;
    case nrrdAxisInfoLabel:
      AIR_FREE(nrrd->axis[d].label);
      nrrd->axis[d].label = airStrdup(info.CP[d]);
      break;
    case nrrdAxisInfoUnit:
      AIR_FREE(nrrd->axis[d].unit);
      nrrd->axis[d].unit = airStrdup(info.CP[d]);
      break;
    }
  }
  return;
}

/*
******** nrrdAxisInfoSet()
**
** var args front-end for nrrdAxisInfoSet_nva
*/
void
nrrdAxisInfoSet(Nrrd *nrrd, int axInfo, ...) {
  NRRD_TYPE_BIGGEST *space[NRRD_DIM_MAX];
  _nrrdAxisInfoSetPtrs info;
  int d;
  va_list ap;

  if (!( nrrd 
	 && AIR_IN_CL(1, nrrd->dim, NRRD_DIM_MAX) 
	 && AIR_IN_OP(nrrdAxisInfoUnknown, axInfo, nrrdAxisInfoLast) )) {
    return;
  }

  info.P = space;
  va_start(ap, axInfo);
  for (d=0; d<nrrd->dim; d++) {
    switch (axInfo) {
    case nrrdAxisInfoSize:
      info.I[d] = va_arg(ap, int);
      /*
      printf("!%s: got int[%d] = %d\n", "nrrdAxisInfoSet", d, info.I[d]);
      */
      break;
    case nrrdAxisInfoCenter:
      info.I[d] = va_arg(ap, int);
      /*
      printf("!%s: got int[%d] = %d\n", 
	     "nrrdAxisInfoSet", d, info.I[d]);
      */
      break;
    case nrrdAxisInfoSpacing:
    case nrrdAxisInfoMin:
    case nrrdAxisInfoMax:
      info.D[d] = va_arg(ap, double);
      /*
      printf("!%s: got double[%d] = %g\n", 
	     "nrrdAxisInfoSet", d, info.D[d]); 
      */
      break;
    case nrrdAxisInfoLabel:
      /* we DO NOT do the airStrdup() here because this pointer value is
	 just going to be handed to nrrdAxisInfoSet_nva(), which WILL do the
	 airStrdup(); we're not violating the rules for axis labels */
      info.CP[d] = va_arg(ap, char *);
      /*
      printf("!%s: got char*[%d] = |%s|\n", 
	     "nrrdAxisInfoSet", d, info.CP[d]);
      */
      break;
    case nrrdAxisInfoUnit:
      /* same explanation as above */
      info.CP[d] = va_arg(ap, char *);
      break;
    }
  }
  va_end(ap);

  /* now set the quantities which we've gotten from the var args */
  nrrdAxisInfoSet_nva(nrrd, axInfo, info.P);
  
  return;
}

/*
******** nrrdAxisInfoGet_nva()
**
** get any of the axis fields into an array
**
** Note that getting axes labels and units involves implicitly
** allocating space for them, due to the action of airStrdup().  The
** user is responsible for free()ing these strings when done with
** them.
*/
void
nrrdAxisInfoGet_nva(const Nrrd *nrrd, int axInfo, void *_info) {
  _nrrdAxisInfoGetPtrs info;
  int d;
  
  if (!( nrrd 
	 && AIR_IN_CL(1, nrrd->dim, NRRD_DIM_MAX) 
	 && AIR_IN_OP(nrrdAxisInfoUnknown, axInfo, nrrdAxisInfoLast) )) {
    return;
  }
  
  info.P = _info;
  for (d=0; d<nrrd->dim; d++) {
    switch (axInfo) {
    case nrrdAxisInfoSize:
      info.I[d] = nrrd->axis[d].size;
      break;
    case nrrdAxisInfoSpacing:
      info.D[d] = nrrd->axis[d].spacing;
      break;
    case nrrdAxisInfoMin:
      info.D[d] = nrrd->axis[d].min;
      break;
    case nrrdAxisInfoMax:
      info.D[d] = nrrd->axis[d].max;
      break;
    case nrrdAxisInfoCenter:
      info.I[d] = nrrd->axis[d].center;
      break;
    case nrrdAxisInfoLabel:
      /* note airStrdup()! */
      info.CP[d] = airStrdup(nrrd->axis[d].label);
      break;
    case nrrdAxisInfoUnit:
      /* note airStrdup()! */
      info.CP[d] = airStrdup(nrrd->axis[d].unit);
      break;
    }
  }

  return;
}

void
nrrdAxisInfoGet(const Nrrd *nrrd, int axInfo, ...) {
  void *space[NRRD_DIM_MAX], *ptr;
  _nrrdAxisInfoGetPtrs info;
  int d;
  va_list ap;

  if (!( nrrd 
	 && AIR_IN_CL(1, nrrd->dim, NRRD_DIM_MAX) 
	 && AIR_IN_OP(nrrdAxisInfoUnknown, axInfo, nrrdAxisInfoLast) )) {
    return;
  }

  info.P = space;
  nrrdAxisInfoGet_nva(nrrd, axInfo, info.P);

  va_start(ap, axInfo);
  for (d=0; d<nrrd->dim; d++) {
    ptr = va_arg(ap, void*);
    switch (axInfo) {
    case nrrdAxisInfoSize:
      *((int*)ptr) = info.I[d];
      /* printf("!%s: got int[%d] = %d\n", 
	 "nrrdAxisInfoGet", d, *((int*)ptr)); */
      break;
    case nrrdAxisInfoSpacing:
    case nrrdAxisInfoMin:
    case nrrdAxisInfoMax:
      *((double*)ptr) = info.D[d];
      /* printf("!%s: got double[%d] = %lg\n", "nrrdAxisInfoGet", d,
       *((double*)ptr)); */
      break;
    case nrrdAxisInfoCenter:
      *((int*)ptr) = info.I[d];
      /* printf("!%s: got int[%d] = %d\n", 
	 "nrrdAxisInfoGet", d, *((int*)ptr)); */
      break;
    case nrrdAxisInfoLabel:
      /* we DO NOT do the airStrdup() here because this pointer value just
	 came from nrrdAxisInfoGet(), which already did the airStrdup() */
      *((char**)ptr) = info.CP[d];
      /* printf("!%s: got char*[%d] = |%s|\n", "nrrdAxisInfoSet", d, 
       *((char**)ptr)); */
      break;
    case nrrdAxisInfoUnit:
      /* same explanation as above */
      *((char**)ptr) = info.CP[d];
      break;
    }
  }
  va_end(ap);
  
  return;
}

/*
** _nrrdCenter()
**
** for nrrdCenterCell and nrrdCenterNode, return will be the same
** as input.  Converts nrrdCenterUnknown into nrrdDefCenter,
** and then clamps to (nrrdCenterUnknown+1, nrrdCenterLast-1).
**
** Thus, this ALWAYS returns nrrdCenterNode or nrrdCenterCell
** (as long as those are the only two centering schemes).
*/
int
_nrrdCenter(int center) {
  
  center =  (nrrdCenterUnknown == center
	     ? nrrdDefCenter
	     : center);
  center = AIR_CLAMP(nrrdCenterUnknown+1, center, nrrdCenterLast-1);
  return center;
}

int
_nrrdCenter2(int center, int defCenter) {
  
  center =  (nrrdCenterUnknown == center
	     ? defCenter
	     : center);
  center = AIR_CLAMP(nrrdCenterUnknown+1, center, nrrdCenterLast-1);
  return center;
}


/*
******** nrrdAxisPos()
** 
** given a nrrd, an axis, and a (floating point) index space position,
** return the position implied the axis's min, max, and center
** Does the opposite of nrrdAxisIdx().
**
** does not use biff
*/
double
nrrdAxisPos(const Nrrd *nrrd, int ax, double idx) {
  int center, size;
  double min, max;
  
  if (!( nrrd && AIR_IN_CL(0, ax, nrrd->dim-1) )) {
    return AIR_NAN;
  }
  center = _nrrdCenter(nrrd->axis[ax].center);
  min = nrrd->axis[ax].min;
  max = nrrd->axis[ax].max;
  size = nrrd->axis[ax].size;
  
  return NRRD_POS(center, min, max, size, idx);
}

/*
******** nrrdAxisIdx()
** 
** given a nrrd, an axis, and a (floating point) world space position,
** return the index implied the axis's min, max, and center.
** Does the opposite of nrrdAxisPos().
**
** does not use biff
*/
double
nrrdAxisIdx(const Nrrd *nrrd, int ax, double pos) {
  int center, size;
  double min, max;
  
  if (!( nrrd && AIR_IN_CL(0, ax, nrrd->dim-1) )) {
    return AIR_NAN;
  }
  center = _nrrdCenter(nrrd->axis[ax].center);
  min = nrrd->axis[ax].min;
  max = nrrd->axis[ax].max;
  size = nrrd->axis[ax].size;

  return NRRD_IDX(center, min, max, size, pos);
}

/*
******** nrrdAxisPosRange()
**
** given a nrrd, an axis, and two (floating point) index space positions,
** return the range of positions implied the axis's min, max, and center
** The opposite of nrrdAxisIdxRange()
*/
void
nrrdAxisPosRange(double *loP, double *hiP,
		 const Nrrd *nrrd, int ax, 
		 double loIdx, double hiIdx) {
  int center, size, flip = 0;
  double min, max, tmp;

  if (!( loP && hiP && nrrd && AIR_IN_CL(0, ax, nrrd->dim-1) )) {
    *loP = *hiP = AIR_NAN;
    return;
  }
  center = _nrrdCenter(nrrd->axis[ax].center);
  min = nrrd->axis[ax].min;
  max = nrrd->axis[ax].max;
  size = nrrd->axis[ax].size;

  if (loIdx > hiIdx) {
    flip = 1;
    tmp = loIdx; loIdx = hiIdx; hiIdx = tmp;
  }
  if (nrrdCenterCell == center) {
    *loP = AIR_AFFINE(0, loIdx, size, min, max);
    *hiP = AIR_AFFINE(0, hiIdx+1, size, min, max);
  } else {
    *loP = AIR_AFFINE(0, loIdx, size-1, min, max);
    *hiP = AIR_AFFINE(0, hiIdx, size-1, min, max);
  }
  if (flip) {
    tmp = *loP; *loP = *hiP; *hiP = tmp;
  }

  return;
}

/*
******** nrrdAxisIdxRange()
**
** given a nrrd, an axis, and two (floating point) world space positions,
** return the range of index space implied the axis's min, max, and center
** The opposite of nrrdAxisPosRange().
**
** Actually- there are situations where sending an interval through
** nrrdAxisIdxRange -> nrrdAxisPosRange -> nrrdAxisIdxRange
** such as in cell centering, when the range of positions given does
** not even span one sample.  Such as:
** axis->size = 4, axis->min = -4, axis->max = 4, loPos = 0, hiPos = 1
** --> nrrdAxisIdxRange == (2, 1.5) --> nrrdAxisPosRange == (2, -1)
** The basic problem is that because of the 0.5 offset inherent in
** cell centering, there are situations where (in terms of the arguments
** to nrrdAxisIdxRange()) loPos < hiPos, but *loP > *hiP.
*/
void
nrrdAxisIdxRange(double *loP, double *hiP,
		 const Nrrd *nrrd, int ax, 
		 double loPos, double hiPos) {
  int center, size, flip = 0;
  double min, max, tmp;

  if (!( loP && hiP && nrrd && AIR_IN_CL(0, ax, nrrd->dim-1) )) {
    *loP = *hiP = AIR_NAN;
    return;
  }
  center = _nrrdCenter(nrrd->axis[ax].center);
  min = nrrd->axis[ax].min;
  max = nrrd->axis[ax].max;
  size = nrrd->axis[ax].size;

  if (loPos > hiPos) {
    flip = 1;
    tmp = loPos; loPos = hiPos; hiPos = tmp;
  }
  if (nrrdCenterCell == center) {
    if (min < max) {
      *loP = AIR_AFFINE(min, loPos, max, 0, size);
      *hiP = AIR_AFFINE(min, hiPos, max, -1, size-1);
    } else {
      *loP = AIR_AFFINE(min, loPos, max, -1, size-1);
      *hiP = AIR_AFFINE(min, hiPos, max, 0, size);
    }
  } else {
    *loP = AIR_AFFINE(min, loPos, max, 0, size-1);
    *hiP = AIR_AFFINE(min, hiPos, max, 0, size-1);
  }
  if (flip) {
    tmp = *loP; *loP = *hiP; *hiP = tmp;
  }

  return;
}

void
nrrdAxisSpacingSet(Nrrd *nrrd, int ax) {
  int sign;
  double min, max, tmp;

  if (!( nrrd && AIR_IN_CL(0, ax, nrrd->dim-1) ))
    return;
  
  min = nrrd->axis[ax].min;
  max = nrrd->axis[ax].max;
  if (!( AIR_EXISTS(min) && AIR_EXISTS(max) )) {
    /* there's no actual basis on which to set the spacing information,
       but we have to set it something, so here goes ... */
    nrrd->axis[ax].spacing = nrrdDefSpacing;
    return;
  }

  if (min > max) {
    tmp = min; min = max; max = tmp;
    sign = -1;
  } else {
    sign = 1;
  }

  /* the skinny */
  nrrd->axis[ax].spacing = NRRD_SPACING(_nrrdCenter(nrrd->axis[ax].center),
					min, max, nrrd->axis[ax].size);
  nrrd->axis[ax].spacing *= sign;

  return;
}

void
nrrdAxisMinMaxSet(Nrrd *nrrd, int ax, int defCenter) {
  int center;
  double spacing;

  if (!( nrrd && AIR_IN_CL(0, ax, nrrd->dim-1) ))
    return;
  
  center = _nrrdCenter2(nrrd->axis[ax].center, defCenter);
  spacing = nrrd->axis[ax].spacing;
  if (!AIR_EXISTS(spacing))
    spacing = nrrdDefSpacing;
  if (nrrdCenterCell == center) {
    nrrd->axis[ax].min = 0;
    nrrd->axis[ax].max = spacing*nrrd->axis[ax].size;
  } else {
    nrrd->axis[ax].min = 0;
    nrrd->axis[ax].max = spacing*(nrrd->axis[ax].size - 1);
  }
  
  return;
}
