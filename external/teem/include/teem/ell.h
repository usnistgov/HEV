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

#ifndef ELL_HAS_BEEN_INCLUDED
#define ELL_HAS_BEEN_INCLUDED

#include <math.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/nrrd.h>

#include "ellMacros.h"

#if defined(_WIN32) && !defined(TEEM_STATIC)
#define ell_export __declspec(dllimport)
#else
#define ell_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ELL ell_biff_key
#define ELL_EPS 1.0e-10

/*
******** ell_cubic_root* enum
**
** return values for ell_cubic
*/
enum {
  ell_cubic_root_unknown,         /* 0 */
  ell_cubic_root_single,          /* 1 */
  ell_cubic_root_triple,          /* 2 */
  ell_cubic_root_single_double,   /* 3 */
  ell_cubic_root_three,           /* 4 */
  ell_cubic_root_last             /* 5 */
};

/* 
** Note: matrix element ordering is:
**
**     0   3   6
**     1   4   7      for 3x3
**     2   5   8
**
**   0   4   8  12
**   1   5   9  13    for 4x4
**   2   6  10  14
**   3   7  11  15
**
** and all vectors are COLUMN vectors
*/

/* miscEll.c */
extern ell_export const char *ell_biff_key;
extern ell_export int ell_debug;
extern void ell_3m_print_f(FILE *f, float s[9]);
extern void ell_3v_print_f(FILE *f, float s[3]);
extern void ell_3m_print_d(FILE *f, double s[9]);
extern void ell_3v_print_d(FILE *f, double s[3]);
extern void ell_4m_print_f(FILE *f, float s[16]);
extern void ell_4v_print_f(FILE *f, float s[4]);
extern void ell_4m_print_d(FILE *f, double s[16]);
extern void ell_4v_print_d(FILE *f, double s[4]);

/* vecEll.c */
extern void ell_3v_perp_f(float p[3], float v[3]);
extern void ell_3v_perp_d(double p[3], double v[3]);
extern void ell_3mv_mul_f(float v2[3], float m[9], float v1[3]);
extern void ell_3mv_mul_d(double v2[3], double m[9], double v1[3]);
extern void ell_4mv_mul_f(float v2[4], float m[16], float v1[4]);
extern void ell_4mv_mul_d(double v2[4], double m[16], double v1[4]);

/* mat.c */
extern void ell_3m_mul_f(float m3[9], float m1[9], float m2[9]);
extern void ell_3m_mul_d(double m3[9], double m1[9], double m2[9]);
extern void ell_3m_pre_mul_f(float m[9], float x[9]);
extern void ell_3m_pre_mul_d(double m[9], double x[9]);
extern void ell_3m_post_mul_f(float m[9], float x[9]);
extern void ell_3m_post_mul_d(double m[9], double x[9]);
extern float ell_3m_det_f(float m[9]);
extern double ell_3m_det_d(double m[9]);
extern void ell_3m_inv_f(float i[9], float m[9]);
extern void ell_3m_inv_d(double i[9], double m[9]);
extern void ell_4m_mul_f(float m3[16], float m1[16], float m2[16]);
extern void ell_4m_mul_d(double m3[16], double m1[16], double m2[16]);
extern void ell_4m_pre_mul_f(float m[16], float x[16]);
extern void ell_4m_pre_mul_d(double m[16], double x[16]);
extern void ell_4m_post_mul_f(float m[16], float x[16]);
extern void ell_4m_post_mul_d(double m[16], double x[16]);
extern float ell_4m_det_f(float m[16]);
extern double ell_4m_det_d(double m[16]);
extern void ell_4m_inv_f(float i[16], float m[16]);
extern void ell_4m_inv_d(double i[16], double m[16]);

/* 
** Note: quaternion element ordering is:
**
**   w  x  y  z
**   0  1  2  3
**
**  where w is the real component and (x,y,z) is the imaginary component
**
** Nowhere in ell is there the assumption that any given quaternion is
** a unit-length quaternion.  Sometimes returned quaternions will be 
** unit-length, and sometimes given quaternions must be normalized internally
** prior to doing some operation, but its not something that ell users
** need worry about.
*/

