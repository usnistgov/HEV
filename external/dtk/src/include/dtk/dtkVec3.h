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
 */

/* dtkVec3 was written by John Kelso. */

/*! \class dtkVec3 dtkVec3.h dtk.h
 * \brief Holds three floats.
 *
 * Holds three floats representing an XYZ positon or an HPR orientation
 * when this is used with the dtkMatrix class.
 */

class DTKAPI dtkVec3
{
 public:

  /*! Creates a new vec3 with all 3 values set to zero.
   */
  dtkVec3(void) ;

  /*! Creates a new vec3 that is initialized.
   *
   * \param x will be the value of the first component.
   * \param y will be the value of the second component.
   * \param z will be the value of the third component.
   */
  dtkVec3(float x, float y, float z) ;
  
  /*! Creates a new vec3 initialized with values from the float array.
   *
   * \param xyz is a 3 float array.
   */
  dtkVec3(float *xyz) ;

  /*! Destructor */
  virtual ~dtkVec3(void);

  /*!
   * Creates a dtkVec3 with values from another dtkVec3.
   *
   * \param v is to be copied.
   */
  dtkVec3(const dtkVec3 &v) ;

  /*!
   * equality operator
   */
  bool operator==( const dtkVec3 &v ) const;

  /*!
   * inequality operator
   */
  bool operator!=( const dtkVec3 &v ) const;

  //! = operator
  /*! Copies all the values from \e v to this dtkVec3.
   */
  dtkVec3& operator=( const dtkVec3 &v );

  /*!
   * vector addition operator
   */
  const dtkVec3 operator+( const dtkVec3 &v ) const;

  /*!
   * vector addition assignment operator
   */
  dtkVec3& operator+=( const dtkVec3 &v );

  /*!
   * vector subtraction operator
   */
  const dtkVec3 operator-( const dtkVec3 &v ) const;

  /*!
   * vector subtraction assignment operator
   */
  dtkVec3& operator-=( const dtkVec3 &v );

  /*!
   * vector cross product  operator
   */
  const dtkVec3 operator*( const dtkVec3 &v ) const;

  /*!
   * vector cross product  assignment operator
   */
  dtkVec3& operator*=( const dtkVec3 &v );

  /*!
   * vector scalar multiply
   */
  const dtkVec3 operator*( const float& s ) const;

  /*!
   * vector scalar assignment multiply
   */
  dtkVec3& operator*=( const float& s );

  /*!
   * vector scalar divide
   */
  const dtkVec3 operator/( const float& s ) const;

  /*!
   * vector scalar assignment divide
   */
  dtkVec3& operator/=( const float& s );

  /*!
   * vector dot product  operator
   */
  const float operator^( const dtkVec3 &v ) const;

  /*!
   * return the angle between the 2 vectors
   */
  const float angleBetweenVectorsRAD( const dtkVec3 &v );
  const float angleBetweenVectorsDEG( const dtkVec3 &v );

  /*!
   * return the length of the vector
   */
  const float getLength() const;

  /*!
   * normalize the vector - make unit length
   */
  dtkVec3& normalize();

  /*!
   * You can refer to each element as x, y, and z, or h, p, and r, or
   * d[0-2], same thing.
   */
  float &x, &y, &z ;
  float &h, &p, &r ;
  float d[3] ;

  /*! Set values.
   */
  void set(float x, float y, float z);

  /*! Set values.
   */
  void set(float *xyz);

  /*! Get values.
   */
  void get(float *x, float *y, float *z);

  /*! Get values.
   */
  void get(float *xyz);

  /*! Print values.
   *
   * \param file is the file stream to print to.  If \e file is NULL
   * this will print to stdout.
   */
  void print(const FILE *file = NULL);

  /*! Set all values to zero.
   */
  void zero(void);

  /*!
   * \param v is a dtkVec3 to compare to.
   *
   * \return Returns 1 if all values are the same in this object
   * and /e v, returns 0 otherwise.
   */
  int equals(dtkVec3 *v);
};

