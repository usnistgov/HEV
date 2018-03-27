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

#ifndef MITE_HAS_BEEN_INCLUDED
#define MITE_HAS_BEEN_INCLUDED

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/hoover.h>

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define mite_export __declspec(dllimport)
#else
#define mite_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MITE miteBiffKey

/*
******** mite_t
** 
*/


/*
typedef float mite_t;
#define mite_nt nrrdTypeFloat
#define mite_at airTypeFloat
*/

typedef double mite_t;
#define mite_nt nrrdTypeDouble
#define mite_at airTypeDouble


enum {
  miteRangeUnknown = -1,
  miteRangeAlpha,        /* 0: "A", opacity */
  miteRangeRed,          /* 1: "R" */
  miteRangeGreen,        /* 2: "G" */
  miteRangeBlue,         /* 3: "B" */
  miteRangeEmissivity,   /* 4: "E" */
  miteRangeKa,           /* 5: "a", phong ambient term */
  miteRangeKd,           /* 6: "d", phong diffuse */
  miteRangeKs,           /* 7: "s", phong specular */
  miteRangeSP,           /* 8: "p", phong specular power */
  miteRangeLast
};
#define MITE_RANGE_NUM      9

/*
** There are some constraints on how the nrrd as transfer function is
** stored and used:
** 1) all transfer functions are lookup tables: there is no 
** interpolation other than nearest neighbor (actually, someday linear
** interpolation may be supported, but that's it)
** 2) regardless of the centerings of the axes of nxtf, the lookup table
** axes will be treated as though they were cell centered (linear 
** interpolation will always use node centering)
** 3) the logical dimension of the transfer function is always one less
** ntxf->dim, with axis 0 always for the range of the function, and axes
** 1 and onwards for the domain.  For instance, a univariate opacity map
** is 2D, with ntxf->axis[0].size == 1.
** 
** So, ntxf->dim-1 is the number of variables in the domain of the transfer
** function, and ntxf->axis[0].size is the number of variables in the range.
*/

/*
******** miteUser struct
**
** all the input parameters for mite specified by the user, as well as
** a mop for cleaning up memory used during rendering.  Currently,
** unlike gage, there is no API for setting these- you go in the
** struct and set them yourself. 
*/
typedef struct {
  Nrrd *nin,             /* volume being rendered */
    **ntxf,              /* array of nrrds containing transfer functions,
			    these are never altered (in contrast to ntxf
			    in miteRender) */
    *nout;               /* output image container: we'll nrrdMaybeAlloc all
			    the image data and put it in here, but we won't
			    nrrdNuke(nout), just like we won't nrrdNuke
			    nin or any of the ntxf[i] */
  int ntxfNum;           /* allocated and valid length of ntxf[] */
  /* for each possible element of the txf range, what value should it
     start at prior to multiplying by the values (if any) learned from
     the txf.  Mainly needed to store non-unity values for the
     quantities not covered by a transfer function */
  mite_t rangeInit[MITE_RANGE_NUM]; 
  double refStep,        /* length of "unity" for doing opacity correction */
    rayStep,             /* distance between sampling planes */
    near1;               /* close enough to unity for the sake of doing early
			    ray termination when opacity gets high */
  hooverContext *hctx;   /* context and input for all hoover-related things,
			    including camera and image parameters */
  /* local initial copies of kernels, later passed to gageKernelSet */
  NrrdKernelSpec *ksp[GAGE_KERNEL_NUM];
  gageContext *gctx0;    /* context and input for all gage-related things,
			    including all kernels.  This is gageContextCopy'd
			    for multi-threaded use (hence the 0) */
  limnLight *lit;        /* a struct for all lighting info */
  int normalSide,        /* determines direction of gradient that is used
			    as normal for shading:
			    1 for normal pointing to lower values (higher
			    values are more "inside"); -1 for normal
			    pointing to higher values (low values more
			    "inside") */
    justSum,             /* don't use opacity: just sum colors */
    noDirLight;          /* forget directional phong lighting, using only
			    the ambient component */
  airArray *umop;        /* for things allocated which are used across
			    multiple renderings */
  /* output information from last rendering */
  double rendTime,       /* rendering time, in seconds */
    sampRate;            /* rate (KHz) at which sampler callback was called */
} miteUser;

struct miteThread_t;

/*
******** miteRender
**
** rendering-parameter-set-specific, but non-thread-specific,
** state relevant for mite's internal use
*/
typedef struct {
  Nrrd **ntxf;                /* array of transfer function nrrds.  The only
				 difference from those in miteUser is that 
				 opacity correction has been applied to
				 these */
  int ntxfNum;                /* allocated and valid length of ntxf[] */
  double time0;               /* rendering start time */

  /* as long as there's no mutex around how the miteThreads are
     airMopAdded to the miteUser's mop, these have to be _allocated_ in
     mrendRenderBegin, instead of mrendThreadBegin, which still has the
     role of initializing them */
  struct miteThread_t *tt[HOOVER_THREAD_MAX];  
  airArray *rmop;             /* for things allocated which are rendering
				 (or rendering parameter) specific */
} miteRender;

