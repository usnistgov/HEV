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

#define INFO "Estimate tensors from a set of DW images"
char *_tend_estimInfoL =
  (INFO
   ". The various DWI volumes must be stacked along axis 0 (as with the "
   "output of \"tend epireg\").  The tensor coefficient weightings associated "
   "with "
   "each of the DWIs, the B-matrix, is given as a seperate array, "
   "see \"tend bmat\" usage "
   "info for details.  A \"confidence\" value is computed with the tensor, "
   "based on a soft thresholding of the sum of all the DWIs, according to "
   "the threshold and softness parameters. ");

int
tend_estimMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd **nin, *nbmat, *nterr=NULL, *nB0=NULL, *nout;
  char *outS, *terrS, *eb0S;
  float thresh, soft, b, scale;
  int ninLen, eret, knownB0;

  hestOptAdd(&hopt, "ee", "filename", airTypeString, 1, 1, &terrS, "",
	     "Giving a filename here allows you to save out the tensor estimation "
	     "error: a value which measures how much error there is between "
	     "the tensor model and the given diffusion weighted measurements "
	     "for each sample.  By default, no such error calculation is "
	     "saved.");
  hestOptAdd(&hopt, "eb", "filename", airTypeString, 1, 1, &eb0S, "",
	     "In those cases where there is no B=0 reference image given "
	     "(\"-knownB0 false\"), "
	     "giving a filename here allows you to save out the B=0 image "
	     "which is estimated from the data.  By default, this image value "
	     "is estimated but not saved.");
  hestOptAdd(&hopt, "t", "thresh", airTypeFloat, 1, 1, &thresh, "nan",
	     "value at which to threshold the mean DWI value per pixel "
	     "in order to generate the \"confidence\" mask.  By default, "
	     "the threshold value is calculated automatically, based on "
	     "histogram analysis.");
  hestOptAdd(&hopt, "soft", "soft", airTypeFloat, 1, 1, &soft, "0",
	     "how fuzzy the confidence boundary should be.  By default, "
	     "confidence boundary is perfectly sharp");
  hestOptAdd(&hopt, "scale", "scale", airTypeFloat, 1, 1, &scale, "1",
	     "Ffter estimating the tensor, scale all of its elements "
	     "(but not the confidence value) by this amount.  Can help with "
	     "downstream numerical precision if values are very large "
	     "or small.");
  hestOptAdd(&hopt, "B", "B-list", airTypeOther, 1, 1, &nbmat, NULL,
	     "6-by-N list of B-matrices characterizing "
	     "the diffusion weighting for each "
	     "image.  \"tend bmat\" is one source for such a matrix; see "
	     "its usage info for specifics on how the coefficients of "
	     "the B-matrix are ordered. "
	     "An unadorned plain text file is a great way to "
	     "specify the B-matrix", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "b", "b", airTypeFloat, 1, 1, &b, "1",
	     "additional b scaling factor ");
  hestOptAdd(&hopt, "knownB0", "bool",
	     airTypeBool, 1, 1, &knownB0, NULL,
	     "Determines of the B=0 non-diffusion-weighted reference image "
	     "is known, or if it has to be estimated along with the tensor "
	     "elements.\n "
	     "\b\bo if \"true\": the B=0 image is "
	     "the FIRST input image given to \"-i\", and hence the B-matrix "
	     "has ONE LESS row than the number of of input images.\n "
	     "\b\bo if \"false\": there is no \"reference\" image; "
	     "all the input "
	     "images are diffusion-weighted in some way or another, and there "
	     "exactly as many rows in the B-matrix as there are input images");
  hestOptAdd(&hopt, "i", "dwi0 dwi1", airTypeOther, 1, -1, &nin, NULL,
	     "all the diffusion-weighted images (DWIs), as seperate 3D nrrds, "
	     "**OR**: One 4D nrrd of all DWIs stacked along axis 0",
	     &ninLen, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
	     "output tensor volume");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_estimInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (1 == ninLen) {
    eret = tenEstimateLinear4D(nout, airStrlen(terrS) ? &nterr : NULL, &nB0,
			       nin[0], nbmat, knownB0, thresh, soft, b);
  } else {
    eret = tenEstimateLinear3D(nout, airStrlen(terrS) ? &nterr : NULL, &nB0,
			       nin, ninLen, nbmat, knownB0, thresh, soft, b);
  }
  if (1 != scale) {
    if (tenSizeScale(nout, nout, scale)) {
      airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble doing scaling:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (eret) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making tensor volume:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  if (nterr) {
    if (nrrdSave(terrS, nterr, NULL)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble writing error image:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }
  if (!knownB0 && airStrlen(eb0S)) {
    if (nrrdSave(eb0S, nB0, NULL)) {
      airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble writing estimated B=0 image:\n%s\n", me, err);
      airMopError(mop); return 1;
    }
  }

  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
/* TEND_CMD(estim, INFO); */
unrrduCmd tend_estimCmd = { "estim", INFO, tend_estimMain };
