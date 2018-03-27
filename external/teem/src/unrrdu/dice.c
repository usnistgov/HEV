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

#define INFO "Slice *everywhere* along one axis"
char *_unrrdu_diceInfoL = 
(INFO
 ". Calls \"unu slice\" for each position "
 "along the indicated axis, and saves out a different "
 "nrrd for each position. ");

int
unrrdu_diceMain(int argc, char **argv, char *me, hestParm *hparm) {
  hestOpt *opt = NULL;
  char *base, out[512], *err, format[512];
  Nrrd *nin, *nout;
  int pos, axis, top, pret, start;
  airArray *mop;

  OPT_ADD_AXIS(axis, "axis to slice along");
  OPT_ADD_NIN(nin, "input nrrd");
  hestOptAdd(&opt, "s", "start", airTypeInt, 1, 1, &start, "0",
	     "integer value to start numbering with");
  hestOptAdd(&opt, "o", "prefix", airTypeString, 1, 1, &base, NULL,
	     "output filename prefix. Output nrrds will be saved out as "
	     "<prefix>00.nrrd, <prefix>01.nrrd, <prefix>02.nrrd, and so on "
	     "(with \"-s\" option, numbering will be different). If this is a "
	     "directory name, you probably want to end it with a \"/\".");

  mop = airMopNew();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(_unrrdu_diceInfoL);
  PARSE();
  airMopAdd(mop, opt, (airMopper)hestParseFree, airMopAlways);

  if (start < 0) {
    fprintf(stderr, "%s: given start index (%d) less than zero\n", me, start);
    airMopError(mop);
    return 1;
  }
  if (!(AIR_IN_CL(0, axis, nin->dim-1))) {
    fprintf(stderr, "%s: given axis (%d) outside range [0,%d]\n",
	    me, axis, nin->dim-1);
    airMopError(mop);
    return 1;
  }

  top = start + nin->axis[axis].size-1;
  if (top > 9999999)
    sprintf(format, "%%s%%08d.nrrd");
  else if (top > 999999)
    sprintf(format, "%%s%%07d.nrrd");
  else if (top > 99999)
    sprintf(format, "%%s%%06d.nrrd");
  else if (top > 9999)
    sprintf(format, "%%s%%05d.nrrd");
  else if (top > 999)
    sprintf(format, "%%s%%04d.nrrd");
  else if (top > 99)
    sprintf(format, "%%s%%03d.nrrd");
  else if (top > 9)
    sprintf(format, "%%s%%02d.nrrd");
  else
    sprintf(format, "%%s%%01d.nrrd");
  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  for (pos=0; pos<=top; pos++) {
    if (nrrdSlice(nout, nin, axis, pos)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error slicing nrrd:%s\n", me, err);
      airMopError(mop);
      return 1;
    }
    if (0 == pos) {
      /* See if these slices would be better saved as PNG images.
	 Altering the file name will tell nrrdSave() to use a different
	 file format. */
      if (nrrdFormatPNG->fitsInto(nout, nrrdEncodingRaw, AIR_FALSE)) {
	strcpy(format + strlen(format) - 4, "png");
      }
    }
    sprintf(out, format, base, pos+start);
    fprintf(stderr, "%s: %s ...\n", me, out);
    if (nrrdSave(out, nout, NULL)) {
      airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
      fprintf(stderr, "%s: error writing nrrd to \"%s\":%s\n", me, out, err);
      airMopError(mop);
      return 1;
    }
  }

  airMopOkay(mop);
  return 0;
}

UNRRDU_CMD(dice, INFO);
