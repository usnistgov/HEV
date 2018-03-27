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

limnObj *
limnObjNew(int incr, int edges) {
  limnObj *obj;

  obj = (limnObj *)calloc(1, sizeof(limnObj));
  obj->p = NULL;
  obj->v = NULL;
  obj->e = NULL;
  obj->f = NULL;
  obj->r = NULL;
  obj->s = NULL;

  /* create all various airArrays */
  obj->pA = airArrayNew((void**)&(obj->p), NULL, 
			sizeof(limnPoint), incr);
  obj->vA = airArrayNew((void**)&(obj->v), NULL,
			sizeof(int), incr);
  obj->eA = airArrayNew((void**)&(obj->e), NULL,
			sizeof(limnEdge), incr);
  obj->fA = airArrayNew((void**)&(obj->f), NULL,
			sizeof(limnFace), incr);
  obj->rA = airArrayNew((void**)&(obj->r), NULL,
			sizeof(limnPart), incr);
  obj->sA = airArrayNew((void**)&(obj->s), NULL,
			sizeof(limnSP), incr);
  obj->rCurr = NULL;

  obj->edges = edges;
    
  return obj;
}

limnObj *
limnObjNix(limnObj *obj) {

  airArrayNuke(obj->pA);
  airArrayNuke(obj->vA);
  airArrayNuke(obj->eA);
  airArrayNuke(obj->fA);
  airArrayNuke(obj->rA);
  airArrayNuke(obj->sA);
  free(obj);
  return NULL;
}

int
limnObjPartStart(limnObj *obj) {
  int rBase;
  limnPart *r;

  rBase = airArrayIncrLen(obj->rA, 1);
  r = &(obj->r[rBase]);
  r->fBase = obj->fA->len;  r->fNum = 0;
  r->eBase = obj->eA->len;  r->eNum = 0;
  r->pBase = obj->pA->len;  r->pNum = 0;
  r->origIdx = rBase;
  ELL_4V_SET(r->rgba, 1.0, 1.0, 1.0, 1.0);
  obj->rCurr = r;

  return rBase;
}

int
limnObjPointAdd(limnObj *obj, int sp, float x, float y, float z) {
  limnPoint *p;
  int pBase;

  pBase = airArrayIncrLen(obj->pA, 1);
  p = &(obj->p[pBase]);
  ELL_4V_SET(p->w, x, y, z, 1);
  ELL_3V_SET(p->v, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(p->s, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(p->n, AIR_NAN, AIR_NAN, AIR_NAN);
  p->d[0] = p->d[1] = AIR_NAN;
  p->sp = sp;
  obj->rCurr->pNum++;

  return pBase;
}

void
_limnEdgeInit(limnEdge *e, int sp, int face, int v0, int v1) {
  
  e->v0 = v0;
  e->v1 = v1;
  e->f0 = face;
  e->f1 = -1;
  e->sp = sp;
  e->visib = 0;
}

int
limnObjEdgeAdd(limnObj *obj, int sp, int face, int v0, int v1) {
  int ret, t, i, eNum, eBase;
  limnEdge *e=NULL;
  
  eBase = obj->rCurr->eBase;
  eNum = obj->rCurr->eNum;
  
  if (v0 > v1) {
    ELL_SWAP2(v0, v1, t);
  }
  
  /* do a linear search through this part's edges */
  for (i=0; i<=eNum-1; i++) {
    e = &(obj->e[eBase+i]);
    if (e->v0 == v0 && e->v1 == v1) {
      break;
    }
  }
  if (i == eNum) {
    /* edge not found */
    eBase = airArrayIncrLen(obj->eA, 1);
    e = &(obj->e[eBase]);
    _limnEdgeInit(e, sp, face, v0, v1);
    ret = eBase;
    obj->rCurr->eNum++;
  }
  else {
    /* edge already exists */
    e->f1 = face;
    ret = eBase+i;
  }

  return ret;
}

int
limnObjFaceAdd(limnObj *obj, int sp, int numVert, int *vert) {
  int i, vBase, fBase;
  limnFace *f;

  fBase = airArrayIncrLen(obj->fA, 1);
  vBase = airArrayIncrLen(obj->vA, numVert);
  
  f = &(obj->f[fBase]);
  ELL_3V_SET(f->wn, AIR_NAN, AIR_NAN, AIR_NAN);
  ELL_3V_SET(f->sn, AIR_NAN, AIR_NAN, AIR_NAN);
  f->vBase = vBase;
  f->vNum = numVert;
  for (i=0; i<=numVert-1; i++) {
    obj->v[vBase + i] = vert[i];
    if (obj->edges) {
      limnObjEdgeAdd(obj, 1, fBase, vert[i], vert[AIR_MOD(i+1, numVert)]);
    }
  }
  f->sp = sp;
  f->visib = AIR_FALSE;
  obj->rCurr->fNum++;
  
  return fBase;
}

int
limnObjPartFinish(limnObj *obj) {
  
  obj->rCurr = NULL;
  
  return 0;
}

int
limnObjSPAdd(limnObj *obj) {

  return airArrayIncrLen(obj->sA, 1);
}

