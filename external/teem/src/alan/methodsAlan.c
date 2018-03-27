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


#include "alan.h"

const char *
alanBiffKey = "alan";

void
alanContextInit(alanContext *actx) {
  if (actx) {
    actx->verbose = 0;
    actx->wrap = AIR_FALSE;
    actx->textureType = alanTextureTypeUnknown;
    actx->dim = 0;
    ELL_3V_SET(actx->size, 0, 0, 0);
    actx->oversample = 0;
    actx->numThreads = 1;
    actx->frameInterval = 10;
    actx->saveInterval = 100;
    actx->maxIteration = 100000;
    actx->minAverageChange = 0.00002;
    actx->maxPixelChange = 6;
    actx->K = AIR_NAN;
    actx->F = AIR_NAN;
    actx->H = 1.25;
    actx->alpha = 16;
    actx->beta = 12;
    actx->speed = 1.0;
    actx->initA = actx->initB = 0;
    actx->diffA = actx->diffB = 0;
    actx->perIteration = NULL;
    actx->randRange = 3;
    actx->_nlev[0] = nrrdNuke(actx->_nlev[0]);
    actx->_nlev[1] = nrrdNuke(actx->_nlev[1]);
    actx->nlev = actx->_nlev[0];
    actx->nparm = nrrdNuke(actx->nparm);
    actx->nten = nrrdNuke(actx->nten);
    actx->constFilename = AIR_FALSE;
  }
  return;
}

alanContext *
alanContextNew(void) {
  alanContext *actx;

  actx = (alanContext *)calloc(1, sizeof(alanContext));
  actx->_nlev[0] = actx->_nlev[1] = NULL;
  actx->nlev = NULL;
  actx->nparm = NULL;
  actx->nten = NULL;
  alanContextInit(actx);
  return actx;
}

alanContext *
alanContextNix(alanContext *actx) {

  if (actx) {
    actx->_nlev[0] = nrrdNuke(actx->_nlev[0]);
    actx->_nlev[1] = nrrdNuke(actx->_nlev[1]);
    actx->nparm = nrrdNuke(actx->nparm);
    actx->nten = nrrdNuke(actx->nten);
    free(actx);
  }
  return NULL;
}

#define GOT_NULL \
  if (!actx) { \
    sprintf(err, "%s: got NULL pointer", me); \
    biffAdd(ALAN, err); return 1; \
  }

#define DIM_SET \
  if (0 == actx->dim) { \
    sprintf(err, "%s: dimension of texture not set", me); \
    biffAdd(ALAN, err); return 1; \
  }

int
alanDimensionSet(alanContext *actx, int dim) {
  char me[]="alanDimensionSet", err[AIR_STRLEN_MED];

  GOT_NULL;
  if (!( dim == 2 || dim == 3 )) {
    sprintf(err, "%s: dimension must be 2 or 3, not %d", me, dim);
    biffAdd(ALAN, err); return 1;
  }

  actx->dim = dim;

  return 0;
}

int
alan2DSizeSet(alanContext *actx, int sizeX, int sizeY) {
  char me[]="alan2DSizeSet", err[AIR_STRLEN_MED];

  GOT_NULL;
  DIM_SET;
  if (2 != actx->dim) {
    sprintf(err, "%s: texture not two-dimensional", me);
    biffAdd(ALAN, err); return 1;
  }
  if (!( sizeX >= 10 && sizeY >= 10 )) {
    sprintf(err, "%s: sizes (%d,%d) invalid (too small?)", me, sizeX, sizeY);
    biffAdd(ALAN, err); return 1;
  }

  actx->size[0] = sizeX;
  actx->size[1] = sizeY;
  return 0;
}

int
alan3DSizeSet(alanContext *actx, int sizeX, int sizeY, int sizeZ) {
  char me[]="alan2DSizeSet", err[AIR_STRLEN_MED];

  GOT_NULL;
  DIM_SET;
  if (3 != actx->dim) {
    sprintf(err, "%s: texture not three-dimensional", me);
    biffAdd(ALAN, err); return 1;
  }
  if (!( sizeX >= 10 && sizeY >= 10 && sizeZ >= 10 )) {
    sprintf(err, "%s: sizes (%d,%d,%d) invalid (too small?)",
	    me, sizeX, sizeY, sizeZ);
    biffAdd(ALAN, err); return 1;
  }

  actx->size[0] = sizeX;
  actx->size[1] = sizeY;
  actx->size[2] = sizeZ;
  return 0;
}

