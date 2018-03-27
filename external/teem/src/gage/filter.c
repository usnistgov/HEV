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

#include "gage.h"
#include "privateGage.h"

/*
** sets the filter sample location (fsl) array based
** on probe location (xf,yf,zf)
**
** One possible rare surpise: if a filter is not continuous with 0
** at the end of its support, and if the sample location is at the
** highest possible point (xi == N-2, xf = 1.0), then the filter
** weights may not be the desired ones.  Forward differencing (via
** nrrdKernelForwDiff) is a good example of this.
*/
void
_gageFslSet (gageContext *ctx) {
  int fr, i;
  gage_t *fslx, *fsly, *fslz;
  gage_t xf, yf, zf;

  fr = GAGE_FR(ctx);
  fslx = ctx->fsl + 0*2*fr;
  fsly = ctx->fsl + 1*2*fr;
  fslz = ctx->fsl + 2*2*fr;
  xf = ctx->point.xf;
  yf = ctx->point.yf;
  zf = ctx->point.zf;
  switch (fr) {
  case 1:
    fslx[0] = xf; fslx[1] = xf-1;
    fsly[0] = yf; fsly[1] = yf-1;
    fslz[0] = zf; fslz[1] = zf-1;
    break;
  case 2:
    fslx[0] = xf+1; fslx[1] = xf; fslx[2] = xf-1; fslx[3] = xf-2;
    fsly[0] = yf+1; fsly[1] = yf; fsly[2] = yf-1; fsly[3] = yf-2;
    fslz[0] = zf+1; fslz[1] = zf; fslz[2] = zf-1; fslz[3] = zf-2;
    break;
  default:
    /* filter radius bigger than 2 */
    for (i=-fr+1; i<=fr; i++) {
      fslx[i+fr-1] = xf-i;
      fsly[i+fr-1] = yf-i;
      fslz[i+fr-1] = zf-i;
    }
    break;
  }
  return;
}

/*
** renormalize weights of a reconstruction kernel with
** constraint: the sum of the weights must equal the continuous
** integral of the kernel
*/
void
_gageFwValueRenormalize (gageContext *ctx, int wch) {
  gage_t integral, sumX, sumY, sumZ, *fwX, *fwY, *fwZ;
  int i, fd;

  fd = GAGE_FD(ctx);
  fwX = ctx->fw + 0 + fd*(0 + 3*wch);
  fwY = ctx->fw + 0 + fd*(1 + 3*wch);
  fwZ = ctx->fw + 0 + fd*(2 + 3*wch);
  integral = ctx->ksp[wch]->kernel->integral(ctx->ksp[wch]->parm);
  sumX = sumY = sumZ = 0;
  for (i=0; i<fd; i++) {
    sumX += fwX[i];
    sumY += fwY[i];
    sumZ += fwZ[i];
  }
  for (i=0; i<fd; i++) {
    fwX[i] *= integral/sumX;
    fwY[i] *= integral/sumY;
    fwZ[i] *= integral/sumZ;
  }
  return;
}

/*
** renormalize weights of a derivative kernel with
** constraint: the sum of the weights must be zero, but
** sign of individual weights must be preserved
*/
void
_gageFwDerivRenormalize (gageContext *ctx, int wch) {
  char me[]="_gageFwDerivRenormalize";
  gage_t negX, negY, negZ, posX, posY, posZ, fixX, fixY, fixZ,
    *fwX, *fwY, *fwZ;
  int i, fd;

  fd = GAGE_FD(ctx);
  fwX = ctx->fw + 0 + fd*(0 + 3*wch);
  fwY = ctx->fw + 0 + fd*(1 + 3*wch);
  fwZ = ctx->fw + 0 + fd*(2 + 3*wch);
  negX = negY = negZ = 0;
  posX = posY = posZ = 0;
  for (i=0; i<fd; i++) {
    if (fwX[i] <= 0) { negX += -fwX[i]; } else { posX += fwX[i]; }
    if (fwY[i] <= 0) { negY += -fwY[i]; } else { posY += fwY[i]; }
    if (fwZ[i] <= 0) { negZ += -fwZ[i]; } else { posZ += fwZ[i]; }
  }
  /* fix is the sqrt() of factor by which the positive values
     are too big.  negative values are scaled up by fix;
     positive values are scaled down by fix */
  fixX = sqrt(posX/negX);
  fixY = sqrt(posY/negY);
  fixZ = sqrt(posZ/negZ);
  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: fixX = % 10.4f, fixY = % 10.4f, fixX = % 10.4f\n",
	    me, (float)fixX, (float)fixY, (float)fixZ);
  }
  for (i=0; i<fd; i++) {
    if (fwX[i] <= 0) { fwX[i] *= fixX; } else { fwX[i] /= fixX; }
    if (fwY[i] <= 0) { fwY[i] *= fixY; } else { fwY[i] /= fixY; }
    if (fwZ[i] <= 0) { fwZ[i] *= fixZ; } else { fwZ[i] /= fixZ; }
  }
  return;
}

