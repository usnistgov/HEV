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
/****** dtkRealtime.h ******/

/*! \file dtkRealtime.h
 * 
 * A group of functions to get your process a periodic heart beat
 * without CPU spinning.  They can be used to pause your process at
 * regular intervals.  A group of functions that use a system interval
 * timer and an ALARM signal.
 *
 * Since there is only one ALARM signal per process that can be used
 * on Linux and IRIX and most UNIX like systems, there can be only one
 * instance of this class. So we inforce this restriction by making a
 * single "C like class" with its one set of data kept as static
 * variables in the one object.
 *
 * I haven't seen any of these calls fail yet.  The returned values
 * are just the refletion of an error in an under-lying system call.
 *
 \code
 // dtkRealtime user example code //
 
  #include <dtk.h>

  int running = 1;

  int main()
   {
     init_sim_stuff();
     dtkRealtime_init( 0.01 );
     while(running)
      {
        do_sim_stuff();
        // Wait for the next alarm
        dtkRealtime_wait();
      }
     dtkRealtime_close();
     return 0;
   }
 \endcode

*/


/*!
 * Sets ups a system interval timer to a given period of \e sec
 * seconds and \e usec micro seconds.
 * \return 0 on success
 * \return nonzero on falure
 */
extern int dtkRealtime_init(int sec, int usec);

/*!
 * Sets ups a system interval timer to a given period of \e sec
 * seconds.
 * \return 0 on success
 * \return nonzero on falure
 */
extern int dtkRealtime_init(double sec);

/*!
 * Pause the current process until the ALARM is received.  If the
 * ALARM has already expired than this returns immediately.
 * \return 0 on success
 * \return nonzero on falure
 */

extern int dtkRealtime_wait(void);

/*!
 * Removes the system interval timer that was setup by calling
 * dtkRealtime_init()
 * \return 0 on success
 * \return nonzero on falure
 */
extern int dtkRealtime_close(void);
