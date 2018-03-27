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
/* dtkCoord was written by John Kelso. */

#include "config.h"
#include "_config.h"
#include "privateConstants.h"
#include "dtkConfigure.h"

#include <stdio.h>

#include "dtkVec3.h"
#include "dtkCoord.h"

// six floats representing an XYZ positon and an HPR orientation
dtkCoord::dtkCoord(void) :
  x(d[0]), y(d[1]), z(d[2]), h(d[3]), p(d[4]), r(d[5])
{
  zero() ;
}

dtkCoord::dtkCoord(float dx, float dy, float dz, 
		   float dh, float dp, float dr) :
  x(d[0]), y(d[1]), z(d[2]), h(d[3]), p(d[4]), r(d[5])
{
  set(dx, dy, dz, dh, dp, dr) ;
}

dtkCoord::dtkCoord(float *d) :
  x(d[0]), y(d[1]), z(d[2]), h(d[3]), p(d[4]), r(d[5])
{
  set(d) ;
}

dtkCoord::dtkCoord(dtkVec3 *xyz, dtkVec3 *hpr) :
  x(d[0]), y(d[1]), z(d[2]), h(d[3]), p(d[4]), r(d[5])
{
  set(xyz, hpr) ;
}

dtkCoord::dtkCoord(const dtkCoord &c) :
  x(d[0]), y(d[1]), z(d[2]), h(d[3]), p(d[4]), r(d[5])
{
  set((float *) c.d);
}

// Need a virtual destructor so others may inherit this.
dtkCoord::~dtkCoord(void) {}

void dtkCoord::set(float dx, float dy, float dz, 
	      float dh, float dp, float dr) {
  x = dx ; y = dy; z = dz ;
  h = dh ; p = dp; r = dr ;
}

const dtkCoord &dtkCoord::operator=(const dtkCoord &c)
{
  x = c.x;
  y = c.y;
  z = c.z;
  h = c.h;
  p = c.p;
  r = c.r;

  return *this;
}

void dtkCoord::set(float *d) {
  x = d[0] ; y = d[1]; z = d[2] ;
  h = d[3] ; p = d[4]; r = d[5] ;
}

void dtkCoord::set(dtkVec3 *xyz, dtkVec3 *hpr) {
  x = xyz->x; y = xyz->y; z = xyz->z;
  h = hpr->h; p = hpr->p; r = hpr->r;
}

void dtkCoord::get(float *dx, float *dy, float *dz, 
		   float *dh, float *dp, float *dr) {
  *dx = x; *dy = y; *dz = z;
  *dh = h; *dp = p; *dr = r;
}

void dtkCoord::get(dtkVec3 *xyz, dtkVec3 *hpr) {
  xyz->x = x; xyz->y = y; xyz->z = z;
  hpr->h = h; hpr->p = p; hpr->r = r;
}

void dtkCoord::get(float *d) {
  d[0] = x ; d[1] = y ; d[2] = z ;
  d[3] = h ; d[4] = p ; d[5] = r ;
}

void dtkCoord::print(const FILE *file_in) {
  FILE *file = (file_in)?((FILE *) file_in):stdout;

  fprintf(file, "\n"
	  "xyz = %+3.3f %+3.3f %+3.3f"
	  "  hpr= %+3.3f %+3.3f %+3.3f\n",
	  x,y,z,h,p,r);
}

void dtkCoord::zero(void) {
  x = y = z = h = p = r = 0.f ;
}

int dtkCoord::equals(dtkCoord *c) {
  if (x == c->x && 
      y == c->y &&
      z == c->z &&
      h == c->h &&
      p == c->p &&
      r == c->r)
    return 1 ;
  else
    return 0 ;
}

bool dtkCoord::operator==( const dtkCoord& c )
{
  if (x == c.x && 
      y == c.y &&
      z == c.z &&
      h == c.h &&
      p == c.p &&
      r == c.r)
    return true;
  else
    return false;
}

bool dtkCoord::operator!=( const dtkCoord& c )
{
  if( *this == c )
    return false;
  else
    return true;
}

