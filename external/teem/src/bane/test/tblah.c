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

#include "../bane.h"

char *me;

void
rangeTest(char *me, double imin, double imax) {
  double omin, omax;
  baneRange *range;
  int i;

  printf("input range (%g,%g) ---------------------\n", imin, imax);
  for (i=1; i<=BANE_RANGE_MAX; i++) {
    range = baneRangeArray[i];
    range->ans(&omin, &omax, imin, imax);
    printf("%s: range %s --> (%g,%g)\n",
	   me, range->name, omin, omax);
  }
}

void
incTest(char *me, int num, baneRange *range) {
  double *val, tmp, incParm[BANE_INC_PARM_NUM], omin, omax, rmin, rmax;
  baneInc *inc;
  Nrrd *hist;
  int i, j;

  airSrand();

  val = (double*)malloc(num*sizeof(double));
  /* from <http://www.itl.nist.gov/div898/handbook/index.htm>:
     the standard dev of a uniform distribution between A and B is
     sqrt((B-A)^2/12) */
  for (j=0; j<num; j++) {
    tmp = AIR_AFFINE(0.0, airRand(), 1.0, -1.0, 1.0);
    /* val[j] = tmp*tmp*tmp; */
    val[j] = tmp;
  }
  rmin = rmax = val[0];
  for (j=0; j<num; j++) {
    rmin = AIR_MIN(rmin, val[j]);
    rmax = AIR_MAX(rmax, val[j]);
  }
  fprintf(stderr, "incTest: real min,max = %g,%g\n", rmin, rmax);
  
  for (i=1; i<=BANE_INC_MAX; i++) {
    inc = baneIncArray[i];
    printf("%s: inclusion %s ------\n", me, inc->name);
    switch(i) {
    case baneIncAbsolute_e:
      ELL_3V_SET(incParm, -0.8, 1.5, AIR_NAN);
      break;
    case baneIncRangeRatio_e:
      ELL_3V_SET(incParm, 0.99, AIR_NAN, AIR_NAN);
      break;
    case baneIncPercentile_e:
      ELL_3V_SET(incParm, 1024, 10, AIR_NAN);
      break;
    case baneIncStdv_e:
      ELL_3V_SET(incParm, 1.0, AIR_NAN, AIR_NAN);
      break;
    }
    hist = inc->histNew(incParm);
    if (inc->passA) {
      for (j=0; j<num; j++)
	inc->passA(hist, val[j], incParm);
    }
    if (inc->passB) {
      for (j=0; j<num; j++)
	inc->passB(hist, val[j], incParm);
    }
    inc->ans(&omin, &omax, hist, incParm, range);
    printf(" --> (%g,%g)\n", omin, omax);
  }

  free(val);
}

int
main(int argc, char *argv[]) {
  me = argv[0];

  printf("%s ================================ range testing\n", me);
  rangeTest(me, -1, 2);
  rangeTest(me, -3, 2);

  printf("%s ================================ inclusion testing\n", me);
  incTest(me, 10000, baneRangeFloat);

  return 0;
}
