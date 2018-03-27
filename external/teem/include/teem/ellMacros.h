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

#ifndef ELLMACROS_HAS_BEEN_INCLUDED
#define ELLMACROS_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
******** ELL_SWAP2, ELL_SWAP3
**
** used to interchange 2 or 3 values, using the given temp variable
*/
#define ELL_SWAP2(a, b, t)    ((t)=(a),(a)=(b),(b)=(t))
#define ELL_SWAP3(a, b, c, t) ((t)=(a),(a)=(b),(b)=(c),(c)=(t))

/*
******** ELL_SORT3
**
** sorts v0, v1, v2 in descending order, using given temp variable t,
*/
#define ELL_SORT3(v0, v1, v2, t)             \
  if (v0 > v1) {                             \
    if (v1 < v2) {                           \
      if (v0 > v2) { ELL_SWAP2(v1, v2, t); } \
      else { ELL_SWAP3(v0, v2, v1, t); }     \
    }                                        \
  }                                          \
  else {                                     \
    if (v1 > v2) {                           \
      if (v0 > v2) { ELL_SWAP2(v0, v1, t); } \
      else { ELL_SWAP3(v0, v1, v2, t); }     \
    }                                        \
    else {                                   \
      ELL_SWAP2(v0, v2, t);                  \
    }                                        \
  }

/*
******** ELL_MAX3_IDX
**
** returns 0, 1, 2, to indicate which of the three arguments is largest
*/
#define ELL_MAX3_IDX(v0, v1, v2) \
  (v0 > v1                       \
   ? (v1 > v2                    \
      ? 0                        \
      : (v0 > v2                 \
	 ? 0                     \
	 : 2))                   \
   : (v2 > v1                    \
      ? 2                        \
      : 1))

/*
******** ELL_MIN3_IDX
**
** returns 0, 1, 2, to indicate which of the three arguments is smallest
*/
#define ELL_MIN3_IDX(v0, v1, v2) \
  (v0 < v1                       \
   ? (v1 < v2                    \
      ? 0                        \
      : (v0 < v2                 \
	 ? 0                     \
	 : 2))                   \
   : (v2 < v1                    \
      ? 2                        \
      : 1))

#define _ELL_2M_DET(a,b,c,d) ((a)*(d) - (c)*(b))

#define ELL_2M_DET(m) _ELL_2M_DET((m)[0],(m)[1],(m)[2],(m)[3])

/*
** the 3x3 matrix-related macros assume that the matrix indexing is:
** 0  3  6
** 1  4  7
** 2  5  8
*/

#define ELL_3V_SET(v, a, b, c) \
  ((v)[0] = (a), (v)[1] = (b), (v)[2] = (c))

#define ELL_3V_GET(a, b, c, v) \
  ((a) = (v)[0], (b) = (v)[1], (c) = (v)[2])

#define ELL_3V_EQUAL(a, b) \
  ((a)[0] == (b)[0] && (a)[1] == (b)[1] && (a)[2] == (b)[2])

#define ELL_3V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0], (v2)[1] = (v1)[1], (v2)[2] = (v1)[2])

#define ELL_3V_INCR(v2, v1) \
  ((v2)[0] += (v1)[0],      \
   (v2)[1] += (v1)[1],      \
   (v2)[2] += (v1)[2])

#define ELL_3V_ADD2(v3, v1, v2) \
  ((v3)[0] = (v1)[0] + (v2)[0], \
   (v3)[1] = (v1)[1] + (v2)[1], \
   (v3)[2] = (v1)[2] + (v2)[2])

#define ELL_3V_ADD3(v4, v1, v2, v3)       \
  ((v4)[0] = (v1)[0] + (v2)[0] + (v3)[0], \
   (v4)[1] = (v1)[1] + (v2)[1] + (v3)[1], \
   (v4)[2] = (v1)[2] + (v2)[2] + (v3)[2])

#define ELL_3V_SUB(v3, v1, v2)  \
  ((v3)[0] = (v1)[0] - (v2)[0], \
   (v3)[1] = (v1)[1] - (v2)[1], \
   (v3)[2] = (v1)[2] - (v2)[2])

#define ELL_3V_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])

#define ELL_3V_SCALE(v2, a, v1) \
  ((v2)[0] = (a)*(v1)[0],       \
   (v2)[1] = (a)*(v1)[1],       \
   (v2)[2] = (a)*(v1)[2])

#define ELL_3V_SCALE_ADD2(v2, s0, v0, s1, v1) \
  ((v2)[0] = (s0)*(v0)[0] + (s1)*(v1)[0],     \
   (v2)[1] = (s0)*(v0)[1] + (s1)*(v1)[1],     \
   (v2)[2] = (s0)*(v0)[2] + (s1)*(v1)[2])

#define ELL_3V_SCALE_INCR(v2, s0, v0) \
  ((v2)[0] += (s0)*(v0)[0], \
   (v2)[1] += (s0)*(v0)[1], \
   (v2)[2] += (s0)*(v0)[2])

#define ELL_3V_SCALE_ADD3(v3, s0, v0, s1, v1, s2, v2)        \
  ((v3)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0],    \
   (v3)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1],    \
   (v3)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2])

