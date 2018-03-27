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

#include "nrrd.h"
#include "privateNrrd.h"

/*
******** nrrdSplice()
**
** (opposite of nrrdSlice): replaces one slice of a nrrd with
** another nrrd.  Will allocate memory for output only if nout != nin.
*/
int
nrrdSplice(Nrrd *nout, const Nrrd *nin, const Nrrd *nslice,
	   int axis, int pos) {
  char me[]="nrrdSplice", func[]="splice", err[AIR_STRLEN_MED];
  size_t 
    I, 
    rowLen,                  /* length of segment */
    colStep,                 /* distance between start of each segment */
    colLen;                  /* number of periods */
  int i;
  char *src, *dest, *sliceCont;

  if (!(nin && nout && nslice)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nslice) {
    sprintf(err, "%s: nout==nslice disallowed", me);
    biffAdd(NRRD, err); return 1;
  }

  /* check that desired slice location is legit */
  if (!(AIR_IN_CL(0, axis, nin->dim-1))) {
    sprintf(err, "%s: slice axis %d out of bounds (0 to %d)", 
	    me, axis, nin->dim-1);
    biffAdd(NRRD, err); return 1;
  }
  if (!(AIR_IN_CL(0, pos, nin->axis[axis].size-1) )) {
    sprintf(err, "%s: position %d out of bounds (0 to %d)", 
	    me, pos, nin->axis[axis].size-1);
    biffAdd(NRRD, err); return 1;
  }

  /* check that slice will fit in nin */
  if (nrrdCheck(nslice) || nrrdCheck(nin)) {
    sprintf(err, "%s: input or slice not valid nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->dim-1 == nslice->dim )) {
    sprintf(err, "%s: dim of slice (%d) not one less than dim of input (%d)",
	    me, nslice->dim, nin->dim-1);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->type == nslice->type )) {
    sprintf(err, "%s: type of slice (%s) != type of input (%s)",
	    me, airEnumStr(nrrdType, nslice->type),
	    airEnumStr(nrrdType, nin->type));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    if (!( nin->blockSize == nslice->blockSize )) {
      sprintf(err, "%s: input's blockSize (%d) != subvolume's blockSize (%d)",
	      me, nin->blockSize, nslice->blockSize);
      biffAdd(NRRD, err); return 1;
    }
  }
  for (i=0; i<nslice->dim; i++) {
    if (!( nin->axis[i + (i >= axis)].size == nslice->axis[i].size )) {
      sprintf(err, "%s: input's axis %d size (%d) != slices axis %d size (%d)",
	      me, i + (i >= axis),
	      nin->axis[i + (i >= axis)].size, i, nslice->axis[i].size);
      biffAdd(NRRD, err); return 1;
    }
  }

  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
  } 
  /* else we're going to splice in place */

  /* the following was copied from nrrdSlice() */
  /* set up control variables */
  rowLen = colLen = 1;
  for (i=0; i<nin->dim; i++) {
    if (i < axis) {
      rowLen *= nin->axis[i].size;
    } else if (i > axis) {
      colLen *= nin->axis[i].size;
    }
  }
  rowLen *= nrrdElementSize(nin);
  colStep = rowLen*nin->axis[axis].size;

  /* the skinny */
  src = nout->data;    /* switched src,dest from nrrdSlice() */
  dest = nslice->data;
  src += rowLen*pos;
  for (I=0; I<colLen; I++) {
    /* HEY: replace with AIR_MEMCPY() or similar, when applicable */
    memcpy(src, dest, rowLen);  /* switched src,dest from nrrdSlice() */
    src += colStep;
    dest += rowLen;
  }
  
  sliceCont = _nrrdContentGet(nslice);
  if (nrrdContentSet(nout, func, nin, "%s,%d,%d", sliceCont, axis, pos)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); free(sliceCont); return 1;
  }
  free(sliceCont);

  nrrdPeripheralInit(nout);

  return 0;
}

