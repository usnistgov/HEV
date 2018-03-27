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
/****** dtkDiffEq.h ******/
#ifndef __dtkDiffEq_h__
#define __dtkDiffEq_h__

/*! \class dtkDiffEq dtk/dtkDiffEq.h
 *
 *  \brief System of ordinary differential equations as a class.
 *
 * dtkDiffEq is a pure virtual base class that contains the
 * information associated with a system of ordinary differential
 * equations.  This is a base class to a derived class that you can
 * pass to a integrator object, like an dtkRK4, or other integrator
 * object.  It's for integrating (solving) ordinary differential
 * equations.
 *
 * Example code using class dtkDiffEq with class dtkRK4:
 * \include examples/dtk/examplesByClass/dtkDiffEq.cpp
 * 
 */

template <class X>
class dtkDiffEq
{
 public:

  /*!
   * Destructor
   */
  virtual ~dtkDiffEq(void);

  //! Returns the derivatives with respect to the independent variable.
  /*! getDerivatives() is a call-back for an integrator object, like
   * dtkRK4, which returns \e xdot given state variable array \e x and
   * independent variable \e t.  */
  virtual void
    getDerivatives(X *xdot, const X *x, long double t) const = 0;
  /*!
   * Return the number of 1st order ordinary differential equations.
   */
  virtual int getNumberOfEquations(void);

 protected:

  //! Number of 1st order ordinary differential equations.
  int numberOfEquations;
};

template <class X>
int dtkDiffEq<X>::getNumberOfEquations(void)
{
  return numberOfEquations;
}

template <class X>
dtkDiffEq<X>::~dtkDiffEq(void) {}

#endif /* #ifndef __dtkDiffEq_h__ */