#define ELL_3V_LEN(v) (sqrt(ELL_3V_DOT((v),(v))))

#define ELL_3V_NORM(v2, v1, length) \
  (length = ELL_3V_LEN(v1), ELL_3V_SCALE(v2, 1.0/length, v1))

#define ELL_3V_CROSS(v3, v1, v2) \
  ((v3)[0] = (v1)[1]*(v2)[2] - (v1)[2]*(v2)[1], \
   (v3)[1] = (v1)[2]*(v2)[0] - (v1)[0]*(v2)[2], \
   (v3)[2] = (v1)[0]*(v2)[1] - (v1)[1]*(v2)[0])

#define ELL_3V_MIN(v3,v1,v2) (         \
  (v3)[0] = AIR_MIN((v1)[0], (v2)[0]), \
  (v3)[1] = AIR_MIN((v1)[1], (v2)[1]), \
  (v3)[2] = AIR_MIN((v1)[2], (v2)[2]))

#define ELL_3V_MAX(v3,v1,v2) (         \
  (v3)[0] = AIR_MAX((v1)[0], (v2)[0]), \
  (v3)[1] = AIR_MAX((v1)[1], (v2)[1]), \
  (v3)[2] = AIR_MAX((v1)[2], (v2)[2]))

#define ELL_3V_AFFINE(v,i,x,I,o,O) ( \
  (v)[0] = AIR_AFFINE((i)[0],(x)[0],(I)[0],(o)[0],(O)[0]), \
  (v)[1] = AIR_AFFINE((i)[1],(x)[1],(I)[1],(o)[1],(O)[1]), \
  (v)[2] = AIR_AFFINE((i)[2],(x)[2],(I)[2],(o)[2],(O)[2]))

#define ELL_3V_ABS(v2,v1) ( \
  (v2)[0] = AIR_ABS((v1)[0]), \
  (v2)[1] = AIR_ABS((v1)[1]), \
  (v2)[2] = AIR_ABS((v1)[2]))

#define ELL_3M_SCALE(m2, s, m1) \
  (ELL_3V_SCALE((m2)+0, (s), (m1)+0), \
   ELL_3V_SCALE((m2)+3, (s), (m1)+3), \
   ELL_3V_SCALE((m2)+6, (s), (m1)+6))

#define ELL_3M_SCALE_ADD2(m2, s0, m0, s1, m1) \
  (ELL_3V_SCALE_ADD2((m2)+0, (s0), (m0)+0, (s1), (m1)+0), \
   ELL_3V_SCALE_ADD2((m2)+3, (s0), (m0)+3, (s1), (m1)+3), \
   ELL_3V_SCALE_ADD2((m2)+6, (s0), (m0)+6, (s1), (m1)+6))

#define ELL_3M_ADD2(m3, m1, m2) \
  ((m3)[0] = (m1)[0] + (m2)[0],  \
   (m3)[1] = (m1)[1] + (m2)[1],  \
   (m3)[2] = (m1)[2] + (m2)[2],  \
   (m3)[3] = (m1)[3] + (m2)[3],  \
   (m3)[4] = (m1)[4] + (m2)[4],  \
   (m3)[5] = (m1)[5] + (m2)[5],  \
   (m3)[6] = (m1)[6] + (m2)[6],  \
   (m3)[7] = (m1)[7] + (m2)[7],  \
   (m3)[8] = (m1)[8] + (m2)[8])

#define ELL_3M_SUB(m3, m1, m2) \
  ((m3)[0] = (m1)[0] - (m2)[0],  \
   (m3)[1] = (m1)[1] - (m2)[1],  \
   (m3)[2] = (m1)[2] - (m2)[2],  \
   (m3)[3] = (m1)[3] - (m2)[3],  \
   (m3)[4] = (m1)[4] - (m2)[4],  \
   (m3)[5] = (m1)[5] - (m2)[5],  \
   (m3)[6] = (m1)[6] - (m2)[6],  \
   (m3)[7] = (m1)[7] - (m2)[7],  \
   (m3)[8] = (m1)[8] - (m2)[8])

#define ELL_3M_COPY(m2, m1) \
  (ELL_3V_COPY((m2)+0, (m1)+0), \
   ELL_3V_COPY((m2)+3, (m1)+3), \
   ELL_3V_COPY((m2)+6, (m1)+6))

#define ELL_3M_IDENTITY_SET(m) \
  (ELL_3V_SET((m)+0,  1 ,  0 ,  0), \
   ELL_3V_SET((m)+3,  0 ,  1 ,  0), \
   ELL_3V_SET((m)+6,  0 ,  0 ,  1))

#define ELL_3M_ZERO_SET(m) \
  (ELL_3V_SET((m)+0,  0 ,  0 ,  0), \
   ELL_3V_SET((m)+3,  0 ,  0 ,  0), \
   ELL_3V_SET((m)+6,  0 ,  0 ,  0))

#define ELL_3M_DIAG_SET(m, a, b, c) \
  ((m)[0] = (a), (m)[4] = (b), (m)[8] = (c))