/*
******** nrrdInset()
**
** (opposite of nrrdCrop()) replace some sub-volume inside a nrrd with
** another given nrrd.
**
*/
int
nrrdInset(Nrrd *nout, const Nrrd *nin, const Nrrd *nsub, const int *min) {
  char me[]="nrrdInset", func[] = "inset", err[AIR_STRLEN_MED],
    buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_SMALL];
  int d,
    lineSize,                /* #bytes in one scanline to be copied */
    typeSize,                /* size of data type */
    cIn[NRRD_DIM_MAX],       /* coords for line start, in input */
    cOut[NRRD_DIM_MAX],      /* coords for line start, in output */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX];
  size_t I,
    idxIn, idxOut,           /* linear indices for input and output */
    numLines;                /* number of scanlines in output nrrd */
  char *dataIn, *dataOut, *subCont;
  double zeros[1024];

  memset(zeros, 0, 1024*sizeof(double));

  /* errors */
  if (!(nout && nin && nsub && min)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nsub) {
    sprintf(err, "%s: nout==nsub disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdCheck(nsub) || nrrdCheck(nin)) {
    sprintf(err, "%s: input or subvolume not valid nrrd", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->dim == nsub->dim )) {
    sprintf(err, "%s: input's dim (%d) != subvolume's dim (%d)",
	    me, nin->dim, nsub->dim);
    biffAdd(NRRD, err); return 1;
  }
  if (!( nin->type == nsub->type )) {
    sprintf(err, "%s: input's type (%s) != subvolume's type (%s)", me,
	    airEnumStr(nrrdType, nin->type),
	    airEnumStr(nrrdType, nsub->type));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type) {
    if (!( nin->blockSize == nsub->blockSize )) {
      sprintf(err, "%s: input's blockSize (%d) != subvolume's blockSize (%d)",
	      me, nin->blockSize, nsub->blockSize);
      biffAdd(NRRD, err); return 1;
    }
  }
  for (d=0; d<nin->dim; d++) {
    if (!( 0 <= min[d] && 
	   min[d] + nsub->axis[d].size - 1 <= nin->axis[d].size - 1)) {
      sprintf(err, "%s: axis %d range of inset indices [%d,%d] not within "
	      "input indices [0,%d]", me, d,
	      min[d], min[d] + nsub->axis[d].size - 1,
	      nin->axis[d].size - 1);
      biffAdd(NRRD, err); return 1;
    }
  }
  
  if (nout != nin) {
    if (nrrdCopy(nout, nin)) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); return 1;
    }
  } 
  /* else we're going to inset in place */

  /* WARNING: following code copied/modified from nrrdCrop(),
     so the meanings of "in"/"out", "src"/"dest" are all messed up */

  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  nrrdAxisInfoGet_nva(nsub, nrrdAxisInfoSize, szOut);
  numLines = 1;
  for (d=1; d<nin->dim; d++) {
    numLines *= szOut[d];
  }
  lineSize = szOut[0]*nrrdElementSize(nin);
  
  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = nout->data;
  dataOut = nsub->data;
  memset(cOut, 0, NRRD_DIM_MAX*sizeof(int));
  for (I=0; I<numLines; I++) {
    for (d=0; d<nin->dim; d++)
      cIn[d] = cOut[d] + min[d];
    NRRD_INDEX_GEN(idxOut, cOut, szOut, nin->dim);
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    memcpy(dataIn + idxIn*typeSize, dataOut + idxOut*typeSize, lineSize);
    /* the lowest coordinate in cOut[] will stay zero, since we are 
       copying one (1-D) scanline at a time */
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 1);
  }
  
  strcpy(buff1, "[");
  for (d=0; d<nin->dim; d++) {
    sprintf(buff2, "%s%d", (d ? "," : ""), min[d]);
    strcat(buff1, buff2);
  }
  strcat(buff1, "]");
  subCont = _nrrdContentGet(nsub);
  if (nrrdContentSet(nout, func, nin, "%s,%s", subCont, buff1)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); free(subCont); return 1;
  }
  free(subCont); 

  nrrdPeripheralInit(nout);

  return 0;
}

