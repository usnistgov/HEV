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

#define INFO "Make an RGB volume from an eigenvector and an anisotropy"
char *_tend_evecrgbInfoL =
  (INFO
   ". ");

int
tend_evecrgbMain(int argc, char **argv, char *me, hestParm *hparm) {
  int pret;
  hestOpt *hopt = NULL;
  char *perr, *err;
  airArray *mop;

  int aniso, cc;
  Nrrd *nin, *nout;
  char *outS;
  float bg, gray, gamma;

  hestOptAdd(&hopt, "c", "evec index", airTypeInt, 1, 1, &cc, NULL,
	     "which eigenvector will be colored. \"0\" for the "
	     "principal, \"1\" for the middle, \"2\" for the minor");
  hestOptAdd(&hopt, "a", "aniso", airTypeEnum, 1, 1, &aniso, NULL,
	     "Which anisotropy to use for modulating the saturation "
	     "of the colors.  " TEN_ANISO_DESC,
	     NULL, tenAniso);
  hestOptAdd(&hopt, "bg", "background", airTypeFloat, 1, 1, &bg, "0",
	     "gray level to use for voxels who's confidence is zero ");
  hestOptAdd(&hopt, "gr", "gray", airTypeFloat, 1, 1, &gray, "0",
	     "the gray level to desaturate towards as anisotropy "
	     "decreases (while confidence remains 1.0)");
  hestOptAdd(&hopt, "gam", "gamma", airTypeFloat, 1, 1, &gamma, "1",
	     "gamma to use on color components");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, "-",
	     "input diffusion tensor volume", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
	     "output image (floating point)");

  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  USAGE(_tend_evecrgbInfoL);
  PARSE();
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (tenEvecRGB(nout, nin, cc, aniso, gamma, bg, gray)) {
    airMopAdd(mop, err=biffGetDone(TEN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble doing colormapping:\n%s\n", me, err);
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
TEND_CMD(evecrgb, INFO);
