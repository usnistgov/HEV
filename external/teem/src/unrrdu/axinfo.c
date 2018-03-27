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

#define INFO "Modify attributes of an axis"
char *_unrrdu_axinfoInfoL =
(INFO
 ". The only attributes which are set are those for which command-line "
 "options are given.");

int
unrrdu_axinfoMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *out, *err, *label;
  Nrrd *nin, *nout;
  int axis, pret;
  double mm[2], spc;
  airArray *mop;

  OPT_ADD_AXIS(axis, "dimension (axis index) to modify");
  hestOptAdd(&opt, "l", "label", airTypeString, 1, 1, &label, "",
	     "label to associate with axis");
  hestOptAdd(&opt, "mm", "min max", airTypeDouble, 2, 2, mm, "nan nan",
	     "min and max values along axis");
  hestOptAdd(&opt, "sp", "spacing", airTypeDouble, 1, 1, &spc, "nan",
	     "spacing between samples along axis");
  /* HEY: this is currently a fundamental (but only rarely annoying)
     problem in hest.  Because there is functionally no difference
     between whether an option's information comes from the default
     string or from the command-line, there is no real way to tell
     hest, "hey, its just fine for this option to not be used, and
     if its not used, DON'T DO ANYTHING".  The games of setting strings
     to "" and floats/doubles to NaN are ways of compensating for
     this.  However, there is no analogous trick for airEnums.
  hestOptAdd(&opt, "c", "center", airTypeEnum, 1, 1, &cent, "unknown",
	     "centering of axis: \"cell\" or \"node\"", 
	     NULL, nrrdCenter);
  */
  OPT_ADD_NIN(nin, "input nrrd");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_axinfoInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (!AIR_IN_CL(0, axis, nin->dim-1)) {
    fprintf(stderr, "%s: axis %d not in valid range [0,%d]\n", 
	    me, axis, nin->dim-1);
    airMopError(mop);
    return 1;
  }
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nout, nin)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error copying input:\n%s", me, err);
    airMopError(mop);
    return 1;
  }
  
  if (strlen(label)) {
    AIR_FREE(nout->axis[axis].label);
    nout->axis[axis].label = airStrdup(label);
  }
  if (AIR_EXISTS(mm[0])) {
    nout->axis[axis].min = mm[0];
  }
  if (AIR_EXISTS(mm[1])) {
    nout->axis[axis].max = mm[1];
  }
  if (AIR_EXISTS(spc)) {
    nout->axis[axis].spacing = spc;
  }
  /* see above
  if (nrrdCenterUnknown != cent) {
    nout->axis[axis].center = cent;
  }
  */

  SAVE(out, nout, NULL);

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(axinfo, INFO);
