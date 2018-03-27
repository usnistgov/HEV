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
limnObjDescribe(FILE *file, limnObj *obj) {
  int j, i, vi;
  limnFace *f;
  limnEdge *e;
  limnPoint *p;
  limnPart *r;
  
  fprintf(file, "parts: %d\n", obj->rA->len);
  for (j=0; j<=obj->rA->len-1; j++) {
    r = &(obj->r[j]);
    fprintf(file, "%d | points: %d\n", j, r->pNum);
    for (i=0; i<=r->pNum-1; i++) {
      p = &(obj->p[r->pBase + i]);
      fprintf(file, "%d | %d(%d): w=(%g,%g,%g)\tv=(%g,%g,%g)\ts(%g,%g,%g)\n", 
	      j, i, r->pBase + i, 
	      p->w[0], p->w[1], p->w[2],
	      p->v[0], p->v[1], p->v[2],
	      p->s[0], p->s[1], p->s[2]);
    }
    fprintf(file, "%d | edges: %d\n", j, r->eNum);
    for (i=0; i<=r->eNum-1; i++) {
      e = &(obj->e[r->eBase + i]);
      fprintf(file, "%d | %d(%d): vert(%d,%d), face(%d,%d)\n", 
	      j, i, r->eBase + i, e->v0, e->v1, e->f0, e->f1);
    }
    fprintf(file, "%d | faces: %d\n", j, r->fNum);
    for (i=0; i<=r->fNum-1; i++) {
      f = &(obj->f[r->fBase + i]);
      fprintf(file, "%d | %d(%d): [", j, i, r->fBase + i);
      for (vi=0; vi<=f->vNum-1; vi++) {
	fprintf(file, "%d", obj->v[f->vBase + vi]);
	if (vi < f->vNum-1)
	  fprintf(file, ",");
      }
      fprintf(file, "]; wn = (%g,%g,%g)\n", f->wn[0], f->wn[1], f->wn[2]);
    }
  }

  return 0;
}