#define ELL_3M_TRAN(m2, m1) \
  ((m2)[0] = (m1)[0],            \
   (m2)[1] = (m1)[3],            \
   (m2)[2] = (m1)[6],            \
   (m2)[3] = (m1)[1],            \
   (m2)[4] = (m1)[4],            \
   (m2)[5] = (m1)[7],            \
   (m2)[6] = (m1)[2],            \
   (m2)[7] = (m1)[5],            \
   (m2)[8] = (m1)[8])

#define ELL_3M_TRAN_IP(m, t) \
  (ELL_SWAP2((m)[1],(m)[3],(t)),  \
   ELL_SWAP2((m)[2],(m)[6],(t)),  \
   ELL_SWAP2((m)[5],(m)[7],(t)))

#define ELL_3M_TRACE(m) ((m)[0] + (m)[4] + (m)[8])

#define ELL_3M_FROB(m) \
  (sqrt(ELL_3V_DOT((m)+0, (m)+0) + \
        ELL_3V_DOT((m)+3, (m)+3) + \
        ELL_3V_DOT((m)+6, (m)+6)))

#define _ELL_3M_DET(a,b,c,d,e,f,g,h,i) \
  (  (a)*(e)*(i) \
   + (d)*(h)*(c) \
   + (g)*(b)*(f) \
   - (g)*(e)*(c) \
   - (d)*(b)*(i) \
   - (a)*(h)*(f))

#define ELL_3M_DET(m) _ELL_3M_DET((m)[0],(m)[1],(m)[2],\
                                  (m)[3],(m)[4],(m)[5],\
                                  (m)[6],(m)[7],(m)[8])

#define ELL_3MV_COL0_GET(v, m) \
  (ELL_3V_SET((v), (m)[0], (m)[1], (m)[2]))

#define ELL_3MV_COL1_GET(v, m) \
  (ELL_3V_SET((v), (m)[3], (m)[4], (m)[5]))

#define ELL_3MV_COL2_GET(v, m) \
  (ELL_3V_SET((v), (m)[6], (m)[7], (m)[8]))

#define ELL_3MV_ROW0_GET(v, m) \
  (ELL_3V_SET((v), (m)[0], (m)[3], (m)[6]))

#define ELL_3MV_ROW1_GET(v, m) \
  (ELL_3V_SET((v), (m)[1], (m)[4], (m)[7]))

#define ELL_3MV_ROW2_GET(v, m) \
  (ELL_3V_SET((v), (m)[2], (m)[5], (m)[8]))

#define ELL_3MV_COL0_SET(m, v) \
  (ELL_3V_GET((m)[0], (m)[1], (m)[2], (v)))

#define ELL_3MV_COL1_SET(m, v) \
  (ELL_3V_GET((m)[3], (m)[4], (m)[5], (v)))

#define ELL_3MV_COL2_SET(m, v) \
  (ELL_3V_GET((m)[6], (m)[7], (m)[8], (v)))

#define ELL_3MV_ROW0_SET(m, v) \
  (ELL_3V_GET((m)[0], (m)[3], (m)[6], (v)))

#define ELL_3MV_ROW1_SET(m, v) \
  (ELL_3V_GET((m)[1], (m)[4], (m)[7], (v)))

#define ELL_3MV_ROW2_SET(m, v) \
  (ELL_3V_GET((m)[2], (m)[5], (m)[8], (v)))

#define ELL_3MV_OUTER(m, v1, v2) \
  (ELL_3V_SCALE((m)+0, (v2)[0], (v1)), \
   ELL_3V_SCALE((m)+3, (v2)[1], (v1)), \
   ELL_3V_SCALE((m)+6, (v2)[2], (v1)))

#define ELL_3MV_OUTER_ADD(m, v1, v2) \
  (ELL_3V_SCALE_ADD2((m)+0, 1, (m)+0, (v2)[0], (v1)), \
   ELL_3V_SCALE_ADD2((m)+3, 1, (m)+3, (v2)[1], (v1)), \
   ELL_3V_SCALE_ADD2((m)+6, 1, (m)+6, (v2)[2], (v1)))

#define ELL_3MV_MUL(v2, m, v1) \
  ((v2)[0] = (m)[0]*(v1)[0] + (m)[3]*(v1)[1] + (m)[6]*(v1)[2], \
   (v2)[1] = (m)[1]*(v1)[0] + (m)[4]*(v1)[1] + (m)[7]*(v1)[2], \
   (v2)[2] = (m)[2]*(v1)[0] + (m)[5]*(v1)[1] + (m)[8]*(v1)[2])

#define ELL_3MV_TMUL(v2, m, v1) \
  ((v2)[0] = (m)[0]*(v1)[0] + (m)[1]*(v1)[1] + (m)[2]*(v1)[2], \
   (v2)[1] = (m)[3]*(v1)[0] + (m)[4]*(v1)[1] + (m)[5]*(v1)[2], \
   (v2)[2] = (m)[6]*(v1)[0] + (m)[7]*(v1)[1] + (m)[8]*(v1)[2])

