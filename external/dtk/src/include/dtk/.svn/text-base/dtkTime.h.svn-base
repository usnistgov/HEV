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
/****** dtkTime.h ******/

class dtkBase;

/*! \class dtkTime dtkTime.h dtk.h
 *
 *  \brief A small <b>gettimeofday()</b> wrapper.
 *
 * This is a small wrapper on the standared C library function
 * <b>gettimeofday()</b>.  It keeps a timer offset so that you can
 * get timing differences.
 *
 * The methods in this class return the current time minus an offset
 * time as a \e long \e doubles.  A \e double is not large enough to
 * incode all the information in a \e struct timeval which is returned
 * by the standared C library function <b>gettimeofday()</b>.
 *
 */

class DTKAPI dtkTime : public dtkBase
{
public:

  /*! This constructor sets an offset time.
   *
   * \param time0
   * \param flag If \e flag is zero then the offset time will be set
   * to \e time0 seconds, and so the time returned by get() will be
   * the the time in seconds since January 1, 1970 minus \e time0
   * seconds.  If \e flag is nonzero then the offset time will be set
   * so that the time when this is called the time will be \e time0
   * seconds.
   */
  dtkTime(double time0=0.0, int flag=0);

  /*! This constructor sets an offset time.
   *
   * \param time0
   * \param flag If \e flag is zero then the offset time will be set
   * to \e time0 seconds, and so the time returned by get() will be
   * the the time in seconds since January 1, 1970 minus \e time0
   * seconds.  If \e flag is nonzero then the offset time will be set
   * so that the time when this is called the time will be \e time0
   * seconds.
   */
  dtkTime(long double time0, int flag=0);

  /*! This constructor sets an offset time.
   *
   * \param time0
   * \param flag If \e flag is zero then the offset time will be set
   * to \e time0 seconds, and so the time returned by get() will be
   * the the time in seconds since January 1, 1970 minus \e time0
   * seconds.  If \e flag is nonzero then the offset time will be set
   * so that the time when this is called the time will be \e time0
   * seconds.
   */
  dtkTime(struct timeval *time0, int flag=0);

  /*! Destructor */
  virtual ~dtkTime(void);

  //! Get the change in time.
  /*! Get the change in time since the last call to delta().  If
   * reset(), or one of the constructors was called before delta(),
   * with no delta() called between, the returned time value of
   * delta() will depend on the arguments to reset(), or of the
   * constructor.
   */
  long double delta(void);

  //! Get the time.
  /*! Get the elapsed time since the last call to delta(),
   * reset(), or one of the constructors was called.
   */
  long double get(void);

  /*! Set the offset time.
   *
   * \param time0
   * \param flag If \e flag is zero then the offset time will be set
   * to \e time0 seconds, and so the time returned by get() will be
   * the the time in seconds since January 1, 1970 minus \e time0
   * seconds.  If \e flag is nonzero then the offset time will be set
   * so that the time when this is called the time will be \e time0
   * seconds.
   */
  void reset(double time0=0.0, int flag=0);

  /*! Set the offset time.
   *
   * \param time0
   * \param flag If \e flag is zero then the offset time will be set
   * to \e time0 seconds, and so the time returned by get() will be
   * the the time in seconds since January 1, 1970 minus \e time0
   * seconds.  If \e flag is nonzero then the offset time will be set
   * so that the time when this is called the time will be \e time0
   * seconds.
   */
  void reset(long double time0, int flag=0);

  /*! Set the offset time.
   *
   * \param time0
   * \param flag If \e flag is zero then the offset time will be set
   * to \e time0 seconds, and so the time returned by get() will be
   * the the time in seconds since January 1, 1970 minus \e time0
   * seconds.  If \e flag is nonzero then the offset time will be set
   * so that the time when this is called the time will be \e time0
   * seconds.
   */
  void reset(struct timeval *time0, int flag=0);

  // get the timer offset.  seconds and micro seconds.
  //time_t getTimeOffsetSec(void);
  //long getTimeOffsetUSec(void);

private:

  struct timeval t0;
};

