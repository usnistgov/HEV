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

#ifndef TEN_HAS_BEEN_INCLUDED
#define TEN_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/unrrdu.h>
#include <teem/dye.h>
#include <teem/gage.h>
#include <teem/limn.h>
#include <teem/echo.h>

#include "tenMacros.h"

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define ten_export __declspec(dllimport)
#else
#define ten_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TEN tenBiffKey

enum {
  tenAnisoUnknown,    /*  0: nobody knows */
  tenAniso_Cl1,       /*  1: Westin's linear (first version) */
  tenAniso_Cp1,       /*  2: Westin's planar (first version) */
  tenAniso_Ca1,       /*  3: Westin's linear + planar (first version) */
  tenAniso_Cs1,       /*  4: Westin's spherical (first version) */
  tenAniso_Ct1,       /*  5: gk's anisotropy type (first version) */
  tenAniso_Cl2,       /*  6: Westin's linear (second version) */
  tenAniso_Cp2,       /*  7: Westin's planar (second version) */
  tenAniso_Ca2,       /*  8: Westin's linear + planar (second version) */
  tenAniso_Cs2,       /*  9: Westin's spherical (second version) */
  tenAniso_Ct2,       /* 10: gk's anisotropy type (second version) */
  tenAniso_RA,        /* 11: Bass+Pier's relative anisotropy */
  tenAniso_FA,        /* 12: (Bass+Pier's fractional anisotropy)/sqrt(2) */
  tenAniso_VF,        /* 13: volume fraction = 1-(Bass+Pier's volume ratio) */
  tenAniso_Q,         /* 14: radius of root circle is 2*sqrt(Q/9) 
		             (this is 9 times proper Q in cubic solution) */
  tenAniso_R,         /* 15: phase of root circle is acos(R/Q^3) */
  tenAniso_S,         /* 16: sqrt(Q^3 - R^2) */
  tenAniso_Th,        /* 17: R/Q^3 */
  tenAniso_Cz,        /* 18: Zhukov's invariant-based anisotropy metric */
  tenAniso_Det,       /* 19: plain old determinant */
  tenAniso_Tr,        /* 20: plain old trace */
  tenAnisoLast
};
#define TEN_ANISO_MAX    20

/*
******** tenGlyphType* enum
** 
** the different types of glyphs that may be used for tensor viz
*/
enum {
  tenGlyphTypeUnknown,    /* 0: nobody knows */
  tenGlyphTypeBox,        /* 1 */
  tenGlyphTypeSphere,     /* 2 */
  tenGlyphTypeCylinder,   /* 3 */
  tenGlyphTypeSuperquad,  /* 4 */
  tenGlyphTypeLast
};
#define TEN_GLYPH_TYPE_MAX   4

/*
******** tenGlyphParm struct
**
** all input parameters to tenGlyphGen
*/
typedef struct {
  /* glyphs will be shown at samples that have confidence >= confThresh,
     and anisotropy anisoType >= anisoThresh, and if nmask is non-NULL,
     then the corresponding mask value must be >= maskThresh.  If 
     onlyPositive, then samples with a non-positive eigenvalue will 
     be skipped, regardless of their purported anisotropy */
  Nrrd *nmask;
  int anisoType, onlyPositive;
  float confThresh, anisoThresh, maskThresh;

  /* glyphs have shape glyphType and size glyphScale. Superquadrics
     are tuned by sqdSharp, and things that must polygonalize do so
     according to facetRes.  Postscript rendering of glyph edges is
     governed by edgeWidth[] */
  int glyphType, facetRes;
  float glyphScale, sqdSharp;
  float edgeWidth[5];     /* same as limnOptsPS */

  /* glyphs are colored by eigenvector colEvec with the standard XYZ-RGB
     colormapping, with maximal saturation colMaxSat (use 0.0 to turn off
     coloring).  Saturation is modulated by anisotropy colAnisoType, to a
     degree controlled by colAnisoModulate (if 0, saturation is not at all
     modulated by anistropy).  Post-saturation, there is a per-channel
     gamma of colGamma. */
  int colEvec, colAnisoType;
  float colMaxSat, colIsoGray, colGamma, colAnisoModulate;

  /* if doSlice, a slice of anisotropy sliceAnisoType will be depicted
     in grayscale as a sheet of grayscale squares, one per sample. As
     with glyphs, these are thresholded by confThresh and maskThresh
     (but not anisoThresh).  Things can be lightened up with a
     sliceAnisoGamma > 1.  The squares will be at their corresponding
     sample locations, but offset by sliceOffset */
  int doSlice, sliceAxis, slicePos, sliceAnisoType;
  float sliceOffset, sliceAnisoGamma;
} tenGlyphParm;

