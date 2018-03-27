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

#ifndef TENMACROS_HAS_BEEN_INCLUDED
#define TENMACROS_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* 
******** TEN_LIST2MAT, TEN_MAT2LIST
**
** for going between 7-element list and 9-element matrix
** representations of a symmetric tensor
**
** the ordering of the tensor elements is assumed to be:
**
** threshold        0
** Dxx Dxy Dxz      1   2   3
** Dxy Dyy Dyz  =  (2)  4   5
** Dxz Dyz Dzz     (3) (5)  6 
**
** As in ell, the matrix ordering is given by:
**
**   0  3  6
**   1  4  7
**   2  5  8
**
** Note that TEN_MAT2LIST does NOT set the threshold element (index 0),
** and that the threshold value plays no role in TEN_LIST2MAT.
*/

#define TEN_LIST2MAT(m, l) ( \
   (m)[0] = (l)[1],          \
   (m)[1] = (l)[2],          \
   (m)[2] = (l)[3],          \
   (m)[3] = (l)[2],          \
   (m)[4] = (l)[4],          \
   (m)[5] = (l)[5],          \
   (m)[6] = (l)[3],          \
   (m)[7] = (l)[5],          \
   (m)[8] = (l)[6] )

#define TEN_MAT2LIST(l, m) ( \
   (l)[1] = (m)[0],          \
   (l)[2] = (m)[3],          \
   (l)[3] = (m)[6],          \
   (l)[4] = (m)[4],          \
   (l)[5] = (m)[7],          \
   (l)[6] = (m)[8] )

#define TEN_LIST_COPY(d, s) ( \
   (d)[0] = (s)[0],           \
   (d)[1] = (s)[1],           \
   (d)[2] = (s)[2],           \
   (d)[3] = (s)[3],           \
   (d)[4] = (s)[4],           \
   (d)[5] = (s)[5],           \
   (d)[6] = (s)[6] )

#define TEN_LIST_DET(l) ( \
  (l)[1]*((l)[4]*(l)[6] - (l)[5]*(l)[5]) \
  + (l)[2]*((l)[5]*(l)[3] - (l)[2]*(l)[6]) \
  + (l)[3]*((l)[2]*(l)[5] - (l)[3]*(l)[4]))

#define TEN_LIST_SCALE(a, s, b) ( \
   (a)[0] = (b)[0],               \
   (a)[1] = (s)*(b)[1],           \
   (a)[2] = (s)*(b)[2],           \
   (a)[3] = (s)*(b)[3],           \
   (a)[4] = (s)*(b)[4],           \
   (a)[5] = (s)*(b)[5],           \
   (a)[6] = (s)*(b)[6])

#define TEN_3VLIST_MUL(b, l, a) (                         \
  (b)[0] = (l)[1]*(a)[0] + (l)[2]*(a)[1] + (l)[3]*(a)[2], \
  (b)[1] = (l)[2]*(a)[0] + (l)[4]*(a)[1] + (l)[5]*(a)[2], \
  (b)[2] = (l)[3]*(a)[0] + (l)[5]*(a)[1] + (l)[6]*(a)[2])

#ifdef __cplusplus
}
#endif

#endif /* TENMACROS_HAS_BEEN_INCLUDED */
