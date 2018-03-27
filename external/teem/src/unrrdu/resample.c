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

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Filtering and {up,down}sampling with a seperable kernel"
char *_unrrdu_resampleInfoL =
(INFO
 ". Provides simplified access to nrrdSpatialResample() "
 "by assuming (among other things) that the same kernel "
 "is used for resampling "
 "every axis (every axis which is being resampled), and "
 "by assuming that the whole axis is being resampled "
 "(no cropping or padding).  Chances are, you should "
 "use defaults for \"-b\" and \"-v\" and worry only "
 "about the \"-s\" and \"-k\" options.  This resampling "
 "respects the difference between cell- and "
 "node-centered data.");

int
unrrdu_resampleMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int d, scaleLen, type, bb, pret;
  airArray *mop;
  float *scale;
  double padVal;
  NrrdResampleInfo *info;
  NrrdKernelSpec *unuk;

  hparm->elideSingleOtherDefault = AIR_FALSE;
  hestOptAdd(&opt, "s", "sz0", airTypeOther, 1, -1, &scale, NULL,
	     "For each axis, information about how many samples in output:\n "
	     "\b\bo \"=\": leave this axis completely untouched: no "
	     "resampling whatsoever\n "
	     "\b\bo \"x<float>\": multiply the number of input samples by "
	     "<float>, and round to the nearest integer, to get the number "
	     "of output samples.  Use \"x1\" to resample the axis but leave "
	     "the number of samples unchanged\n "
	     "\b\bo \"<int>\": exact number of output samples",
	     &scaleLen, NULL, &unrrduHestScaleCB);
  hestOptAdd(&opt, "k", "kern", airTypeOther, 1, 1, &unuk, "cubic:0,0.5",
	     "The kernel to use for resampling.  Possibilities include:\n "
	     "\b\bo \"box\": nearest neighbor interpolation\n "
	     "\b\bo \"tent\": linear interpolation\n "
	     "\b\bo \"cubic:B,C\": Mitchell/Netravali BC-family of "
	     "cubics:\n "
	     "\t\t\"cubic:1,0\": B-spline; maximal blurring\n "
	     "\t\t\"cubic:0,0.5\": Catmull-Rom; good interpolating kernel\n "
	     "\b\bo \"quartic:A\": 1-parameter family of "
	     "interpolating quartics (\"quartic:0.0834\" is most accurate)\n "
	     "\b\bo \"hann:R\": Hann (cosine bell) windowed sinc, radius R\n "
	     "\b\bo \"black:R\": Blackman windowed sinc, radius R\n "
	     "\b\bo \"gauss:S,C\": Gaussian blurring, with standard deviation "
	     "S and cut-off at C standard deviations",
	     NULL, NULL, nrrdHestKernelSpec);
  hestOptAdd(&opt, "b", "behavior", airTypeEnum, 1, 1, &bb, "bleed",
	     "How to handle samples beyond the input bounds:\n "
	     "\b\bo \"pad\": use some specified value\n "
	     "\b\bo \"bleed\": extend border values outward\n "
	     "\b\bo \"wrap\": wrap-around to other side", 
	     NULL, nrrdBoundary);
  hestOptAdd(&opt, "v", "value", airTypeDouble, 1, 1, &padVal, "0.0",
	     "for \"pad\" boundary behavior, pad with this value");
  hestOptAdd(&opt, "t", "type", airTypeOther, 1, 1, &type, "default",
	     "type to save OUTPUT as. By default (not using this option), "
	     "the output type is the same as the input type",
             NULL, NULL, &unrrduHestMaybeTypeCB);
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);
  info = nrrdResampleInfoNew();
  airMopAdd(mop, info, (airMopper)nrrdResampleInfoNix, airMopAlways);
  
  USAGE(_unrrdu_resampleInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (scaleLen != nin->dim) {
    fprintf(stderr, "%s: # sampling sizes (%d) != input nrrd dimension (%d)\n",
	    me, scaleLen, nin->dim);
    return 1;
  }
  for (d=0; d<=nin->dim-1; d++) {
    /* this may be over-written below */
    info->kernel[d] = unuk->kernel;
    switch((int)scale[0 + 2*d]) {
    case 0:
      /* no resampling */
      info->kernel[d] = NULL;
      break;
    case 1:
      /* scaling of input # samples */
      info->samples[d] = AIR_ROUNDUP(scale[1 + 2*d]*nin->axis[d].size);
      break;
    case 2:
      /* explicit # of samples */
      info->samples[d] = scale[1 + 2*d];
      break;
    }
    memcpy(info->parm[d], unuk->parm, NRRD_KERNEL_PARMS_NUM*sizeof(double));
    if (info->kernel[d] &&
	(!( AIR_EXISTS(nin->axis[d].min) && AIR_EXISTS(nin->axis[d].max))) ) {
       nrrdAxisMinMaxSet(nin, d, 
			 (nin->axis[d].center
			  ? nin->axis[d].center 
			  : nrrdDefCenter));
    }
    info->min[d] = nin->axis[d].min;
    info->max[d] = nin->axis[d].max;
  }
  info->boundary = bb;
  info->type = type;
  info->padValue = padVal;
  info->renormalize = AIR_TRUE;

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  
  if (nrrdSpatialResample(nout, nin, info)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error resampling nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(resample, INFO);
