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


#include "ten.h"
#include "tenPrivate.h"

/*
  tenGageTensor,        *  0: "t", the reconstructed tensor: GT[7] *
  tenGageTrace,         *  1: "tr", trace of tensor: *GT *
  tenGageFrobTensor,    *  2: "fro", frobenius norm of tensor: *GT *
  tenGageEval,          *  3: "eval", eigenvalues of tensor
			       (sorted descending) : GT[3] *
  tenGageEvec,          *  4: "evec", eigenvectors of tensor: GT[9] *
  tenGageTensorGrad,    *  5: "tg", all tensor component gradients, starting
			       with the confidence value gradient: GT[21] *
  tenGageQ,             *  6: "q", Q anisotropy: *GT *
  tenGageQGradVec,      *  7: "qv", gradient of Q anisotropy: GT[3] *
  tenGageQGradMag,      *  8: "qg", grad mag of Q anisotropy: *GT *
  tenGageQNormal,       *  9: "qn", normalized gradient of Q
			       anisotropy: GT[3] *
  tenGageMultiGrad,     * 10: "mg", sum of outer products of the tensor 
			       matrix elements, correctly counting the
			       off-diagonal entries twice, but not counting
			       the confidence value: GT[9] *
  tenGageFrobMG,        * 11: "frmg", frobenius norm of multi gradient: *GT *
  tenGageMGEval,        * 12: "mgeval", eigenvalues of multi gradient: GT[3]*
  tenGageMGEvec,        * 13: "mgevec", eigenvectors of multi
			       gradient: GT[9] *
  tenGageAniso,         * 14: "an", all anisotropies: GT[TEN_ANISO_MAX+1] *
  tenGageLast

*/

int
tenGageAnsLength[TEN_GAGE_MAX+1] = {
  7,  1,  1,  3,  9, 21, 1,  3,  1,  3,  9,  1,  3,  9, TEN_ANISO_MAX+1
};

int
tenGageAnsOffset[TEN_GAGE_MAX+1] = {
  0,  7,  8,  9, 12, 21, 42, 43, 46, 47, 50, 59, 60, 63, 72
  /* --> 72+TEN_ANISO_MAX+1 == TEN_GAGE_TOTAL_ANS_LENGTH */
};

/*
** _tenGageNeedDeriv[]
**
** each value is a BIT FLAG representing the different value/derivatives
** that are needed to calculate the quantity.  
**
** 1: need value interpolation reconstruction (as with k00)
** 2: need first derivatives (as with k11)
** 4: need second derivatives (as with k22)
*/
int
_tenGageNeedDeriv[TEN_GAGE_MAX+1] = {
  1,  1,  1,  1,  1,  2,  1,  2,  2,  2,  2,  2,  2,  2,  1
};

/*
** _tenGagePrereq[]
** 
** this records the measurements which are needed as ingredients for any
** given measurement, but it is not necessarily the recursive expansion of
** that requirement (that role is performed by gageQuerySet())
*/
unsigned int
_tenGagePrereq[TEN_GAGE_MAX+1] = {
  /* 0: tenGageTensor */
  0,

  /* 1: tenGageTrace */
  (1<<tenGageTensor),

  /* 2: tenGageFrobTensor */
  (1<<tenGageTensor),

  /* 3: tenGageEval */
  (1<<tenGageTensor),

  /* 4: tenGageEvec */
  (1<<tenGageTensor),

  /* 5: tenGageTensorGrad */
  0,
  
  /* 6: tenGageQ */
  (1<<tenGageTensor),

  /* 7: tenGageQGradVec */
  (1<<tenGageTensor) | (1<<tenGageTensorGrad),

  /* 8: tenGageQGradMag */
  (1<<tenGageQGradVec),

  /* 9: tenGageQNormal */
  (1<<tenGageQGradMag) | (1<<tenGageQGradVec),

  /* 10: tenGageMultiGrad */
  (1<<tenGageTensorGrad),

  /* 11: tenGageFrobMG */
  (1<<tenGageMultiGrad),
  
  /* 12: tenGageMGEval */
  (1<<tenGageMultiGrad),

  /* 13: tenGageMGEvec */
  (1<<tenGageMultiGrad),

  /* 14: tenGageAniso */
  (1<<tenGageEval)
};

