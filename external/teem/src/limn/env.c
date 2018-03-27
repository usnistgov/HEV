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

#include "limn.h"

int
limnEnvMapFill(Nrrd *map, limnEnvMapCB cb, int qnMethod, void *data) {
  char me[]="limnEnvMapFill", err[128];
  int sx, sy;
  int qn;
  float vec[3], *mapData;

  if (!(map && cb)) {
    sprintf(err, "%s: got NULL pointer", me);
    biffAdd(LIMN, err); return 1;
  }
  if (!AIR_IN_OP(limnQN_Unknown, qnMethod, limnQN_Last)) {
    sprintf(err, "%s: QN method %d invalid", me, qnMethod);
    biffAdd(LIMN, err); return 1;
  }
  switch(qnMethod) {
  case limnQN_16checker:
    sx = sy = 256;
    if (nrrdMaybeAlloc(map, nrrdTypeFloat, 3, 3, sx, sy)) {
      sprintf(err, "%s: couldn't alloc output", me);
      biffMove(LIMN, err, NRRD); return 1;
    }
    mapData = map->data;
    for (qn=0; qn<=sx*sy-1; qn++) {
      limnQNtoV[limnQN_16checker](vec, qn);
      cb(mapData + 3*qn, vec, data);
    }
    break;
  default:
    sprintf(err, "%s: sorry, QN method %d not implemented", me, qnMethod);
    biffAdd(LIMN, err); return 1;
  }

  return 0;
}

void
limnLightDiffuseCB(float rgb[3], float vec[3], void *_lit) {
  float dot, r, g, b, norm;
  limnLight *lit;
  int i;

  lit = _lit;
  ELL_3V_NORM(vec, vec, norm);
  r = lit->amb[0];
  g = lit->amb[1];
  b = lit->amb[2];
  for (i=0; i<LIMN_LIGHT_NUM; i++) {
    if (!lit->on[i])
      continue;
    dot = ELL_3V_DOT(vec, lit->dir[i]);
    dot = AIR_MAX(0, dot);
    r += dot*lit->col[i][0];
    g += dot*lit->col[i][1];
    b += dot*lit->col[i][2];
  }
  /* not really our job to be doing clamping here ... */
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

int
limnEnvMapCheck(Nrrd *envMap) {
  char me[]="limnEnvMapCheck", err[AIR_STRLEN_MED];

  if (nrrdCheck(envMap)) {
    sprintf(err, "%s: basic nrrd validity check failed", me);
    biffMove(LIMN, err, NRRD); return 1;
  }
  if (!(nrrdTypeFloat == envMap->type)) {
    sprintf(err, "%s: type should be %s, not %s", me,
	    airEnumStr(nrrdType, nrrdTypeFloat),
	    airEnumStr(nrrdType, envMap->type));
    biffAdd(LIMN, err); return 1;
  }
  if (!(3 == envMap->dim)) {
    sprintf(err, "%s: dimension should be 3, not %d", me, envMap->dim);
    biffAdd(LIMN, err); return 1;
  }
  if (!(3 == envMap->axis[0].size
	&& 256 == envMap->axis[1].size
	&& 256 == envMap->axis[2].size)) {
    sprintf(err, "%s: dimension should be 3x256x256, not %dx%dx%d", me,
	    envMap->axis[0].size, 
	    envMap->axis[1].size, 
	    envMap->axis[2].size);
    biffAdd(LIMN, err); return 1;
  }
  return 0;
}
