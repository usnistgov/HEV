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


#include "ell.h"

/*
** ========================
** NOTE NOTE NOTE NOTE NOTE
** ========================
** All the functions in this file use a matrix ordering which
** is opposite if the (current) ordering in the rest of ell.
**
** Rest of ell:
**  0 . . .
**  1 . . .
**  2 . . .
**  . . . .
**
** This file:
**  0 1 2 .
**  . . . .
**  . . . .
**  . . . .
*/

int
ell_Nm_valid(Nrrd *mat) {
  char me[]="ell_Nm_valid", err[AIR_STRLEN_MED];

  if (!mat) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(ELL, err); return 0;
  }
  /* nrrdCheck(mat)? */
  if (!( 2 == mat->dim )) {
    sprintf(err, "%s: nrrd must be 2-D (not %d-D)", me, mat->dim);
    biffAdd(ELL, err); return 0;
  }
  if (!( nrrdTypeDouble == mat->type )) {
    sprintf(err, "%s: nrrd must be type %s (not %s)", me,
	    airEnumStr(nrrdType, nrrdTypeDouble),
	    airEnumStr(nrrdType, mat->type));
    biffAdd(ELL, err); return 0;
  }

  return 1;
}

int
ell_Nm_tran (Nrrd *ntrn, Nrrd *nmat) {
  char me[]="ell_Nm_tran", err[AIR_STRLEN_MED];

  if (!( ntrn && ell_Nm_valid(nmat) )) {
    sprintf(err, "%s: NULL or invalid args", me);
    biffAdd(ELL, err); return 1;
  }
  if (ntrn == nmat) {
    sprintf(err, "%s: sorry, can't work in-place yet", me);
    biffAdd(ELL, err); return 1;
  }
  if (nrrdAxesSwap(ntrn, nmat, 0, 1)) {
    sprintf(err, "%s: trouble", me);
    biffMove(ELL, err, NRRD); return 1;
  }

  return 0;
}

/*
******** ell_Nm_mul
**
** Currently, only useful for matrix-matrix multiplication
**
** matrix-matrix:      M       N
**                  L [A] . M [B]
*/
int
ell_Nm_mul (Nrrd *nAB, Nrrd *nA, Nrrd *nB) {
  char me[]="ell_Nm_mul", err[AIR_STRLEN_MED];
  double *A, *B, *AB, tmp;
  int LL, MM, NN, ll, mm, nn;
  
  if (!( nAB && ell_Nm_valid(nA) && ell_Nm_valid(nB) )) {
    sprintf(err, "%s: NULL or invalid args", me);
    biffAdd(ELL, err); return 1;
  }
  if (nAB == nA || nAB == nB) {
    sprintf(err, "%s: can't do in-place multiplication", me);
    biffAdd(ELL, err); return 1;
  }
  LL = nA->axis[1].size;
  MM = nA->axis[0].size;
  NN = nB->axis[0].size;
  if (MM != nB->axis[1].size) {
    sprintf(err, "%s: size mismatch: %d-by-%d times %d-by-%d",
	    me, LL, MM, nB->axis[1].size, NN);
    biffAdd(ELL, err); return 1;
  }
  if (nrrdMaybeAlloc(nAB, nrrdTypeDouble, 2, NN, LL)) {
    sprintf(err, "%s: trouble", me);
    biffMove(ELL, err, NRRD); return 1;
  }
  A = (double*)(nA->data);
  B = (double*)(nB->data);
  AB = (double*)(nAB->data);
  for (ll=0; ll<LL; ll++) {
    for (nn=0; nn<NN; nn++) {
      tmp = 0;
      for (mm=0; mm<MM; mm++) {
	tmp += A[ll*MM + mm]*B[mm*NN + nn];
      }
      AB[ll*NN + nn] = tmp;
    }
  }

  return 0;
}

