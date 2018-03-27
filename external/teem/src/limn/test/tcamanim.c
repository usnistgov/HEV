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


#include "../limn.h"

char *info = ("Works with camanim.tcl to test camera path splines.");

#define _LIMNMAGIC "LIMN0000"

int
_limnReadCamanim(int imgSize[2], limnCamera **keycamP, double **timeP,
		 double **isoValueP, int *numKeysP, FILE *fin) {
  char me[]="_limnReadCamanim", err[AIR_STRLEN_MED];
  char line[AIR_STRLEN_HUGE];
  int lret, ki;
  double *tmp, *dwell, di, dn, df, va, vm, um, fr[3], at[3], up[3];
  airArray *mop, *camA, *dwellA, *isoValueA;
  
  if (!( 0 < (lret = airOneLine(fin, line, AIR_STRLEN_HUGE)) 
	 && !strcmp(_LIMNMAGIC, line) )) {
    sprintf(err, "%s: couldn't read first line or it wasn't \"%s\"",
	    me, _LIMNMAGIC);
    biffAdd(LIMN, err); return 1;
  }
  if (!( 0 < (lret = airOneLine(fin, line, AIR_STRLEN_HUGE))
	 && 2 == (airStrtrans(airStrtrans(line, '{', ' '), '}', ' '),
		  sscanf(line, "imgSize %d %d", imgSize+0, imgSize+1)) )) {
    sprintf(err, "%s: couldn't read second line or it wasn't "
	    "\"imgSize <sizeX> <sizeY>\"", me);
    biffAdd(LIMN, err); return 1;
  }
  
  mop = airMopNew();
  camA = airArrayNew((void **)keycamP, numKeysP, sizeof(limnCamera), 1);
  dwellA = airArrayNew((void **)&dwell, NULL, sizeof(double), 1);
  isoValueA = airArrayNew((void **)isoValueP, NULL, sizeof(double), 1);
  airMopAdd(mop, camA, (airMopper)airArrayNix, airMopAlways);
  airMopAdd(mop, dwellA, (airMopper)airArrayNuke, airMopAlways);
  airMopAdd(mop, isoValueA, (airMopper)airArrayNix, airMopAlways);

  while ( 0 < (lret = airOneLine(fin, line, AIR_STRLEN_HUGE)) ) {
    airStrtrans(airStrtrans(line, '{', ' '), '}', ' ');
    ki = airArrayIncrLen(camA, 1);
    airArrayIncrLen(dwellA, 1);
    airArrayIncrLen(isoValueA, 1);
    if (15 != sscanf(line, "isoValue %lg cam.di %lg cam.at %lg %lg %lg "
		     "cam.up %lg %lg %lg cam.dn %lg cam.df %lg cam.va %lg "
		     "relDwell %lg cam.fr %lg %lg %lg",
		     (*isoValueP)+ki, &di, at+0, at+1, at+2,
		     up+0, up+1, up+2, &dn, &df, &va,
		     dwell+ki, fr+0, fr+1, fr+2)) {
      sprintf(err, "%s: trouble parsing line %d: \"%s\"", me, ki, line);
      biffAdd(LIMN, err); airMopError(mop); return 1;
    }
    (*keycamP)[ki].neer = dn;
    (*keycamP)[ki].faar = df;
    (*keycamP)[ki].dist = di;
    ELL_3V_COPY((*keycamP)[ki].from, fr);
    ELL_3V_COPY((*keycamP)[ki].at, at);
    ELL_3V_COPY((*keycamP)[ki].up, up);
    vm = tan((M_PI/180)*(va/2))*di;
    um = vm*imgSize[0]/imgSize[1];
    (*keycamP)[ki].uRange[0] = -um;
    (*keycamP)[ki].uRange[1] = um;
    (*keycamP)[ki].vRange[0] = -vm;
    (*keycamP)[ki].vRange[1] = vm;
  }
  (*keycamP)[0].atRelative = AIR_FALSE;
  (*keycamP)[0].orthographic = AIR_FALSE;
  (*keycamP)[0].rightHanded = AIR_TRUE;

  tmp = (double*)calloc(*numKeysP, sizeof(double));
  *timeP = (double*)calloc(*numKeysP, sizeof(double));
  for (ki=0; ki<*numKeysP; ki++) {
    tmp[ki] = tan(AIR_AFFINE(-0.01, dwell[ki], 2.01, 0.0, M_PI/2));
  }
  (*timeP)[0] = 0;
  for (ki=1; ki<*numKeysP; ki++) {
    (*timeP)[ki] = (*timeP)[ki-1] + (tmp[ki-1] + tmp[ki])/2;
  } 
  for (ki=0; ki<*numKeysP; ki++) {
    (*timeP)[ki] /= (*timeP)[*numKeysP-1];
  }

  airMopOkay(mop);
  return 0;
}

