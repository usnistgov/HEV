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
** _nrrdCC_settle()
**
** takes a mapping map[i], i in [0..len-1], and shifts the range of the
** mapping downward so that the range is a contiguous set of integers,
** starting at 0.
**
** returns the highest mapping output value, after the settling.
*/
int
_nrrdCC_settle (int *map, int len) {
  int i, j, count, max, *hit;
  
  max = 0;
  for (i=0; i<len; i++) {
    max = AIR_MAX(max, map[i]);
  }
  hit = (int *)calloc(1+max, sizeof(int));
  for (i=0; i<len; i++) {
    hit[map[i]] = 1;
  }
  count = 0;
  for (j=0; j<=max; j++) {
    if (hit[j]) {
      hit[j] = count;
      count += 1;
    }
  }
  max = 0;
  for (i=0; i<len; i++) {
    map[i] = hit[map[i]];
    max = AIR_MAX(max, map[i]);
  }
  free(hit);
  return max;
}

/*
** _nrrdCC_eclass ()
**
** takes the equivalence pairs in eqvArr, and an array of ints map of 
** length len, and puts in map[i] the integer that CC i's value should
** be changed to.  
** 
** based on numerical recipes, C edition, pg. 346
** modifications: 
**  - when resolving ancestors, map to the one with the lower index.
**  - applies settling to resulting map 
**
** returns the highest CC id in the mapping
*/
int
_nrrdCC_eclass (int *map, int len, airArray *eqvArr) {
  int eqi, *eqv, j, k, t;

  eqv = (int*)(eqvArr->data);
  for (j=0; j<len; j++) {
    map[j] = j;
  }
  for (eqi=0; eqi<eqvArr->len; eqi++) {
    j = eqv[0 + 2*eqi];
    k = eqv[1 + 2*eqi];
    while (map[j] != j) 
      j = map[j];
    while (map[k] != k)
      k = map[k];
    if (j != k) {
      if (j < k) {
	t = j; j = k; k = t;
      }
      map[j] = k;
    }
  }
  for (j=0; j<len; j++) {
    while (map[j] != map[map[j]]) {
      map[j] = map[map[j]];
    }
  }
  return _nrrdCC_settle(map, len);
}

int
nrrdCCValid(const Nrrd *nin) {
  char me[]="nrrdCCValid", err[AIR_STRLEN_MED];
  
  if (nrrdCheck(nin)) {
    sprintf(err, "%s: basic validity check failed", me);
    biffAdd(NRRD, err); return 0;
  }
  if (!( nrrdTypeIsIntegral[nin->type] )) {
    sprintf(err, "%s: need an integral type (not %s)", me,
	    airEnumStr(nrrdType, nin->type));
    biffAdd(NRRD, err); return 0;
  }
  if (!( nrrdTypeSize[nin->type] <= 2 || nrrdTypeInt == nin->type )) {
    sprintf(err, "%s: valid connected component types are 1- and 2-byte "
	    "integers, and %s", me, airEnumStr(nrrdType, nrrdTypeInt));
    biffAdd(NRRD, err); return 0;
  }
  return 1;
}

/*
** things we could sensibly measure on CCs: 
** - size
** - # neighbors (needs conny argument)
** - what else?
*/

int
nrrdCCSize(Nrrd *nout, const Nrrd *nin) {
  char me[]="nrrdCCSize", func[]="ccsize", err[AIR_STRLEN_MED];
  int *out, maxid, (*lup)(const void *, size_t);
  size_t I, NN;

  if (!( nout && nrrdCCValid(nin) )) {
    sprintf(err, "%s: invalid args", me);
    biffAdd(NRRD, err); return 1;
  }
  maxid = nrrdCCMax(nin);
  if (nrrdMaybeAlloc(nout, nrrdTypeInt, 1, maxid+1)) {
    sprintf(err, "%s: can't allocate output", me);
    biffAdd(NRRD, err); return 1;
  }
  out = (int *)(nout->data);
  lup = nrrdILookup[nin->type];
  NN = nrrdElementNumber(nin);
  for (I=0; I<NN; I++) {
    out[lup(nin->data, I)] += 1;
  }
  if (nrrdContentSet(nout, func, nin, "")) {
    sprintf(err, "%s:", me);
    biffAdd(NRRD, err); return 1;
  }
  
  return 0;
}

/*
******** nrrdCCMax
**
** returns the highest cc id, or 0 if there were problems
**
** does NOT use biff
*/
int
nrrdCCMax(const Nrrd *nin) {
  int (*lup)(const void *, size_t), id, max;
  size_t I, NN;

  if (!nrrdCCValid(nin)) {
    return -1;
  }
  lup = nrrdILookup[nin->type];
  NN = nrrdElementNumber(nin);
  max = 0;
  for (I=0; I<NN; I++) {
    id = lup(nin->data, I);
    max = AIR_MAX(max, id);
  }
  return max;
}

int
nrrdCCNum(const Nrrd *nin) {
  int (*lup)(const void *, size_t), max, num=-1;
  size_t I, NN;
  unsigned char *hist;
  
  if (!nrrdCCValid(nin)) {
    return -1;
  }
  lup = nrrdILookup[nin->type];
  NN = nrrdElementNumber(nin);
  max = nrrdCCMax(nin);
  hist = (unsigned char *)calloc(max+1, sizeof(unsigned char));
  if (!hist) {
    return -1;
  }
  for (I=0; I<NN; I++) {
    hist[lup(nin->data, I)] = 1;
  }
  num = 0;
  for (I=0; I<=max; I++) {
    num += hist[I];
  }
  free(hist);
  return num;
}