#define TEN_ANISO_DESC \
  "All the Westin metrics come in two versions.  Currently supported:\n " \
  "\b\bo \"cl1\", \"cl2\": Westin's linear\n " \
  "\b\bo \"cp1\", \"cp2\": Westin's planar\n " \
  "\b\bo \"ca1\", \"ca2\": Westin's linear + planar\n " \
  "\b\bo \"cs1\", \"cs2\": Westin's spherical (1-ca)\n " \
  "\b\bo \"ct1\", \"ct2\": GK's anisotropy type (cp/ca)\n " \
  "\b\bo \"ra\": Basser/Pierpaoli relative anisotropy\n " \
  "\b\bo \"fa\": Basser/Pierpaoli fractional anisotropy/sqrt(2)\n " \
  "\b\bo \"vf\": volume fraction = 1-(Basser/Pierpaoli volume ratio)\n " \
  "\b\bo \"tr\": trace"

/*
******** tenGage* enum
** 
** all the possible queries supported in the tenGage gage kind
*/
enum {
  tenGageUnknown = -1,  /* -1: nobody knows */
  tenGageTensor,        /*  0: "t", the reconstructed tensor: GT[7] */
  tenGageTrace,         /*  1: "tr", trace of tensor: *GT */
  tenGageFrobTensor,    /*  2: "fro", frobenius norm of tensor: *GT */
  tenGageEval,          /*  3: "eval", eigenvalues of tensor
			       (sorted descending) : GT[3] */
  tenGageEvec,          /*  4: "evec", eigenvectors of tensor: GT[9] */
  tenGageTensorGrad,    /*  5: "tg", all tensor component gradients, starting
			       with the confidence value gradient: GT[21] */
  tenGageQ,             /*  6: "q", Q anisotropy (or 9 times it): *GT */
  tenGageQGradVec,      /*  7: "qv", gradient of Q anisotropy: GT[3] */
  tenGageQGradMag,      /*  8: "qg", grad mag of Q anisotropy: *GT */
  tenGageQNormal,       /*  9: "qn", normalized gradient of Q
			       anisotropy: GT[3] */
  tenGageMultiGrad,     /* 10: "mg", sum of outer products of the tensor 
			       matrix elements, correctly counting the
			       off-diagonal entries twice, but not counting
			       the confidence value: GT[9] */
  tenGageFrobMG,        /* 11: "frmg", frobenius norm of multi gradient: *GT */
  tenGageMGEval,        /* 12: "mgeval", eigenvalues of multi gradient: GT[3]*/
  tenGageMGEvec,        /* 13: "mgevec", eigenvectors of multi
			       gradient: GT[9] */
  tenGageAniso,         /* 14: "an", all anisotropies: GT[TEN_ANISO_MAX+1] */
  tenGageLast
};
#define TEN_GAGE_MAX       14
#define TEN_GAGE_TOTAL_ANS_LENGTH (72+TEN_ANISO_MAX+1)

/*
******** tenFiberType* enum
**
** the different kinds of fiber tractography that we do
*/
enum {
  tenFiberTypeUnknown,    /* 0: nobody knows */
  tenFiberTypeEvec1,      /* 1: standard following of principal eigenvector */
  tenFiberTypeTensorLine, /* 2: Weinstein-Kindlmann tensorlines */
  tenFiberTypePureLine,   /* 3: "pure" tensorlines- multiplication only */
  tenFiberTypeZhukov,     /* 4: Zhukov's oriented tensor reconstruction */
  tenFiberTypeLast
};
#define TEN_FIBER_TYPE_MAX   4

/*
******** tenFiberIntg* enum
**
** the different integration styles supported.  Obviously, this is more
** general purpose than fiber tracking, so this will be moved (elsewhere
** in teem) as needed
*/
enum {
  tenFiberIntgUnknown,   /* 0: nobody knows */
  tenFiberIntgEuler,     /* 1: dumb but fast */
  tenFiberIntgRK4,       /* 2: 4rth order runge-kutta */
  tenFiberIntgLast
};
#define TEN_FIBER_INTG_MAX  2

