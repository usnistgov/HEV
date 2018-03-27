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

/* bad bad bad Gordon */
extern int _nrrdOneLine(int *lenP, NrrdIO *io, FILE *file);

#define INFO "Print out min and max values in one or more nrrds"
char *_unrrdu_minmaxInfoL =
(INFO ". Unlike other commands, this doesn't produce a nrrd.  It only "
 "prints to standard out the min and max values found in the input nrrd(s), "
 "and it also indicates if there are non-existant values.");

int
unrrdu_minmaxDoit(char *me, char *inS, FILE *fout) {
  char err[AIR_STRLEN_MED];
  Nrrd *nrrd;
  NrrdRange *range;
  airArray *mop;

  mop = airMopNew();
  airMopAdd(mop, nrrd=nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdLoad(nrrd, inS, NULL)) {
    sprintf(err, "%s: trouble loading \"%s\"", me, inS);
    biffMove(me, err, NRRD); airMopError(mop); return 1;
  }

  range = nrrdRangeNewSet(nrrd, nrrdBlind8BitRangeFalse);
  airMopAdd(mop, range, (airMopper)nrrdRangeNix, airMopAlways);
  airSinglePrintf(stderr, NULL, "min: %f\n", range->min);
  airSinglePrintf(stderr, NULL, "max: %f\n", range->max);
  if (range->hasNonExist) {
    fprintf(stderr, "# has non-existent values\n");
  }

  airMopOkay(mop);
  return 0;
}

int
unrrdu_minmaxMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *err, **inS;
  airArray *mop;
  int pret, ni, ninLen;

  mop = airMopNew();
  hestOptAdd(&opt, NULL, "nin1", airTypeString, 1, -1, &inS, NULL,
	     "input nrrd(s)", &ninLen);
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_minmaxInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  for (ni=0; ni<ninLen; ni++) {
    if (ninLen > 1) {
      fprintf(stdout, "==> %s <==\n", inS[ni]);
    }
    if (unrrdu_minmaxDoit(me, inS[ni], stdout)) {
      airMopAdd(mop, err = biffGetDone(me), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble with \"%s\":\n%s",
	      me, inS[ni], err);
      /* continue working on the remaining files */
    }
    if (ninLen > 1 && ni < ninLen-1) {
      fprintf(stdout, "\n");
    }
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(minmax, INFO);
