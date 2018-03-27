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
/****** dtkSharedTime.h ******/

/*! \class dtkSharedTime dtkSharedTime.h dtk.h
 *
 *  \brief A gettimeofday() wrapper that shares a time offset in DTK shared memory
 *
 * This is a small wrapper on the standard C library function
 * <b>gettimeofday()</b>.  It keeps a timer offset so that you can get
 * timing differences.  The timer offset is put in dtk shared memory.
 * This assumes that time cannot be negative and time does not run
 * backwards.
 *
 * The methods in this class return the current time minus an offset
 * time as a <em>long doubles</em>.  A <em>double</em> is not large
 * enough to encode all the information in a <em>struct timeval</em>
 * which is returned by the standard C library function
 * <b>gettimeofday()</b>.
 *
 */

// This gets put in shared memory
struct dtkSharedTime_
{
  struct timeval t0;    // time offset
  struct timeval stop; // gettimeofday() time when last stopped
  struct timeval start; // gettimeofday() time when last started
  int running;      // running == 1  or not == 0
};


class DTKAPI dtkSharedTime : public dtkBase
{
public:

  /*!
   * If the file \e shmFile does not exist, this sets the current time
   * as the time offset value, i.e. time is set to zero.  If \e
   * start_paused is non-zero and \e shmFile does not exist, than the
   * time will be paused when this object is created.
   */
  dtkSharedTime(const char *shmFile="time", int start_paused=0);

  /*! Destructor */
  virtual ~dtkSharedTime(void);

  //! Get the change in time.
  /*!
   * Get the change in time since the last call to <b>delta</b>,
   * returns 0 if time is stopped, or the time since the last stop.
   */
  long double delta(void);

  //! Get the time.
  /*!
   * Get the elapsed time minus the stopped periods.
   */
  long double get(void);
  //! Set the offset time.
  /*!
   * Gets the current time and sets the offset time to the current
   * time minus <b><em>time0</em></b>.
   */
  int reset(long double time0 = ((long double) 0.0));
  //! Pause time.
  /*!
   * Pause at the current time.  This is ignored if it is already
   * paused.
   */
  int pause(void);
  //! Resume time.
  /*! Resume. This is ignored if it is
   * time is already running.
   */
  int resume(void);
  //! Quiery if the time is running.
  /*!
   * Returns 1 if it is running (not paused).  Returns 0 if it is
   * paused.  Return -1 on error.
   */
  int isRunning(void);

private:

  dtkSharedMem *shm;
  struct dtkSharedTime_ st;
  struct timeval delta_t_offset; // for getting delta() time.
};

