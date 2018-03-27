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

void
gageShapeReset (gageShape *shape) {
  int i, ai;
  
  if (shape) {
    ELL_3V_SET(shape->size, -1, -1, -1);
    shape->defaultCenter = gageDefDefaultCenter;
    shape->center = nrrdCenterUnknown;
    ELL_3V_SET(shape->spacing, AIR_NAN, AIR_NAN, AIR_NAN);
    for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      /* valgrind complained about AIR_NAN at -O2 */
      for (ai=0; ai<=2; ai++) {
	shape->fwScale[i][ai] = airNaN();
      }
    }
    ELL_3V_SET(shape->volHalfLen, AIR_NAN, AIR_NAN, AIR_NAN);
    ELL_3V_SET(shape->voxLen, AIR_NAN, AIR_NAN, AIR_NAN);
  }
  return;
}

gageShape *
gageShapeNew () {
  gageShape *shape;
  
  shape = (gageShape *)calloc(1, sizeof(gageShape));
  if (shape) {
    gageShapeReset(shape);
  }
  return shape;
}

gageShape *
gageShapeNix (gageShape *shape) {
  
  return airFree(shape);
}

/*
** _gageShapeSet
**
** we are serving two masters here.  If ctx is non-NULL, we are being called
** from within gage, and we are to be lax or strict according to the settings
** of ctx->parm.requireAllSpacings and ctx->parm.requireEqualCenters.  If
** ctx is NULL, gageShapeSet was called, in which case we go with lax
** behavior (nothing "required")
**
** This function has subsumed the old gageVolumeCheck.
*/
int
_gageShapeSet (gageContext *ctx, gageShape *shape, Nrrd *nin, int baseDim) {
  char me[]="_gageShapeSet", err[AIR_STRLEN_MED];
  int i, ai, minsize, cx, cy, cz, sx, sy, sz, num[3], defCenter;
  NrrdAxis *ax[3];
  double maxLen, xs, ys, zs, defSpacing;

  /* ------ basic error checking */
  if (!( shape && nin )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(GAGE, err);  if (shape) { gageShapeReset(shape); }
    return 1;
  }
  if (nrrdCheck(nin)) {
    sprintf(err, "%s: basic nrrd validity check failed", me);
    biffMove(GAGE, err, NRRD); gageShapeReset(shape);
    return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    sprintf(err, "%s: need a non-block type nrrd", me);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  if (!(nin->dim == 3 + baseDim)) {
    sprintf(err, "%s: nrrd should be %d-D, not %d-D",
	    me, 3 + baseDim, nin->dim);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  ax[0] = &(nin->axis[baseDim+0]);
  ax[1] = &(nin->axis[baseDim+1]);
  ax[2] = &(nin->axis[baseDim+2]);

  /* ------ find centering */
  cx = ax[0]->center;
  cy = ax[1]->center;
  cz = ax[2]->center;
  if (ctx && ctx->parm.requireEqualCenters) {
    if (!( cx == cy && cx == cz )) {
      sprintf(err, "%s: axes %d,%d,%d centerings (%s,%s,%s) not equal", me,
	      baseDim+0, baseDim+1, baseDim+2,
	      airEnumStr(nrrdCenter, cx),
	      airEnumStr(nrrdCenter, cy),
	      airEnumStr(nrrdCenter, cz));
      biffAdd(GAGE, err); gageShapeReset(shape);
      return 1;
    }
  } else {
    if ( (nrrdCenterUnknown != cx && nrrdCenterUnknown != cy && cx != cy) ||
	 (nrrdCenterUnknown != cy && nrrdCenterUnknown != cz && cy != cz) ||
	 (nrrdCenterUnknown != cx && nrrdCenterUnknown != cz && cx != cz) ) {
      sprintf(err, "%s: two known centerings (of %s,%s,%s) are unequal", me,
	      airEnumStr(nrrdCenter, cx),
	      airEnumStr(nrrdCenter, cy),
	      airEnumStr(nrrdCenter, cz));
      biffAdd(GAGE, err); gageShapeReset(shape);
      return 1;
    }
  }
  defCenter = ctx ? ctx->parm.defaultCenter : shape->defaultCenter;
  shape->center = (nrrdCenterUnknown != cx ? cx
		   : (nrrdCenterUnknown != cy ? cy
		      : (nrrdCenterUnknown != cz ? cz
			 : defCenter)));

  /* ------ find sizes */
  sx = ax[0]->size;
  sy = ax[1]->size;
  sz = ax[2]->size;
  minsize = (nrrdCenterCell == shape->center ? 1 : 2);
  if (!(sx >= minsize && sy >= minsize && sz >= minsize )) {
    sprintf(err, "%s: sizes (%d,%d,%d) must all be greater than %d "
	    "(min number of %s-centered samples)", me, 
	    sx, sy, sz, minsize, airEnumStr(nrrdCenter, shape->center));
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  shape->size[0] = sx;
  shape->size[1] = sy;
  shape->size[2] = sz;

  /* ------ find spacings */
  xs = ax[0]->spacing;
  ys = ax[1]->spacing;
  zs = ax[2]->spacing;
  if (ctx && ctx->parm.requireAllSpacings) {
    if (!( AIR_EXISTS(xs) && AIR_EXISTS(ys) && AIR_EXISTS(zs) )) {
      sprintf(err, "%s: spacings for axes %d,%d,%d don't all exist",
	      me, baseDim+0, baseDim+1, baseDim+2);
      biffAdd(GAGE, err); gageShapeReset(shape);
      return 1;
    }
  }
  /* there is no shape->defaultSpacing, we'll go out on a limb ... */
  defSpacing = ctx ? ctx->parm.defaultSpacing : nrrdDefSpacing;
  xs = AIR_EXISTS(xs) ? xs : defSpacing;
  ys = AIR_EXISTS(ys) ? ys : defSpacing;
  zs = AIR_EXISTS(zs) ? zs : defSpacing;
  if (!( xs != 0 && ys != 0 && zs != 0 )) {
    sprintf(err, "%s: spacings (%g,%g,%g) for axes %d,%d,%d not all "
	    "non-zero", me, xs, ys, zs, baseDim+0, baseDim+1, baseDim+2);
    biffAdd(GAGE, err); gageShapeReset(shape);
    return 1;
  }
  shape->spacing[0] = xs;
  shape->spacing[1] = ys;
  shape->spacing[2] = zs; 
  
  /* ------ set spacing-dependent filter weight scalings */
  for (i=0; i<GAGE_KERNEL_NUM; i++) {
    switch (i) {
    case gageKernel00:
    case gageKernel10:
    case gageKernel20:
      /* interpolation requires no re-weighting for non-unit spacing */
      for (ai=0; ai<=2; ai++) {
	shape->fwScale[i][ai] = 1.0;
      }
      break;
    case gageKernel11:
    case gageKernel21:
      for (ai=0; ai<=2; ai++) {
	shape->fwScale[i][ai] = 1.0/(shape->spacing[ai]);
      }
      break;
    case gageKernel22:
      for (ai=0; ai<=2; ai++) {
	shape->fwScale[i][ai] = 
	  1.0/((shape->spacing[ai])*(shape->spacing[ai]));
      }
      break;
    }
  }

  /* ------ learn lengths for bounding nrrd in bi-unit cube */
  maxLen = 0.0;
  for (ai=0; ai<=2; ai++) {
    num[ai] = (nrrdCenterNode == shape->center
	       ? shape->size[ai]-1
	       : shape->size[ai]);
    shape->volHalfLen[ai] = num[ai]*shape->spacing[ai];
    maxLen = AIR_MAX(maxLen, shape->volHalfLen[ai]);
  }
  for (ai=0; ai<=2; ai++) {
    shape->volHalfLen[ai] /= maxLen;
    shape->voxLen[ai] = 2*shape->volHalfLen[ai]/num[ai];
  }

  return 0;
}

int
gageShapeSet (gageShape *shape, Nrrd *nin, int baseDim) {
  char me[]="gageShapeSet", err[AIR_STRLEN_MED];

  if (_gageShapeSet(NULL, shape, nin, baseDim)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(GAGE, err); return 1;
  }
  return 0;
}

void
gageShapeUnitWtoI (gageShape *shape, double index[3], double world[3]) {
  int i;
  
  if (nrrdCenterNode == shape->center) {
    for (i=0; i<=2; i++) {
      index[i] = NRRD_NODE_IDX(-shape->volHalfLen[i], shape->volHalfLen[i],
			       shape->size[i], world[i]);
    }
  } else {
    for (i=0; i<=2; i++) {
      index[i] = NRRD_CELL_IDX(-shape->volHalfLen[i], shape->volHalfLen[i],
			       shape->size[i], world[i]);
    }
  }
}

void
gageShapeUnitItoW (gageShape *shape, double world[3], double index[3]) {
  int i;
  
  if (nrrdCenterNode == shape->center) {
    for (i=0; i<=2; i++) {
      world[i] = NRRD_NODE_POS(-shape->volHalfLen[i], shape->volHalfLen[i],
			       shape->size[i], index[i]);
    }
  } else {
    for (i=0; i<=2; i++) {
      world[i] = NRRD_CELL_POS(-shape->volHalfLen[i], shape->volHalfLen[i],
			       shape->size[i], index[i]);
    }
  }
}

int
gageShapeEqual (gageShape *shape1, char *_name1,
		gageShape *shape2, char *_name2) {
  char me[]="_gageShapeEqual", err[AIR_STRLEN_MED],
    *name1, *name2, what[] = "???";

  name1 = _name1 ? _name1 : what;
  name2 = _name2 ? _name2 : what;
  if (!( shape1->size[0] == shape2->size[0] &&
	 shape1->size[1] == shape2->size[1] &&
	 shape1->size[2] == shape2->size[2] )) {
    sprintf(err, "%s: dimensions of %s (%d,%d,%d) != %s's (%d,%d,%d)", me,
	    name1, shape1->size[0], shape1->size[1], shape1->size[2],
	    name2, shape2->size[0], shape2->size[1], shape2->size[2]);
    biffAdd(GAGE, err); return 0;
  }
  if (!( shape1->spacing[0] == shape2->spacing[0] &&
	 shape1->spacing[1] == shape2->spacing[1] &&
	 shape1->spacing[2] == shape2->spacing[2] )) {
    sprintf(err, "%s: spacings of %s (%g,%g,%g) != %s's (%g,%g,%g)", me,
	    name1, shape1->spacing[0], shape1->spacing[1], shape1->spacing[2],
	    name2, shape2->spacing[0], shape2->spacing[1], shape2->spacing[2]);
    biffAdd(GAGE, err); return 0;
  }
  if (!( shape1->center == shape2->center )) {
    sprintf(err, "%s: centering of %s (%s) != %s's (%s)", me,
	    name1, airEnumStr(nrrdCenter, shape1->center),
	    name2, airEnumStr(nrrdCenter, shape2->center));
    biffAdd(GAGE, err); return 0;
  }

  return 1;
}
