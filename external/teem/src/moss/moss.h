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

#ifndef MOSS_HAS_BEEN_INCLUDED
#define MOSS_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define moss_export __declspec(dllimport)
#else
#define moss_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MOSS mossBiffKey

#define MOSS_NCOL(img) (3 == (img)->dim ? (img)->axis[0].size : 1)
#define MOSS_AXIS0(img) (3 == (img)->dim ? 1 : 0)
#define MOSS_SX(img) (3 == (img)->dim \
		      ? (img)->axis[1].size \
		      : (img)->axis[0].size )
#define MOSS_SY(img) (3 == (img)->dim \
		      ? (img)->axis[2].size \
		      : (img)->axis[1].size )

enum {
  mossFlagUnknown=-1,  /* -1: nobody knows */
  mossFlagImage,       /*  0: image being sampled */
  mossFlagKernel,      /*  1: kernel(s) used for sampling */
  mossFlagLast
};
#define MOSS_FLAG_NUM      2

typedef struct {
  Nrrd *image;                         /* the image to sample */
  NrrdKernel *kernel;                  /* which kernel to use on both axes */
  double kparm[NRRD_KERNEL_PARMS_NUM]; /* kernel arguments */
  float *ivc;                          /* intermediate value cache */
  double *xFslw, *yFslw;               /* filter sample locations->weights */
  int fdiam, ncol;                     /* filter diameter; ivc is allocated
					  for (fdiam+1) x (fdiam+1) x ncol
					  doubles, with that axis ordering */
  int *xIdx, *yIdx;                    /* arrays for x and y coordinates,
					  both allocated for fdiam */
  float *bg;                           /* background color */
  int boundary;                        /* from nrrdBoundary* enum */
  int flag[MOSS_FLAG_NUM];             /* I'm a flag-waving struct */
} mossSampler;

/* defaultsMoss.c */
extern moss_export const char *mossBiffKey;
extern moss_export int mossDefBoundary;
extern moss_export int mossDefCenter;
extern moss_export int mossVerbose;

/* methodsMoss.c */
extern mossSampler *mossSamplerNew();
extern int mossSamplerFill(mossSampler *smplr, int fdiam, int ncol);
extern void mossSamplerEmpty(mossSampler *smplr);
extern mossSampler *mossSamplerNix(mossSampler *smplr);
extern int mossImageCheck(Nrrd *image);
extern int mossImageAlloc(Nrrd *image, int type, int sx, int sy, int ncol);

/* sampler.c */
extern int mossSamplerImageSet(mossSampler *smplr, Nrrd *image, float *bg);
extern int mossSamplerKernelSet(mossSampler *smplr,
				NrrdKernel *kernel, double *kparm);
extern int mossSamplerUpdate(mossSampler *smplr);
extern int mossSamplerSample(float *val, mossSampler *smplr,
			     double xPos, double yPos);

/* hestMoss.c */
extern moss_export hestCB *mossHestTransform;
extern moss_export hestCB *mossHestOrigin;

/* xform.c */
extern void mossMatPrint(FILE *f, double *mat);
extern double *mossMatRightMultiply(double *mat, double *x);
extern double *mossMatLeftMultiply (double *mat, double *x);
extern double *mossMatInvert(double *inv, double *mat);
extern double *mossMatIdentitySet(double *mat);
extern double *mossMatTranslateSet(double *mat, double tx, double ty);
extern double *mossMatRotateSet(double *mat, double angle);
extern double *mossMatFlipSet(double *mat, double angle);
extern double *mossMatShearSet(double *mat, double angleFixed,
			       double amount);
extern double *mossMatScaleSet(double *mat, double sx, double sy);
extern void mossMatApply(double *ox, double *oy, double *mat,
			 double ix, double iy);
extern int mossLinearTransform(Nrrd *nout, Nrrd *nin, float *bg,
			       double *mat,
			       mossSampler *msp,
			       double xMin, double xMax,
			       double yMin, double yMax,
			       int sx, int sy);


#ifdef __cplusplus
}
#endif

#endif /* MOSS_HAS_BEEN_INCLUDED */
