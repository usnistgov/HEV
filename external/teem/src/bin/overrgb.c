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

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/nrrd.h>

char *overInfo = ("Composites an RGBA nrrd over "
		  "a background color, after doing gamma correction, "
		  "then quantizes to an 8-bit ppm.  Actually, the "
		  "input nrrd can have more than 4 values per pixel, "
		  "but only the first four are used.  If the RGBA nrrd "
		  "is floating point, the values are taken at face value; "
		  "if it is fixed point, the values interpreted as having "
		  "been quantized (so that 8-bit RGBA images will act as "
		  "you expect).");

int
main(int argc, char *argv[]) {
  hestOpt *hopt=NULL;
  Nrrd *nin, *nout,    /* initial input and final output */
    *ninD,             /* input converted to double */
    *nrgbaD;           /* rgba input as double */
  char *me, *outS, *errS;
  double gamma, back[3], *rgbaD, r, g, b, a;
  airArray *mop;
  int E, N, min[3], max[3], i, rI, gI, bI;
  unsigned char *outUC;

  me = argv[0];
  mop = airMopNew();
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &nin, NULL,
	     "input nrrd to composite", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "g", "gamma", airTypeDouble, 1, 1, &gamma, "1.0",
	     "gamma to apply to image data");
  hestOptAdd(&hopt, "b", "background", airTypeDouble, 3, 3, back, "0 0 0",
	     "background color to composite against; white is "
	     "1 1 1, not 255 255 255.");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS,
	     NULL, "file to write output PPM image to");
  hestParseOrDie(hopt, argc-1, argv+1, NULL, me, overInfo,
		 AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  if (!(3 == nin->dim && 4 <= nin->axis[0].size)) {
    fprintf(stderr, "%s: doesn't look like an RGBA nrrd\n", me);
    airMopError(mop); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    fprintf(stderr, "%s: can't use a %s nrrd", me,
	    airEnumStr(nrrdType, nrrdTypeBlock));
    airMopError(mop); return 1;
  }

  ninD = nrrdNew();
  airMopAdd(mop, ninD, (airMopper)nrrdNuke, airMopAlways);
  nrgbaD = nrrdNew();
  airMopAdd(mop, nrgbaD, (airMopper)nrrdNuke, airMopAlways);  
  nout=nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  E = 0;
  if (nrrdTypeIsIntegral[nin->type]) {
    if (!E) E |= nrrdUnquantize(ninD, nin, nrrdTypeDouble);
  } else if (nrrdTypeFloat == nin->type) {
    if (!E) E |= nrrdConvert(ninD, nin, nrrdTypeDouble);
  } else {
    if (!E) E |= nrrdCopy(ninD, nin);
  }  
  min[0] = min[1] = min[2] = 0;
  max[0] = 3;
  max[1] = nin->axis[1].size-1;
  max[2] = nin->axis[2].size-1;
  if (!E) E |= nrrdCrop(nrgbaD, ninD, min, max);
  if (!E) E |= nrrdPPM(nout, nin->axis[1].size, nin->axis[2].size);
  if (E) {
    fprintf(stderr, "%s: trouble:\n%s", me, errS = biffGetDone(NRRD));
    free(errS); return 1;
  }
  
  outUC = (unsigned char*)nout->data;
  rgbaD = (double *)nrgbaD->data;
  N = nin->axis[1].size * nin->axis[2].size;
  for (i=0; i<N; i++) {
    r = AIR_CLAMP(0, rgbaD[0], 1);
    g = AIR_CLAMP(0, rgbaD[1], 1);
    b = AIR_CLAMP(0, rgbaD[2], 1);
    a = AIR_CLAMP(0, rgbaD[3], 1);
    r = pow(r, 1.0/gamma);
    g = pow(g, 1.0/gamma);
    b = pow(b, 1.0/gamma);
    r = a*r + (1-a)*back[0];
    g = a*g + (1-a)*back[1];
    b = a*b + (1-a)*back[2];
    AIR_INDEX(0.0, r, 1.0, 256, rI);
    AIR_INDEX(0.0, g, 1.0, 256, gI);
    AIR_INDEX(0.0, b, 1.0, 256, bI);
    outUC[0] = rI;
    outUC[1] = gI;
    outUC[2] = bI;
    outUC += 3;
    rgbaD += 4;
  }

  if (nrrdSave(outS, nout, NULL)) {
    fprintf(stderr, "%s: trouble:\n%s", me, errS = biffGetDone(NRRD));
    free(errS); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