#define ELL_3M_MUL(m3, m1, m2)                                    \
  ((m3)[0] = (m1)[0]*(m2)[0] + (m1)[3]*(m2)[1] + (m1)[6]*(m2)[2], \
   (m3)[1] = (m1)[1]*(m2)[0] + (m1)[4]*(m2)[1] + (m1)[7]*(m2)[2], \
   (m3)[2] = (m1)[2]*(m2)[0] + (m1)[5]*(m2)[1] + (m1)[8]*(m2)[2], \
                                                                  \
   (m3)[3] = (m1)[0]*(m2)[3] + (m1)[3]*(m2)[4] + (m1)[6]*(m2)[5], \
   (m3)[4] = (m1)[1]*(m2)[3] + (m1)[4]*(m2)[4] + (m1)[7]*(m2)[5], \
   (m3)[5] = (m1)[2]*(m2)[3] + (m1)[5]*(m2)[4] + (m1)[8]*(m2)[5], \
                                                                  \
   (m3)[6] = (m1)[0]*(m2)[6] + (m1)[3]*(m2)[7] + (m1)[6]*(m2)[8], \
   (m3)[7] = (m1)[1]*(m2)[6] + (m1)[4]*(m2)[7] + (m1)[7]*(m2)[8], \
   (m3)[8] = (m1)[2]*(m2)[6] + (m1)[5]*(m2)[7] + (m1)[8]*(m2)[8])


/*
** the 4x4 matrix-related macros assume that the matrix indexing is:
**
** 0   4   8  12
** 1   5   9  13
** 2   6  10  14
** 3   7  11  15
*/

#define ELL_4V_SET(v, a, b, c, d) \
  ((v)[0] = (a), (v)[1] = (b), (v)[2] = (c), (v)[3] = (d))

#define ELL_4V_GET(a, b, c, d, v) \
  ((a) = (v)[0], (b) = (v)[1], (c) = (v)[2], (d) = (v)[3])

#define ELL_4V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0],       \
   (v2)[1] = (v1)[1],       \
   (v2)[2] = (v1)[2],       \
   (v2)[3] = (v1)[3])

#define ELL_4V_ADD2(v3, v1, v2)  \
  ((v3)[0] = (v1)[0] + (v2)[0], \
   (v3)[1] = (v1)[1] + (v2)[1], \
   (v3)[2] = (v1)[2] + (v2)[2], \
   (v3)[3] = (v1)[3] + (v2)[3])

#define ELL_4V_SUB(v3, v1, v2)  \
  ((v3)[0] = (v1)[0] - (v2)[0], \
   (v3)[1] = (v1)[1] - (v2)[1], \
   (v3)[2] = (v1)[2] - (v2)[2], \
   (v3)[3] = (v1)[3] - (v2)[3])

#define ELL_4V_DOT(v1, v2) \
  ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2] + (v1)[3]*(v2)[3])

#define ELL_4V_SCALE(v2, a, v1) \
  ((v2)[0] = (v1)[0]*a, (v2)[1] = (v1)[1]*a, \
   (v2)[2] = (v1)[2]*a, (v2)[3] = (v1)[3]*a)

#define ELL_4V_SCALE_ADD2(v2, s0, v0, s1, v1) \
  ((v2)[0] = (s0)*(v0)[0] + (s1)*(v1)[0],    \
   (v2)[1] = (s0)*(v0)[1] + (s1)*(v1)[1],    \
   (v2)[2] = (s0)*(v0)[2] + (s1)*(v1)[2],    \
   (v2)[3] = (s0)*(v0)[3] + (s1)*(v1)[3])

#define ELL_4V_SCALE_ADD3(v, s0, v0, s1, v1, s2, v2)    \
  ((v)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0], \
   (v)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1], \
   (v)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2], \
   (v)[3] = (s0)*(v0)[3] + (s1)*(v1)[3] + (s2)*(v2)[3])

#define ELL_4V_SCALE_ADD4(v, s0, v0, s1, v1, s2, v2, s3, v3)           \
  ((v)[0] = (s0)*(v0)[0] + (s1)*(v1)[0] + (s2)*(v2)[0] + (s3)*(v3)[0], \
   (v)[1] = (s0)*(v0)[1] + (s1)*(v1)[1] + (s2)*(v2)[1] + (s3)*(v3)[1], \
   (v)[2] = (s0)*(v0)[2] + (s1)*(v1)[2] + (s2)*(v2)[2] + (s3)*(v3)[2], \
   (v)[3] = (s0)*(v0)[3] + (s1)*(v1)[3] + (s2)*(v2)[3] + (s3)*(v3)[3])

#define ELL_4V_SCALE_INCR(v2, s0, v0) \
  ((v2)[0] += (s0)*(v0)[0], \
   (v2)[1] += (s0)*(v0)[1], \
   (v2)[2] += (s0)*(v0)[2], \
   (v2)[3] += (s0)*(v0)[3])

#define ELL_4V_LEN(v) (sqrt(ELL_4V_DOT((v),(v))))

#define ELL_4V_NORM(v2, v1, length) \
  (length = ELL_4V_LEN(v1), ELL_4V_SCALE(v2, 1.0/length, v1))

#define ELL_4M_ADD2(m3, m1, m2)            \
  (ELL_4V_ADD2((m3)+ 0, (m1)+ 0, (m2)+ 0), \
   ELL_4V_ADD2((m3)+ 4, (m1)+ 4, (m2)+ 4), \
   ELL_4V_ADD2((m3)+ 8, (m1)+ 8, (m2)+ 8), \
   ELL_4V_ADD2((m3)+12, (m1)+12, (m2)+12))

