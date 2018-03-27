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

#include "../air.h"
#include <teemQnanhibit.h>

char *me;

int
main(int argc, char *argv[]) {
  int c, hibit, ret;
  float f, g, parsed;
  double d;
  char str[128];
  
  me = argv[0];

  c = AIR_NAN;
  g = 0.0;
  g = g/g;
  printf("0.0/0.0 = %f\n", g);
  airFPFprintf_f(stdout, g);
  hibit = (*((int*)&g) >> 22) & 1;
  printf("hi bit of 23-bit fraction field = %d\n", hibit);
  if (hibit == airMyQNaNHiBit) {
    printf("(agrees with airMyQNaNHiBit)\n");
  }
  else {
    printf("%s: !!!!\n", me);
    printf("%s: !!!! PROBLEM: nan's hi bit is NOT airMyQNaNHiBit (%d)\n",
	   me, airMyQNaNHiBit);
    printf("%s: !!!!\n", me);
  }    

  printf(" - - - - - - - - - - - - - - - -\n");
  printf(" - - - - - - - - - - - - - - - -\n");

  for(c=airFP_Unknown+1; c<airFP_Last; c++) {
    f = airFPGen_f(c);
    sprintf(str, "%f", f);
    ret = airSingleSscanf(str, "%f", &parsed);
    printf("********** airFPGen_f(%d) = %f (-> %f(%d)) (AIR_EXISTS = %d)\n", 
	   c, f, parsed, ret, AIR_EXISTS(f));
    airSinglePrintf(stdout, NULL, "--<%f>--\n", f);
    if (c != airFPClass_f(f)) {
      printf("\n\n%s: Silly hardware!!!\n", me);
      printf("%s: can't return a float of class %d %sfrom a function\n\n\n",
	     me, c, airFP_SNAN == c ? "(signaling NaN) " : "");
    }
    airFPFprintf_f(stdout, f);
    d = f;
    /* I think solaris turns the SNAN into a QNAN */
    printf("to double and back:\n");
    airFPFprintf_f(stdout, d);
    printf("AIR_ISNAN_F = %d\n", AIR_ISNAN_F(f));
    /* starting with double */
    d = airFPGen_d(c);
  }

  printf(" - - - - - - - - - - - - - - - -\n");
  printf(" - - - - - - - - - - - - - - - -\n");

  f = AIR_SNAN;
  printf("SNaN test: f = SNaN = float(0x%x) = %f; (QNaNHiBit = %d)\n", 
	 airFloatSNaN.i, f, airMyQNaNHiBit);
  airFPFprintf_f(stdout, f);
  g = f*f;
  printf("g = f*f = %f\n", g);
  airFPFprintf_f(stdout, g);
  g = sin(f);
  printf("g = sin(f) = %f\n", g);
  airFPFprintf_f(stdout, g);

  printf("\n");

  printf("FLT_MAX:\n"); airFPFprintf_f(stdout, FLT_MAX); printf("\n");
  printf("FLT_MIN:\n"); airFPFprintf_f(stdout, FLT_MIN); printf("\n");
  printf("DBL_MAX:\n"); airFPFprintf_d(stdout, DBL_MAX); printf("\n");
  printf("DBL_MIN:\n"); airFPFprintf_d(stdout, DBL_MIN); printf("\n");

  printf("AIR_NAN = %f; AIR_EXISTS(AIR_NAN) = %d\n", 
	 AIR_NAN, AIR_EXISTS(AIR_NAN));
  printf("AIR_POS_INF = %f; AIR_EXISTS(AIR_POS_INF) = %d\n", 
	 AIR_POS_INF, AIR_EXISTS(AIR_POS_INF));
  printf("AIR_NEG_INF = %f; AIR_EXISTS(AIR_NEG_INF) = %d\n", 
	 AIR_NEG_INF, AIR_EXISTS(AIR_NEG_INF));
  exit(0);
}