typedef struct {
  gage_t *val;                  /* the gage-measured txf axis variable */
  int size;                     /* number of entries */
  double min, max;              /* min, max (copied from nrrd axis) */
  mite_t *data;                 /* pointer to txf data.  If non-NULL, the
				   rest of the variables are meaningful */
  int rangeIdx[MITE_RANGE_NUM], /* indices into miteThread's range */
    rangeNum;                   /* number of range variables set by the txf
				   == number of pointers in range[] to use */
} miteStage;

/*
******** miteScl* enum
** 
** the quantities not measured by gage which can appear in the
** transfer function domain.  All of these are cheap to compute,
** so all of them are saved/computed per sample, and stored in
** the miteThread
*/
enum {
  miteSclUnknown=-1,    /* -1: nobody knows */
  miteSclXw,            /*  0: "Xw", X position, world space */
  miteSclXi,            /*  1: "Xi", X     "   , index   "   */
  miteSclYw,            /*  2: "Yw", Y     "   , world   "   */
  miteSclYi,            /*  3: "Yi", Y     "   , index   "   */
  miteSclZw,            /*  4: "Zw", Z     "   , world   "   */
  miteSclZi,            /*  5: "Zi", Z     "   , index   "   */
  miteSclTw,            /*  6: "Tw", ray position */
  miteSclTi,            /*  7: "Ti", ray index (ray sample #) */
  miteSclNdotV,         /*  8: "NdotV", surface normal dotted w/ view vector
			        (towards eye) */
  miteSclNdotL,         /*  9: "NdotL", surface normal dotted w/ light vector
			        (towards the light source) */
  miteSclGTdotV,        /* 10: "GTdotV", normal curvature in view direction */
  miteSclLast
};
#define MITE_SCL_MAX       10

/*
******** miteThread
**
** thread-specific state for mite's internal use
*/
typedef struct miteThread_t {
  gageContext *gctx;            /* per-thread context */
  gage_t *ans,                  /* shortcut to gctx->pvl[0]->ans */
    *norm,                      /* shortcut to ans[gageSclNormal] */
    *nPerp,                     /* shortcut to ans[gageSclNPerp] */
    *gten,                      /* shortcut to ans[gageSclGeomTens] */
    mscl[MITE_SCL_MAX+1];       /* all the miteScl */
  int verbose,                  /* blah blah blah */
    thrid,                      /* thread ID */
    ui, vi,                     /* image coords */
    samples;                    /* number of samples handled so far
				   by this thread */
  miteStage *stage;             /* array of stages for txf computation */
  int stageNum;                 /* number of stages == length of stage[] */
  mite_t range[MITE_RANGE_NUM], /* rendering variables, which are either
				   copied from miteUser's rangeInit[], or
				   over-written by txf evaluation */
    rayStep,                    /* per-ray step, to implement sampling
				   on planes */
    V[3],                       /* per-ray view direction */
    RR, GG, BB, TT;             /* per-ray composited values */
} miteThread;

/* defaultsMite.c */
extern mite_export const char *miteBiffKey;
extern mite_export double miteDefRefStep;
extern mite_export int miteDefRenorm;
extern mite_export int miteDefNormalSide;
extern mite_export double miteDefNear1;

/* txf.c */
extern mite_export airEnum *miteScl;
extern mite_export char miteRangeChar[MITE_RANGE_NUM];
extern int miteNtxfCheck(Nrrd *ntxf, gageKind *kind);

/* user.c */
extern miteUser *miteUserNew();
extern miteUser *miteUserNix(miteUser *muu);

/* renderMite.c */
extern int miteRenderBegin(miteRender **mrrP, miteUser *muu);
extern int miteRenderEnd(miteRender *mrr, miteUser *muu);

/* thread.c */
extern int miteThreadBegin(miteThread **mttP, miteRender *mrr, miteUser *muu,
			   int whichThread);
extern int miteThreadEnd(miteThread *mtt, miteRender *mrr, miteUser *muu);

/* ray.c */
extern int miteRayBegin(miteThread *mtt, miteRender *mrr, miteUser *muu,
			int uIndex, int vIndex, 
			double rayLen,
			double rayStartWorld[3], double rayStartIndex[3],
			double rayDirWorld[3], double rayDirIndex[3]);
extern double miteSample(miteThread *mtt, miteRender *mrr, miteUser *muu,
			 int num, double rayT, int inside,
			 double samplePosWorld[3],
			 double samplePosIndex[3]);
extern int miteRayEnd(miteThread *mtt, miteRender *mrr,
		      miteUser *muu);

#ifdef __cplusplus
}
#endif

#endif /* MITE_HAS_BEEN_INCLUDED */