#define ELL_4M_SUB(m3, m1, m2)            \
  (ELL_4V_SUB((m3)+ 0, (m1)+ 0, (m2)+ 0), \
   ELL_4V_SUB((m3)+ 4, (m1)+ 4, (m2)+ 4), \
   ELL_4V_SUB((m3)+ 8, (m1)+ 8, (m2)+ 8), \
   ELL_4V_SUB((m3)+12, (m1)+12, (m2)+12))

#define ELL_4M_SCALE(m2, a, m1)         \
  (ELL_4V_SCALE((m2)+ 0, (a), (m1)+ 0), \
   ELL_4V_SCALE((m2)+ 4, (a), (m1)+ 4), \
   ELL_4V_SCALE((m2)+ 8, (a), (m1)+ 8), \
   ELL_4V_SCALE((m2)+12, (a), (m1)+12))

#define ELL_4M_COPY(m2, m1)     \
  (ELL_4V_COPY((m2)+ 0, (m1)+ 0), \
   ELL_4V_COPY((m2)+ 4, (m1)+ 4), \
   ELL_4V_COPY((m2)+ 8, (m1)+ 8), \
   ELL_4V_COPY((m2)+12, (m1)+12))

#define ELL_4M_TRAN(m2, m1) \
  ((m2)[ 0] = (m1)[ 0],          \
   (m2)[ 1] = (m1)[ 4],          \
   (m2)[ 2] = (m1)[ 8],          \
   (m2)[ 3] = (m1)[12],          \
   (m2)[ 4] = (m1)[ 1],          \
   (m2)[ 5] = (m1)[ 5],          \
   (m2)[ 6] = (m1)[ 9],          \
   (m2)[ 7] = (m1)[13],          \
   (m2)[ 8] = (m1)[ 2],          \
   (m2)[ 9] = (m1)[ 6],          \
   (m2)[10] = (m1)[10],          \
   (m2)[11] = (m1)[14],          \
   (m2)[12] = (m1)[ 3],          \
   (m2)[13] = (m1)[ 7],          \
   (m2)[14] = (m1)[11],          \
   (m2)[15] = (m1)[15])

#define ELL_4M_TRAN_IP(m, t)   \
  (ELL_SWAP2((m)[ 1],(m)[ 4],(t)),  \
   ELL_SWAP2((m)[ 2],(m)[ 8],(t)),  \
   ELL_SWAP2((m)[ 3],(m)[12],(t)),  \
   ELL_SWAP2((m)[ 6],(m)[ 9],(t)),  \
   ELL_SWAP2((m)[ 7],(m)[13],(t)),  \
   ELL_SWAP2((m)[11],(m)[14],(t)))

#define ELL_4MV_COL0_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 0], (m)[ 1], (m)[ 2], (m)[ 3]))

#define ELL_4MV_COL1_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 4], (m)[ 5], (m)[ 6], (m)[ 7]))

#define ELL_4MV_COL2_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 8], (m)[ 9], (m)[10], (m)[11])

#define ELL_4MV_COL3_GET(v, m) \
  (ELL_4V_SET((v), (m)[12], (m)[13], (m)[14], (m)[15]))

#define ELL_4MV_ROW0_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 0], (m)[ 4], (m)[ 8], (m)[12]))

#define ELL_4MV_ROW1_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 1], (m)[ 5], (m)[ 9], (m)[13]))

#define ELL_4MV_ROW2_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 2], (m)[ 6], (m)[10], (m)[14]))

#define ELL_4MV_ROW3_GET(v, m) \
  (ELL_4V_SET((v), (m)[ 3], (m)[ 7], (m)[11], (m)[15]))

#define ELL_4MV_COL0_SET(m, v) \
  (ELL_4V_GET((m)[ 0], (m)[ 1], (m)[ 2], (m)[ 3], (v)))

#define ELL_4MV_COL1_SET(m, v) \
  (ELL_4V_GET((m)[ 4], (m)[ 5], (m)[ 6], (m)[ 7], (v)))

#define ELL_4MV_COL2_SET(m, v) \
  (ELL_4V_GET((m)[ 8], (m)[ 9], (m)[10], (m)[11], (v)))

#define ELL_4MV_COL3_SET(m, v) \
  (ELL_4V_GET((m)[12], (m)[13], (m)[14], (m)[15], (v)))

#define ELL_4MV_ROW0_SET(m, v) \
  (ELL_4V_GET((m)[ 0], (m)[ 4], (m)[ 8], (m)[12], (v)))

#define ELL_4MV_ROW1_SET(m, v) \
  (ELL_4V_GET((m)[ 1], (m)[ 5], (m)[ 9], (m)[13], (v)))

#define ELL_4MV_ROW2_SET(m, v) \
  (ELL_4V_GET((m)[ 2], (m)[ 6], (m)[10], (m)[14], (v)))

#define ELL_4MV_ROW3_SET(m, v) \
  (ELL_4V_GET((m)[ 3], (m)[ 7], (m)[11], (m)[15], (v)))

