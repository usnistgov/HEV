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

/* dtkCoord was written by John Kelso. */

/*! \class dtkCoord dtkCoord.h dtk.h
 * \brief Holds six floats representing an XYZ positon and an HPR orientation
 *
 */

class DTKAPI dtkCoord
{
 public:

  /*!
   * Creates a new zeroed dtkCoord.
   */
  dtkCoord(void) ;

  /*!
   * Creates a new dtkCoord with values set.
   *
   * \param x set the X component to  x
   * \param y set the Y component to  y
   * \param z set the Z component to  z
   * \param h set the H component to  h
   * \param p set the P component to  p
   * \param r set the R component to  r
   */
  dtkCoord(float x, float y, float z, float h, float p, float r) ;

  /*!
   * Creates a new dtkCoord with values set.
   *
   * \param xyzhpr
   * Set the X component to  xyzhpr[0].
   * Set the Y component to  xyzhpr[1].
   * Set the Z component to  xyzhpr[2].
   * Set the H component to  xyzhpr[3].
   * Set the P component to  xyzhpr[4].
   * Set the R component to  xyzhpr[5].
   */
  dtkCoord(float *xyzhpr) ;

  /*!
   * Creates a new dtkCoord with values set.
   *
   * \param xyz
   * Set the X component to  xyz->x.
   * Set the Y component to  xyz->y.
   * Set the Z component to  xyz->z.
   * \param hpr
   * Set the H component to  hpr->h.
   * Set the P component to  hpr->p.
   * Set the R component to  hpr->r.
   */
  dtkCoord(dtkVec3 *xyz, dtkVec3 *hpr) ;

  /*!
   * Creates a copy of a dtkCoord
   *
   * \param c The values in  are copied to the constructed dtkCoord.
   */
  dtkCoord(const dtkCoord &c) ;

  /*!
   * Destructor
   */
  virtual ~dtkCoord(void);

  //! = operator
  /*!
   * Copies all the corrisponding components for \e c to this dtkCoord.
   */
  const dtkCoord &operator=(const dtkCoord &c);

  /*! X component
   */
  float &x ;

  /*! Y component
   */
  float &y ;

   /*! Z component
   */
  float &z ;

  /*! H component, heading
   */
  float &h ;

  /*! P component, pitch
   */
  float &p ;

  /*! R component, roll
   */
  float &r ;

  /*! you can refer to them as an array too, d[0] - d[5]
   */
  float d[6] ;

  /*!
   * Get dtkVec3 conatining xyz values
   */
  dtkVec3 xyz() { return dtkVec3( d ); };

  /*!
   * Get dtkVec3 containing hpr values
   */
  dtkVec3 hpr() { return dtkVec3( d + 3 ); };

  /*!
   * Set values.
   *
   * \param x set the X component to  x
   * \param y set the Y component to  y
   * \param z set the Z component to  z
   * \param h set the H component to  h
   * \param p set the P component to  p
   * \param r set the R component to  r
   */
  void set(float x, float y, float z, float h, float p, float r);

  /*!
   * Set values.
   *
   * \param xyz
   * Set the X component to  xyz->x.
   * Set the Y component to  xyz->y.
   * Set the Z component to  xyz->z.
   * \param hpr
   * Set the H component to  hpr->h.
   * Set the P component to  hpr->p.
   * Set the R component to  hpr->r.
   */
  void set(dtkVec3 *xyz, dtkVec3 *hpr);

  /*! Set values.
   *
   * \param xyzhpr
   * Set the X component to  xyzhpr[0].
   * Set the Y component to  xyzhpr[1].
   * Set the Z component to  xyzhpr[2].
   * Set the H component to  xyzhpr[3].
   * Set the P component to  xyzhpr[4].
   * Set the R component to  xyzhpr[5].
   */
  void set(float *xyzhpr);

  /*! Get values.
   *
   * \param x the value pointed to by  x is set to the X component.
   * \param y the value pointed to by  y is set to the Y component.
   * \param z the value pointed to by  z is set to the Z component.
   * \param h the value pointed to by  h is set to the H component.
   * \param p the value pointed to by  p is set to the P component.
   * \param r the value pointed to by  r is set to the R component.
 
   */
  void get(float *x, float *y, float *z, float *h, float *p, float *r);

  /*! Get values.
   *
   * \param xyz the values in X, Y, and than Z component are put into
   * the dtkVec3 pointed to by  xyz.
   *
   * \param hpr the values in H, P, and than R component are put into
   * the dtkVec3 pointed to by  hpr.
   */
  void get(dtkVec3 *xyz, dtkVec3 *hpr);

   /*! Get values.
    *
    * \param xyzhpr A 6-D float array that will have it's values
    * changed to the X, Y, Z, H, P, and R components, in that order.
    */
  void get(float *xyzhpr);

  /*! Print values.
   *
   * \param file a file stream to print to.  If \e file is NULL stdout
   * will be used.
   */
  void print(const FILE *file = NULL);

  /*! Zero all 6 values.
   */
  void zero(void);

  /*! Check if all components of this object are the same as in  c.
   *
   * \param c is a pointer to dtkCoord object.
   *
   * \return Returns 1 if all 6 corrisponding values in this dtkCoord
   * are the same as those in the dtkCoord pointed to by  c, else
   * return 0.
   */
  int equals(dtkCoord *c);

  bool operator==( const dtkCoord& c );
  bool operator!=( const dtkCoord& c );
};
