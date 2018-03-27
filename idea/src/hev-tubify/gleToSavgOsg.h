
#ifndef _GLE_TO_SAVG_OSG_H_

#define _GLE_TO_SAVG_OSG_H_   1


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gle.h"
#include "vvector.h"


#define OUTPUT_FORMAT_SAVG  ('s')
#define OUTPUT_FORMAT_OSG   ('o')

int setExtrusionOutputFormat (int format);

void writeExtrusionComment (char *comment);

int BGNTMESH(int i, double len);

int ENDTMESH(void);

int ENDPOLYGON(void);

int BGNPOLYGON(void);

int V3F_F(float *x, int j, int id);

int V3F_D(double *x, int j, int id);

int N3F_F(float *x);

int N3F_D(double *x);

int C3F(float *x);

int T2F_F(float x, double y);

int T2F_D(double x, double y);

int POPMATRIX(void);

int PUSHMATRIX(void);

int MULTMATRIX_F(float *x);

int LOADMATRIX_F(float *x);

int LOADMATRIX_D(double *x);

int MULTMATRIX_D(double x[4][4]);



#endif  /*   _GLE_TO_SAVG_OSG_H_  */