int
alanTensorSet(alanContext *actx, Nrrd *nten, int oversample) {
  char me[]="alanTensorSet", err[AIR_STRLEN_MED];

  if (!( actx && nten )) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(ALAN, err); return 1;
  }
  DIM_SET;
  if (!( oversample > 0 )) {
    sprintf(err, "%s: oversample %d invalid", me, oversample);
    biffAdd(ALAN, err); return 1;
  }
  if (2 == actx->dim) {
    if (!( 3 == nten->dim && 4 == nten->axis[0].size )) {
      sprintf(err, "%s: didn't get 3-D (4,X,Y) nrrd", me);
      biffAdd(ALAN, err); return 1;
    }
  } else {
    if (!( 4 == nten->dim && 7 == nten->axis[0].size )) {
      sprintf(err, "%s: didn't get 4-D (7,X,Y,Z) nrrd", me);
      biffAdd(ALAN, err); return 1;
    }
  }

  if (1 != oversample) {
    sprintf(err, "%s: sorry, can only handle oversample==1 now", me);
    biffAdd(ALAN, err); return 1;
  }
  actx->nten = nrrdNuke(actx->nten);
  actx->nten = nrrdNew();
  if (nrrdConvert(actx->nten, nten, alan_nt)) {
    sprintf(err, "%s: trouble converting tensors to alan_t", me);
    biffMove(ALAN, err, NRRD); return 1;
  }
  actx->size[0] = oversample*nten->axis[1].size;
  actx->size[1] = oversample*nten->axis[2].size;
  if (3 == actx->dim) {
    actx->size[2] = oversample*nten->axis[3].size;
  }

  return 0;
}

int
alanParmSet(alanContext *actx, int whichParm, double parm) {
  char me[]="alanParmSet", err[AIR_STRLEN_MED];
  int parmI;

  GOT_NULL;
  DIM_SET;
  switch (whichParm) {
  case alanParmVerbose:
    parmI = parm;
    actx->verbose = parmI;
    break;
  case alanParmTextureType:
    parmI = parm;
    switch(parmI) {
    case alanTextureTypeTuring:
      actx->initA = 4.0;
      actx->initB = 4.0;
      actx->diffA = 0.25;
      actx->diffB = 0.0625;
      break;
    case alanTextureTypeGrayScott:
      actx->initA = 1;
      actx->initB = 0;
      actx->diffA = 0.00002;
      actx->diffB = 0.00002;
      break;
    default:
      sprintf(err, "%s: texture type %d invalid", me, parmI);
      biffAdd(ALAN, err); return 1;
      break;
    }
    actx->textureType = parmI;
    break;
  case alanParmNumThreads:
    parmI = parm;
    if (!airThreadCapable) {
      fprintf(stderr, "%s: WARNING: no multi-threading available, so 1 thread "
	      "will be used, not %d\n", me, parmI);
      parmI = 1;
    }
    actx->numThreads = parmI;
    break;
  case alanParmSaveInterval:
    parmI = parm;
    actx->saveInterval = parmI;
    break;
  case alanParmFrameInterval:
    parmI = parm;
    actx->frameInterval = parmI;
    break;
  case alanParmMaxIteration:
    parmI = parm;
    actx->maxIteration = parmI;
    break;
  case alanParmConstantFilename:
    parmI = parm;
    actx->constFilename = parmI;
    break;
  case alanParmSpeed:
    actx->speed = parm;
    break;
  case alanParmDiffA:
    actx->diffA = parm;
    break;
  case alanParmDiffB:
    actx->diffB = parm;
    break;
  case alanParmRandRange:
    actx->randRange = parm;
    break;
  case alanParmK:
    actx->K = parm;
    break;
  case alanParmF:
    actx->F = parm;
    break;
  case alanParmH:
    actx->H = parm;
    break;
  case alanParmMinAverageChange:
    actx->minAverageChange = parm;
    break;
  case alanParmMaxPixelChange:
    actx->maxPixelChange = parm;
    break;
  case alanParmAlpha:
    actx->alpha = parm;
    break;
  case alanParmBeta:
    actx->beta = parm;
    break;
  case alanParmWrapAround:
    parmI = parm;
    actx->wrap = parmI;
    break;
  default:
    sprintf(err, "%s: parameter %d invalid", me, whichParm);
    biffAdd(ALAN, err); return 1;
    break;
  }

  return 0;
}