/* quat.c */
extern void ell_3m_to_q_f( float q[4],  float m[9]);
extern void ell_3m_to_q_d(double q[4], double m[9]);
extern void ell_4m_to_q_f( float q[4],  float m[16]);
extern void ell_4m_to_q_d(double q[4], double m[16]);
extern void ell_q_to_3m_f( float m[9],  float q[4]);
extern void ell_q_to_3m_d(double m[9], double q[4]);
extern void ell_q_to_4m_f( float m[16],  float q[4]);
extern void ell_q_to_4m_d(double m[16], double q[4]);
extern  float ell_q_to_aa_f( float axis[3],  float q[4]);
extern double ell_q_to_aa_d(double axis[3], double q[4]);
extern void ell_aa_to_q_f( float q[4],  float angle,  float axis[3]);
extern void ell_aa_to_q_d(double q[4], double angle, double axis[3]);
extern void ell_aa_to_3m_f( float m[9],  float angle,  float axis[3]);
extern void ell_aa_to_3m_d(double m[9], double angle, double axis[3]);
extern void ell_aa_to_4m_f( float m[16],  float angle,  float axis[3]);
extern void ell_aa_to_4m_d(double m[16], double angle, double axis[3]);
extern  float ell_3m_to_aa_f( float axis[3],  float m[9]);
extern double ell_3m_to_aa_d(double axis[3], double m[9]);
extern  float ell_4m_to_aa_f( float axis[3],  float m[16]);
extern double ell_4m_to_aa_d(double axis[3], double m[16]);
extern void ell_q_mul_f( float q3[4],  float q1[4],  float q2[4]);
extern void ell_q_mul_d(double q3[4], double q1[4], double q2[4]);
extern void ell_q_inv_f( float qi[4],  float q[4]);
extern void ell_q_inv_d(double qi[4], double q[4]);
extern void ell_q_pow_f( float q2[4],  float q1[4],  float p);
extern void ell_q_pow_d(double q2[4], double q1[4], double p);
extern void ell_q_div_f( float q3[4],  float q1[4],  float q2[4]);
extern void ell_q_div_d(double q3[4], double q1[4], double q2[4]);
extern void ell_q_exp_f( float q2[4],  float q1[4]);
extern void ell_q_exp_d(double q2[4], double q1[4]);
extern void ell_q_log_f( float q2[4],  float q1[4]);
extern void ell_q_log_d(double q2[4], double q1[4]);
extern void ell_q_3v_rotate_f( float v2[3],  float q[4],  float v1[3]);
extern void ell_q_3v_rotate_d(double v2[3], double q[4], double v1[3]);
extern void ell_q_4v_rotate_f( float v2[4],  float q[4],  float v1[4]);
extern void ell_q_4v_rotate_d(double v2[4], double q[4], double v1[4]);
extern void ell_q_avg4_d(double avg[4], double eps, double wght[4],
			 double q1[4], double q2[4],
			 double q3[4], double q4[4]);

/* genmat.c */
extern int ell_Nm_valid(Nrrd *mat);
extern int ell_Nm_tran(Nrrd *dest, Nrrd *src);
extern int ell_Nm_mul(Nrrd *dest, Nrrd *A, Nrrd *B);
extern int ell_Nm_inv(Nrrd *dest, Nrrd *src);
extern int ell_Nm_pseudo_inv(Nrrd *dest, Nrrd *src);

/* cubic.c */
extern int ell_cubic(double root[3], double A, double B, double C, int newton);

/* eigen.c */
extern void ell_3m_1d_nullspace_d(double ans[3], double n[9]);
extern void ell_3m_2d_nullspace_d(double ans0[3], double ans1[3], double n[9]);
extern int ell_3m_eigenvalues_d(double eval[3], double m[9], 
				int newton);
extern int ell_3m_eigensolve_d(double eval[3], double evec[9], double m[9],
			       int newton);

#ifdef __cplusplus
}
#endif

#endif /* ELL_HAS_BEEN_INCLUDED */