/*
******** nrrdPad()
**
** strictly for padding
*/
int
nrrdPad(Nrrd *nout, const Nrrd *nin,
	const int *min, const int *max, int boundary, ...) {
  char me[]="nrrdPad", func[]="pad", err[AIR_STRLEN_MED],
    buff1[NRRD_DIM_MAX*30], buff2[AIR_STRLEN_MED];
  double padValue=AIR_NAN;
  int d, outside, dim, typeSize,
    cIn[NRRD_DIM_MAX],       /* coords for line start, in input */
    cOut[NRRD_DIM_MAX],      /* coords for line start, in output */
    szIn[NRRD_DIM_MAX],
    szOut[NRRD_DIM_MAX];
  size_t
    idxIn, idxOut,           /* linear indices for input and output */
    numOut;                  /* number of elements in output nrrd */
  va_list ap;
  char *dataIn, *dataOut;
  
  if (!(nout && nin && min && max)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    sprintf(err, "%s: boundary behavior %d invalid", me, boundary);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryWeight == boundary) {
    sprintf(err, "%s: boundary strategy %s not applicable here", me,
	    airEnumStr(nrrdBoundary, boundary));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdTypeBlock == nin->type && nrrdBoundaryPad == boundary) {
    sprintf(err, "%s: with nrrd type %s, boundary %s not valid", me,
	    airEnumStr(nrrdType, nrrdTypeBlock),
	    airEnumStr(nrrdBoundary, nrrdBoundaryPad));
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    va_start(ap, boundary);
    padValue = va_arg(ap, double);
    va_end(ap);
  }
  switch(boundary) {
  case nrrdBoundaryPad:
  case nrrdBoundaryBleed:
  case nrrdBoundaryWrap:
    break;
  default:
    fprintf(stderr, "%s: PANIC: boundary %d unimplemented\n", 
	    me, boundary); exit(1); break;
  }
  /*
  printf("!%s: boundary = %d, padValue = %g\n", me, boundary, padValue);
  */

  dim = nin->dim;
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  for (d=0; d<dim; d++) {
    if (!(min[d] <= 0)) {
      sprintf(err, "%s: axis %d min (%d) not <= 0", 
	      me, d, min[d]);
      biffAdd(NRRD, err); return 1;
    }
    if (!(max[d] >= szIn[d]-1)) {
      sprintf(err, "%s: axis %d max (%d) not >= size-1 (%d)", 
	      me, d, max[d], szIn[d]-1);
      biffAdd(NRRD, err); return 1;
    }
  }
  /* this shouldn't actually be necessary ... */
  if (!nrrdElementSize(nin)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }

  /* allocate */
  numOut = 1;
  for (d=0; d<dim; d++) {
    numOut *= (szOut[d] = -min[d] + max[d] + 1);
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, dim, szOut)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  
  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = nin->data;
  dataOut = nout->data;
  memset(cOut, 0, NRRD_DIM_MAX*sizeof(int));
  for (idxOut=0; idxOut<numOut; idxOut++) {
    outside = 0;
    for (d=0; d<dim; d++) {
      cIn[d] = cOut[d] + min[d];
      switch(boundary) {
      case nrrdBoundaryPad:
      case nrrdBoundaryBleed:
	if (!AIR_IN_CL(0, cIn[d], szIn[d]-1)) {
	  cIn[d] = AIR_CLAMP(0, cIn[d], szIn[d]-1);
	  outside = 1;
	}
	break;
      case nrrdBoundaryWrap:
	if (!AIR_IN_CL(0, cIn[d], szIn[d]-1)) {
	  cIn[d] = AIR_MOD(cIn[d], szIn[d]);
	  outside = 1;
	}
	break;
      }
    }
    NRRD_INDEX_GEN(idxIn, cIn, szIn, dim);
    if (!outside) {
      /* the cIn coords are within the input nrrd: do memcpy() of whole
	 1-D scanline, then artificially bump for-loop to the end of
	 the scanline */
      memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize,
	     szIn[0]*typeSize);
      idxOut += nin->axis[0].size-1;
      cOut[0] += nin->axis[0].size-1;
    } else {
      /* we copy only a single value */
      if (nrrdBoundaryPad == boundary) {
	nrrdDInsert[nout->type](dataOut, idxOut, padValue);
      } else {
	memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize, typeSize);
      }
    }
    NRRD_COORD_INCR(cOut, szOut, dim, 0);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, (NRRD_AXIS_INFO_SIZE_BIT |
					 NRRD_AXIS_INFO_MIN_BIT |
					 NRRD_AXIS_INFO_MAX_BIT ))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  for (d=0; d<dim; d++) {
    nrrdAxisPosRange(&(nout->axis[d].min), &(nout->axis[d].max),
		     nin, d, min[d], max[d]);
  }
  strcpy(buff1, "");
  for (d=0; d<dim; d++) {
    sprintf(buff2, "%s[%d,%d]", (d ? "x" : ""), min[d], max[d]);
    strcat(buff1, buff2);
  }
  if (nrrdBoundaryPad == boundary) {
    sprintf(buff2, "%s(%g)", airEnumStr(nrrdBoundary, nrrdBoundaryPad), 
	    padValue);
  } else {
    strcpy(buff2, airEnumStr(nrrdBoundary, boundary));
  }
  if (nrrdContentSet(nout, func, nin, "%s,%s", buff1, buff2)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  nrrdPeripheralInit(nout);
  /* leave comments alone */

  return 0;
}