#define ELL_4MV_MUL(v2, m, v1)                                              \
  ((v2)[0]=(m)[ 0]*(v1)[0]+(m)[ 4]*(v1)[1]+(m)[ 8]*(v1)[2]+(m)[12]*(v1)[3], \
   (v2)[1]=(m)[ 1]*(v1)[0]+(m)[ 5]*(v1)[1]+(m)[ 9]*(v1)[2]+(m)[13]*(v1)[3], \
   (v2)[2]=(m)[ 2]*(v1)[0]+(m)[ 6]*(v1)[1]+(m)[10]*(v1)[2]+(m)[14]*(v1)[3], \
   (v2)[3]=(m)[ 3]*(v1)[0]+(m)[ 7]*(v1)[1]+(m)[11]*(v1)[2]+(m)[15]*(v1)[3])

#define ELL_4MV_TMUL(v2, m, v1)                                             \
  ((v2)[0]=(m)[ 0]*(v1)[0]+(m)[ 1]*(v1)[1]+(m)[ 2]*(v1)[2]+(m)[ 3]*(v1)[3], \
   (v2)[1]=(m)[ 4]*(v1)[0]+(m)[ 5]*(v1)[1]+(m)[ 6]*(v1)[2]+(m)[ 7]*(v1)[3], \
   (v2)[2]=(m)[ 8]*(v1)[0]+(m)[ 9]*(v1)[1]+(m)[10]*(v1)[2]+(m)[11]*(v1)[3], \
   (v2)[3]=(m)[12]*(v1)[0]+(m)[13]*(v1)[1]+(m)[14]*(v1)[2]+(m)[15]*(v1)[3])

#define ELL_34V_HOMOG(v2, v1) \
  ((v2)[0] = (v1)[0]/(v1)[3], \
   (v2)[1] = (v1)[1]/(v1)[3], \
   (v2)[2] = (v1)[2]/(v1)[3])

#define ELL_4V_HOMOG(v2, v1)  \
  ((v2)[0] = (v1)[0]/(v1)[3], \
   (v2)[1] = (v1)[1]/(v1)[3], \
   (v2)[2] = (v1)[2]/(v1)[3], \
   (v2)[3] = 1.0)

/*
** the ELL_4M_SET... macros are setting the matrix one _column_
** at a time- so the matrix components appear below in transpose
**
** These macros are intended to be used as aids with homogeneous transforms
*/

#define ELL_4M_COLS_SET(m, a, b, c, d)  \
  (ELL_4V_COPY((m)+ 0, a),              \
   ELL_4V_COPY((m)+ 4, b),              \
   ELL_4V_COPY((m)+ 8, c),              \
   ELL_4V_COPY((m)+12, d))

#define ELL_4M_ROWS_SET(m, a, b, c, d)                 \
  (ELL_4V_SET((m)+ 0, (a)[0], (b)[0], (c)[0], (d)[0]), \
   ELL_4V_SET((m)+ 4, (a)[1], (b)[1], (c)[1], (d)[1]), \
   ELL_4V_SET((m)+ 8, (a)[2], (b)[2], (c)[2], (d)[2]), \
   ELL_4V_SET((m)+12, (a)[3], (b)[3], (c)[3], (d)[3]))

#define ELL_4M_IDENTITY_SET(m) \
  (ELL_4V_SET((m)+ 0,  1 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  1 ,  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 ,  1 , 0), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 , 1))

#define ELL_4M_ZERO_SET(m) \
  (ELL_4V_SET((m)+ 0,  0 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 , 0))

#define ELL_4M_SCALE_SET(m, x, y, z)     \
  (ELL_4V_SET((m)+ 0, (x),  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 , (y),  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 , (z), 0), \
   ELL_4V_SET((m)+12,  0 ,  0 ,  0 , 1))

#define ELL_4M_TRANSLATE_SET(m, x, y, z) \
  (ELL_4V_SET((m)+ 0,  1 ,  0 ,  0 , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  1 ,  0 , 0), \
   ELL_4V_SET((m)+ 8,  0 ,  0 ,  1 , 0), \
   ELL_4V_SET((m)+12, (x), (y), (z), 1))

#define ELL_4M_ROTATE_X_SET(m, th)                   \
  (ELL_4V_SET((m)+ 0,  1 ,     0    ,     0    , 0), \
   ELL_4V_SET((m)+ 4,  0 ,  cos(th) , +sin(th) , 0), \
   ELL_4V_SET((m)+ 8,  0 , -sin(th) ,  cos(th) , 0), \
   ELL_4V_SET((m)+12,  0 ,     0    ,     0    , 1))

#define ELL_4M_ROTATE_Y_SET(m, th)                   \
  (ELL_4V_SET((m)+ 0,  cos(th) ,  0 , -sin(th) , 0), \
   ELL_4V_SET((m)+ 4,     0    ,  1 ,     0    , 0), \
   ELL_4V_SET((m)+ 8, +sin(th) ,  0 ,  cos(th) , 0), \
   ELL_4V_SET((m)+12,     0    ,  0 ,     0    , 1))

#define ELL_4M_ROTATE_Z_SET(m, th)                   \
  (ELL_4V_SET((m)+ 0,  cos(th) , +sin(th) ,  0 , 0), \
   ELL_4V_SET((m)+ 4, -sin(th) ,  cos(th) ,  0 , 0), \
   ELL_4V_SET((m)+ 8,     0    ,     0    ,  1 , 0), \
   ELL_4V_SET((m)+12,     0    ,     0    ,  0 , 1))

