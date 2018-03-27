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
#include <teem/limn.h>

char *emapInfo = ("Creates environment maps based on limn's 16checker "
		  "normal quantization method.  By taking into account "
		  "camera parameters, this allows for both lights in "
		  "both world and view space.  Solely out of laziness, "
		  "the nrrd format is used for specifying the lights. "
		  "For N lights, the nrrd must be a 7 x N array of floats; "
		  "the 7 values are: 0/1 (world/view space), R\tG\tB color, "
		  "X\tY\tZ position.  The most convenient way of doing this "
		  "is to put this information in a text file, one line "
		  "per light.");

int
main(int argc, char *argv[]) {
  hestOpt *hopt=NULL;
  hestParm *hparm;
  Nrrd *nlight, *nmap, *ndebug;
  char *me, *outS, *errS, *debugS;
  airArray *mop;
  float amb[3], *linfo, *debug, *map, W[3], V[3];
  int li, ui, vi, qn;
  limnLight *light;
  limnCamera *cam;
  double u, v, r, w, V2W[9];
  
  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hparm->elideSingleEmptyStringDefault = AIR_TRUE;
  cam = limnCameraNew();
  airMopAdd(mop, cam, (airMopper)limnCameraNix, airMopAlways);
  hestOptAdd(&hopt, "i", "nlight", airTypeOther, 1, 1, &nlight, NULL,
	     "input nrrd containing light information",
	     NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "amb", "ambient RGB", airTypeFloat, 3, 3, amb, "0 0 0",
	     "ambient light color");
  hestOptAdd(&hopt, "fr", "from point", airTypeDouble, 3, 3, cam->from,"1 0 0",
	     "position of camera, used to determine view vector");
  hestOptAdd(&hopt, "at", "at point", airTypeDouble, 3, 3, cam->at, "0 0 0",
	     "camera look-at point, used to determine view vector");
  hestOptAdd(&hopt, "up", "up vector", airTypeDouble, 3, 3, cam->up, "0 0 1",
	     "camera pseudo-up vector, used to determine view coordinates");
  hestOptAdd(&hopt, "rh", NULL, airTypeInt, 0, 0, &(cam->rightHanded), NULL,
	     "use a right-handed UVN frame (V points down)");
  hestOptAdd(&hopt, "o", "filename", airTypeString, 1, 1, &outS, NULL,
	     "file to write output envmap to");
  hestOptAdd(&hopt, "d", "filename", airTypeString, 1, 1, &debugS, "",
	     "Use this option to save out (to the given filename) a rendering "
	     "of the front (on the left) and back (on the right) of a sphere "
	     "as shaded with the new environment map.  U increases "
	     "right-ward, V increases downward.  The back sphere half is "
	     "rendered as though the front half was removed");
  hestParseOrDie(hopt, argc-1, argv+1, hparm, me, emapInfo,
		 AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!(nrrdTypeFloat == nlight->type &&
	2 == nlight->dim && 
	7 == nlight->axis[0].size &&
	LIMN_LIGHT_NUM >= nlight->axis[1].size)) {
    fprintf(stderr, "%s: nlight isn't valid format for light specification, "
	    "must be: float type, 2-dimensional, 7\tx\tN size, N <= %d\n",
	    me, LIMN_LIGHT_NUM);
    airMopError(mop); return 1;
  }
  light = limnLightNew();
  airMopAdd(mop, light, (airMopper)limnLightNix, airMopAlways);

  limnLightSetAmbient(light, amb[0], amb[1], amb[2]);
  for (li=0; li<nlight->axis[1].size; li++) {
    linfo = (float *)(nlight->data) + 7*li;
    limnLightSet(light, li, !!linfo[0], 
		 linfo[1], linfo[2], linfo[3], 
		 linfo[4], linfo[5], linfo[6]);
  }
  
  cam->neer = -0.000000001;
  cam->dist = 0;
  cam->faar = 0.0000000001;
  cam->atRelative = AIR_TRUE;
  if (limnCameraUpdate(cam) || limnLightUpdate(light, cam)) {
    airMopAdd(mop, errS = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: problem with camera or lights:\n%s\n", me, errS);
    airMopError(mop); return 1;
  }
  nmap=nrrdNew();
  airMopAdd(mop, nmap, (airMopper)nrrdNuke, airMopAlways);
  if (limnEnvMapFill(nmap, limnLightDiffuseCB, limnQN_16checker, light)) {
    airMopAdd(mop, errS = biffGetDone(LIMN), airFree, airMopAlways);
    fprintf(stderr, "%s: problem making environment map:\n%s\n", me, errS);
    airMopError(mop); return 1;
  }
  map = nmap->data;

  if (nrrdSave(outS, nmap, NULL)) {
    fprintf(stderr, "%s: trouble:\n%s", me, errS = biffGetDone(NRRD));
    free(errS); return 1;
  }

  if (airStrlen(debugS)) {
    ELL_34M_EXTRACT(V2W, cam->V2W);
    ndebug = nrrdNew();
    nrrdMaybeAlloc(ndebug, nrrdTypeFloat, 3, 3, 1024, 512);
    debug = ndebug->data;
    for (vi=0; vi<=511; vi++) {
      v = AIR_AFFINE(0, vi, 511, -0.999, 0.999);
      for (ui=0; ui<=511; ui++) {
	u = AIR_AFFINE(0, ui, 511, -0.999, 0.999);
	r = sqrt(u*u + v*v);
	if (r > 1) {
	  continue;
	}
	w = sqrt(1 - r*r);
	
	/* first, the near side of the sphere */
	ELL_3V_SET(V, u, v, -w);
	ELL_3MV_MUL(W, V2W, V);
	qn = limnVtoQN[limnQN_16checker](W);
	ELL_3V_COPY(debug + 3*(ui + 1024*vi), map + 3*qn);
	
	/* second, the far side of the sphere */
	ELL_3V_SET(V, u, v, w);
	ELL_3MV_MUL(W, V2W, V);
	qn = limnVtoQN[limnQN_16checker](W);
	ELL_3V_COPY(debug + 3*(ui + 512 + 1024*vi), map + 3*qn);
      }
    }
    nrrdSave(debugS, ndebug, NULL);
  }
  
  airMopOkay(mop);
  return 0;
}
