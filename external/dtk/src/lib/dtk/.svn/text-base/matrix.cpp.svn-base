/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This software, the DIVERSE Toolkit library, is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License (LGPL) as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software, in the top level source directory in
 * a file named "COPYING.LGPL"; if not, see it at:
 * http://www.gnu.org/copyleft/lesser.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
/* Written by John Kelso and Lance Arsenault.
 */

#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

#ifdef DTK_ARCH_WIN32_VCPP
# include <memory.h>
#else
# include <strings.h>
#endif


#include "dtkVec3.h"
#include "dtkCoord.h"
#include "dtkMatrix.h"

/* Matrix array convention: matrix[down][across]
 *
 * Here is the rotation part of the matrix

  where ch = cos(h), sh = sin(h),
        sp = sin(p), cp = cos(p),
        sr = sim(r), cr = cos(r),

   rotation part of the matrixes 


         /  ch  -sh  0  \ /  1  0    0   \ /   cr   0  sr  \
         |              | |              | |               |
H P R  = |  sh   ch  0  | |  0  cp  -sp  | |   0    1  0   |
         |              | |              | |               |
         \  0    0   1  / \  0  sp   cp  / \  -sr   0  cr  /


           /  ch  -sh  0  \ /   cr       0    sr       \
           |              | |                          |
H (P R)  = |  sh   ch  0  | |  (sp sr)   cp  (-sp cr)  |
           |              | |                          |
           \  0    0   1  / \  (-cp sr)  sp  (cp cr)   /


         /  (ch cr - sh sp sr)  (-sh cp)  (ch sr + sh sp cr)  \
         |                                                    |
H P R =  |  (sh cr + ch sp sr)  (ch cp)   (sh sr - ch sp cr)  |
         |                                                    |
         \  (-cp sr)            (sp)      (cp cr)             /




Matrix array convention: matrix[down][across]


  rot[0][0] =  ch*cr - sh*sp*sr;
  rot[1][0] =  sh*cr + ch*sp*sr;
  rot[2][0] = -cp*sr;
  
  rot[0][1] = -sh*cp;
  rot[1][1] =  ch*cp;
  rot[2][1] =  sp;

  rot[0][2] =  ch*sr + sh*sp*cr;
  rot[1][2] =  sh*sr - ch*sp*cr;
  rot[2][2] =  cp*cr;


With Scale(s) than rotation and than translation (tx,ty,tz):


                 /  (ch cr - sh sp sr) s   (-sh cp) s  (ch sr + sh sp cr) s   tx  \
                 |                                                                |
                 |  (sh cr + ch sp sr) s   (ch cp) s   (sh sr - ch sp cr) s   ty  |
T H P R Scale =  |                                                                |
                 |  (-cp sr) s             (sp) s     (cp cr) s               tz  |
                 |                                                                |
                 \   0                      0          0                      1   /

T H P R Scale = 



// If pitch (P) near +/- pi/2  Gimball lock has occurred

sp = +/- 1

cp = 0

h = 0
=> sh = 0
   ch = 1



         /  (cr)       0      (sr)      \
         |                              |
R P H =  |  (sr sp)    0      (-cr sp)  |
         |                              |
         \   0        (sp)     0        /




*/

#ifndef M_PI
# define M_PI  3.14159265358f
#endif

#define DEG2RAD(x)  ((float) ((x)*M_PI/180.0f))
#define RAD2DEG(x)  ((float) ((x)*180.0f/M_PI))
#define ABS(x)   (((x) > 0.0f)?(x):(-(x)))

// defaults to an identity matrix
dtkMatrix::dtkMatrix(void)
{
  identity();
}

// creates with dtkCoord data
dtkMatrix::dtkMatrix(dtkCoord d)
{
  identity() ;
  coord(d) ;
}

// creates with dtkVec3 data
dtkMatrix::dtkMatrix(dtkVec3 xyz, dtkVec3 hpr)
{
  identity() ;
  rotateHPR(hpr) ;
  translate(xyz) ;
}