void
_tenGageIv3Print (FILE *file, gageContext *ctx, gagePerVolume *pvl) {
  
  fprintf(file, "_tenGageIv3Print() not implemented\n");
}

void
_tenGageFilter (gageContext *ctx, gagePerVolume *pvl) {
  char me[]="_gageVecFilter";
  gage_t *fw00, *fw11, *fw22, *tensor, *tgrad;
  int fd;

  fd = GAGE_FD(ctx);
  tensor = GAGE_ANSWER_POINTER(pvl, tenGageTensor);
  tgrad = GAGE_ANSWER_POINTER(pvl, tenGageTensorGrad);
  if (!ctx->parm.k3pack) {
    fprintf(stderr, "!%s: sorry, 6pack filtering not implemented\n", me);
    return;
  }
  fw00 = ctx->fw + fd*3*gageKernel00;
  fw11 = ctx->fw + fd*3*gageKernel11;
  fw22 = ctx->fw + fd*3*gageKernel22;
  /* perform the filtering */
  switch (fd) {
  case 2:
#define DOIT_2(J) \
      gageScl3PFilter2(pvl->iv3 + J*8, pvl->iv2 + J*4, pvl->iv1 + J*2, \
		       fw00, fw11, fw22, \
                       tensor + J, tgrad + J*3, NULL, \
		       pvl->needD[0], pvl->needD[1], AIR_FALSE)
    DOIT_2(0); DOIT_2(1); DOIT_2(2); DOIT_2(3); DOIT_2(4); DOIT_2(5); DOIT_2(6); 
    break;
  case 4:
#define DOIT_4(J) \
      gageScl3PFilter4(pvl->iv3 + J*64, pvl->iv2 + J*16, pvl->iv1 + J*4, \
		       fw00, fw11, fw22, \
                       tensor + J, tgrad + J*3, NULL, \
		       pvl->needD[0], pvl->needD[1], AIR_FALSE)
    DOIT_4(0); DOIT_4(1); DOIT_4(2); DOIT_4(3); DOIT_4(4); DOIT_4(5); DOIT_4(6); 
    break;
  default:
#define DOIT_N(J)\
      gageScl3PFilterN(fd, \
                       pvl->iv3 + J*fd*fd*fd, \
                       pvl->iv2 + J*fd*fd, pvl->iv1 + J*fd, \
		       fw00, fw11, fw22, \
                       tensor + J, tgrad + J*3, NULL, \
		       pvl->needD[0], pvl->needD[1], AIR_FALSE)
    DOIT_N(0); DOIT_N(1); DOIT_N(2); DOIT_N(3); DOIT_N(4); DOIT_N(5); DOIT_N(6); 
    break;
  }

  return;
}

