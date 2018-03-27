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

/*
** "16simple": 16 bit representation based on 
** <http://www.gamedev.net/reference/articles/article1252.asp>
**
** info: [z sign] [y sign] [x sign] [y component] [x component]
** bits:    1        1        1          7             6
**
** despite appearances, this is isotropic in X and Y
*/

int
limnQNBytes[LIMN_QN_MAX+1] = {
  0, /* limnQN_Unknown */
  2, /* limnQN_16checker */
  2, /* limnQN_16simple */
  2, /* limnQN_16border1 */
  2  /* limnQN_15checker */
};


void
_limnQN_16simple_QNtoV(float *vec, int qn) {
  int xi, yi;
  float x, y, z, n;
  
  xi = qn & 0x3F;
  yi = (qn >> 6) & 0x7F;
  if (xi + yi >= 127) {
    xi = 127 - xi;
    yi = 127 - yi;
  }
  x = xi/126.0;
  y = yi/126.0;
  z = 1.0 - x - y;
  x = (qn & 0x2000) ? -x : x;
  y = (qn & 0x4000) ? -y : y;
  z = (qn & 0x8000) ? -z : z;
  n = 1.0/sqrt(x*x + y*y + z*z);
  vec[0] = x*n;
  vec[1] = y*n;
  vec[2] = z*n;
}

int
_limnQN_16simple_VtoQN(float *vec) {
  float x, y, z, L, w;
  int sgn = 0;
  int xi, yi;

  x = vec[0];
  y = vec[1];
  z = vec[2];
  if (x < 0) {
    sgn |= 0x2000;
    x = -x;
  }
  if (y < 0) {
    sgn |= 0x4000;
    y = -y;
  }
  if (z < 0) {
    sgn |= 0x8000;
    z = -z;
  }
  L = x + y + z;
  if (!L) {
    return 0;
  }
  w = 126.0/L;
  xi = x*w;
  yi = y*w;
  if (xi >= 64) {
    xi = 127 - xi;
    yi = 127 - yi;
  }
  return sgn | (yi << 6) | xi;
}

/* ----------------------------------------------------------------  */

  /*
    x =  [  1.0   1.0 ] u
    y    [  1.0  -1.0 ] v
  */

  /*
    u =  [  0.5   0.5 ] x
    v    [  0.5  -0.5 ] y
  */

  /* xor of low bits == 0 --> z<0 ; xor == 1 --> z>0 */

/* May 11 2003 GK verified that except at equator seam,
   the error due to quantization is unbiased */

void
_limnQN_16checker_QNtoV(float *vec, int qn) {
  int ui, vi;
  float u, v, x, y, z, n;

  ui = qn & 0xFF;
  vi = (qn >> 8) & 0xFF;
  u = AIR_AFFINE(0, ui, 255, -0.5, 0.5);
  v = AIR_AFFINE(0, vi, 255, -0.5, 0.5);
  x =  u + v;
  y =  u - v;
  z = 1 - AIR_ABS(x) - AIR_ABS(y);
  /* would this be better served by a branch? */
  z *= (((ui ^ vi) & 0x01) << 1) - 1;
  n = 1.0/sqrt(x*x + y*y + z*z);
  vec[0] = x*n; 
  vec[1] = y*n; 
  vec[2] = z*n;
}

int
_limnQN_16checker_VtoQN(float *vec) {
  float L, x, y, z;
  int xi, yi, ui, vi;
  
  x = vec[0];
  y = vec[1];
  z = vec[2];
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z);
  if (!L) {
    return 0;
  }
  x /= L;
  y /= L;
  if (z > 0) {
    AIR_INDEX(-1.0, x, 1.0, 255, xi);
    AIR_INDEX(-1.0 - 1.0/255, y, 1.0 + 1.0/255, 256, yi);
    ui = xi + yi - 127;
    vi = xi - yi + 128;
  }
  else {
    AIR_INDEX(-1.0 - 1.0/255, x, 1.0 + 1.0/255, 256, xi);
    AIR_INDEX(-1, y, 1, 255, yi);
    ui = xi + yi - 127;
    vi = xi - yi + 127;
  }
  return (vi << 8) | ui;
}