void
_gageFwSet (gageContext *ctx) {
  char me[]="_gageFwSet";
  int i, j, fd;
  gage_t *fwX, *fwY, *fwZ;

  fd = GAGE_FD(ctx);
  for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
    if (!ctx->needK[i])
      continue;
    /* we evaluate weights for all three axes with one call */
    ctx->ksp[i]->kernel->EVALN(ctx->fw + 3*fd*i, ctx->fsl,
			       3*fd, ctx->ksp[i]->parm);
  }

  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: filter weights after kernel evaluation:\n", me);
    _gagePrint_fslw(stderr, ctx);
  }
  if (ctx->parm.renormalize) {
    for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      if (!ctx->needK[i])
	continue;
      switch (i) {
      case gageKernel00:
      case gageKernel10:
      case gageKernel20:
	_gageFwValueRenormalize(ctx, i);
	break;
      default:
	_gageFwDerivRenormalize(ctx, i);
	break;
      }
    }
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s: filter weights after renormalization:\n", me);
      _gagePrint_fslw(stderr, ctx);
    }
  }

  /* fix weightings for non-unit-spacing samples */
  if (!( 1.0 == ctx->shape->spacing[0] &&
	 1.0 == ctx->shape->spacing[1] &&
	 1.0 == ctx->shape->spacing[2] )) {
    for (i=gageKernelUnknown+1; i<gageKernelLast; i++) {
      if (!ctx->needK[i])
	continue;
      if (gageKernel00 == i || gageKernel10 == i || gageKernel20 == i)
	continue;
      fwX = ctx->fw + 0 + fd*(0 + 3*i);
      fwY = ctx->fw + 0 + fd*(1 + 3*i);
      fwZ = ctx->fw + 0 + fd*(2 + 3*i);
      for (j=0; j<fd; j++) {
	fwX[j] *= ctx->shape->fwScale[i][0];
	fwY[j] *= ctx->shape->fwScale[i][1];
	fwZ[j] *= ctx->shape->fwScale[i][2];
      }
    }
    if (ctx->verbose > 1) {
      fprintf(stderr, "%s: filter weights after non-unit fix:\n", me);
      _gagePrint_fslw(stderr, ctx);
    }
  }
}

/*
** _gageLocationSet
**
** updates probe location in general context, and things which
** depend on it:
** fsl, fw
**
** (x,y,z) is index space position in the unpadded volume
**
** does NOT use biff, but returns 1 on error and 0 if all okay
** Currently only error is probing outside volume, which sets
** gageErrNum=0 and sprints message into gageErrStr.
*/
int
_gageLocationSet (gageContext *ctx, gage_t x, gage_t y, gage_t z) {
  char me[]="_gageProbeLocationSet";
  int tx, ty, tz,  /* "top" x, y, z: highest valid index in UNPADDED volume */
    xi, yi, zi;    /* computed integral positions in PADDED volume */
  gage_t xf, yf, zf;

  tx = ctx->shape->size[0] - 1;
  ty = ctx->shape->size[1] - 1;
  tz = ctx->shape->size[2] - 1;
  /* 
  ** the {x,y,z}i integral positions are first computed in unpadded
  ** space (as are the {x,y,z}f fractional positions) ... 
  */
  if (nrrdCenterNode == ctx->shape->center) {
    if (!( AIR_IN_CL(0,x,tx) && 
	   AIR_IN_CL(0,y,ty) && 
	   AIR_IN_CL(0,z,tz) )) {
      sprintf(gageErrStr, "%s: position (%g,%g,%g) outside (node-centered) "
	      "bounds [0,%d]x[0,%d]x[0,%d]",
	      me, (float)x, (float)y, (float)z, tx, ty, tz);
      gageErrNum = 0;
      return 1;
    }
    xi = x; xi -= xi == tx; xf = x - xi;
    yi = y; yi -= yi == ty; yf = y - yi;
    zi = z; zi -= zi == tz; zf = z - zi;
  } else {
    if (!( AIR_IN_CL(-0.5,x,tx+0.5) &&
	   AIR_IN_CL(-0.5,y,ty+0.5) &&
	   AIR_IN_CL(-0.5,z,tz+0.5) )) {
      sprintf(gageErrStr, "%s: position (%g,%g,%g) outside (cell-centered) "
	      "bounds [-0.5,%f]x[-0.5,%f]x[-0.5,%f]",
	      me, (float)x, (float)y, (float)z, tx+0.5, ty+0.5, tz+0.5);
      gageErrNum = 0;
      return 1;
    }
    xi = (int)(x+1)-1; xi = AIR_CLAMP(0, xi, tx-1); xf = x - xi;
    yi = (int)(y+1)-1; yi = AIR_CLAMP(0, yi, ty-1); yf = y - yi;
    zi = (int)(z+1)-1; zi = AIR_CLAMP(0, zi, tz-1); zf = z - zi;
  }
  /* now {x,y,z}i are shifted to padded space */
  ctx->point.xi = xi + ctx->havePad;
  ctx->point.yi = yi + ctx->havePad;
  ctx->point.zi = zi + ctx->havePad;
  if (ctx->verbose > 1) {
    fprintf(stderr, "%s: \n"
	    "        pos (% 15.7f,% 15.7f,% 15.7f) (unpadded) \n"
	    "        -> i(%5d,%5d,%5d) (unpadded) \n"
	    "        -> i(%5d,%5d,%5d) (padded) \n"
	    "         + f(% 15.7f,% 15.7f,% 15.7f) \n",
	    me,
	    (float)x, (float)y, (float)z,
	    xi, yi, zi,
	    ctx->point.xi, ctx->point.yi, ctx->point.zi,
	    (float)xf, (float)yf, (float)zf);
  }
  
  if (!( ctx->point.xf == xf &&
	 ctx->point.yf == yf &&
	 ctx->point.zf == zf )) {
    ctx->point.xf = xf;
    ctx->point.yf = yf;
    ctx->point.zf = zf;
    /* these may take some time (especially if using renormalization),
       hence the conditional above */
    _gageFslSet(ctx);
    _gageFwSet(ctx);
  }

  return 0;
}
