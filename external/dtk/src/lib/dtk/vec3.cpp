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
/* dtkVec3 was written by John Kelso. */

#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"
#ifdef DTK_ARCH_WIN32_VCPP
#  define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <stdio.h>

#include "dtkVec3.h"

// three floats representing an XYZ positon or an HPR orientation
dtkVec3::dtkVec3(void) :
  x(d[0]), y(d[1]), z(d[2]),
  h(d[0]), p(d[1]), r(d[2]) 
{
  zero() ;
}

dtkVec3::dtkVec3(float d0, float d1, float d2) :
  x(d[0]), y(d[1]), z(d[2]),
  h(d[0]), p(d[1]), r(d[2]) 
{
  set(d0, d1, d2);
}

dtkVec3::dtkVec3(float *d) :
  x(d[0]), y(d[1]), z(d[2]),
  h(d[0]), p(d[1]), r(d[2]) 
{
  set(d[0], d[1], d[2]);
}

dtkVec3::dtkVec3(const dtkVec3 &v) :
  x(d[0]), y(d[1]), z(d[2]), h(d[0]), p(d[1]), r(d[2])
{
  set((float *) v.d);
}

dtkVec3::~dtkVec3(void) {}

bool dtkVec3::operator==(const dtkVec3 &v) const
{
	if( x == v.x && y == v.y && z == v.z )
		return true;

	return false;
}

bool dtkVec3::operator!=( const dtkVec3 &v ) const
{
	return !( *this == v );
}

dtkVec3& dtkVec3::operator=(const dtkVec3 &v)
{
  if( this != &v )
  {
    set((float *) v.d);
  }

  return *this;
}

const dtkVec3 dtkVec3::operator+( const dtkVec3 &v ) const
{
  return dtkVec3( *this ) += v;
}

dtkVec3& dtkVec3::operator+=( const dtkVec3 &v )
{
  x += v.x;
  y += v.y;
  z += v.z;

  return *this;
}

const dtkVec3 dtkVec3::operator-( const dtkVec3 &v ) const
{
  return dtkVec3( *this ) -= v;
}

dtkVec3& dtkVec3::operator-=( const dtkVec3 &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;

  return *this;
}

const dtkVec3 dtkVec3::operator*( const dtkVec3 &v ) const
{
  return dtkVec3( *this ) *= v;
}

dtkVec3& dtkVec3::operator*=( const dtkVec3 &v )
{
  dtkVec3 t( *this );
  x = t.y * v.z - t.z * v.y;
  y = t.z * v.x - t.x * v.z;
  z = t.x * v.y - t.y * v.x;

  return *this;
}

const dtkVec3 dtkVec3::operator*( const float& s ) const
{
  return dtkVec3( *this ) *= s;
}

dtkVec3& dtkVec3::operator*=( const float& s )
{
  x *= s;
  y *= s;
  z *= s;

  return *this;
}

const float dtkVec3::operator^( const dtkVec3 &v ) const
{
  return x * v.x + y * v.y + z * v.z;
}

const float dtkVec3::angleBetweenVectorsRAD( const dtkVec3 &v )
{
	float length_this = getLength();
	float length_v = v.getLength();

	return acosf( ( *this ^ v ) / ( length_this * length_v ) );
}

const float dtkVec3::angleBetweenVectorsDEG( const dtkVec3 &v )
{
	float length_this = getLength();
	float length_v = v.getLength();

	return 180.0f / M_PI * acosf( ( *this ^ v ) / ( length_this * length_v ) );
}

const float dtkVec3::getLength() const
{
	return sqrtf( x * x + y * y + z * z );
}

dtkVec3& dtkVec3::normalize()
{
	float length = getLength();

	x /= length;
	y /= length;
	z /= length;

	return *this;
}

void dtkVec3::set(float d0, float d1, float d2)
{
  d[0] = d0; d[1] = d1; d[2] = d2;
}

void dtkVec3::get(float *d0, float *d1, float *d2) {
  *d0 = d[0]; *d1 = d[1]; *d2 = d[2];
}

void dtkVec3::set(float *xyz)
{
  d[0] = xyz[0]; d[1] = xyz[1]; d[2] = xyz[2];
}

void dtkVec3::get(float *xyz) {
  xyz[0] = d[0] ; xyz[1] = d[1] ; xyz[2] = d[2] ;
}

void dtkVec3::print(const FILE *file_in) {
  FILE *file = (file_in)?((FILE *) file_in):stdout;

  fprintf(file, "\n"
	  "vec = %+3.3f %+3.3f %+3.3f\n",
	  d[0],d[1],d[2]);
}

void dtkVec3::zero(void) {
  d[0] = d[1] = d[2] = 0.f ;
}

int dtkVec3::equals(dtkVec3 *v) {
  if (d[0] == v->d[0] && 
      d[1] == v->d[1] &&
      d[2] == v->d[2])
    return 1 ;
  else
    return 0 ;
}