/* ----------------------------------------------------------------  */

void
_limnQN_16border1_QNtoV(float *vec, int qn) {
  int ui, vi;
  float u, v, x, y, z, n;
  
  ui = qn & 0xFF;
  vi = qn >> 8;
  u = AIR_AFFINE(0.5, ui, 254.5, -0.5, 0.5);
  v = AIR_AFFINE(0.5, vi, 254.5, -0.5, 0.5);
  x =  u + v;
  y =  u - v;
  z = 1 - AIR_ABS(x) - AIR_ABS(y);
  z *= (((ui ^ vi) & 0x01) << 1) - 1;
  n = 1.0/sqrt(x*x + y*y + z*z);
  vec[0] = x*n; 
  vec[1] = y*n; 
  vec[2] = z*n;
}

int
_limnQN_16border1_VtoQN(float *vec) {
  float L, u, v, x, y, z;
  int ui, vi, zi;
  char me[]="limnQNVto16PB1";
  
  x = vec[0];
  y = vec[1];
  z = vec[2];
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z);
  if (!L) {
    return 0;
  }
  x /= L;
  y /= L;
  u = x + y;
  v = x - y;
  AIR_INDEX(-1, u, 1, 254, ui); ui++;
  AIR_INDEX(-1, v, 1, 254, vi); vi++;
  zi = (ui ^ vi) & 0x01;
  if (!zi && z > 1.0/128.0) {
    ui -= (((ui >> 7) & 0x01) << 1) - 1;
  } 
  else if (zi && z < -1.0/128.0) {
    vi -= (((vi >> 7) & 0x01) << 1) - 1;
  }
  zi = (ui ^ vi) & 0x01;
  if (!zi && z > 1.0/127.0) {
    printf("%s: panic01\n", me);
  } 
  else if (zi && z < -1.0/127.0) {
    printf("%s: panic02\n", me);
  }
  return (vi << 8) | ui;
}

/* ----------------------------------------------------------------  */

void
_limnQN_15checker_QNtoV(float *vec, int qn) {
  int ui, vi, zi;
  float u, v, x, y, z, n;
  
  ui = qn & 0x7F;
  vi = (qn >> 7) & 0x7F;
  zi = (qn >> 14) & 0x01;
  u = AIR_AFFINE(-0.5, ui, 127.5, -0.5, 0.5);
  v = AIR_AFFINE(-0.5, vi, 127.5, -0.5, 0.5);
  x =  u + v;
  y =  u - v;
  z = 1 - AIR_ABS(x) - AIR_ABS(y);
  z *= (zi << 1) - 1;
  n = 1.0/sqrt(x*x + y*y + z*z);
  vec[0] = x*n; 
  vec[1] = y*n; 
  vec[2] = z*n;
}

int
_limnQN_15checker_VtoQN(float *vec) {
  float L, u, v, x, y, z;
  int ui, vi, zi;
  
  x = vec[0];
  y = vec[1];
  z = vec[2];
  L = AIR_ABS(x) + AIR_ABS(y) + AIR_ABS(z);
  if (!L) {
    return 0;
  }
  x /= L;
  y /= L;
  u = x + y;
  v = x - y;
  AIR_INDEX(-1, u, 1, 128, ui);
  AIR_INDEX(-1, v, 1, 128, vi);
  zi = (z > 0);
  return (zi << 14) | (vi << 7) | ui;
}

void (*limnQNtoV[LIMN_QN_MAX+1])(float *, int) = {
  NULL,
  _limnQN_16checker_QNtoV,
  _limnQN_16simple_QNtoV,
  _limnQN_16border1_QNtoV,
  _limnQN_15checker_QNtoV
};
  
int (*limnVtoQN[LIMN_QN_MAX+1])(float *vec) = {
  NULL,
  _limnQN_16checker_VtoQN,
  _limnQN_16simple_VtoQN,
  _limnQN_16border1_VtoQN,
  _limnQN_15checker_VtoQN
};

