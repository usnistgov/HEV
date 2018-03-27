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

#ifndef DYE_HAS_BEEN_INCLUDED
#define DYE_HAS_BEEN_INCLUDED

#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/ell.h>

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define dye_export __declspec(dllimport)
#else
#define dye_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DYE dyeBiffKey

enum {
  dyeSpaceUnknown,        /* 0: nobody knows */
  dyeSpaceHSV,            /* 1: single hexcone */
  dyeSpaceHSL,            /* 2: double hexcone */
  dyeSpaceRGB,            /* 3: obscure, deprecated */
  dyeSpaceXYZ,            /* 4: perceptual primaries */
  dyeSpaceLAB,            /* 5: 1976 CIE (L*a*b*) (based on Munsell) */
  dyeSpaceLUV,            /* 6: 1976 CIE (L*u*v*) */
  dyeSpaceLast
};
#define DYE_MAX_SPACE 6

#define DYE_VALID_SPACE(spc) \
  (AIR_IN_OP(dyeSpaceUnknown, (spc), dyeSpaceLast))

typedef struct {
  float val[2][3];        /* room for two colors: two triples of floats */
  float xWhite, yWhite;   /* chromaticity for white point */
  signed char spc[2],     /* the spaces the two colors belong to */
    ii;                   /* which (0 or 1) of the two values is current */
} dyeColor;

/* methodsDye.c */
extern dye_export const char *dyeBiffKey;
extern dye_export char dyeSpaceToStr[][AIR_STRLEN_SMALL];
extern int dyeStrToSpace(char *str);
extern dyeColor *dyeColorInit(dyeColor *col);
extern dyeColor *dyeColorSet(dyeColor *col, int space, 
			     float v0, float v1, float v2);
extern int dyeColorGet(float *v0P, float *v1P, float *v2P, dyeColor *col);
extern int dyeColorGetAs(float *v0P, float *v1P, float *v2P, 
			 dyeColor *col, int space);
extern dyeColor *dyeColorNew();
extern dyeColor *dyeColorCopy(dyeColor *c1, dyeColor *c0);
extern dyeColor *dyeColorNix(dyeColor *col);
extern int dyeColorParse(dyeColor *col, char *str);
extern char *dyeColorSprintf(char *str, dyeColor *col);

/* convertDye.c */
typedef void (*dyeConverter)(float*, float*, float*, float, float, float);
extern void dyeRGBtoHSV(float *H, float *S, float *V,
			float  R, float  G, float  B);
extern void dyeHSVtoRGB(float *R, float *G, float *B,
			float  H, float  S, float  V);
extern void dyeRGBtoHSL(float *H, float *S, float *L,
			float  R, float  G, float  B);
extern void dyeHSLtoRGB(float *R, float *G, float *B,
			float  H, float  S, float  L);
extern void dyeRGBtoXYZ(float *X, float *Y, float *Z,
			float  R, float  G, float  B);
extern void dyeXYZtoRGB(float *R, float *G, float *B,
			float  X, float  Y, float  Z);
extern void dyeXYZtoLAB(float *L, float *A, float *B,
			float  X, float  Y, float  Z);
extern void dyeXYZtoLUV(float *L, float *U, float *V,
			float  X, float  Y, float  Z);
extern void dyeLABtoXYZ(float *X, float *Y, float *Z,
			float  L, float  A, float  B);
extern void dyeLUVtoXYZ(float *X, float *Y, float *Z,
			float  L, float  U, float  V);
extern dye_export dyeConverter 
dyeSimpleConvert[DYE_MAX_SPACE+1][DYE_MAX_SPACE+1];
extern int dyeConvert(dyeColor *col, int space);

#ifdef __cplusplus
}
#endif

#endif /* DYE_HAS_BEEN_INCLUDED */
