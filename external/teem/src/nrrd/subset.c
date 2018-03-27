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
******** nrrdSample_nva()
**
** given coordinates within a nrrd, copies the 
** single element into given *val
*/
int
nrrdSample_nva(void *val, const Nrrd *nrrd, const int *coord) {
  char me[]="nrrdSample_nva", err[AIR_STRLEN_MED];
  int typeSize, size[NRRD_DIM_MAX], d;
  size_t I;
  
  if (!(nrrd && coord && val)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  /* this shouldn't actually be necessary ... */
  if (!nrrdElementSize(nrrd)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }
  
  typeSize = nrrdElementSize(nrrd);
  nrrdAxisInfoGet_nva(nrrd, nrrdAxisInfoSize, size);
  for (d=0; d<nrrd->dim; d++) {
    if (!(AIR_IN_CL(0, coord[d], size[d]-1))) {
      sprintf(err, "%s: coordinate %d on axis %d out of bounds (0 to %d)", 
	      me, coord[d], d, size[d]-1);
      biffAdd(NRRD, err); return 1;
    }
  }

  NRRD_INDEX_GEN(I, coord, size, nrrd->dim);

  memcpy(val, (char*)(nrrd->data) + I*typeSize, typeSize);
  return 0;
}

/*
******** nrrdSample()
**
** var-args version of nrrdSample_nva()
*/
int
nrrdSample(void *val, const Nrrd *nrrd, ...) {
  char me[]="nrrdSample", err[AIR_STRLEN_MED];
  int d, coord[NRRD_DIM_MAX];
  va_list ap;
  
  if (!(nrrd && val)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }

  va_start(ap, nrrd);
  for (d=0; d<nrrd->dim; d++) {
    coord[d] = va_arg(ap, int);
  }
  va_end(ap);
  
  if (nrrdSample_nva(val, nrrd, coord)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

/*
******** nrrdSlice()
**
** slices a nrrd along a given axis, at a given position.
**
** This is a newer version of the procedure, which is simpler, faster,
** and requires less memory overhead than the first one.  It is based
** on the observation that any slice is a periodic square-wave pattern
** in the original data (viewed as a one- dimensional array).  The
** characteristics of that periodic pattern are how far from the
** beginning it starts (offset), the length of the "on" part (length),
** the period (period), and the number of periods (numper). 
*/
int
nrrdSlice(Nrrd *nout, const Nrrd *nin, int axis, int pos) {
  char me[]="nrrdSlice", func[]="slice", err[AIR_STRLEN_MED];
  size_t 
    I, 
    rowLen,                  /* length of segment */
    colStep,                 /* distance between start of each segment */
    colLen;                  /* number of periods */
  int i, outdim, map[NRRD_DIM_MAX], szOut[NRRD_DIM_MAX];
  char *src, *dest;

  if (!(nin && nout)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  if (1 == nin->dim) {
    sprintf(err, "%s: can't slice a 1-D nrrd; use nrrd{I,F,D}Lookup[]", me);
    biffAdd(NRRD, err); return 1;
  }
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
  /* this shouldn't actually be necessary ... */
  if (!nrrdElementSize(nin)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }

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

  outdim = nin->dim-1;
  for (i=0; i<outdim; i++) {
    map[i] = i + (i >= axis);
    szOut[i] = nin->axis[map[i]].size;
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, outdim, szOut)) {
    sprintf(err, "%s: failed to create slice", me);
    biffAdd(NRRD, err); return 1;
  }

  /* the skinny */
  src = nin->data;
  dest = nout->data;
  src += rowLen*pos;
  for (I=0; I<colLen; I++) {
    /* HEY: replace with AIR_MEMCPY() or similar, when applicable */
    memcpy(dest, src, rowLen);
    src += colStep;
    dest += rowLen;
  }

  /* copy the peripheral information */
  if (nrrdAxisInfoCopy(nout, nin, map, NRRD_AXIS_INFO_NONE)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nrrdContentSet(nout, func, nin, "%d,%d", axis, pos)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  nrrdPeripheralInit(nout);
  /* leave comments alone */

  return 0;
}

/*
******** nrrdCrop()
**
** select some sub-volume inside a given nrrd, producing an output
** nrrd with the same dimensions, but with equal or smaller sizes
** along each axis.
*/
int
nrrdCrop(Nrrd *nout, const Nrrd *nin, int *min, int *max) {
  char me[]="nrrdCrop", func[] = "crop", err[AIR_STRLEN_MED],
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
  char *dataIn, *dataOut;

  /* errors */
  if (!(nout && nin && min && max)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  if (nout == nin) {
    sprintf(err, "%s: nout==nin disallowed", me);
    biffAdd(NRRD, err); return 1;
  }
  for (d=0; d<nin->dim; d++) {
    if (!(min[d] <= max[d])) {
      sprintf(err, "%s: axis %d min (%d) not <= max (%d)", 
	      me, d, min[d], max[d]);
      biffAdd(NRRD, err); return 1;
    }
    if (!(AIR_IN_CL(0, min[d], nin->axis[d].size-1) &&
	  AIR_IN_CL(0, max[d], nin->axis[d].size-1))) {
      sprintf(err, "%s: axis %d min (%d) or max (%d) out of bounds [0,%d]",
	      me, d, min[d], max[d], nin->axis[d].size-1);
      biffAdd(NRRD, err); return 1;
    }
  }
  /* this shouldn't actually be necessary ... */
  if (!nrrdElementSize(nin)) {
    sprintf(err, "%s: nrrd reports zero element size!", me);
    biffAdd(NRRD, err); return 1;
  }

  /* allocate */
  nrrdAxisInfoGet_nva(nin, nrrdAxisInfoSize, szIn);
  numLines = 1;
  for (d=0; d<nin->dim; d++) {
    szOut[d] = max[d] - min[d] + 1;
    if (d)
      numLines *= szOut[d];
  }
  nout->blockSize = nin->blockSize;
  if (nrrdMaybeAlloc_nva(nout, nin->type, nin->dim, szOut)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  lineSize = szOut[0]*nrrdElementSize(nin);
  
  /* the skinny */
  typeSize = nrrdElementSize(nin);
  dataIn = nin->data;
  dataOut = nout->data;
  memset(cOut, 0, NRRD_DIM_MAX*sizeof(int));
  /*
  printf("!%s: nin->dim = %d\n", me, nin->dim);
  printf("!%s: min  = %d %d %d\n", me, min[0], min[1], min[2]);
  printf("!%s: szIn = %d %d %d\n", me, szIn[0], szIn[1], szIn[2]);
  printf("!%s: szOut = %d %d %d\n", me, szOut[0], szOut[1], szOut[2]);
  printf("!%s: lineSize = %d\n", me, lineSize);
  printf("!%s: typeSize = %d\n", me, typeSize);
  printf("!%s: numLines = %d\n", me, (int)numLines);
  */
  for (I=0; I<numLines; I++) {
    for (d=0; d<nin->dim; d++)
      cIn[d] = cOut[d] + min[d];
    NRRD_INDEX_GEN(idxOut, cOut, szOut, nin->dim);
    NRRD_INDEX_GEN(idxIn, cIn, szIn, nin->dim);
    /*
    printf("!%s: %5d: cOut=(%3d,%3d,%3d) --> idxOut = %5d\n",
	   me, (int)I, cOut[0], cOut[1], cOut[2], (int)idxOut);
    printf("!%s: %5d:  cIn=(%3d,%3d,%3d) -->  idxIn = %5d\n",
	   me, (int)I, cIn[0], cIn[1], cIn[2], (int)idxIn);
    */
    memcpy(dataOut + idxOut*typeSize, dataIn + idxIn*typeSize, lineSize);
    /* the lowest coordinate in cOut[] will stay zero, since we are 
       copying one (1-D) scanline at a time */
    NRRD_COORD_INCR(cOut, szOut, nin->dim, 1);
  }
  if (nrrdAxisInfoCopy(nout, nin, NULL, (NRRD_AXIS_INFO_SIZE_BIT |
					 NRRD_AXIS_INFO_MIN_BIT |
					 NRRD_AXIS_INFO_MAX_BIT ))) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  for (d=0; d<nin->dim; d++) {
    nrrdAxisPosRange(&(nout->axis[d].min), &(nout->axis[d].max),
		     nin, d, min[d], max[d]);
  }
  strcpy(buff1, "");
  for (d=0; d<nin->dim; d++) {
    sprintf(buff2, "%s[%d,%d]", (d ? "x" : ""), min[d], max[d]);
    strcat(buff1, buff2);
  }
  if (nrrdContentSet(nout, func, nin, "%s", buff1)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  nrrdPeripheralInit(nout);
  /* leave comments alone */

  return 0;
}

/*
******** nrrdSimpleCrop()
**
*/
int
nrrdSimpleCrop(Nrrd *nout, const Nrrd *nin, int crop) {
  char me[]="nrrdSimpleCrop", err[AIR_STRLEN_MED];
  int d, min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];

  if (!(nout && nin)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(NRRD, err); return 1;
  }
  for (d=0; d<nin->dim; d++) {
    min[d] = crop;
    max[d] = nin->axis[d].size-1 - crop;
  }
  if (nrrdCrop(nout, nin, min, max)) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  return 0;
}

