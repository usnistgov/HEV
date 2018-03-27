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


#include "bane.h"
#include "privateBane.h"

void
_baneAxisInit(baneAxis *ax) {
  int i;

  ax->res = -1;
  ax->measr = baneMeasrUnknown;
  for (i=0; i<BANE_MEASR_PARM_NUM; i++) {
    ax->measrParm[i] = AIR_NAN;
  }
  ax->inc = baneIncUnknown;
  for (i=0; i<BANE_INC_PARM_NUM; i++) {
    ax->incParm[i] = AIR_NAN;
  }
}

baneHVolParm *
baneHVolParmNew() {
  baneHVolParm *hvp;
  int i, j;
  
  hvp = calloc(1, sizeof(baneHVolParm));
  if (hvp) {
    hvp->verbose = baneDefVerbose;
    hvp->makeMeasrVol = baneDefMakeMeasrVol;
    hvp->measrVol = NULL;
    hvp->measrVolDone = AIR_FALSE;
    _baneAxisInit(hvp->ax + 0);
    _baneAxisInit(hvp->ax + 1);
    _baneAxisInit(hvp->ax + 2);
    hvp->k3pack = AIR_TRUE;
    for(i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      hvp->k[i] = NULL;
      for (j=0; j<NRRD_KERNEL_PARMS_NUM; j++)
	hvp->kparm[i][j] = AIR_NAN;
    }
    hvp->renormalize = baneDefRenormalize;
    hvp->clip = baneClipUnknown;
    for (i=0; i<BANE_CLIP_PARM_NUM; i++) {
      hvp->clipParm[i] = AIR_NAN;
    }
    hvp->incLimit = baneDefIncLimit;
  }
  return hvp;
}

baneHVolParm *
baneHVolParmNix(baneHVolParm *hvp) {
  
  if (hvp) {
    if (hvp->measrVol) {
      nrrdNuke(hvp->measrVol);
    }
    free(hvp);
  }
  return NULL;
}

/*
******** baneHVolParmGKMSInit()
**
** The way Gordon does it.
*/
void
baneHVolParmGKMSInit(baneHVolParm *hvp) {

  if (hvp) {
    hvp->ax[0].res = 256;
    hvp->ax[0].measr = baneMeasrGradMag;
    hvp->ax[0].inc = baneIncPercentile;
    hvp->ax[0].incParm[0] = 1024;
    hvp->ax[0].incParm[1] = 0.15;
    /*
    hvp->ax[0].inc = baneIncRangeRatio;
    hvp->ax[0].incParm[0] = 1.0;
    */

    hvp->ax[1].res = 256;
    hvp->ax[1].measr = baneMeasrHess;
    hvp->ax[1].inc = baneIncPercentile;
    hvp->ax[1].incParm[0] = 1024;
    hvp->ax[1].incParm[1] = 0.25;
    /*
    hvp->ax[1].inc = baneIncRangeRatio;
    hvp->ax[1].incParm[0] = 1.0;
    */

    hvp->ax[2].res = 256;
    hvp->ax[2].measr = baneMeasrVal;
    hvp->ax[2].inc = baneIncRangeRatio;
    hvp->ax[2].incParm[0] = 1.0;

    hvp->verbose = 1;
    hvp->clip = baneClipAbsolute;
    hvp->clipParm[0] = 256;
    
    nrrdKernelParse(&(hvp->k[gageKernel00]), hvp->kparm[gageKernel00],
		    "cubic:0,0.5");  /* catmull-rom */
    nrrdKernelParse(&(hvp->k[gageKernel11]), hvp->kparm[gageKernel11],
		    "cubicd:1,0");   /* b-spline */
    nrrdKernelParse(&(hvp->k[gageKernel22]), hvp->kparm[gageKernel22],
		    "cubicdd:1,0");  /* b-spline */
  }
}
