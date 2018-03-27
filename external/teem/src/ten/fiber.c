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

#define TEN_FIBER_INCR 512

int
_tenFiberProbe(tenFiberContext *tfx, double wPos[3]) {
  double iPos[3];
  
  gageShapeUnitWtoI(tfx->gtx->shape, iPos, wPos);
  return gageProbe(tfx->gtx, iPos[0], iPos[1], iPos[2]);
}

int
_tenFiberCheckStop(tenFiberContext *tfx) {
  char me[]="_tenFiberCheckStop";

  if (tfx->numSteps[tfx->dir] >= TEN_FIBER_NUM_STEPS_MAX) {
    fprintf(stderr, "%s: numSteps[%d] exceeded sanity check value of %d!!\n",
	    me, tfx->dir, TEN_FIBER_NUM_STEPS_MAX);
    fprintf(stderr, "%s: Check fiber termination conditions, or recompile "
	    "with a larger value for TEN_FIBER_NUM_STEPS_MAX\n", me);
    return tenFiberStopNumSteps;
  }
  if (1 & (tfx->stop >> tenFiberStopConfidence)) {
    if (tfx->dten[0] < tfx->confThresh) {
      return tenFiberStopConfidence;
    }
  }
  if (1 & (tfx->stop >> tenFiberStopAniso)) {
    if (tfx->aniso[tfx->anisoType] < tfx->anisoThresh) {
      return tenFiberStopAniso;
    }
  }
  if (1 & (tfx->stop >> tenFiberStopNumSteps)) {
    if (tfx->numSteps[tfx->dir] >= tfx->maxNumSteps) {
      return tenFiberStopNumSteps;
    }
  }
  if (1 & (tfx->stop >> tenFiberStopLength)) {
    if (tfx->halfLen[tfx->dir] >= tfx->maxHalfLen) {
      return tenFiberStopLength;
    }
  }
  return 0;
}

void
_tenFiberAlign(tenFiberContext *tfx, double vec[3]) {

  if (!(ELL_3V_LEN(tfx->lastDir))) {
    /* this is the first step in this fiber half; first half follows
       whatever comes from eigenvector calc, second half goes opposite it */
    if (tfx->dir) {
      ELL_3V_SCALE(vec, -1, vec);
    }
  } else {
    /* we have some history in this fiber half */
    if (ELL_3V_DOT(tfx->lastDir, vec) < 0) {
      ELL_3V_SCALE(vec, -1, vec);
    }
  }
  return;
}

/*
** the _tenFiberStep_* routines put their UNIT_LENGTH output in 
** the given step[] vector
*/
void
_tenFiberStep_Evec1(tenFiberContext *tfx, double step[3]) {
  
  ELL_3V_COPY(step, tfx->evec + 3*0);
  _tenFiberAlign(tfx, step);
}

void
_tenFiberStep_TensorLine(tenFiberContext *tfx, double step[3]) {
  double cl, evec0[3], vout[3], vin[3], len;
  
  ELL_3V_COPY(evec0, tfx->evec + 3*0);
  _tenFiberAlign(tfx, evec0);

  if (ELL_3V_DOT(tfx->lastDir, tfx->lastDir)) {
    ELL_3V_COPY(vin, tfx->lastDir);
    TEN_3VLIST_MUL(vout, tfx->dten, tfx->lastDir);
    ELL_3V_NORM(vout, vout, len);
    _tenFiberAlign(tfx, vout);  /* HEY: is this needed? */
  } else {
    ELL_3V_COPY(vin, evec0);
    ELL_3V_COPY(vout, evec0);
  }

  cl = (tfx->eval[0] - tfx->eval[1])/(tfx->eval[0] + 0.00001);

  ELL_3V_SCALE_ADD3(step,
		    cl, evec0,
		    (1-cl)*(1-tfx->wPunct), vin,
		    (1-cl)*tfx->wPunct, vout);
  /* _tenFiberAlign(tfx, step); */
  ELL_3V_NORM(step, step, len);
}

void
_tenFiberStep_PureLine(tenFiberContext *tfx, double step[3]) {
  char me[]="_tenFiberStep_PureLine";
  fprintf(stderr, "%s: sorry, unimplemented!\n", me);
}

void
_tenFiberStep_Zhukov(tenFiberContext *tfx, double step[3]) {
  char me[]="_tenFiberStep_Zhukov";
  fprintf(stderr, "%s: sorry, unimplemented!\n", me);
}

void (*
_tenFiberStep[TEN_FIBER_TYPE_MAX+1])(tenFiberContext *, double *) = {
  NULL,
  _tenFiberStep_Evec1,
  _tenFiberStep_TensorLine,
  _tenFiberStep_PureLine,
  _tenFiberStep_Zhukov
};

/*
** The _tenFiberIntegrate_* routines must assume that 
** _tenFiberProbe(tfx, tfx->wPos) has just been called
*/

int
_tenFiberIntegrate_Euler(tenFiberContext *tfx, double forwDir[3]) {
  
  _tenFiberStep[tfx->fiberType](tfx, forwDir);
  ELL_3V_SCALE(forwDir, tfx->stepSize, forwDir);
  return 0;
}

