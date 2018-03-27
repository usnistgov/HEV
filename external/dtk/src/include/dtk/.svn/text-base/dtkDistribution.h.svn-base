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
/****** dtkDistribution.h ******/
//!Define the variable to -1
#define DTK_DISTRO_NO_NORM (-1.0)

/*! \class dtkDistribution dtkDistribution.h dtk.h
 * \brief  This class provides operations on a number of points and values
 *
 *  This class provides a storage mechanism for building distribution
 *  functions. You can reset, put, calculate, setNormal, print, and
 *  return the number of points and values.
 */

class DTKAPI dtkDistribution : public dtkBase
{
public:

  //! Constructor
  /*!
   * Initialize a distribution with \e num_points points, minimum value
   * \e min, maximum value \e max, and the normalization value of \e
   * norm.
   */
  dtkDistribution(int num_points, double min,
		  double max, double norm=DTK_DISTRO_NO_NORM);
  //! Destructor
  virtual ~dtkDistribution(void);
  /*!
   * Removes all values from the distribution.
   */
  void reset();
  /*!
   * Add a value \e x to the distribution.
   */
  void put(double x);
  /*!
   * Return the mean value in the distribution.
   */
  double mean(void) const;
  /*!
   * Calculate the current distribution function values.
   */
  void calculate();
  /*!
   * Set the normalization constant.
   */
  void setNormal(double norm); // make the sum curve normalized to norm
  double *x, *y, *ynorm, *sum; // sum is normalized to norm
  /*!
   * Print the distribution to the file \e file in ASCII format.
   */
  void print(FILE *file); // default is stdout
  /*!
   * Return the number of points that are in the
   * distribution.
   */
  inline int numberOfPoints(void) const { return num_points; }
  /*!
   * Return the number of values that have been written to the
   * distribution.
   */
  inline int numberOfValues(void) const { return value_count; }

private:

  void init(int num_points, double min, double max, double norm);
  void remove_object(void);
  double  min, max, norm, delta;
  int num_points; // number of bins
  double x_total;
  int value_count; // number of values loaded
};
