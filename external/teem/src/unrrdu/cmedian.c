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

#define INFO "Cheap histogram-based median/mode filtering"
char *_unrrdu_cmedianInfoL =
(INFO
 ". Only works on 1, 2, or 3 dimensions.  The window "
 "over which filtering is done is always square, and "
 "only a simplistic weighting scheme is available. "
 "The method is cheap because it does the median "
 "or mode based on a histogram, which enforces a quantization to the number "
 "of bins in the histogram, which probably means a loss of precision for "
 "anything except 8-bit data.  Also, 8-bit integer values can be recovered "
 "exactly only when the number of bins is exactly min-max+1 (as reported "
 "by \"unu minmax\"). ");

int
unrrdu_cmedianMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *ntmp, *nout;
  int bins, radius, pad, pret, mode;
  airArray *mop;
  float wght;

  hestOptAdd(&opt, "r", "radius", airTypeInt, 1, 1, &radius, NULL,
	     "how big a window to filter over. \"-r 1\" leads to a "
	     "3x3 window in an image, and a 3x3x3 window in a volume");
  hestOptAdd(&opt, "mode", NULL, airTypeInt, 0, 0, &mode, NULL,
	     "By default, median filtering is done.  Using this option "
	     "enables mode filtering, in which the most common value is "
	     "used as output");
  hestOptAdd(&opt, "b", "bins", airTypeInt, 1, 1, &bins, "256",
	     "# of bins in histogram.  It is in your interest to minimize "
	     "this number, since big histograms mean slower execution "
	     "times.  8-bit data needs at most 256 bins.");
  hestOptAdd(&opt, "w", "weight", airTypeFloat, 1, 1, &wght, "1.0",
	     "How much higher to preferentially weight samples that are "
	     "closer to the center of the window.  \"1.0\" weight means that "
	     "all samples are uniformly weighted over the window, which "
	     "facilitates a simple speed-up. ");
  hestOptAdd(&opt, "p", NULL, airTypeInt, 0, 0, &pad, NULL,
	     "Pad the input (with boundary method \"bleed\"), "
	     "and crop the output, so as to "
	     "overcome our cheapness and correctly "
	     "handle the border.  Obviously, this takes more memory.");
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_cmedianInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);
  
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (pad) {
    ntmp=nrrdNew();
    airMopAdd(mop, ntmp, (airMopper)nrrdNuke, airMopAlways);
    if (nrrdSimplePad(ntmp, nin, radius, nrrdBoundaryBleed)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error padding:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    /* We want to free up memory now that we have a padded copy of the
       input.  We can't nuke the input because that will be a memory
       error with nrrdNuke() called by hestParseFree() called by
       airMopOkay(), but we can empty it without any harm */
    nrrdEmpty(nin);
  }
  else {
    ntmp = nin;
  }

  if (nrrdCheapMedian(nout, ntmp, mode, radius, wght, bins)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error doing cheap median:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  if (pad) {
    if (nrrdSimpleCrop(ntmp, nout, radius)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error cropping:\n%s", me, err);
      airMopError(mop);
      return 1;
    }
    SAVE(out, ntmp, NULL);
  }
  else {
    SAVE(out, nout, NULL);
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(cmedian, INFO);