dtkMatrix::dtkMatrix( const dtkMatrix& m )
{
  for( int i=0;i<4;i++ )
    for( int j=0;j<4;j++ )
      mat[i][j] = m.mat[i][j];
  _scale = m._scale;
}

dtkMatrix::~dtkMatrix(void) {}

void dtkMatrix::identity(void)
{
  mat[0][0] = 1.0f;
  mat[1][0] = 0.0f;
  mat[2][0] = 0.0f;
  mat[3][0] = 0.0f;

  mat[0][1] = 0.0f;
  mat[1][1] = 1.0f;
  mat[2][1] = 0.0f;
  mat[3][1] = 0.0f;

  mat[0][2] = 0.0f;
  mat[1][2] = 0.0f;
  mat[2][2] = 1.0f;
  mat[3][2] = 0.0f;

  mat[0][3] = 0.0f;
  mat[1][3] = 0.0f;
  mat[2][3] = 0.0f;
  mat[3][3] = 1.0f;

  _scale = 1.0f;
}

void dtkMatrix::scale(float s)
{
  mat[0][0] *= s;
  mat[1][0] *= s;
  mat[2][0] *= s;

  mat[0][1] *= s;
  mat[1][1] *= s;
  mat[2][1] *= s;

  mat[0][2] *= s;
  mat[1][2] *= s;
  mat[2][2] *= s;

  mat[0][3] *= s;
  mat[1][3] *= s;
  mat[2][3] *= s;

  _scale *= s;
}


float dtkMatrix::scale(void) const
{
  return _scale;
}


void dtkMatrix::rotateHPR(float *h, float *p, float *r) const
{
  // We choose to make pitch (Rx) between -pi/2 and +pi/2

  /* I would some day like to know why this seems to work when pitch
   * is close to +/- pi/2.  */

  float p_angle = asinf(mat[2][1]/_scale);
  *p = RAD2DEG(p_angle);
  float cos_p = cosf(p_angle);

  if(cos_p > 1.0e-6f || cos_p < -1.0e-6f)
    {
      //            atan (  sin,      cos)
      *r = RAD2DEG(atan2f(-mat[2][0]/(cos_p*_scale), mat[2][2]/(cos_p*_scale)));
      *h = RAD2DEG(atan2f(-mat[0][1]/(cos_p*_scale), mat[1][1]/(cos_p*_scale)));
    }
  else // heading and roll have the same affect if pitch near +/- pi.
    { /* Gimball lock has occurred */

      *h = 0.0f;
      *r = RAD2DEG(atan2f(mat[0][2]/(_scale), mat[0][0]/(_scale)));
    }
}


void dtkMatrix::rotateHPR(float h, float p, float r)
{
  float rot[3][3]; // 3x3 rotation matrix

  float sp = sinf(DEG2RAD(p));  float cp = cosf(DEG2RAD(p));
  float sr = sinf(DEG2RAD(r));  float cr = cosf(DEG2RAD(r));
  float sh = sinf(DEG2RAD(h));  float ch = cosf(DEG2RAD(h));


  rot[0][0] =  ch*cr - sh*sp*sr;
  rot[1][0] =  sh*cr + ch*sp*sr;
  rot[2][0] = -cp*sr;
  
  rot[0][1] = -sh*cp;
  rot[1][1] =  ch*cp;
  rot[2][1] =  sp;

  rot[0][2] =  ch*sr + sh*sp*cr;
  rot[1][2] =  sh*sr - ch*sp*cr;
  rot[2][2] =  cp*cr;

  int i;

  float m[4][4];
  memcpy(m,mat,sizeof(float [4][4]));

#if 0
  printf("\nrot=\n");
  for(i=0;i<3;i++)
    printf("%+3.3f  %+3.3f  %+3.3f\n",
	    rot[i][0], rot[i][1], rot[i][2]);

  printf("\nM3=\n");
  for(i=0;i<3;i++)
    printf("%+3.3f  %+3.3f  %+3.3f\n",
	    mat[i][0], mat[i][1], mat[i][2]);
#endif

  // just compute whats needed for this kind of matrix

  for(i=0; i<3; i++)
    for(int j=0; j<3; j++)
      mat[i][j] = rot[i][0]*m[0][j] + rot[i][1]*m[1][j] + rot[i][2]*m[2][j];

  for(i=0; i<3; i++)
    mat[i][3] = rot[i][0]*m[0][3] + rot[i][1]*m[1][3] + rot[i][2]*m[2][3];
}

