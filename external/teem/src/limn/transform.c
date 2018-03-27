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
_limnObjWHomog(limnObj *obj) {
  int pi;
  limnPoint *p;
  float h;
  
  for (pi=0; pi<=obj->pA->len-1; pi++) {
    p = obj->p + pi;
    h = 1.0/p->w[3];
    ELL_3V_SCALE(p->w, h, p->w);
    p->w[3] = 1.0;
  }
  
  return 0;
}

int
_limnObjNormals(limnObj *obj, int space) {
  int vn, vi, vb, fi;
  limnFace *f;
  limnPoint *p0, *p1, *p2;
  float e1[3], e2[3], x[3], n[3], norm;
  
  for (fi=0; fi<=obj->fA->len-1; fi++) {
    f = obj->f + fi;
    vn = f->vNum;
    vb = f->vBase;
    ELL_3V_SET(n, 0, 0, 0);
    for (vi=0; vi<vn; vi++) {
      p0 = obj->p + obj->v[vb + vi];
      p1 = obj->p + obj->v[vb + AIR_MOD(vi+1,vn)];
      p2 = obj->p + obj->v[vb + AIR_MOD(vi-1,vn)];
      if (limnSpaceWorld == space) {
	ELL_3V_SUB(e1, p1->w, p0->w);
	ELL_3V_SUB(e2, p2->w, p0->w);
      }
      else {
	ELL_3V_SUB(e1, p1->s, p0->s);
	ELL_3V_SUB(e2, p2->s, p0->s);
      }
      ELL_3V_CROSS(x, e1, e2);
      ELL_3V_ADD2(n, n, x);
    }
    if (limnSpaceWorld == space) {
      ELL_3V_NORM(f->wn, n, norm);
      /*
      printf("%s: wn[%d] = %g %g %g\n", "_limnObjNormals", fi,
	     f->wn[0], f->wn[1], f->wn[2]);
      */
    }
    else {
      ELL_3V_NORM(f->sn, n, norm);
      /*
      printf("%s: sn[%d] = %g %g %g\n", "_limnObjNormals", fi,
	     f->sn[0], f->sn[1], f->sn[2]);
      */
    }
  }

  return 0;
}

int
_limnObjVTransform(limnObj *obj, limnCamera *cam) {
  int pi;
  limnPoint *p;
  float d;

  for (pi=0; pi<=obj->pA->len-1; pi++) {
    p = obj->p + pi;
    ELL_4MV_MUL(p->v, cam->W2V, p->w);
    d = 1.0/p->w[3];
    ELL_4V_SCALE(p->v, d, p->v);
    /*
    printf("%s: w[%d] = %g %g %g %g --> v = %g %g %g\n", "_limnObjVTransform",
	  pi, p->w[0], p->w[1], p->w[2], p->w[3], p->v[0], p->v[1], p->v[2]);
    */
  }
  return 0;
}

int
_limnObjSTransform(limnObj *obj, limnCamera *cam) {
  int pi;
  limnPoint *p;
  float d;

  for (pi=0; pi<=obj->pA->len-1; pi++) {
    p = obj->p + pi;
    d = (cam->orthographic 
	 ? 1
	 : cam->vspDist/p->v[2]);
    p->s[0] = d*p->v[0];
    p->s[1] = d*p->v[1];
    p->s[2] = p->v[2];
    /*
    printf("%s: v[%d] = %g %g %g --> s = %g %g %g\n", "_limnObjSTransform",
	   pi, p->v[0], p->v[1], p->v[2], p->s[0], p->s[1], p->s[2]);
    */
  }
  return 0;
}