/*
******** tenFiberStop* enum
**
** the different reasons why fibers stop going (or never start)
*/
enum {
  tenFiberStopUnknown,    /* 0: nobody knows */
  tenFiberStopAniso,      /* 1: specified aniso got below specified level */
  tenFiberStopLength,     /* 2: fiber length in world space got too long */
  tenFiberStopNumSteps,   /* 3: took too many steps along fiber */
  tenFiberStopConfidence, /* 4: tensor "confidence" value went too low */
  tenFiberStopBounds,     /* 5: fiber position stepped outside volume */
  tenFiberStopLast
};
#define TEN_FIBER_STOP_MAX   5

/*
******** #define TEN_FIBER_NUM_STEPS_MAX
**
** whatever the stop criteria are for fiber tracing, no fiber half can
** have more points than this- a useful sanity check against fibers
** done amok.
*/
#define TEN_FIBER_NUM_STEPS_MAX 10240

enum {
  tenFiberParmUnknown,         /* 0: nobody knows */
  tenFiberParmStepSize,        /* 1: base step size */
  tenFiberParmUseIndexSpace,   /* 2: non-zero iff output of fiber should be 
				  seeded in and output in index space,
				  instead of default world */
  tenFiberParmWPunct,          /* 3: tensor-line parameter */
  tenFiberParmLast
};
#define TEN_FIBER_PARM_MAX        3

/*
******** tenFiberContext
**
** catch-all for input, state, and output of fiber tracing.  Luckily, like
** in a gageContext, NOTHING in here is set directly by the user; everything
** should be through the tenFiber* calls
*/
typedef struct {
  /* ---- input -------- */
  Nrrd *dtvol;          /* the volume being analyzed */
  NrrdKernelSpec *ksp;  /* how to interpolate tensor values in dtvol */
  int fiberType,        /* from tenFiberType* enum */
    intg,               /* from tenFiberIntg* enum */
    anisoType,          /* which aniso we do a threshold on */
    stop;               /* BITFLAG for different reasons to stop a fiber */
  double anisoThresh;   /* anisotropy threshold */
  int maxNumSteps,      /* max # steps allowed on one fiber half */
    useIndexSpace;      /* output in index space, not world space */
  double stepSize,      /* step size in world space */
    maxHalfLen;         /* longest propagation (forward or backward) allowed
			   from midpoint */
  double confThresh;    /* confidence threshold */
  double wPunct;        /* knob for tensor lines */
  /* ---- internal ----- */
  int query,            /* query we'll send to gageQuerySet */
    dir;                /* current direction being computed (0 or 1) */
  double wPos[3],       /* current world space location */
    wDir[3],            /* difference between this and last world space pos */
    lastDir[3];         /* previous value of wDir */
  gageContext *gtx;     /* wrapped around dtvol */
  gage_t *dten,         /* gageAnswerPointer(gtx->pvl[0], tenGageTensor) */
    *eval,              /* gageAnswerPointer(gtx->pvl[0], tenGageEval) */
    *evec,              /* gageAnswerPointer(gtx->pvl[0], tenGageEvec) */
    *aniso;             /* gageAnswerPointer(gtx->pvl[0], tenGageAniso) */
  /* ---- output ------- */
  double halfLen[2];    /* length of each fiber half in world space */
  int numSteps[2],      /* how many samples are used for each fiber half */
    whyStop[2],         /* why backward/forward (0/1) tracing stopped
			   (from tenFiberStop* enum) */
    whyNowhere;         /* why fiber never got started (from tenFiberStop*) */
} tenFiberContext;

/*
******** struct tenEmBimodalParm
**
** input and output parameters for tenEMBimodal (for fitting two
** gaussians to a histogram).  Currently, all fields are directly
** set/read; no API help here.
**
** "fraction" means prior probability
**
** In the output, material #1 is the one with the lower mean
*/
typedef struct {
  /* ----- input -------- */
  double minProb,        /* threshold for negligible posterior prob. values */
    minProb2,            /* minProb for 2nd stage fitting */
    minDelta,            /* convergence test for maximization */
    minFraction,         /* smallest fraction (in 0.0 to 1.0) that material
		 	    1 or 2 can legitimately have */
    minConfidence;       /* smallest confidence value that the model fitting
			    is allowed to have */
  int maxIteration,      /* cap on # of non-convergent iterations allowed */
    twoStage,            /* wacky two-stage fitting */
    verbose;             /* output messages and/or progress images */
  /* ----- internal ----- */
  double *histo,         /* double version of histogram */
    *pp1, *pp2,          /* pre-computed posterior probabilities for the
			    current iteration */
    vmin, vmax,          /* value range represented by histogram. This is
			    saved from given histogram, and used to inform
			    final output values, but it is not used for 
			    any intermediate histogram calculations, all of
			    which are done entirely in index space */
    delta;               /* some measure of model change between iters */
  int N,                 /* number of bins in histogram */
    stage,               /* current stage (1 or 2) */
    iteration;           /* current iteration */
  /* ----- output ------- */
  double mean1, stdv1,   /* material 1 mean and  standard dev */
    mean2, stdv2,        /* same for material 2 */
    fraction1,           /* fraction of material 1 (== 1 - fraction2) */
    confidence,          /* (mean2 - mean1)/(stdv1 + stdv2) */
    threshold;           /* minimum-error threshold */
} tenEMBimodalParm;