#define ELL_4M_MUL(n, l, m)                                                 \
  ((n)[ 0]=(l)[ 0]*(m)[ 0]+(l)[ 4]*(m)[ 1]+(l)[ 8]*(m)[ 2]+(l)[12]*(m)[ 3], \
   (n)[ 1]=(l)[ 1]*(m)[ 0]+(l)[ 5]*(m)[ 1]+(l)[ 9]*(m)[ 2]+(l)[13]*(m)[ 3], \
   (n)[ 2]=(l)[ 2]*(m)[ 0]+(l)[ 6]*(m)[ 1]+(l)[10]*(m)[ 2]+(l)[14]*(m)[ 3], \
   (n)[ 3]=(l)[ 3]*(m)[ 0]+(l)[ 7]*(m)[ 1]+(l)[11]*(m)[ 2]+(l)[15]*(m)[ 3], \
                                                                            \
   (n)[ 4]=(l)[ 0]*(m)[ 4]+(l)[ 4]*(m)[ 5]+(l)[ 8]*(m)[ 6]+(l)[12]*(m)[ 7], \
   (n)[ 5]=(l)[ 1]*(m)[ 4]+(l)[ 5]*(m)[ 5]+(l)[ 9]*(m)[ 6]+(l)[13]*(m)[ 7], \
   (n)[ 6]=(l)[ 2]*(m)[ 4]+(l)[ 6]*(m)[ 5]+(l)[10]*(m)[ 6]+(l)[14]*(m)[ 7], \
   (n)[ 7]=(l)[ 3]*(m)[ 4]+(l)[ 7]*(m)[ 5]+(l)[11]*(m)[ 6]+(l)[15]*(m)[ 7], \
                                                                            \
   (n)[ 8]=(l)[ 0]*(m)[ 8]+(l)[ 4]*(m)[ 9]+(l)[ 8]*(m)[10]+(l)[12]*(m)[11], \
   (n)[ 9]=(l)[ 1]*(m)[ 8]+(l)[ 5]*(m)[ 9]+(l)[ 9]*(m)[10]+(l)[13]*(m)[11], \
   (n)[10]=(l)[ 2]*(m)[ 8]+(l)[ 6]*(m)[ 9]+(l)[10]*(m)[10]+(l)[14]*(m)[11], \
   (n)[11]=(l)[ 3]*(m)[ 8]+(l)[ 7]*(m)[ 9]+(l)[11]*(m)[10]+(l)[15]*(m)[11], \
                                                                            \
   (n)[12]=(l)[ 0]*(m)[12]+(l)[ 4]*(m)[13]+(l)[ 8]*(m)[14]+(l)[12]*(m)[15], \
   (n)[13]=(l)[ 1]*(m)[12]+(l)[ 5]*(m)[13]+(l)[ 9]*(m)[14]+(l)[13]*(m)[15], \
   (n)[14]=(l)[ 2]*(m)[12]+(l)[ 6]*(m)[13]+(l)[10]*(m)[14]+(l)[14]*(m)[15], \
   (n)[15]=(l)[ 3]*(m)[12]+(l)[ 7]*(m)[13]+(l)[11]*(m)[14]+(l)[15]*(m)[15])

#define ELL_34M_EXTRACT(m, l) \
  ((m)[0] = (l)[ 0], (m)[1] = (l)[ 1], (m)[2] = (l)[ 2], \
   (m)[3] = (l)[ 4], (m)[4] = (l)[ 5], (m)[5] = (l)[ 6], \
   (m)[6] = (l)[ 8], (m)[7] = (l)[ 9], (m)[8] = (l)[10])

#define ELL_43M_INSET(l, m) \
  ((l)[ 0] = (m)[0], (l)[ 1] = (m)[1], (l)[ 2] = (m)[2], (l)[ 3] = 0, \
   (l)[ 4] = (m)[3], (l)[ 5] = (m)[4], (l)[ 6] = (m)[5], (l)[ 7] = 0, \
   (l)[ 8] = (m)[6], (l)[ 9] = (m)[7], (l)[10] = (m)[8], (l)[11] = 0, \
   (l)[12] =   0   , (l)[13] =   0   , (l)[14] =   0   , (l)[15] = 1)

#define ELL_4M_FROB(m) \
  (sqrt(ELL_4V_DOT((m)+ 0, (m)+ 0) + \
        ELL_4V_DOT((m)+ 4, (m)+ 4) + \
        ELL_4V_DOT((m)+ 8, (m)+ 8) + \
        ELL_4V_DOT((m)+12, (m)+12)))

#define ELL_4M_DET(m) \
  (  (m)[ 0] * _ELL_3M_DET((m)[ 5], (m)[ 6], (m)[ 7], \
                           (m)[ 9], (m)[10], (m)[11], \
                           (m)[13], (m)[14], (m)[15]) \
   - (m)[ 4] * _ELL_3M_DET((m)[ 1], (m)[ 2], (m)[ 3], \
                           (m)[ 9], (m)[10], (m)[11], \
                           (m)[13], (m)[14], (m)[15]) \
   + (m)[ 8] * _ELL_3M_DET((m)[ 1], (m)[ 2], (m)[ 3], \
                           (m)[ 5], (m)[ 6], (m)[ 7], \
                           (m)[13], (m)[14], (m)[15]) \
   - (m)[12] * _ELL_3M_DET((m)[ 1], (m)[ 2], (m)[ 3], \
                           (m)[ 5], (m)[ 6], (m)[ 7], \
                           (m)[ 9], (m)[10], (m)[11]))