int
_limnWriteCamanim(FILE *fout, int imgSize[2], double isoValue, 
		  limnCamera *cam, int numFrames) {
  /* char me[]="_limnWriteCamanim", err[AIR_STRLEN_MED]; */
  double di, vm, va;
  int fi;
  
  fprintf(fout, "%s\n", _LIMNMAGIC);
  fprintf(fout, "imgSize {%d %d}\n", imgSize[0], imgSize[1]);
  for (fi=0; fi<numFrames; fi++) {
    di = cam[fi].dist;
    vm = cam[fi].vRange[1];
    va = 2*atan2(vm, di)/(M_PI/180);
    fprintf(fout, "isoValue %g cam.di %g cam.at {%g %g %g } "
	    "cam.up {%g %g %g } cam.dn %g cam.df %g cam.va %g "
	    "relDwell 1.0 cam.fr {%g %g %g }\n",
	    isoValue, cam[fi].dist,
	    cam[fi].at[0], cam[fi].at[1], cam[fi].at[2],
	    cam[fi].up[0], cam[fi].up[1], cam[fi].up[2],
	    cam[fi].neer, cam[fi].faar, va,
	    cam[fi].from[0], cam[fi].from[1], cam[fi].from[2]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  char *me;
  hestOpt *hopt=NULL;
  airArray *mop;

  char *inS, *outS, *err;
  limnCamera *keycam, *cam;
  limnSplineTypeSpec *quatType, *posType, *distType, *uvType;
  double *time, *isoValue;
  FILE *fin, *fout;
  int N, numKeys, imgSize[2], tf;

  mop = airMopNew();
  
  me = argv[0];
  hestOptAdd(&hopt, "i", "input", airTypeString, 1, 1, &inS, NULL,
	     "keyframe output from camanim.tcl");
  hestOptAdd(&hopt, "n", "# frames", airTypeInt, 1, 1, &N, "128",
	     "number of frames in output");
  hestOptAdd(&hopt, "tf", "track from point", airTypeInt, 0, 0, &tf, NULL,
	     "track the from (eye) point, instead of the at point");
  hestOptAdd(&hopt, "q", "spline", airTypeOther, 1, 1, 
	     &quatType, "tent", "spline type for quaternions",
	     NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "p", "spline", airTypeOther, 1, 1, 
	     &posType, "tent", "spline type for at/from point",
	     NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "d", "spline", airTypeOther, 1, 1, 
	     &distType, "tent", "spline type for image plane distances",
	     NULL, NULL, limnHestSplineTypeSpec);
  hestOptAdd(&hopt, "uv", "spline", airTypeOther, 1, 1, 
	     &uvType, "tent", "spline type for image extents",
	     NULL, NULL, limnHestSplineTypeSpec);

  hestOptAdd(&hopt, "o", "output", airTypeString, 1, 1, &outS, NULL,
	     "keyframe output from camanim.tcl");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
		 me, info, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!( fin = airFopen(inS, stdin, "r") )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for reading\n", me, inS);
    airMopError(mop); return 1;
  }
  if (!( fout = airFopen(outS, stdout, "w") )) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fin, (airMopper)airFclose, airMopAlways);
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);

  if (_limnReadCamanim(imgSize, &keycam, &time, &isoValue, &numKeys, fin)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble reading keyframe file:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  cam = (limnCamera *)calloc(N, sizeof(limnCamera));
  airMopAdd(mop, cam, airFree, airMopAlways);
  if (limnCameraPathMake(cam, N, keycam, time, numKeys, tf,
			 quatType, posType, distType, uvType)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble making camera path:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  if (_limnWriteCamanim(fout, imgSize, isoValue[0], cam, N)) {
    airMopAdd(mop, err = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble writing frame file:\n%s\n", me, err);
    airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}
