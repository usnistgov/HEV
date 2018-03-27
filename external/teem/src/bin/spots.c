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


#include <stdio.h>

#include <teem/hest.h>
#include <teem/nrrd.h>
#include <teem/alan.h>
#include <teem/ten.h>

char *spotsInfo = ("Generate reaction-diffusion textures based on "
		   "Turing's formulation. ");

int
main(int argc, char *argv[]) {
  char *me, *err;
  hestOpt *hopt = NULL;
  airArray *mop;

  char *outS;
  alanContext *actx;
  int *size, sizeLen, fi, si, wrap, nt, cfn;
  double speed, mch, alphabeta[2], time0, time1, H;
  Nrrd *ninit=NULL, *nten=NULL, *nparm=NULL;

  me = argv[0];
  hestOptAdd(&hopt, "s", "sx sy", airTypeInt, 2, 3, &size, "128 128",
	     "size of texture, and also determines its dimension", 
	     &sizeLen);
  hestOptAdd(&hopt, "i", "tensors", airTypeOther, 1, 1, &nten, "",
	     "diffusion tensors to use for guiding the texture generation. "
	     "If used, over-rides the \"-s\" option, both for setting "
	     "texture dimension and size (in connection with \"-ovs\")",
	     NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "wrap", NULL, airTypeInt, 0, 0, &wrap, NULL,
	     "wrap edges of texture around a topological torus (which "
	     "makes a texture suitable for tiling)");
  hestOptAdd(&hopt, "ab", "alpha beta", airTypeDouble, 2, 2, alphabeta, 
	     "16.0 12.0",
	     "the growth and decay parameters appearing in the reaction "
	     "terms of the reaction-diffusion equations.  The default "
	     "values were the ones published by Turing.");
  hestOptAdd(&hopt, "dt", "time", airTypeDouble, 1, 1, &speed, "1.0",
	     "time-step size in Euler integration.  Can be larger, at "
	     "risk of hitting divergent instability.");
  hestOptAdd(&hopt, "dx", "size", airTypeDouble, 1, 1, &H, "1.3",
	     "nominal size of simulation grid element.");
  hestOptAdd(&hopt, "mch", "change", airTypeDouble, 1, 1, &mch, "0.00001",
	     "the minimum change, averaged over the whole texture, in the "
	     "first morphogen, that signifies convergence");
  hestOptAdd(&hopt, "fi", "frame inter", airTypeInt, 1, 1, &fi, "0",
	     "the number of iterations between which to save out an 8-bit "
	     "image of the texture, or \"0\" to disable such action");
  hestOptAdd(&hopt, "si", "snap inter", airTypeInt, 1, 1, &si, "0",
	     "the number of iterations between which to save out a complete "
	     "floating-point snapshot of the morphogen state, suitable for "
	     "later re-initialization, or \"0\" to disable such action");
  hestOptAdd(&hopt, "cfn", NULL, airTypeInt, 0, 0, &cfn, NULL,
	     "when saving out frames or snapshots, use a constant filename, "
	     "instead of incrementing it each save");
  hestOptAdd(&hopt, "nt", "# threads", airTypeInt, 1, 1, &nt, "1",
	     (airThreadCapable
	      ? "number of threads to use in computation"
	      : "number of \"threads\" to use in computation, which is "
	      "moot here because this teem build doesn't support "
	      "multi-threading. "));
  hestOptAdd(&hopt, "o", "nout", airTypeString, 1, 1, &outS, NULL,
	     "filename for output of final converged (two-channel) texture");
  hestParseOrDie(hopt, argc-1, argv+1, NULL,
		 me, spotsInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  actx = alanContextNew();
  airMopAdd(mop, actx, (airMopper)alanContextNix, airMopAlways);
  if (nten) {
    if (alanDimensionSet(actx, nten->dim - 1)
	|| alanTensorSet(actx, nten, 1)) {
      airMopAdd(mop, err = biffGetDone(ALAN), airFree, airMopAlways);
      fprintf(stderr, "%s: trouble setting parameters:\n%s\n", me, err);
      airMopError(mop);
      return 1;
    }
  } else {
    alanDimensionSet(actx, sizeLen);
    if (2 == sizeLen) {
      alan2DSizeSet(actx, size[0], size[1]);
    } else {
      alan3DSizeSet(actx, size[0], size[1], size[2]);
    }
  }

  if (alanParmSet(actx, alanParmVerbose, 1)
      || alanParmSet(actx, alanParmTextureType, alanTextureTypeTuring)
      || alanParmSet(actx, alanParmRandRange, 4.0)
      || alanParmSet(actx, alanParmK, 0.0125)
      || alanParmSet(actx, alanParmH, 1.2)
      || alanParmSet(actx, alanParmAlpha, alphabeta[0])
      || alanParmSet(actx, alanParmBeta, alphabeta[1])
      || alanParmSet(actx, alanParmSpeed, speed)
      || alanParmSet(actx, alanParmH, H)
      || alanParmSet(actx, alanParmMinAverageChange, mch)
      || alanParmSet(actx, alanParmSaveInterval, si)
      || alanParmSet(actx, alanParmFrameInterval, fi)
      || alanParmSet(actx, alanParmConstantFilename, cfn)
      || alanParmSet(actx, alanParmWrapAround, wrap)
      || alanParmSet(actx, alanParmNumThreads, nt)) {
    airMopAdd(mop, err = biffGetDone(ALAN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble setting parameters:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }
      
  if (alanUpdate(actx) || alanInit(actx, ninit, nparm)) {
    airMopAdd(mop, err = biffGetDone(ALAN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble initializing texture: %s\n", me, err); 
    airMopError(mop);
    return 1;
  }
  fprintf(stderr, "%s: going to run (%d threads) ...\n", me, actx->numThreads);
  time0 = airTime();
  if (alanRun(actx)) {
    airMopAdd(mop, err = biffGetDone(ALAN), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble generating texture: %s\n", me, err); 
    airMopError(mop);
    return 1;
  }
  time1 = airTime();
  fprintf(stderr, "%s: stopped after %d iterations (%g seconds): %s\n",
	  me, actx->iter, time1 - time0,
	  airEnumDesc(alanStop, actx->stop));

  if (nrrdSave(outS, actx->nlev, NULL)) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: trouble saving output:\n%s\n", me, err);
    airMopError(mop);
    return 1;
  }


  airMopOkay(mop);
  return 0;
}
