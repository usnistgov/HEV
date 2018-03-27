/*
 * The DIVERSE Toolkit
 * Copyright (C) 1994 - 2001 Lance Arsenault 
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
/****** dtkRK4.h ******/
#ifndef __dtkRK4_h__
#define __dtkRK4_h__

#include <stdlib.h>

/*! \class dtkRK4 dtkRK4.h dtk/dtkRK4.h
 *
 *  \brief A class for solving ordinary differential
 equations using a simple 4th order Runge-Kutta integrator
 *
 * 4th order Runge-Kutta integrator class for integrating ordinary
 * differential equations with a given time step.  No unit of time or
 * other measuring units are assumed.
 *
 * All the state variables the system of ordinary differential
 * equations are kept by the user.  The user passes pointers to state
 * variables to the methods in this class and then this class advances
 * these state variables forward in time the amount specified.
 *
 * Sometimes this is used to integrate in real-time. Time on todays
 * computers (when this was written) comes as two 4 byte numbers = 8
 * bytes, and because an 8 byte double has less then 8 bytes in the
 * manisa the next biggest float (long double) is used to measure
 * time.  If small differences in large times must be calculated then
 * the time variables must be kept from losing precision.  This is the
 * start of the justification for using long double time values in the
 * systems of differential equations with doubles.
 */

// This class does not contain state information.  It does however
// keep the last independent variable (time) that has been integrated
// to.

// X is state type.  T is time step type. Absolute time is stored as a
// long double so that it may be compared to wall clock time
// (real-time).
template <class X, class T>
class dtkRK4
{
 public:

  //! Constructor.
  /*! \param difeq \e difeq is a pointer to a set of ordinary differential
   * equations.
   * \param num_diffeq \e num_diffeq is the number of first order
   * differential equations.
   * \param tstep \e tstep is the time to
   * advance the differential equations when they are integrated
   * by one 4th order Runge-Kutta step.
   * \param t_0 the starting time.
   */
  dtkRK4(void (*difeq)(X *xdot, const X *x, long double t),
	 int num_diffeq, T tstep, long double t_0=(long double)0.0);

  //! Constructor.
  /*!
   * \param difeq \e difeq is a pointer to a dtkDiffEq object
   * which contains the information about the ordinary differential
   * equations that are to be integrated.
   * \param tstep \e tstep is the time to
   * advance the differential equations when they are integrated
   * by one 4th order Runge-Kutta step.
   * \param t_0 the starting time.
   */
  dtkRK4(dtkDiffEq<X> *difeq, T tstep, long double t_0=(long double)0.0);

  //! Destructor.
  /*! Frees memory and deletes the object.
   */
  virtual ~dtkRK4(void);

  //! Set the time step.
  /*! Set the time step for on 4th order Runge-Kutta step
   * to \e tstep.
   */
  void setTimeStep(T tstep);

  //! Advance one time step.
  /*! Advance the state pointed to by \e t and
   * \e x to a state with time \e *t plus the
   * current time step.
   */
  void go(X *x, long double *t);

  //! Advance to a given time.
  /*! Advance the state with initial time \e t_from and
   * \e x to a state with time \e t_to.
   * The 4th order Runge-Kutta method is applied one or more times.
   * The last application of the 4th order Runge-Kutta method may
   * use a smaller value time step value than what the current time step
   * is set at in order to get to time \e t_to.
   */
  void go(X *x, long double t_to, long double t_from);
  void go(X *x, long double t_to);

  int getNumDiffEq(void) const;
  void setCurrentTime(long double t);

 private:

  void init(int num_diffeq_in, T tstep, long double t_0);
  void clean_up(void);

  X *k1, *k2,
    *k3, *k4,
    *nx, *xdot;
  T tstep;
  int num_diffeq;

  void (*difeq)(X *xdot, const X *x, long double t);
  dtkDiffEq<X> *diffEq;

  // Time last intergrated to.
  long double current_time;
};


template <class X, class T>
int dtkRK4<X,T>::getNumDiffEq(void) const
{
  return num_diffeq;
}

template <class X, class T>
void dtkRK4<X,T>::setCurrentTime(long double t)
{
  current_time = t;
}

template <class X, class T>
void dtkRK4<X,T>::clean_up(void)
{
  if(k1) free(k1);
  if(k2) free(k2);
  if(k3) free(k3);
  if(nx) free(nx);
  if(xdot) free(xdot);
  num_diffeq = 0;
  k1 = k2 = k3 = nx = xdot = NULL;
  difeq = NULL;
  diffEq = NULL;
}