/*
** _ell_LU_decomp()
**
** in-place LU decomposition
*/
int
_ell_LU_decomp (double *a, int *indx, int NN)  {
  char me[]="_ell_LU_decomp", err[AIR_STRLEN_MED];
  int ret=0, i, imax=0, j, k;
  double big, sum, tmp;
  double *vv;
  
  if (!( vv = (double*)calloc(NN, sizeof(double)) )) {
    sprintf(err, "%s: couldn't allocate vv[]!", me);
    biffAdd(ELL, err); ret = 1; goto seeya;
  }

  /* find vv[i]: max of abs of everything in row i */
  for (i=0; i<NN; i++) {
    big = 0.0;
    for (j=0; j<NN; j++) {
      if ((tmp=AIR_ABS(a[i*NN + j])) > big) {
	big = tmp;
      }
    }
    if (!big) {
      sprintf(err, "%s: singular matrix since row %d all zero", me, i);
      biffAdd(ELL, err); ret = 1; goto seeya;
    }
    vv[i] = big;
  }

  for (j=0; j<NN; j++) {
    /* for a[i][j] in lower triangle (below diagonal), subtract from a[i][j]
       the dot product of all elements to its left with elements above it 
       (starting at the top) */
    for (i=0; i<j; i++) {
      sum = a[i*NN + j];
      for (k=0; k<i; k++)
	sum -= a[i*NN + k]*a[k*NN + j];
      a[i*NN + j] = sum;
    }
    
    /* for a[i][j] in upper triangle (including diagonal), subtract from 
       a[i][j] the dot product of all elements above it with elements to
       its left (starting from the left) */
    big = 0.0;
    for (i=j; i<NN; i++) {
      sum = a[i*NN + j];
      for (k=0; k<j; k++)
	sum -= a[i*NN + k]*a[k*NN + j];
      a[i*NN + j] = sum;
      /* imax column is one in which abs(a[i][j])/vv[i] */
      if ((tmp = AIR_ABS(sum)/vv[i]) >= big) {
	big = tmp;
	imax = i;
      }
    }
    
    /* unless we're on the imax column, swap this column the with imax column,
       and permute vv[] accordingly */
    if (j != imax) {
      /* could record parity # of permutes here */
      for (k=0; k<NN; k++) {
	tmp = a[imax*NN + k];
	a[imax*NN + k] = a[j*NN + k];
	a[j*NN + k] = tmp;
      }
      tmp = vv[imax];
      vv[imax] = vv[j];
      vv[j] = tmp;
    }
     
    indx[j] = imax;

    if (a[j*NN + j] == 0.0) 
      a[j*NN + j] = ELL_EPS;
     
    /* divide everything right of a[j][j] by a[j][j] */
    if (j != NN) {
      tmp = 1.0/a[j*NN + j];
      for (i=j+1; i<NN; i++) {
	a[i*NN + j] *= tmp;
      }
    }
  }
 seeya:
  airFree(vv);
  return ret;
}

/*
** _ell_LU_back_sub
**
** given the matrix and index array from _ellLUDecomp generated from
** some matrix M, solves for x in the linear equation Mx = b, and 
** puts the result back into b
*/
void
_ell_LU_back_sub (double *a, int *indx, double *b, int NN) {
  int i, j;
  double sum;

  /* Forward substitution, with lower triangular matrix */
  for (i=0; i<NN; i++) {
    sum = b[indx[i]];
    b[indx[i]] = b[i];
    for (j=0; j<i; j++)
      sum -= a[i*NN + j]*b[j];
    b[i] = sum;
  }
 
  /* Backward substitution, with upper triangular matrix */
  for (i=NN-1; i>=0; i--) {
    sum = b[i];
    for (j=i+1; j<NN; j++)
      sum -= a[i*NN + j]*b[j];
    b[i] = sum / a[i*NN + i];
  }
  return;
}

