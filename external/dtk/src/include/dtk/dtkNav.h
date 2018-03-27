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
/****** dtkNav.h ******/
/*! \class dtkNav dtkNav.h dtk.h
 * \brief Base class for a DTK nav object
 *
 * dtkNav provides a base class for navigational techniques.  Navigation can
 * be specified by using descriptive data such as location, scale, pivot,
 * response, and velocity, which are transformed by this class into a
 * dtkMatrix, or by directly providing a dtkMatrix.
 *
 * This class invokes methods of the dtkDisplay class to add itself to a
 * list of navigatons.
 *  
*/


class DTKAPI dtkNav : public dtkAugment
{
 public: 
  //! Constructor
  /*!
   * The \e name is passed to and copied in the dtkAugment base class.
   */
  dtkNav(dtkManager *manager, const char *name);
  //! Destructor
  virtual ~dtkNav(void);

  /*! Register the nav object with the display objects
   */
  virtual int postConfig(void);

  /*! The dtkMatrix is calculated once per loop, after Frame.
   */
  virtual int postFrame(void);

  /*! Set/return the navigation speed (or responsiveness).
   */
  virtual int response(float);
  virtual float response(void);
  
  /*!
   * Anyone can change the location and scale.  By default, the
   * postFrame callback sets the location and scale into the
   * dtkMmatrix _matrix.  So this transformation has 7 degrees of
   * freedom: location.x,location.y, location.z, location.h,
   * location.p, location.r.
   */
  dtkCoord location ;

  /*!
   * Transformation scale.
   */
  float scale ;

  /*! velocity is xyz/hpr translational and angular velocites, using a
   * pointer to a dtkCoord.
   */
  virtual dtkCoord *velocity(void) ;

  /*! You can set a pointer to your own array of floats, in which case
   * the it is used instead of the default velocity.  Setting it to
   * NULL restores the default velocity.  Setting it if *velocity() is
   * NULL returns 1, an error.  Otherwise 0 is returned.\
   */
  virtual void velocity(dtkCoord *) ;

  /*! Some navigations need a pivot around which to rotate.  You get pointers
   * to the pivot so that the application programmer may vary them all
   * through the running of the application.  The particular dtkNav should
   * document what it does with pivot, if anything.
   */
  virtual dtkCoord *pivot(void) ;

  /*! You can set a pointer to your own dtkCoord, in which case it is
   * used instead of the default pivot behavior.  Setting it to NULL
   * restores the default behavior. 
   */
  virtual void pivot(dtkCoord *) ;

  /*! Turn on the actions (function) of this object by adding the
   * callback from the manager's list.
   */
  virtual int activate(void);
  /*! Turn off the actions (function) of this object by removing the
   * callback from the manager's list.
   */
  virtual int deactivate(void);

  /*! Provide a matrix that contains the navigation- if this is set
   * then the usual calculation with location, scale and so forth are
   * bypassed.
   */
  int userMatrix(dtkMatrix *) ;

  /*! Get the matrix that contains the navigation.
   */
  dtkMatrix *userMatrix(void) ;
  
  /*! Returns a pointer to a dtkMatrix containing the current navigation.
   */
  dtkMatrix *matrix(void) ;
  
 protected:

  dtkManager *m ;

  float _response ;
  dtkMatrix *_userMatrix ;

  // the currently used velocity, by default equal to defVelocity
  dtkCoord *_velocity ;
  // the default velocity
  dtkCoord *defVelocity ;

  // the currently used pivot, by default equal to defPivot
  dtkCoord *_pivot ;
  // the default pivot
  dtkCoord *defPivot ;

 private:
  dtkMatrix _matrix ;
  dtkCoord old_location; // value that was in the last frame
  float old_scale;      // value that was in the last frame

};