void dtkMatrix::rotateHPR(dtkVec3 *hpr) const {
  rotateHPR(&(hpr->h), &(hpr->p), &(hpr->r)) ;
}

void dtkMatrix::rotateHPR(dtkVec3 hpr) {
  rotateHPR(hpr.h, hpr.p, hpr.r) ;
}


// print in OpenGL form
void dtkMatrix::print(FILE *file) const
{
  if(!file) file = stdout;

  for(int i=0;i<4;i++)
    fprintf(file,"%+8.8f  %+8.8f  %+8.8f  %+8.8f\n",
	    mat[i][0], mat[i][1], mat[i][2], mat[i][3]);

  float x,y,z, h,p,r;

  rotateHPR(&h, &p, &r);
  translate(&x, &y, &z);

  fprintf(file, "\n"
	 "xyz = %+3.3f %+3.3f %+3.3f"
	 "  hpr= %+3.3f %+3.3f %+3.3f"
	 "  scale= %+3.3f\n",
	 x,y,z,h,p,r,scale());
  fprintf(file, "*********************************************\n");

}


// get translation
void dtkMatrix::translate(float *x, float *y, float *z) const
{
  *x = mat[0][3];
  *y = mat[1][3];
  *z = mat[2][3];
}

// multiply by a matrix with this translation
void dtkMatrix::translate(float x, float y, float z)
{
  mat[0][3] += x;
  mat[1][3] += y;
  mat[2][3] += z;
}

void dtkMatrix::translate(dtkVec3 *xyz) const
{
  translate(&(xyz->x), &(xyz->y), &(xyz->z));
}

void dtkMatrix::translate(dtkVec3 xyz) {
  translate(xyz.x, xyz.y, xyz.z) ;
}

// set the matrix with this coord
void dtkMatrix::coord(dtkCoord c)
{
  rotateHPR(c.h, c.p, c.r);
  translate(c.x, c.y, c.z);
}

// get the coord from the matrix
void dtkMatrix::coord(dtkCoord *c) const
{
  translate(&(c->x), &(c->y), &(c->z));
  rotateHPR(&(c->h), &(c->p), &(c->r));
}

// The matrix is assumed to be of the form:
// Transtation X Rotation X uniformScale.

void dtkMatrix::invert(void)
{
  float m[4][4];
  memcpy(m,mat,sizeof(float [4][4]));

  int i;
  for(i=0; i<3; i++)
    for(int j=0; j<3; j++)
      mat[i][j] = m[j][i]/(_scale*_scale); // (inverse rotation)/scale

  // inverse translation
  m[0][3] *= - 1.0f; 
  m[1][3] *= - 1.0f; 
  m[2][3] *= - 1.0f; 

  for(i=0; i<3; i++) // (inverse rotation) * (inverse translation) /scale
    mat[i][3] = mat[i][0]*m[0][3] + mat[i][1]*m[1][3] + mat[i][2]*m[2][3];

  _scale = 1.0f/_scale;
}


// This is a post multiply

void dtkMatrix::mult(const dtkMatrix *p)
{
  float m[4][4];
  memcpy(m,mat,sizeof(float [4][4]));

#ifdef DTK_ARCH_WIN32_VCPP
  memset(mat, 0, sizeof(float)*16);
#else
  bzero(mat, sizeof(float [4][4]));
#endif

  for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
      for(int k=0;k<4;k++)
        mat[i][j] += p->mat[i][k] * m[k][j];

  _scale *= p->scale();
}

