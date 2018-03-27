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

#define INFO "Calculate tensors from DW images"
char *_tend_calcInfoL =
  (INFO
   ".  The tensors are calculated everwhere but the threshold and slope "
   "arguments determine what the confidence values are.  The output is a "
   "volume with 7 values per float: confidence, Dxx, Dxy, Dxz, Dyy, Dyz, "
   "Dzz.  The confidence values range from 0.0 to 1.0");

int
tend_calcMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret, version;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  Nrrd *nin, *nout;
  char *outS;
  float thresh, slope, b;

  hestOptAdd(&hopt, "t", "thresh", airTypeFloat, 1, 1, &thresh, NULL,
	     "confidence threshold");
  hestOptAdd(&hopt, "s", "slope", airTypeFloat, 1, 1, &slope, "0.01",
	     "d(confidence)/dv at threshold");
  hestOptAdd(&hopt, "b", "b", airTypeFloat, 1, 1, &b, "1",
	     "b value from scan");
  hestOptAdd(&hopt, "v", "version", airTypeInt, 1, 1, &version, "1",
	     "which set of gradient directions are used, either 1 or 2");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
	     "input volume of diffusion-weighted images",
	     NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
	     "output image (floating point)");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_calcInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (tenCalcTensor(nout, nin, version, thresh, slope, b)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: tenCalcTensor failed:\n%s", me, err);
    airMopError(mop); return 1;
  }
  if (nrrdSave(outS, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
TEND_CMD(calc, INFO);
