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

#include "ell.h"

#define W 0
#define X 1
#define Y 2
#define Z 3

/*
 0  3  6
 1  4  7
 2  5  8 

 0   4   8  12
 1   5   9  13
 2   6  10  14
 3   7  11  15
*/

/*
** note: this will always produce a unit length quaternion, 
** by the ELL_4V_NORM at the end.  However, for proper rotation
** matrices, that normalization should be far from a divide by zero,
** so it should be stable.  It *IS* necessary, since it accomplishes
** division by w, x, y, or z, whichever's squared magnitude is biggest
*/
#define _ELL_M_TO_Q(type, i0, i1, i2, i3, i4, i5, i6, i7, i8)  \
  type s[4], wx, wy, wz, xy, xz, yz, len; \
  int mi;                                 \
                                          \
  s[W] = 1 + m[i0] + m[i4] + m[i8];       \
  s[X] = 1 + m[i0] - m[i4] - m[i8];       \
  s[Y] = 1 - m[i0] + m[i4] - m[i8];       \
  s[Z] = 1 - m[i0] - m[i4] + m[i8];       \
  wx = m[i5] - m[i7];                     \
  wy = m[i6] - m[i2];                     \
  wz = m[i1] - m[i3];                     \
  xy = m[i1] + m[i3];                     \
  xz = m[i2] + m[i6];                     \
  yz = m[i5] + m[i7];                     \
  mi =  s[W] > s[X] ?  W : X;             \
  mi = s[mi] > s[Y] ? mi : Y;             \
  mi = s[mi] > s[Z] ? mi : Z;             \
  switch (mi) {                           \
  case W:                                 \
    ELL_4V_SET(q, s[W],  wx,  wy,  wz);   \
    break;                                \
  case X:                                 \
    ELL_4V_SET(q,  wx, s[X],  xy,  xz);   \
    break;                                \
  case Y:                                 \
    ELL_4V_SET(q,  wy,  xy, s[Y],  yz);   \
    break;                                \
  case Z:                                 \
    ELL_4V_SET(q,  wz,  xz,  yz, s[Z]);   \
    break;                                \
  }                                       \
  ELL_4V_NORM(q, q, len)

void
ell_3m_to_q_f(float q[4], float m[9]) {
  _ELL_M_TO_Q( float, 0, 1, 2,    3, 4, 5,    6, 7, 8);
}

void 
ell_3m_to_q_d(double q[4], double m[9]) {
  _ELL_M_TO_Q(double, 0, 1, 2,    3, 4, 5,    6, 7, 8);
}

void 
ell_4m_to_q_f(float q[4], float m[16]) {
  _ELL_M_TO_Q( float, 0, 1, 2,    4, 5, 6,    8, 9, 10);
}

void 
ell_4m_to_q_d(double q[4], double m[16]) {
  _ELL_M_TO_Q(double, 0, 1, 2,    4, 5, 6,    8, 9, 10);
}

/*
** note: normalizes the quaternion on the way in, to insure
** creation of a proper rotation matrix.  Without the normalization
** the coefficients in the matrix would be off by a factor of 
** w*w + x*x + y*y + z*z
*/
#define _ELL_Q_TO_3M(type)           \
  type u[4], w, x, y, z;             \
                                     \
  ELL_4V_NORM(u, q, w);              \
  ELL_4V_GET(w, x, y, z, u);         \
  ELL_3V_SET(m+0,                    \
             w*w + x*x - y*y - z*z,  \
             2*(x*y + w*z),          \
             2*(x*z - w*y));         \
  ELL_3V_SET(m+3,                    \
             2*(x*y - w*z),          \
             w*w - x*x + y*y - z*z,  \
             2*(y*z + w*x));         \
  ELL_3V_SET(m+6,                    \
             2*(x*z + w*y),          \
             2*(y*z - w*x),          \
             w*w - x*x - y*y + z*z)