template <class X, class T>
void dtkRK4<X,T>::init(int num_diffeq_in, T tstep_in, long double t_0)
{
  k1 = NULL;
  k2 = NULL;
  k3 = NULL;
  nx = NULL;
  xdot = NULL;
  difeq = NULL;
  diffEq = NULL;

  num_diffeq = num_diffeq_in;
  tstep = tstep_in;
  current_time = t_0;

  if(num_diffeq_in < 1)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRK4 initialization failed is number of "
		 "equations requested is: %s%d%s which is less than 1.\n",
		 dtkMsg.color.vil, num_diffeq_in, dtkMsg.color.end);
      return;
    }

  if(  (NULL == (k1   =(X *)dtk_malloc(sizeof(X)*num_diffeq_in)))
    || (NULL == (k2   =(X *)dtk_malloc(sizeof(X)*num_diffeq_in)))
    || (NULL == (k3   =(X *)dtk_malloc(sizeof(X)*num_diffeq_in)))
    || (NULL == (nx   =(X *)dtk_malloc(sizeof(X)*num_diffeq_in)))
    || (NULL == (xdot =(X *)dtk_malloc(sizeof(X)*num_diffeq_in))))
    {
      dtkMsg.add(DTKMSG_ERROR, 1,
		 "dtkRK4::dtkRK4() initialization "
		 "failed:\n");
      clean_up();
    }
}


template <class X, class T>
dtkRK4<X,T>::dtkRK4(dtkDiffEq<X> *diffEq_in, T tstep_in, long double t_0)
{
  if(!diffEq_in)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRK4::dtkRK4(%p,%g initialization "
		 "failed your differential"
		 " equations call-back object at %p is invalid.\n",
		 diffEq_in, tstep_in, diffEq_in);
      return;
    }

  init(diffEq_in->getNumberOfEquations(), tstep_in, t_0);
  diffEq = diffEq_in;
}


template <class X, class T>
dtkRK4<X,T>::
dtkRK4(void (*difeq_in)(X *xdot, const X *x, long double t),
       int num_diffeq_in, T tstep_in, long double t_0)
{
  if(!difeq_in)
    {
      dtkMsg.add(DTKMSG_ERROR,
		 "dtkRK4::dtkRK4(%p,%d,%g) initialization"
		 "failed: your differential"
		 " equations call-back function is NULL.\n",
		 difeq_in, num_diffeq_in, tstep_in);
      return;
    }

  init(num_diffeq_in, tstep_in, t_0);
  difeq = difeq_in;
}

template <class X, class T>
dtkRK4<X,T>::~dtkRK4(void)
{
  clean_up();
}

template <class X, class T>
void dtkRK4<X,T>::setTimeStep(T tstep_in)
{
  tstep = tstep_in;
}

// go one time step (tstep)

template <class X, class T>
void dtkRK4<X,T>::go(X *x, long double *t)
{
  int i;
  long double tt;
  const X six = 6.0, three = 3.0, two = 2.0;

  if(!k1) return; // error

  if(diffEq)
    {
      diffEq->getDerivatives(xdot,x,*t);
      for(i=0;i<num_diffeq;i++)
	nx[i] = x[i] + (k1[i]=tstep*xdot[i])/two;
      tt = *t + tstep/two;
      diffEq->getDerivatives(xdot,nx,tt);
      for(i=0;i<num_diffeq;i++)
	nx[i] = x[i] + (k2[i]=tstep*xdot[i])/two;
      diffEq->getDerivatives(xdot,nx,tt);
      for(i=0;i<num_diffeq;i++)
	nx[i] = x[i] + (k3[i]=tstep*xdot[i]);
      *t += tstep;
      diffEq->getDerivatives(xdot,nx,*t);
      for(i=0;i<num_diffeq;i++)
	x[i] = x[i]+k1[i]/six+k2[i]/three+k3[i]/three+tstep*xdot[i]/six;
    }

  else if(difeq)
    {
      difeq(xdot,x,*t);
      for(i=0;i<num_diffeq;i++)
	nx[i] = x[i] + (k1[i]=tstep*xdot[i])/two;
      tt = *t + tstep/two;
      difeq(xdot,nx,tt);
      for(i=0;i<num_diffeq;i++)
	nx[i] = x[i] + (k2[i]=tstep*xdot[i])/two;
      difeq(xdot,nx,tt);
      for(i=0;i<num_diffeq;i++)
	nx[i] = x[i] + (k3[i]=tstep*xdot[i]);
      *t += tstep;
      difeq(xdot,nx,*t);
      for(i=0;i<num_diffeq;i++)
	x[i] = x[i]+k1[i]/six+k2[i]/three+k3[i]/three+tstep*xdot[i]/six;
    }
  current_time = *t;
}

template <class X, class T>
void dtkRK4<X,T>::go(X *x, long double t_to)
{
  go(x, t_to, current_time);
}

template <class X, class T>
void dtkRK4<X,T>::go(X *x, long double t_to, long double t_from)
{
  T tstep_save = tstep;
  long double t = t_from;
  long double t_to_minus_tstep = t_to - tstep;

  while(t < t_to_minus_tstep)
    go(x, &t);
  tstep = t_to - t;
  if(tstep > (T) 0.0)
    go(x, &t);
  tstep = tstep_save;
}

#endif /* #ifndef __dtkRK4_h__ */