void
_tenGageAnswer (gageContext *ctx, gagePerVolume *pvl) {
  /* char me[]="_tenGageAnswer"; */
  unsigned int query;
  gage_t *ans, *tenAns, *tgradAns, *QgradAns, *evalAns, *evecAns, tmptmp=0,
    dtA=0, dtB=0, dtC=0, dtD=0, dtE=0, dtF=0, cbA, cbB;
  int *offset;

#if !GAGE_TYPE_FLOAT
  int ci;
  float tenAnsF[7], evalAnsF[3], evecAnsF[9], aniso[TEN_ANISO_MAX+1];
#endif

  query = pvl->query;
  ans = pvl->ans;
  offset = tenGageKind->ansOffset;
  tenAns = ans + offset[tenGageTensor];
  tgradAns = ans + offset[tenGageTensorGrad];
  QgradAns = ans + offset[tenGageQGradVec];
  evalAns = ans + offset[tenGageEval];
  evecAns = ans + offset[tenGageEvec];
  if (1 & (query >> tenGageTensor)) {
    /* done if doV */
    dtA = tenAns[1];
    dtB = tenAns[2];
    dtC = tenAns[3];
    dtD = tenAns[4];
    dtE = tenAns[5];
    dtF = tenAns[6];
    if (ctx->verbose) {
      fprintf(stderr, "tensor = (%g) %g %g %g   %g %g   %g\n", tenAns[0],
	      dtA, dtB, dtC, dtD, dtE, dtF);
    }
  }
  if (1 & (query >> tenGageTrace)) {
    ans[offset[tenGageTrace]] = dtA + dtD + dtF;
  }
  if (1 & (query >> tenGageFrobTensor)) {
    ans[offset[tenGageTrace]] = sqrt(dtA*dtA + 2*dtB*dtB + 2*dtC*dtC
				     + dtD*dtD + 2*dtE*dtE + dtF*dtF);
  }
  /* HEY: this is pretty sub-optimal if the only thing we want is the 
     eigenvalues for doing anisotropy determination ... */
  if ( (1 & (query >> tenGageEval)) || (1 & (query >> tenGageEvec)) ) {
#if GAGE_TYPE_FLOAT
      tenEigensolve(evalAns, evecAns, tenAns);
#else
      TEN_LIST_COPY(tenAnsF, tenAns);
      tenEigensolve(evalAnsF, evecAnsF, tenAnsF);
      ELL_3V_COPY(evalAns, evalAnsF);
      ELL_3M_COPY(evecAns, evecAnsF);
#endif
  }
  if (1 & (query >> tenGageTensorGrad)) {
    /* done if doD1 */
  }
  if (1 & (query >> tenGageQ)) {
    cbA = -(dtA + dtD + dtF);
    cbB = dtA*dtD - dtB*dtB + dtA*dtF - dtC*dtC + dtD*dtF - dtE*dtE;
    /*
    cbC = -(dtA*dtD*dtF + 2*dtB*dtE*dtC
	    - dtC*dtC*dtD - dtB*dtB*dtF - dtA*dtE*dtE);
    */
    ans[offset[tenGageQ]] = cbA*cbA - 3*cbB;
  }
  if (1 & (query >> tenGageQGradVec)) {
    ELL_3V_SET(QgradAns, 0, 0, 0);
    ELL_3V_SCALE_INCR(QgradAns,   dtA, tgradAns + 1*3);
    ELL_3V_SCALE_INCR(QgradAns, 2*dtB, tgradAns + 2*3);
    ELL_3V_SCALE_INCR(QgradAns, 2*dtC, tgradAns + 3*3);
    ELL_3V_SCALE_INCR(QgradAns,   dtD, tgradAns + 4*3);
    ELL_3V_SCALE_INCR(QgradAns, 2*dtE, tgradAns + 5*3);
    ELL_3V_SCALE_INCR(QgradAns,   dtF, tgradAns + 6*3);
    tmptmp = -(dtA + dtD + dtF)/3;
    ELL_3V_SCALE_INCR(QgradAns, tmptmp, tgradAns + 1*3);
    ELL_3V_SCALE_INCR(QgradAns, tmptmp, tgradAns + 4*3);
    ELL_3V_SCALE_INCR(QgradAns, tmptmp, tgradAns + 6*3);
  }
  if (1 & (query >> tenGageQGradMag)) {
    tmptmp = ans[offset[tenGageQGradMag]] = ELL_3V_LEN(QgradAns);
  }
  if (1 & (query >> tenGageQNormal)) {
    ELL_3V_SCALE(ans + offset[tenGageQNormal], 1.0/tmptmp, QgradAns);
  }
  if (1 & (query >> tenGageAniso)) {
#if GAGE_TYPE_FLOAT
    tenAnisoCalc(ans + offset[tenGageAniso], evalAns);
#else
    tenAnisoCalc(aniso, evalAnsF);
    for (ci=0; ci<=TEN_ANISO_MAX; ci++) {
      (ans + offset[tenGageAniso])[ci] = aniso[ci];
    }
#endif
  }
  return;
}

gageKind
_tenGageKind = {
  "tensor",
  &_tenGage,
  1,
  7,
  TEN_GAGE_MAX,
  tenGageAnsLength,
  tenGageAnsOffset,
  TEN_GAGE_TOTAL_ANS_LENGTH,
  _tenGageNeedDeriv,
  _tenGagePrereq,
  _tenGageIv3Print,
  _tenGageFilter,
  _tenGageAnswer
};
gageKind *
tenGageKind = &_tenGageKind;