void 
ell_q_to_3m_f(float m[9], float q[4]) {
  _ELL_Q_TO_3M(float);
}

void 
ell_q_to_3m_d(double m[9], double q[4]) {
  _ELL_Q_TO_3M(double);
}

#define _ELL_Q_TO_4M(type)           \
  type u[4], w, x, y, z;             \
                                     \
  ELL_4V_NORM(u, q, w);              \
  ELL_4V_GET(w, x, y, z, u);         \
  ELL_4V_SET(m+0,                    \
             w*w + x*x - y*y - z*z,  \
             2*(x*y + w*z),          \
             2*(x*z - w*y),          \
             0);                     \
  ELL_4V_SET(m+4,                    \
             2*(x*y - w*z),          \
             w*w - x*x + y*y - z*z,  \
             2*(y*z + w*x),          \
             0);                     \
  ELL_4V_SET(m+8,                    \
             2*(x*z + w*y),          \
             2*(y*z - w*x),          \
             w*w - x*x - y*y + z*z,  \
             0);                     \
  ELL_4V_SET(m+12, 0, 0, 0, 1)

void 
ell_q_to_4m_f(float m[16], float q[4]) {
  _ELL_Q_TO_4M(float);
}

void 
ell_q_to_4m_d(double m[16], double q[4]) {
  _ELL_Q_TO_4M(double);
}

/*
** note: by the use of atan2, this does NOT assume a 
** a unit-length quaternion.  The axis output, however,
** will always be unit length, even if the quaternion was
** purely real (rotation angle is zero)
*/
#define _ELL_Q_TO_AA(type)                      \
  type len, angle;                              \
                                                \
  len = ELL_3V_LEN(q+1);                        \
  angle = atan2(len, q[0]);                     \
  if (len) {                                    \
    ELL_3V_SCALE(axis, 1.0/len, q+1);           \
    ELL_3V_NORM(axis, axis, len);               \
  } else {                                      \
    ELL_3V_SET(axis, 1, 0, 0);                  \
  }                                             \
  return 2*angle

float
ell_q_to_aa_f(float axis[3], float q[4]) {
  _ELL_Q_TO_AA(float);
}

double
ell_q_to_aa_d(double axis[3], double q[4]) {
  _ELL_Q_TO_AA(double);
}

/*
** note: assuming that axis is unit length, this produces a 
** a unit length quaternion
*/
#define _ELL_AA_TO_Q(type)                                         \
  float sa;                                                        \
                                                                   \
  sa = sin(angle/2);                                               \
  ELL_4V_SET(q, cos(angle/2), sa*axis[0], sa*axis[1], sa*axis[2])

void 
ell_aa_to_q_f(float q[4], float angle, float axis[3]) {
  _ELL_AA_TO_Q(float);
}

void 
ell_aa_to_q_d(double q[4], double angle, double axis[3]) {
  _ELL_AA_TO_Q(double);
}

float
ell_3m_to_aa_f( float axis[3],  float m[9]) {
  float q[4];

  ell_3m_to_q_f(q, m);
  return ell_q_to_aa_f(axis, q);
}

double
ell_3m_to_aa_d(double axis[3], double m[9]) {
  double q[4];

  ell_3m_to_q_d(q, m);
  return ell_q_to_aa_d(axis, q);
}

float
ell_4m_to_aa_f( float axis[3],  float m[16]) {
  float q[4];

  ell_4m_to_q_f(q, m);
  return ell_q_to_aa_f(axis, q);
}

double
ell_4m_to_aa_d(double axis[3], double m[16]) {
  double q[4];

  ell_4m_to_q_d(q, m);
  return ell_q_to_aa_d(axis, q);
}

void
ell_aa_to_3m_f( float m[9],  float angle,  float axis[3]) {
  float q[4];

  ell_aa_to_q_f(q, angle, axis);
  ell_q_to_3m_f(m, q);
}

void
ell_aa_to_3m_d(double m[9], double angle, double axis[3]) {
  double q[4];

  ell_aa_to_q_d(q, angle, axis);
  ell_q_to_3m_d(m, q);
}