int
_tenFiberIntegrate_RK4(tenFiberContext *tfx, double forwDir[3]) {
  double loc[3], k1[3], k2[3], k3[3], k4[3], c1, c2, c3, c4, h;

  h = tfx->stepSize;
  c1 = h/6.0; c2 = h/3.0; c3 = h/3.0; c4 = h/6.0;

  _tenFiberStep[tfx->fiberType](tfx, k1);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, 0.5*h, k1);
  if (_tenFiberProbe(tfx, loc)) return 1;
  _tenFiberStep[tfx->fiberType](tfx, k2);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, 0.5*h, k2);
  if (_tenFiberProbe(tfx, loc)) return 1;
  _tenFiberStep[tfx->fiberType](tfx, k3);
  ELL_3V_SCALE_ADD2(loc, 1, tfx->wPos, h, k3);
  if (_tenFiberProbe(tfx, loc)) return 1;
  _tenFiberStep[tfx->fiberType](tfx, k4);

  ELL_3V_SET(forwDir,
	     c1*k1[0] + c2*k2[0] + c3*k3[0] + c4*k4[0],
	     c1*k1[1] + c2*k2[1] + c3*k3[1] + c4*k4[1],
	     c1*k1[2] + c2*k2[2] + c3*k3[2] + c4*k4[2]);
  
  return 0;
}

int (*
_tenFiberIntegrate[TEN_FIBER_INTG_MAX+1])(tenFiberContext *tfx, double *) = {
  NULL,
  _tenFiberIntegrate_Euler,
  _tenFiberIntegrate_RK4
};

/*
******** tenFiberTrace
**
** takes a starting position in index or world space, depending on the
** value of tfx->useIndexSpace
*/
int
tenFiberTrace(tenFiberContext *tfx, Nrrd *nfiber, double start[3]) {
  char me[]="tenFiberTrace", err[AIR_STRLEN_MED];
  airArray *fptsArr[2];      /* airArrays of backward (0) and forward (1)
				fiber points */
  double *fpts[2];           /* arrays storing forward and backward
				fiber points */
  double
    tmp[3],
    iPos[3],
    forwDir[3],
    *fiber;                  /* array of both forward and backward points, 
				when finished */
  int i, ret, stop, idx;

  if (!(tfx && nfiber)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(TEN, err); return 1;
  }
  if (tfx->useIndexSpace) {
    ret = gageProbe(tfx->gtx, start[0], start[1], start[2]);
  } else {
    gageShapeUnitWtoI(tfx->gtx->shape, tmp, start);
    ret = gageProbe(tfx->gtx, tmp[0], tmp[1], tmp[2]);
  }
  if (ret) {
    sprintf(err, "%s: first gageProbe failed: %s (%d)", 
	    me, gageErrStr, gageErrNum);
    biffAdd(TEN, err); return 1;
  }
  if ((stop = _tenFiberCheckStop(tfx))) {
    /* stopped immediately at seed point, but that's not an error */
    tfx->whyNowhere = stop;
    nrrdEmpty(nfiber);
    return 0;
  }

  /* initialize the quantities which describe the fiber halves */
  tfx->halfLen[0] = tfx->halfLen[1] = 0.0;
  tfx->numSteps[0] = tfx->numSteps[1] = 0.0;
  tfx->whyStop[0] = tfx->whyStop[1] = tenFiberStopUnknown;
  tfx->whyNowhere = tenFiberStopUnknown;

  for (tfx->dir=0; tfx->dir<=1; tfx->dir++) {
    fptsArr[tfx->dir] = airArrayNew((void**)&(fpts[tfx->dir]), NULL, 
				    3*sizeof(double), TEN_FIBER_INCR);
    tfx->halfLen[tfx->dir] = 0;
    if (tfx->useIndexSpace) {
      ELL_3V_COPY(iPos, start);
      gageShapeUnitItoW(tfx->gtx->shape, tfx->wPos, iPos);
    } else {
      gageShapeUnitWtoI(tfx->gtx->shape, iPos, start);
      ELL_3V_COPY(tfx->wPos, start);
    }
    ELL_3V_SET(tfx->lastDir, 0, 0, 0);
    for (tfx->numSteps[tfx->dir] = 0; 1; tfx->numSteps[tfx->dir]++) {
      if (_tenFiberProbe(tfx, tfx->wPos)) {
	/* even if gageProbe had an error OTHER than going out of bounds,
	   we're not going to report it any differently here, alas */
	tfx->whyStop[tfx->dir] = tenFiberStopBounds;
	break;
      }
      
      if ((stop = _tenFiberCheckStop(tfx))) {
	tfx->whyStop[tfx->dir] = stop;
	break;
      }
      idx = airArrayIncrLen(fptsArr[tfx->dir], 1);
      if (tfx->useIndexSpace) {
	gageShapeUnitWtoI(tfx->gtx->shape, iPos, tfx->wPos);
	ELL_3V_COPY(fpts[tfx->dir] + 3*idx, iPos);
      } else {
	ELL_3V_COPY(fpts[tfx->dir] + 3*idx, tfx->wPos);
      }
      /* forwDir is set by this to a unit-length vector */
      if (_tenFiberIntegrate[tfx->intg](tfx, forwDir)) {
	tfx->whyStop[tfx->dir] = tenFiberStopBounds;
	break;
      }
      ELL_3V_COPY(tfx->lastDir, forwDir);
      ELL_3V_ADD2(tfx->wPos, tfx->wPos, forwDir);
      tfx->halfLen[tfx->dir] += ELL_3V_LEN(forwDir);
    }
  }

  if (nrrdMaybeAlloc(nfiber, nrrdTypeDouble, 2,
		     3, fptsArr[0]->len + fptsArr[1]->len - 1)) {
    sprintf(err, "%s: couldn't allocate fiber nrrd", me);
    biffMove(TEN, err, NRRD); return 1;
  }
  fiber = (double*)(nfiber->data);
  idx = 0;
  for (i=fptsArr[0]->len-1; i>=1; i--) {
    ELL_3V_COPY(fiber + 3*idx, fpts[0] + 3*i);
    idx++;
  }
  for (i=0; i<=fptsArr[1]->len-1; i++) {
    ELL_3V_COPY(fiber + 3*idx, fpts[1] + 3*i);
    idx++;
  }
  return 0;
}