/*
** _ell_inv
**
** Invert NNxNN matrix based on LU-decomposition
**
** The given matrix is copied, turned into its LU-decomposition, and
** then repeated backsubstitution is used to get successive columns of
** the inverse.
*/
int
_ell_inv (double *inv, double *_mat, int NN) {
  char me[]="_ell_inv", err[AIR_STRLEN_MED];
  int i, j;
  double *col=NULL, *mat=NULL;
  int *indx=NULL, ret=0;

  if (!( (col = (double*)calloc(NN, sizeof(double))) &&
	 (mat = (double*)calloc(NN*NN, sizeof(double))) &&
	 (indx = (int*)calloc(NN, sizeof(int))) )) {
    sprintf(err, "%s: couldn't allocate all buffers", me);
    biffAdd(ELL, err); ret = 1; goto seeya;
  }

  memcpy(mat, _mat, NN*NN*sizeof(double));

  if (_ell_LU_decomp(mat, indx, NN)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(ELL, err); ret = 1; goto seeya;
  }
  
  for (j=0; j<NN; j++) {
    memset(col, 0, NN*sizeof(double));
    col[j] = 1.0;
    _ell_LU_back_sub(mat, indx, col, NN);
    /* set column j of inv to result of backsub */
    for (i=0; i<NN; i++) {
      inv[i*NN + j] = col[i];
    }
  }
 seeya:
  airFree(col); airFree(mat); airFree(indx);
  return ret;
}

/*
******** ell_Nm_inv
**
** computes the inverse of given matrix in nmat, and puts the 
** inverse in the (maybe allocated) ninv.  Does not touch the
** values in nmat.
*/
int
ell_Nm_inv (Nrrd *ninv, Nrrd *nmat) {
  char me[]="ell_Nm_inv", err[AIR_STRLEN_MED];
  double *mat, *inv;
  int NN;

  if (!( ninv && ell_Nm_valid(nmat) )) {
    sprintf(err, "%s: NULL or invalid args", me);
    biffAdd(ELL, err); return 1;
  }

  NN = nmat->axis[0].size;
  if (!( NN == nmat->axis[1].size )) {
    sprintf(err, "%s: need a square matrix, not %d-by-%d",
	    me, nmat->axis[1].size, NN);
    biffAdd(ELL, err); return 1;
  }
  if (nrrdMaybeAlloc(ninv, nrrdTypeDouble, 2, NN, NN)) {
    sprintf(err, "%s: trouble", me);
    biffMove(ELL, err, NRRD); return 1;
  }
  inv = (double*)(ninv->data);
  mat = (double*)(nmat->data);
  if (_ell_inv(inv, mat, NN)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(ELL, err); return 1;
  }

  return 0;
}

/*
******** ell_Nm_pseudo_inv()
**
** determines the pseudoinverse of the given matrix M by using the formula
** P = (M^T * M)^(-1) * M^T
**
** I'll get an SVD-based solution working later, since that gives a more
** general solution
*/
int
ell_Nm_pseudo_inv (Nrrd *ninv, Nrrd *nA) {
  char me[]="ell_Nm_pseudo_inv", err[AIR_STRLEN_MED];
  Nrrd *nAt, *nAtA, *nAtAi;
  int ret=0;
  
  if (!( ninv && ell_Nm_valid(nA) )) {
    sprintf(err, "%s: NULL or invalid args", me);
    biffAdd(ELL, err); return 1;
  }
  nAt = nrrdNew();
  nAtA = nrrdNew();
  nAtAi = nrrdNew();
  if (ell_Nm_tran(nAt, nA)
      || ell_Nm_mul(nAtA, nAt, nA)
      || ell_Nm_inv(nAtAi, nAtA)
      || ell_Nm_mul(ninv, nAtAi, nAt)) {
    sprintf(err, "%s: trouble", me);
    biffAdd(ELL, err); ret = 1; goto seeya;
  }
  
 seeya:
  nrrdNuke(nAt); nrrdNuke(nAtA); nrrdNuke(nAtAi);
  return ret;
}