void
ell_aa_to_4m_f( float m[16],  float angle,  float axis[3]) {
  float q[4];

  ell_aa_to_q_f(q, angle, axis);
  ell_q_to_4m_f(m, q);
}

void
ell_aa_to_4m_d(double m[16], double angle, double axis[3]) {
  double q[4];

  ell_aa_to_q_d(q, angle, axis);
  ell_q_to_4m_d(m, q);
}

void 
ell_q_mul_f(float q3[4], float q1[4], float q2[4]) {
  ELL_Q_MUL(q3, q1, q2);
}

void 
ell_q_mul_d(double q3[4], double q1[4], double q2[4]) {
  ELL_Q_MUL(q3, q1, q2);
}

void 
ell_q_inv_f(float qi[4], float q[4]) {
  float N;
  ELL_Q_INV(qi, q, N);
}

void 
ell_q_inv_d(double qi[4], double q[4]) {
  double N;
  ELL_Q_INV(qi, q, N);
}

/*
**  div(a, b) = a^-1 * b
*/
void
ell_q_div_f(float q3[4], float q1[4], float q2[4]) {
  float N, q1i[4];

  ELL_Q_INV(q1i, q1, N);
  ELL_Q_MUL(q3, q1i, q2);
}

void
ell_q_div_d(double q3[4], double q1[4], double q2[4]) {
  float N, q1i[4];

  ELL_Q_INV(q1i, q1, N);
  ELL_Q_MUL(q3, q1i, q2);
}

/*
** this is good for *ALL* quaternions, any length, including zero.
** the behavior on the zero quaternion is governed by the behavior
** of the log() and atan2() functions in the math library
**
** the basic insight is that doing conversion to angle/axis,
** and doing the atan2(l2(x,y,z),w),
** and that doing a logarithm, are all basically the same thing
*/

void 
ell_q_log_f(float q2[4], float q1[4]) {
  float a, b, axis[3];

  a = log(ELL_4V_LEN(q1));
  b = ell_q_to_aa_f(axis, q1)/2;
  ELL_4V_SET(q2, a, b*axis[0], b*axis[1], b*axis[2]);
}

void 
ell_q_log_d(double q2[4], double q1[4]) {
  double a, b, axis[3];

  a = log(ELL_4V_LEN(q1));
  b = ell_q_to_aa_d(axis, q1)/2;
  ELL_4V_SET(q2, a, b*axis[0], b*axis[1], b*axis[2]);
}

/*
** this is good for *ALL* quaternions, any length, including zero
*/
#define _ELL_Q_EXP(type)                                                 \
  type ea, b, sb, axis[3], tmp;                                          \
                                                                         \
  ea = exp(q1[0]);                                                       \
  b = ELL_3V_LEN(q1+1);                                                  \
  if (b) {                                                               \
    ELL_3V_SCALE(axis, 1/b, q1+1);                                       \
    ELL_3V_NORM(axis, axis, tmp);                                        \
  } else {                                                               \
    ELL_3V_SET(axis, 1, 0, 0);                                           \
  }                                                                      \
  sb = sin(b);                                                           \
  ELL_4V_SET(q2, ea*cos(b), ea*sb*axis[0], ea*sb*axis[1], ea*sb*axis[2])

void 
ell_q_exp_f(float q2[4], float q1[4]) {
  _ELL_Q_EXP(float);
}

void 
ell_q_exp_d(double q2[4], double q1[4]) {
  _ELL_Q_EXP(double);
}

void 
ell_q_pow_f(float q2[4], float q1[4], float p) {
  float len, angle, axis[3];

  len = pow(ELL_4V_LEN(q1), p);
  angle = ell_q_to_aa_f(axis, q1);
  ell_aa_to_q_f(q2, p*angle, axis);
  ELL_4V_SCALE(q2, len, q2);
}

