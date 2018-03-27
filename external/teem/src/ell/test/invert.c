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

#include "../ell.h"

char *invInfo = "Tests ell_Nm_inv and ell_Nm_pseudo_inv";

int
main(int argc, char *argv[]) {
  char *me, *outS, *err;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;
  Nrrd *nin, *nmat, *ninv, *nidn;
  int (*func)(Nrrd *, Nrrd *);

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, NULL, "matrix", airTypeOther, 1, 1, &nin, NULL,
	     "transform(s) to apply to image",
	     NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, "-",
	     "file to write output nrrd to");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
		 me, invInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  ninv = nrrdNew();
  airMopAdd(mop, ninv, (airMopper)nrrdNuke, airMopAlways);
  nidn = nrrdNew();
  airMopAdd(mop, nidn, (airMopper)nrrdNuke, airMopAlways);
  nmat = nrrdNew();
  airMopAdd(mop, nmat, (airMopper)nrrdNuke, airMopAlways);
  
  nrrdConvert(nmat, nin, nrrdTypeDouble);
  func = (nmat->axis[0].size == nmat->axis[1].size
	  ? ell_Nm_inv
	  : ell_Nm_pseudo_inv);
  if (func(ninv, nmat)) {
    airMopAdd(mop, err = biffGetDone(ELL), airFree, airMopAlways);
    fprintf(stderr, "%s: problem inverting:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  if (nrrdSave(outS, ninv, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving output:\n%s\n", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  exit(0);
}