int
_limnObjDTransform(limnObj *obj, limnCamera *cam, limnWin *win) {
  int pi;
  limnPoint *p;
  float wy0, wy1, wx0, wx1, t;
  
  wx0 = 0;
  wx1 = (cam->uRange[1] - cam->uRange[0])*win->scale;
  wy0 = 0;
  wy1 = (cam->vRange[1] - cam->vRange[0])*win->scale;
  ELL_4V_SET(win->bbox, wx0, wy0, wx1, wy1);
  if (win->yFlip) {
    ELL_SWAP2(wy0, wy1, t);
  }
  for (pi=0; pi<=obj->pA->len-1; pi++) {
    p = obj->p + pi;
    p->d[0] = AIR_AFFINE(cam->uRange[0], p->s[0], cam->uRange[1], wx0, wx1);
    p->d[1] = AIR_AFFINE(cam->vRange[0], p->s[1], cam->vRange[1], wy0, wy1);
    /*
    printf("%s: s[%d] = %g %g --> s = %g %g\n", "_limnObjDTransform",
	   pi, p->s[0], p->s[1], p->d[0], p->d[1]);
    */
  }
  return 0;
}

int
limnObjHomog(limnObj *obj, int space) {
  char me[]="limnObjHomog";
  int ret;

  switch(space) {
  case limnSpaceWorld:
    ret = _limnObjWHomog(obj);
    break;
  default:
    fprintf(stderr, "%s: space %d unknown or unimplemented\n", me, space);
    ret = 1;
    break;
  }
  
  return ret;
}

int
limnObjNormals(limnObj *obj, int space) {
  char me[]="limnObjNormals";
  int ret;
  
  switch(space) {
  case limnSpaceWorld:
  case limnSpaceScreen:
    ret = _limnObjNormals(obj, space);
    break;
  default:
    fprintf(stderr, "%s: space %d unknown or unimplemented\n", me, space);
    ret = 1;
    break;
  }

  return ret;
}

int
limnObjSpaceTransform(limnObj *obj, limnCamera *cam, limnWin *win, int space) {
  char me[]="limnObjSpaceTransform";
  int ret;

  /* HEY: deal with cam->orthographic */
  switch(space) {
  case limnSpaceView:
    ret = _limnObjVTransform(obj, cam);
    break;
  case limnSpaceScreen:
    ret = _limnObjSTransform(obj, cam);
    break;
  case limnSpaceDevice:
    ret = _limnObjDTransform(obj, cam, win);
    break;
  default:
    fprintf(stderr, "%s: space %d unknown or unimplemented\n", me, space);
    ret = 1;
    break;
  }

  return ret;
}

int
limnObjPartTransform(limnObj *obj, int ri, float tx[16]) {
  int pi, pn, pb;
  limnPart *r;
  limnPoint *p;
  float tmp[4];
  
  r = obj->r + ri;
  pb = r->pBase;
  pn = r->pNum;
  for (pi=pb; pi<=pb+pn-1; pi++) {
    p = obj->p + pi;
    ELL_4MV_MUL(tmp, tx, p->w);
    ELL_4V_COPY(p->w, tmp);
  }

  return 0;
}

int
_limnPartDepthCompare(const void *_a, const void *_b) {
  limnPart *a;
  limnPart *b;

  a = (limnPart *)_a;
  b = (limnPart *)_b;
  return AIR_COMPARE(b->z, a->z);
}

int
limnObjDepthSortParts(limnObj *obj) {
  limnPart *r;
  limnPoint *p;
  int pi, ri, rNum;
  /* double t0, t1; */

  rNum = obj->rA->len;
  for (ri=0; ri<rNum; ri++) {
    r = obj->r + ri;
    r->z = 0;
    for (pi=0; pi<r->pNum; pi++) {
      p = obj->p + r->pBase + pi;
      r->z += p->s[2];
    }
    r->z /= r->pNum;
  }
  
  /* t0 = airTime(); */
  qsort(obj->r, rNum, sizeof(limnPart), _limnPartDepthCompare);
  /* t1 = airTime(); */
  /* printf("limnObjDepthSortParts: qsort took %g seconds\n", t1-t0); */

  return 0;
}