void 
ell_q_pow_d(double q2[4], double q1[4], double p) {
  double len, angle, axis[3];

  len = pow(ELL_4V_LEN(q1), p);
  angle = ell_q_to_aa_d(axis, q1);
  ell_aa_to_q_d(q2, p*angle, axis);
  ELL_4V_SCALE(q2, len, q2);
}

/*
** by the wonders of quaternions, this rotation will be the 
** same regardless of the quaternion length.  This is in
** contrast to doing rotation by first converting to matrix,
** in which an explicit normalization is required.  There is
** a divide here (in ELL_Q_INV), but there's no sqrt().
*/
#define _ELL_Q3V_ROT(type)               \
  type n, a[4], b[4], c[4];              \
                                         \
  ELL_4V_SET(a, 0, v1[0], v1[1], v1[2]); \
  ELL_Q_INV(b, q, n);                    \
  ELL_Q_MUL(c, a, b);                    \
  ELL_Q_MUL(a, q, c);                    \
  ELL_3V_COPY(v2, a+1)

void 
ell_q_3v_rotate_f( float v2[3],  float q[4],  float v1[3]) {
  _ELL_Q3V_ROT(float);
}

void 
ell_q_3v_rotate_d(double v2[3], double q[4], double v1[3]) {
  _ELL_Q3V_ROT(double);
}

/*
** we start by ignoring the last (homogenous) coordinate of
** the vector, but then we copy it to the output
*/
void 
ell_q_4v_rotate_f( float v2[4],  float q[4],  float v1[4]) {
  _ELL_Q3V_ROT(float);
  v2[3] = v1[3];
}

void 
ell_q_4v_rotate_d(double v2[4], double q[4], double v1[4]) {
  _ELL_Q3V_ROT(float);
  v2[3] = v1[3];
}

#define _ELL_Q_AVG_ITER_MAX 30

void
ell_q_avg4_d(double m[4], double eps, double _wght[4],
	     double _q1[4], double _q2[4], double _q3[4], double _q4[4]) {
  double N, err, a[4], b[4], c[4], d[4], 
    tmp[4], la[4], lb[4], lc[4], ld[4], u[4], wght[4];
  int iter;
  
  /* normalize (wrt L2) all given quaternions */
  ELL_4V_NORM(a, _q1, N);
  ELL_4V_NORM(b, _q2, N);
  ELL_4V_NORM(c, _q3, N);
  ELL_4V_NORM(d, _q4, N);

  /* normalize (wrt L1) the given weights */
  ELL_4V_COPY(wght, _wght);
  N = wght[0] + wght[1] + wght[2] + wght[3];
  ELL_4V_SCALE(wght, 1/N, wght);

  /* initialize mean to normalized euclidean mean */
  ELL_4V_SCALE_ADD4(m, wght[0], a, wght[1], b, wght[2], c, wght[3], d);
  ELL_4V_NORM(m, m, N);

  iter = 0;
  do {
    /* take log of everyone */
    ell_q_div_d(tmp, m, a); ell_q_log_d(la, tmp);
    ell_q_div_d(tmp, m, b); ell_q_log_d(lb, tmp);
    ell_q_div_d(tmp, m, c); ell_q_log_d(lc, tmp);
    ell_q_div_d(tmp, m, d); ell_q_log_d(ld, tmp);
    /* average, and find length */
    ELL_4V_SCALE_ADD4(u, wght[0], la, wght[1], lb, wght[2], lc, wght[3], ld);
    err = ELL_4V_LEN(u);
    /* use exp to put it back on S^3 */
    ell_q_exp_d(tmp, u); ell_q_mul_d(m, m, tmp);
    iter++;
  } while (iter < _ELL_Q_AVG_ITER_MAX && err > eps);
  if (err > eps) {
    fprintf(stderr, "%s: still have error %g after %d iterations\n", 
	    "ell_q_avg4_d", err, _ELL_Q_AVG_ITER_MAX);
  }
  return;
}