void dtkMatrix::copy(const dtkMatrix *m)
{
  memcpy(mat,m->mat,sizeof(float [4][4]));
  _scale = m->scale();
}

void dtkMatrix::quat(float x, float y, float z, float w)
{
  float rot[3][3]; // 3x3 rotation matrix
  
  float len = sqrt((x*x + y*y + z*z + w*w));
  
  if (len == 0.f)
    return;
  
  x /= len;
  y /= len;
  z /= len;
  w /= len;
  
  float xx = x*x;
  float yy = y*y;
  float zz = z*z;
  
  float xy = x*y;
  float xz = x*z;
  float xw = x*w;
  
  float yz = y*z;
  float yw = y*w;
  
  float zw = z*w;
  
  rot[0][0] = 1.f - 2.f*(yy + zz);
  rot[0][1] = 2.f*(xy - zw);
  rot[0][2] = 2.f*(xz + yw);
  
  rot[1][0] = 2.f*(xy + zw);
  rot[1][1] = 1.f - 2.f*(xx + zz);
  rot[1][2] = 2.f*(yz - xw);
  
  rot[2][0] = 2.f*(xz - yw);
  rot[2][1] = 2.f*(yz + xw);
  rot[2][2] = 1.f - 2.f*(xx + yy);
  
  int i;
  
  float m[4][4];
  memcpy(m,mat,sizeof(float [4][4]));

  // just compute whats needed for this kind of matrix
  for(i=0; i<3; i++)
    for(int j=0; j<3; j++)
      mat[i][j] = rot[i][0]*m[0][j] + rot[i][1]*m[1][j] + rot[i][2]*m[2][j];

  for(i=0; i<3; i++)
    mat[i][3] = rot[i][0]*m[0][3] + rot[i][1]*m[1][3] + rot[i][2]*m[2][3];

}

void dtkMatrix::quat(float *x, float *y, float *z, float *w)
{

  float trace = mat[0][0] + mat[1][1] + mat[2][2] + 1.f;
  
  if(trace > .001) 
    {
      float s = 0.5f/sqrt(trace);
      *w = 0.25f/s;
      *x = (mat[2][1] - mat[1][2])*s;
      *y = (mat[0][2] - mat[2][0])*s;
      *z = (mat[1][0] - mat[0][1])*s;
    } 
  else 
    {
      if (mat[0][0]>mat[1][1] && mat[0][0]>mat[2][2]) 
	{
	  float s = 2.f*sqrt(1.f + mat[0][0] - mat[1][1] - mat[2][2]);
	  *x = 0.25f*s;
	  *y = (mat[0][1] + mat[1][0])/s;
	  *z = (mat[0][2] + mat[2][0])/s;
	  *w = (mat[2][1] - mat[1][2])/s;
	} 
      else if (mat[1][1] > mat[2][2]) 
	{
	  float s = 2.f*sqrt(1.f + mat[1][1] - mat[0][0] - mat[2][2]);
	  *x = (mat[0][1] + mat[1][0])/s;
	  *y = 0.25f*s;
	  *z = (mat[1][2] + mat[2][1])/s;
	  *w = (mat[0][2] - mat[2][0])/s;
	} 
      else 
	{
	  float s = 2.f*sqrtf(1.0f + mat[2][2] - mat[0][0] - mat[1][1]);
	  *x = (mat[0][2] + mat[2][0])/s;
	  *y = (mat[1][2] + mat[2][1])/s;
	  *z = 0.25f*s;
	  *w = (mat[1][0] - mat[0][1])/s;
	}
    }
  
  float len = sqrt(((*x)*(*x) + (*y)*(*y) + (*z)*(*z) + (*w)*(*w)));
  
  (*x) /= len;
  (*y) /= len;
  (*z) /= len;
  (*w) /= len;
}

const dtkMatrix dtkMatrix::operator *( const dtkMatrix& m ) const
{
	return dtkMatrix( *this ) *= m;
}

dtkMatrix& dtkMatrix::operator *=( const dtkMatrix& m )
{
	mult( &m );

	return *this;
}

