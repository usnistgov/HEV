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

#ifndef ALAN_HAS_BEEN_INCLUDED
#define ALAN_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define alan_export __declspec(dllimport)
#else
#define alan_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ALAN alanBiffKey
#define ALAN_THREAD_MAX 256

enum {
  alanTextureTypeUnknown,
  alanTextureTypeTuring,
  alanTextureTypeGrayScott,
  alanTextureTypeLast
};

enum {
  alanParmUnknown,
  alanParmVerbose,
  alanParmTextureType,
  alanParmNumThreads,
  alanParmFrameInterval,
  alanParmSaveInterval,
  alanParmMaxIteration,
  alanParmRandRange,
  alanParmSpeed,
  alanParmDiffA,
  alanParmDiffB,
  alanParmK,
  alanParmF,
  alanParmH,
  alanParmMinAverageChange,
  alanParmMaxPixelChange,
  alanParmAlpha,
  alanParmBeta,
  alanParmConstantFilename,
  alanParmWrapAround,
  alanParmLast
};

enum {
  alanStopUnknown=0,
  alanStopNot,          /* 1 */
  alanStopMaxIteration, /* 2 */
  alanStopNonExist,     /* 3 */
  alanStopConverged,    /* 4 */
  alanStopDiverged,     /* 5 */
  alanStopLast
};
#define ALAN_STOP_MAX      5

/* all morphogen values are stored as
** 1: floats
** 0: doubles
*/
#if 1
typedef float alan_t;
#define alan_nt nrrdTypeFloat
#define ALAN_FLOAT 1
#else 
typedef double alan_t;
#define alan_nt nrrdTypeDouble
#define ALAN_FLOAT 0
#endif

typedef struct alanContext_t {
  /* INPUT ----------------------------- */
  int verbose,
    wrap,             /* do toroidal boundary wrapping */
    textureType,      /* what kind are we (from alanTextureType* enum) */
    dim,              /* either 2 or 3 */
    size[3],          /* number of texels in X, Y, (Z) */
    oversample,       /* oversampling of tensors to texels */
    numThreads,       /* # of threads, if airThreadCapable */
    frameInterval,    /* # of iterations between which to an image */
    saveInterval,     /* # of iterations between which to save all state */
    maxIteration,     /* cap on # of iterations */
    constFilename;    /* always use the same filename when saving frames */
  alan_t K, F,        /* simulation variables */
    H,                /* size of spatial grid discretization */
    minAverageChange, /* min worthwhile "avergageChange" value (see below),
			 assume convergence if it falls below this */
    maxPixelChange,   /* maximum allowed change in the first morphogen (on
			 any single pixels), assume unstable divergence if 
			 this is exceeded */
    alpha, beta,      /* variables for turing */
    speed,            /* euler integration step size */
    initA, initB,     /* initial (constant) values for each morphogen */
    diffA, diffB,     /* base diffusion rates for each morphogen */
    randRange;        /* amplitude of noise to destabalize Turing */
  Nrrd *nten;         /* tensors guiding texture.  May have 1+3 or 1+6 values
			 per sample, depending on dim */
  /* if non-NULL, this is called once per iteration, at its completion */
  int (*perIteration)(struct alanContext_t *, int iter);

  /* INTERNAL -------------------------- */
  int iter;           /* current iteration */
  Nrrd *_nlev[2],     /* levels of morphogens, alternating buffers */
    *nlev;            /* pointer to last iterations output */
  Nrrd *nparm;        /* alpha, beta values for all texels */
  alan_t 
    averageChange;    /* average amount of "change" in last iteration */
  int changeCount;    /* # of contributions to averageChange */
                      /* to control update of averageChange and changeCount */
  airThreadMutex *changeMutex;  
                      /* to synchronize seperate iterations of simulation */
  airThreadBarrier *iterBarrier;

  /* OUTPUT ---------------------------- */
  int stop;          /* why we stopped */
} alanContext;

/* methodsAlan.c */
extern alan_export const char *alanBiffKey;
extern alanContext *alanContextNew();
extern alanContext *alanContextNix(alanContext *actx);
extern int alanDimensionSet(alanContext *actx, int dim);
extern int alan2DSizeSet(alanContext *actx, int sizeX, int sizeY);
extern int alan3DSizeSet(alanContext *actx, int sizeX, int sizeY, int sizeZ);
extern int alanTensorSet(alanContext *actx, Nrrd *nten, int oversample);
extern int alanParmSet(alanContext *actx, int whichParm, double parm);

/* enumsAlan.c */
extern alan_export airEnum *alanStop;

/* coreAlan.c */
extern int alanUpdate(alanContext *actx);
extern int alanInit(alanContext *actx,
		    const Nrrd *nlevInit, const Nrrd *nparmInit);
extern int alanPriorityParm(alanContext *actx, const Nrrd *npri);
extern int alanRun(alanContext *actx);

#ifdef __cplusplus
}
#endif

#endif /* ALAN_HAS_BEEN_INCLUDED */