/*
******** nrrdPad_nva()
**
** unlike other {X,X_nva} pairs, nrrdPad_nva() is a wrapper around
** nrrdPad() instead of the other way around.
*/
int
nrrdPad_nva(Nrrd *nout, const Nrrd *nin, const int *min, const int *max,
	    int boundary, double padValue) {
  char me[]="nrrdPad_nva", err[AIR_STRLEN_MED];
  int E;

  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    sprintf(err, "%s: boundary behavior %d invalid", me, boundary);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    E = nrrdPad(nout, nin, min, max, boundary, padValue);
  } else {
    E = nrrdPad(nout, nin, min, max, boundary);
  }
  if (E) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  
  return 0;
}

/*
******** nrrdSimplePad()
**
** pads by a given amount on top and bottom of EVERY axis
*/
int
nrrdSimplePad(Nrrd *nout, const Nrrd *nin, int pad, int boundary, ...) {
  char me[]="nrrdSimplePad", err[AIR_STRLEN_MED];
  int d, min[NRRD_DIM_MAX], max[NRRD_DIM_MAX], ret;
  double padValue;
  va_list ap;

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  for (d=0; d<nin->dim; d++) {
    min[d] = -pad;
    max[d] = nin->axis[d].size-1 + pad;
  }
  if (nrrdBoundaryPad == boundary) {
    va_start(ap, boundary);
    padValue = va_arg(ap, double);
    va_end(ap);
    ret = nrrdPad(nout, nin, min, max, boundary, padValue);
  } else {
    ret = nrrdPad(nout, nin, min, max, boundary);
  }
  if (ret) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/*
******** nrrdSimplePad_nva()
**
** unlike other {X,X_nva} pairs, nrrdSimplePad_nva() is a wrapper
** around nrrdSimplePad() instead of the other way around.
*/
int
nrrdSimplePad_nva(Nrrd *nout, const Nrrd *nin, int pad,
		  int boundary, double padValue) {
  char me[]="nrrdSimplePad_nva", err[AIR_STRLEN_MED];
  int E;

  if (!AIR_IN_OP(nrrdBoundaryUnknown, boundary, nrrdBoundaryLast)) {
    sprintf(err, "%s: boundary behavior %d invalid", me, boundary);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdBoundaryPad == boundary) {
    E = nrrdSimplePad(nout, nin, pad, boundary, padValue);
  } else {
    E = nrrdSimplePad(nout, nin, pad, boundary);
  }
  if (E) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }

  return 0;
}