/* defaultsTen.c */
extern ten_export const char *tenBiffKey;
extern ten_export const char tenDefFiberKernel[];
extern ten_export double tenDefFiberStepSize;
extern ten_export int tenDefFiberUseIndexSpace;
extern ten_export double tenDefFiberMaxHalfLen;
extern ten_export int tenDefFiberAnisoType;
extern ten_export double tenDefFiberAnisoThresh;
extern ten_export int tenDefFiberIntg;
extern ten_export double tenDefFiberWPunct;

/* enumsTen.c */
extern ten_export airEnum *tenAniso;
extern ten_export airEnum _tenGage;
extern ten_export airEnum *tenGage;
extern ten_export airEnum *tenFiberType;
extern ten_export airEnum *tenFiberStop;
extern ten_export airEnum *tenGlyphType;

/* glyph.c */
extern tenGlyphParm *tenGlyphParmNew();
extern tenGlyphParm *tenGlyphParmNix(tenGlyphParm *parm);
extern int tenGlyphParmCheck(tenGlyphParm *parm, Nrrd *nten);
extern int tenGlyphGen(limnObj *glyphs, echoScene *scene,
		       Nrrd *nten, tenGlyphParm *parm);

/* tensor.c */
extern ten_export int tenVerbose;
extern int tenTensorCheck(Nrrd *nin, int wantType, int want4D, int useBiff);
extern int tenExpand(Nrrd *tnine, Nrrd *tseven, float scale, float thresh);
extern int tenShrink(Nrrd *tseven, Nrrd *nconf, Nrrd *tnine);
extern int tenEigensolve(float eval[3], float evec[9], float ten[7]);
extern void tenMakeOne(float ten[7], float conf, float eval[3], float evec[9]);
extern int tenMake(Nrrd *nout, Nrrd *nconf, Nrrd *neval, Nrrd *nevec);
extern int tenSlice(Nrrd *nout, Nrrd *nten, int axis, int pos, int dim);

/* chan.c */
/* old tenCalc* functions replaced by tenEstimate* */
extern int tenBMatrixCalc(Nrrd *nbmat, Nrrd *ngrad);
extern int tenEMatrixCalc(Nrrd *nemat, Nrrd *nbmat, int knownB0);
extern void tenEstimateLinearSingle(float *ten, float *B0P, float *dwi,
				    double *emat, int DD, int knownB0,
				    float thresh, float soft, float b);
extern int tenEstimateLinear3D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
			       Nrrd **ndwi, int dwiLen, 
			       Nrrd *nbmat, int knownB0, 
			       float thresh, float soft, float b);
extern int tenEstimateLinear4D(Nrrd *nten, Nrrd **nterrP, Nrrd **nB0P,
			       Nrrd *ndwi, Nrrd *_nbmat, int knownB0,
			       float thresh, float soft, float b);
extern void tenSimulateOne(float *dwi, float B0, float *ten,
			   double *bmat, int DD, float b);
extern int tenSimulate(Nrrd *ndwi, Nrrd *nT2, Nrrd *nten,
		       Nrrd *nbmat, float b);

/* aniso.c */
extern ten_export float tenAnisoSigma;  /* added to denominator
					   in Westin anisos */
extern void tenAnisoCalc(float c[TEN_ANISO_MAX+1], float eval[3]);
extern int tenAnisoPlot(Nrrd *nout, int aniso, int res, int whole);
extern int tenAnisoVolume(Nrrd *nout, Nrrd *nin, int aniso, float thresh);
extern int tenAnisoHistogram(Nrrd *nout, Nrrd *nin,
			     int version, int resolution);

