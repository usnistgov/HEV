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

/*! \class dtkMatrix dtkMatrix.h dtk.h
 * \brief Nongeneral 4x4 graphics matrix
 *
 * 4x4 graphics (less general than affine) matrix class, not for
 * general matrixes. This matrix class does not support skued matrixes.
 * It is optimised for matrixes that have just translation, rotation and scale.
 * This is handy for doing calculations for navigation and rigid body motion.
 * reference: Interactive Computer Graphics, Edward Angel, (c) 1997.
 * <pre>
 
  OpenGL forms

  Translation matrix      Rotation matrix     Scale matrix (uniform)

      1 0 0 dx             r00 r01 r02  0         s  0  0  0
 T =  0 1 0 dy       R =   r10 r11 r12  0    S =  0  s  0  0
      0 0 1 dz             r20 r21 r22  0         0  0  s  0
      0 0 0 1               0   0   0   1         0  0  0  1


     (T R S)  Matrix     

      a00 a01 a02 a03
  M = a10 a11 a12 a13
      a20 a21 a22 a23
       0   0   0   1

 We define  M = T R S  where  R = Heading Pitch Roll.
 Since we require this form the scale matrix must be uniform.

 We say this matrix can be decomposed into scale followed by
 a rotation followed by a translation (without proof here).


    / x \            / x + dx \
v = | y |      T v = | y + dy |
    | z |            | z + dz |
    \ 1 /            \   1    /

 </pre>
 *
 */

class dtkVec3 ;
class dtkCoord ;

class DTKAPI dtkMatrix
{
 public:

  /*! Create an identity matrix */
  dtkMatrix(void);

  /*! Create a matrix with dtkCoord data */
  dtkMatrix(dtkCoord);

  /*! Create a matrix with dtkVec3 data*/
  dtkMatrix(dtkVec3 xyz, dtkVec3 hpr);

  /*! Copy constructor */
  dtkMatrix( const dtkMatrix& m );

  /*! Destructor */
  virtual ~dtkMatrix(void);

  /*! Set the value of the matrix to the identiy matrix */
  void identity(void);

  /*! Multiply the matrix with another matrix <em>m</em> */
  void mult(const dtkMatrix *m);

  /*!
   * Print the matrix. This is used for debuging.
   *
   * \param file If \e file is NULL this will print to stdout.
   */
  void print(FILE *file=NULL) const; // default is stdout
  
  /*!
   * Return the scale of the matrix
   */
  float scale(void) const;

  /*!
   * Multiply the matrix by a scale. The scale, <em>s</em>, must be
   * positive
   */
  void scale(float s);
  
  /*!
   * Invert the matrix
   */
  void invert(void);
  
  /*!
   * Get the Heading=Rz then Pitch=Rx then Roll=Ry part of the matrix
   * assuming the matrix form above.
   */
  void rotateHPR(float *h, float *p, float *r) const;

  /*!
   * Get the heading (Rz) h, pitch (Ry) p and roll (Ry) r assuming the
   * matrix form above.
   *
   * \param h Heading
   * \param p Pitch
   * \param r Roll
   */
  void rotateHPR(float h, float p, float r);
  
  //! Get the heading, pitch, and roll
  /*!
   * Get the rotation, heading, pitch and roll, part of
   * the matrix assuming the matrix form above.
   */
  void rotateHPR(dtkVec3 *hpr) const;

  /*!
   * Rotate the current matrix by a rotation matrix constructed by using the
   * heading, pitch and roll dtkVec3 hpr.
   */
  void rotateHPR(dtkVec3 hpr) ;
  
  /*!
   * Get the translation for the matrix
   */
  void translate(float *x, float *y, float *z) const;

  /*!
   * Set the translation for the matrix
   */
  void translate(float x, float y, float z);
  
  /*!
   * Get the translation for the matrix
   */
  void translate(dtkVec3 *xyz) const;

  /*!
   * Set the translation for the matrix
   */
  void translate(dtkVec3 xyz);
  
  /*!
   * Get the coord from the matrix assuming the matrix form above.
   */
  void coord(dtkCoord *) const;

  /*!
   * Set translate and then rotate the matrix with values from this
   * coord.
   */
  void coord(dtkCoord coord);
  
  /*! Copy the matrix
   *
   * \param m is a pointer to a matrix to copy.
   */
  void copy(const dtkMatrix *m);


  /*!
   * Rotate the current matrix by quaternion (x, y, z, w) 
   * 
   * the code for this routine was adapted from various online sources,
   * including but limited to:
   *   http://www.cs.berkeley.edu/~laura/cs184/quat/quaternion.html
   *   http://www.martinb.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
   */
  void quat(float x, float y, float z, float w) ;
  
  /*!
   * Get the rotation part of the matrix as a quaternion (x, y, z, w)
   *
   * the code for this routine was adapted from various online sources,
   * including but limited to:
   *   http://www.cs.berkeley.edu/~laura/cs184/quat/quaternion.html
   *   http://www.martinb.com/maths/geometry/rotations/conversions/matrixToQuaternion/
   */
  void quat(float *x, float *y, float *z, float *w) ;
  
  /*!
   * Returns a copy of the elements in the 4x4 matrix.  
   * If either parameter is out of bounds the method just returns.
   *
   * \param row is the matrix's row number
   * \param col is the matrix's column number
   */
  inline float element(int row, int col) 
  {
    if (row>3 || row <0 || col>3 || col<0)
      return HUGE_VAL;
    return mat[row][col] ;
  }

  /*!
   * Set a element in the 4x4 matrix.  
   * If either parameter is out of bounds the method just returns.
   *
   * \param row is the matrix's row number
   * \param col is the matrix's column number
   * \param value the new value for this element
   */
    inline void element(int row, int col, float value) 
        {
            if (row <= 3 && row >= 0 && col <= 3 && col >= 0)
                mat[row][col] = value;
        }

	const dtkMatrix operator *( const dtkMatrix& m ) const;
	dtkMatrix& operator *=( const dtkMatrix& m );
    
 private:

  float mat[4][4];
  float _scale;
};


