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


#include "../ten.h"

char *info = ("Sample space of tensor orientation.");

void
_cap2xyz(double xyz[3], double ca, double cp) {
  double cl, cs;
  
  cs = 1 - ca;
  cl = 1 - cs - cp;
  xyz[0] = cs*0.333 + cl*1.0 + cp*0.5;
  xyz[1] = cs*0.333 + cl*0.0 + cp*0.5;
  xyz[2] = cs*0.333 + cl*0.0 + cp*0.0;
  xyz[0] = AIR_AFFINE(0, ca, 1, 1.1*xyz[0], 0.86*xyz[0]);
  xyz[1] = AIR_AFFINE(0, ca, 1, 1.1*xyz[1], 0.86*xyz[1]);
  xyz[2] = AIR_AFFINE(0, ca, 1, 1.1*xyz[2], 0.86*xyz[2]);
}

void
washQtoM3(double m[9], double q[4]) {
  double p[4], w, x, y, z, len;

  ELL_4V_COPY(p, q);
  len = ELL_4V_LEN(p);
  ELL_4V_SCALE(p, 1.0/len, p);
  w = p[0];
  x = p[1];
  y = p[2];
  z = p[3];
  /* mathematica work implies that we should be 
     setting ROW vectors here */
  ELL_3V_SET(m+0, 
	     1 - 2*(y*y + z*z),
	     2*(x*y - w*z),
	     2*(x*z + w*y));
  ELL_3V_SET(m+3,
	     2*(x*y + w*z),
	     1 - 2*(x*x + z*z),
	     2*(y*z - w*x));
  ELL_3V_SET(m+6,
	     2*(x*z - w*y),
	     2*(y*z + w*x),
	     1 - 2*(x*x + y*y));
}

int
main(int argc, char *argv[]) {
  char *me, *err, *outS;
  hestOpt *hopt=NULL;
  airArray *mop;
  
  int xi, yi, samp;
  float *tdata;
  double p[3], xyz[3], q[4], len, hackcp, maxca;
  double ca, cp, mD[9], mRF[9], mRI[9], mT[9];
  Nrrd *nten;
  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "n", "# samples", airTypeInt, 1, 1, &samp, "4",
	     "number of glyphs along each edge of triangle");
  hestOptAdd(&hopt, "p", "x y z", airTypeDouble, 3, 3, p, NULL,
	     "location in quaternion quotient space");
  hestOptAdd(&hopt, "ca", "max ca", airTypeDouble, 1, 1, &maxca, "0.8",
	     "maximum ca to use at bottom edge of triangle");
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, "-",
	     "output file to save tensors into");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
		 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);
  
  nten = nrrdNew();
  airMopAdd(mop, nten, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdMaybeAlloc(nten, nrrdTypeFloat, 4,
		     7, 2*samp-1, samp, 3)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't allocate output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  q[0] = 1.0;
  q[1] = p[0];
  q[2] = p[1];
  q[3] = p[2];
  len = ELL_4V_LEN(q);
  ELL_4V_SCALE(q, 1.0/len, q);
  washQtoM3(mRF, q);
  ELL_3M_TRAN(mRI, mRF);
  for (yi=0; yi<samp; yi++) {
    ca = AIR_AFFINE(0, yi, samp-1, 0.04, maxca);
    hackcp = AIR_AFFINE(0, yi, samp-1, 0.04, 0);
    for (xi=0; xi<=yi; xi++) {
      cp = AIR_AFFINE(0, xi, samp-1, hackcp, maxca-0.02);
      _cap2xyz(xyz, ca, cp);
      /*
      fprintf(stderr, "%s: (%d,%d) -> (%g,%g) -> %g %g %g\n", me,
	      yi, xi, ca, cp, xyz[0], xyz[1], xyz[2]);
      */
      ELL_3M_IDENTITY_SET(mD);
      ELL_3M_DIAG_SET(mD, xyz[0], xyz[1], xyz[2]);
      ELL_3M_IDENTITY_SET(mT);
      ell_3m_post_mul_d(mT, mRI);
      ell_3m_post_mul_d(mT, mD);
      ell_3m_post_mul_d(mT, mRF);
      
      tdata = (float*)nten->data + 
	7*(2*(samp-1-xi) - (samp-1-yi) + (2*samp-1)*((samp-1-yi) + samp));
      tdata[0] = 1.0;
      TEN_MAT2LIST(tdata, mT);
    }
  }
  nten->axis[1].spacing = 1;
  nten->axis[2].spacing = 1.5;
  nten->axis[3].spacing = 1;
  
  if (nrrdSave(outS, nten, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: couldn't save output:\n%s\n", me, err);
    airMopError(mop); 
    return 1;
  }
  airMopOkay(mop);
  return 0;
}