#define ELL_Q_MUL(q3, q1, q2)                                            \
  ELL_4V_SET((q3),                                                       \
  (q1)[0]*(q2)[0] - (q1)[1]*(q2)[1] - (q1)[2]*(q2)[2] - (q1)[3]*(q2)[3], \
  (q1)[0]*(q2)[1] + (q1)[1]*(q2)[0] + (q1)[2]*(q2)[3] - (q1)[3]*(q2)[2], \
  (q1)[0]*(q2)[2] - (q1)[1]*(q2)[3] + (q1)[2]*(q2)[0] + (q1)[3]*(q2)[1], \
  (q1)[0]*(q2)[3] + (q1)[1]*(q2)[2] - (q1)[2]*(q2)[1] + (q1)[3]*(q2)[0])

#define ELL_Q_CONJ(q2, q1) \
  ELL_4V_SET((q2), (q1)[0], -(q1)[1], -(q1)[2], -(q1)[3])

#define ELL_Q_INV(i, q, n)                                            \
 (n = ELL_4V_DOT(q, q),                                               \
  ELL_4V_SET((i), (q)[0]/(n), -(q)[1]/(n), -(q)[2]/(n), -(q)[3]/(n)))

#define ELL_Q_TO_3M(m, q)                                                   \
  (ELL_3V_SET((m)+0,                                                        \
             (q)[0]*(q)[0] + (q)[1]*(q)[1] - (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[1]*(q)[2] + (q)[0]*(q)[3]),                             \
             2*((q)[1]*(q)[3] - (q)[0]*(q)[2])),                            \
  ELL_3V_SET((m)+3,                                                         \
             2*((q)[1]*(q)[2] - (q)[0]*(q)[3]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] + (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[2]*(q)[3] + (q)[0]*(q)[1])),                            \
  ELL_3V_SET((m)+6,                                                         \
             2*((q)[1]*(q)[3] + (q)[0]*(q)[2]),                             \
             2*((q)[2]*(q)[3] - (q)[0]*(q)[1]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] - (q)[2]*(q)[2] + (q)[3]*(q)[3]))

#define ELL_Q_TO_4M(m, q)                                                   \
  (ELL_4V_SET((m)+0,                                                        \
             (q)[0]*(q)[0] + (q)[1]*(q)[1] - (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[1]*(q)[2] + (q)[0]*(q)[3]),                             \
             2*((q)[1]*(q)[3] - (q)[0]*(q)[2]),                             \
             0),                                                            \
  ELL_4V_SET((m)+4,                                                         \
             2*((q)[1]*(q)[2] - (q)[0]*(q)[3]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] + (q)[2]*(q)[2] - (q)[3]*(q)[3], \
             2*((q)[2]*(q)[3] + (q)[0]*(q)[1]),                             \
             0),                                                            \
  ELL_4V_SET((m)+8,                                                         \
             2*((q)[1]*(q)[3] + (q)[0]*(q)[2]),                             \
             2*((q)[2]*(q)[3] - (q)[0]*(q)[1]),                             \
             (q)[0]*(q)[0] - (q)[1]*(q)[1] - (q)[2]*(q)[2] + (q)[3]*(q)[3], \
             0),                                                            \
  ELL_4V_SET((m)+12, 0, 0, 0, 1))

#define ELL_2V_SET(v, a, b) \
  ((v)[0]=(a), (v)[1]=(b))

#define ELL_5V_SET(v, a, b, c, d, e) \
  ((v)[0]=(a), (v)[1]=(b), (v)[2]=(c), (v)[3]=(d), (v)[4]=(e))

#define ELL_5V_COPY(v, w) \
  ((v)[0]=(w)[0], (v)[1]=(w)[1], (v)[2]=(w)[2], (v)[3]=(w)[3], (v)[4]=(w)[4])

#define ELL_6V_SET(v, a, b, c, d, e, f) \
  ((v)[0]=(a), (v)[1]=(b), (v)[2]=(c), (v)[3]=(d), (v)[4]=(e), (v)[5]=(f))

#define ELL_7V_SET(v, a, b, c, d, e, f, g) \
  ((v)[0]=(a), (v)[1]=(b), (v)[2]=(c), (v)[3]=(d), \
   (v)[4]=(e), (v)[5]=(f), (v)[6]=(g))

#define ELL_7V_COPY(v2, v1) \
  ((v2)[0] = (v1)[0],       \
   (v2)[1] = (v1)[1],       \
   (v2)[2] = (v1)[2],       \
   (v2)[3] = (v1)[3],       \
   (v2)[4] = (v1)[4],       \
   (v2)[5] = (v1)[5],       \
   (v2)[6] = (v1)[6])

#ifdef __cplusplus
   }
#endif

#endif /* ELLMACROS_HAS_BEEN_INCLUDED */