/* miscTen.c */
extern int tenEvecRGB(Nrrd *nout, Nrrd *nin, int which, int aniso,
		      float gamma, float bgGray, float isoGray);
extern short tenEvqOne(float vec[3], float scl);
extern int tenEvqVolume(Nrrd *nout, Nrrd *nin, int which,
			int aniso, int scaleByAniso);
extern int tenGradCheck(Nrrd *ngrad);
extern int tenBMatrixCheck(Nrrd *nbmat);
extern int _tenFindValley(float *valP, Nrrd *nhist, float tweak);

/* fiberMethods.c */
extern tenFiberContext *tenFiberContextNew(Nrrd *dtvol);
extern int tenFiberTypeSet(tenFiberContext *tfx, int type);
extern int tenFiberKernelSet(tenFiberContext *tfx,
			     NrrdKernel *kern,
			     double parm[NRRD_KERNEL_PARMS_NUM]);
extern int tenFiberIntgSet(tenFiberContext *tfx, int intg);
extern int tenFiberStopSet(tenFiberContext *tfx, int stop, ...);
extern void tenFiberStopReset(tenFiberContext *tfx);
extern int tenFiberIntSet(tenFiberContext *tfx, int intType);
extern int tenFiberParmSet(tenFiberContext *tfx, int parm, double val);
extern int tenFiberUpdate(tenFiberContext *tfx);
extern tenFiberContext *tenFiberContextNix(tenFiberContext *tfx);

/* fiber.c */
extern int tenFiberTrace(tenFiberContext *tfx, Nrrd *fiber, double start[3]);

/* epireg.c */
extern int tenEpiRegister3D(Nrrd **nout, Nrrd **ndwi, int dwiLen, Nrrd *ngrad,
			    int reference,
			    float bwX, float bwY, float fitFrac, float DWthr,
			    int doCC,
			    NrrdKernel *kern, double *kparm,
			    int progress, int verbose);
extern int tenEpiRegister4D(Nrrd *nout, Nrrd *nin, Nrrd *ngrad,
			    int reference,
			    float bwX, float bwY, float fitFrac,
			    float DWthr, int doCC, 
			    NrrdKernel *kern, double *kparm,
			    int progress, int verbose);

/* mod.c */
extern int tenSizeNormalize(Nrrd *nout, Nrrd *nin, float weight[3],
			    float amount, float target);
extern int tenSizeScale(Nrrd *nout, Nrrd *nin, float amount);
extern int tenAnisoScale(Nrrd *nout, Nrrd *nin, float amount,
			 int fixDet, int makePositive);
extern int tenEigenvaluePower(Nrrd *nout, Nrrd *nin, float expo);
extern int tenEigenvalueClamp(Nrrd *nout, Nrrd *nin, float min, float max);
extern int tenEigenvalueAdd(Nrrd *nout, Nrrd *nin, float val);

/* tenGage.c */
extern ten_export gageKind *tenGageKind;

/* bimod.c */
extern tenEMBimodalParm* tenEMBimodalParmNew(void);
extern tenEMBimodalParm* tenEMBimodalParmNix(tenEMBimodalParm *biparm);
extern int tenEMBimodal(tenEMBimodalParm *biparm, Nrrd *nhisto);

/* tend{Flotsam,Anplot,Anvol,Evec,Eval,...}.c */
#define TEND_DECLARE(C) extern ten_export unrrduCmd tend_##C##Cmd;
#define TEND_LIST(C) &tend_##C##Cmd,
/* removed from below (superseded by estim): F(calc) \ */
#define TEND_MAP(F) \
F(epireg) \
F(bmat) \
F(estim) \
F(sim) \
F(make) \
F(sten) \
F(glyph) \
F(ellipse) \
F(anplot) \
F(anvol) \
F(anscale) \
F(anhist) \
F(point) \
F(slice) \
F(fiber) \
F(norm) \
F(eval) \
F(evalpow) \
F(evalclamp) \
F(evaladd) \
F(evec) \
F(evecrgb) \
F(evq) \
F(expand) \
F(shrink) \
F(satin)
TEND_MAP(TEND_DECLARE)
extern ten_export unrrduCmd *tendCmdList[]; 
extern void tendUsage(char *me, hestParm *hparm);
extern ten_export hestCB *tendFiberStopCB;

#ifdef __cplusplus
}
#endif

#endif /* TEN_HAS_BEEN_INCLUDED */



