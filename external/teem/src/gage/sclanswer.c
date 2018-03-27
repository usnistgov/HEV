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

void
_gageSclAnswer (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_gageSclAnswer";
  unsigned int query;
  gage_t *ans, gmag=0, *hess, *norm, *gvec, *gten, *k1, *k2, sHess[9], curv=0;
  double tmpMat[9], tmpVec[3], hevec[9], heval[3];
  int *offset;

  gage_t len, gp1[3], gp2[3], *nPerp, nProj[9], ncTen[9];
  double T, N, D;

  query = pvl->query;
  ans = pvl->ans;
  /* convenience pointers for work below */
  offset = gageKindScl->ansOffset;
  hess = ans + offset[gageSclHessian];
  gvec = ans + offset[gageSclGradVec];
  norm = ans + offset[gageSclNormal];
  nPerp = ans + offset[gageSclNPerp];
  gten = ans + offset[gageSclGeomTens];
  k1 = ans + offset[gageSclK1];
  k2 = ans + offset[gageSclK2];
  
  if (1 & (query >> gageSclValue)) {
    /* done if doV */
    if (ctx->verbose) {
      fprintf(stderr, "%s: val = % 15.7f\n", me, 
	      (double)(ans[offset[gageSclValue]]));
    }
  }
  if (1 & (query >> gageSclGradVec)) {
    /* done if doD1 */
    if (ctx->verbose) {
      fprintf(stderr, "%s: gvec = ", me);
      ell_3v_PRINT(stderr, gvec);
    }
  }
  if (1 & (query >> gageSclGradMag)) {
    /* this is the true value of gradient magnitude */
    gmag = ans[offset[gageSclGradMag]] = sqrt(ELL_3V_DOT(gvec, gvec));
  }

  /* NB: it would seem that gageParmGradMagMin is completely ignored ... */

  if (1 & (query >> gageSclNormal)) {
    if (gmag) {
      ELL_3V_SCALE(norm, 1.0/gmag, gvec);
      /* polishing ... 
      len = sqrt(ELL_3V_DOT(norm, norm));
      ELL_3V_SCALE(norm, 1.0/len, norm);
      */
    } else {
      ELL_3V_COPY(norm, gageZeroNormal);
    }
  }
  if (1 & (query >> gageSclNPerp)) {
    /* nPerp = I - outer(norm, norm) */
    /* NB: this sets both nPerp and nProj */
    ELL_3MV_OUTER(nProj, norm, norm);
    ELL_3M_SCALE(nPerp, -1, nProj);
    nPerp[0] += 1;
    nPerp[4] += 1;
    nPerp[8] += 1;
  }
  if (1 & (query >> gageSclHessian)) {
    /* done if doD2 */
    if (ctx->verbose) {
      fprintf(stderr, "%s: hess = \n", me);
      ell_3m_PRINT(stderr, hess);
    }
  }
  if (1 & (query >> gageSclLaplacian)) {
    ans[offset[gageSclLaplacian]] = hess[0] + hess[4] + hess[8];
    if (ctx->verbose) {
      fprintf(stderr, "%s: lapl = %g + %g + %g  = %g\n", me,
	      hess[0], hess[4], hess[8], ans[offset[gageSclLaplacian]]);
    }
  }
  if (1 & (query >> gageSclHessEval)) {
    ELL_3M_COPY(tmpMat, hess);
    /* HEY: look at the return value for root multiplicity? */
    /* NB: we have solve and then copy because of possible type
       mismatch between double and gage_t */
    ell_3m_eigensolve_d(heval, hevec, tmpMat, AIR_TRUE);
    ELL_3V_COPY(ans+offset[gageSclHessEval], heval);
  }
  if (1 & (query >> gageSclHessEvec)) {
    ELL_3M_COPY(ans+offset[gageSclHessEvec], hevec);
  }
  if (1 & (query >> gageScl2ndDD)) {
    ELL_3MV_MUL(tmpVec, hess, norm);
    ans[offset[gageScl2ndDD]] = ELL_3V_DOT(norm, tmpVec);
  }
  if (1 & (query >> gageSclGeomTens)) {
    if (gmag > ctx->parm.gradMagCurvMin) {
      /* parm.curvNormalSide applied here to determine the sense of the
	 normal when doing all curvature calculations */
      ELL_3M_SCALE(sHess, -(ctx->parm.curvNormalSide)/gmag, hess);
      
      /* san->gten = nPerp * sHess * nPerp */
      ELL_3M_MUL(tmpMat, sHess, nPerp);
      ELL_3M_MUL(gten, nPerp, tmpMat);

      if (ctx->verbose) {
	fprintf(stderr, "%s: gten: \n", me);
	ell_3m_PRINT(stderr, gten);
	ELL_3MV_MUL(tmpVec, gten, norm);
	len = ELL_3V_LEN(tmpVec);
	fprintf(stderr, "%s: should be small: %30.15f\n", me, (double)len);
	ell_3v_PERP(gp1, norm);
	ELL_3MV_MUL(tmpVec, gten, gp1);
	len = ELL_3V_LEN(tmpVec);
	fprintf(stderr, "%s: should be bigger: %30.15f\n", me, (double)len);
	ELL_3V_CROSS(gp2, gp1, norm);
	ELL_3MV_MUL(tmpVec, gten, gp2);
	len = ELL_3V_LEN(tmpVec);
	fprintf(stderr, "%s: should (also) be bigger: %30.15f\n",
		me, (double)len);
      }
    } else {
      ELL_3M_ZERO_SET(gten);
    }
  }
  if (1 && (query >> gageSclCurvedness)) {
    curv = ans[offset[gageSclCurvedness]] = ELL_3M_FROB(gten);
  }
  if (1 && (query >> gageSclShapeTrace)) {
    ans[offset[gageSclShapeTrace]] = (curv
				      ? ELL_3M_TRACE(gten)/curv
				      : 0);
  }
  if ( (1 && (query >> gageSclK1)) ||
       (1 && (query >> gageSclK2)) ){
    T = ELL_3M_TRACE(gten);
    N = curv;
    D = 2*N*N - T*T;
    /*
    if (D < -0.0000001) {
      fprintf(stderr, "%s: %g %g\n", me, T, N);
      fprintf(stderr, "%s: !!! D curv determinant % 22.10f < 0.0\n", me, D);
      fprintf(stderr, "%s: gten: \n", me);
      ell_3m_PRINT(stderr, gten);
    }
    */
    D = AIR_MAX(D, 0);
    D = sqrt(D);
    k1[0] = 0.5*(T + D);
    k2[0] = 0.5*(T - D);
  }
  if (1 && (query >> gageSclMeanCurv)) {
    ans[offset[gageSclMeanCurv]] = (*k1 + *k2)/2;
  }
  if (1 && (query >> gageSclGaussCurv)) {
    ans[offset[gageSclGaussCurv]] = (*k1)*(*k2);
  }
  if (1 && (query >> gageSclShapeIndex)) {
    ans[offset[gageSclShapeIndex]] = -(2/M_PI)*atan2(*k1 + *k2, *k1 - *k2);
  }
  if (1 & (query >> gageSclCurvDir)) {
    /* HEY: this only works when K1, K2, 0 are all well mutually distinct,
       since these are the eigenvalues of the geometry tensor, and this
       code assumes that the eigenspaces are all one-dimensional */
    ELL_3M_COPY(tmpMat, gten);
    ELL_3M_DIAG_SET(tmpMat, gten[0] - *k1, gten[4]- *k1, gten[8] - *k1);
    ell_3m_1d_nullspace_d(tmpVec, tmpMat);
    ELL_3V_COPY(ans+offset[gageSclCurvDir]+0, tmpVec);
    ELL_3M_DIAG_SET(tmpMat, gten[0] - *k2, gten[4] - *k2, gten[8] - *k2);
    ell_3m_1d_nullspace_d(tmpVec, tmpMat);
    ELL_3V_COPY(ans+offset[gageSclCurvDir]+3, tmpVec);
  }
  if (1 & (query >> gageSclFlowlineCurv)) {
    if (gmag >= ctx->parm.gradMagCurvMin) {
      /* because of the gageSclGeomTens prerequisite, sHess, nPerp, and
	 nProj are all already set */
      /* ncTen = nPerp * sHess * nProj */
      ELL_3M_MUL(tmpMat, sHess, nProj);
      ELL_3M_MUL(ncTen, nPerp, tmpMat);
    } else {
      ELL_3M_ZERO_SET(ncTen);
    }
    ans[offset[gageSclFlowlineCurv]] = sqrt(ELL_3M_FROB(ncTen));
  }
  return;
}

