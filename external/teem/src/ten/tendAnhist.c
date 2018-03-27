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

#define INFO "Generate barycentric histograms of anisotropy"
char *_tend_anhistInfoL =
  (INFO
   ".  The barycentric space used is either one of Westin's "
   "triple of spherical, linear, and planar anisotropy.  The bin "
   "counts in the histogram are weighted by the confidence value.");

int
tend_anhistMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int version, res;
  Nrrd *nin, *nout;
  char *outS;

  hestOptAdd(&hopt, "v", "westin version", airTypeInt, 1, 1, &version, "1",
	     "Which version of Westin's anisotropy metric triple "
	     "to use, either \"1\" or \"2\"");
  hestOptAdd(&hopt, "r", "res", airTypeInt, 1, 1, &res, "256",
	     "resolution of anisotropy plot");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
	     "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, NULL,
	     "output image (floating point)");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_anhistInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (tenAnisoHistogram(nout, nin, version, res)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making histogram:\n%s\n", me, err);
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
/* TEND_CMD(anhist, INFO); */
unrrduCmd tend_anhistCmd = { "anhist", INFO, tend_anhistMain };
