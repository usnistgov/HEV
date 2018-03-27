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

/*! \class dtkDisplay dtkDisplay.h dtk.h
 * \brief Base class for a DTK display object
 *
 * dtkDisplay is a special dtkAugment base class that adds the three
 * more callbacks: config(), frame() and sync() to the list of
 * calbacks, in addition to the dtkAugment callbacks, that the
 * dtkManager calls.  This class provides the basic interface for
 * making a display.  By default, in this base class, config(),
 * frame(), and sync() remove themselves from the dtkManager callback
 * list by returning dtkAugment::REMOVE_CALLBACK.
 *
 * For a typical dtkDisplay the sync() method synchronizes its running
 * with some kind of hardware event, like for example the switching of
 * displayed roster memory for a graphic display.  A non-graphical,
 * dynamics dtkDisplay may use an interval timer to synchronize with
 * the system clock at regular intervals by calling dtkRealTime_wait()
 * in its dtkDisplay::sync(). If you load two or more dtkdisplay
 * objects that have implemented a dtkDisplay::sync() that
 * synchronizes to hardware, then they may need to synchronize in a
 * coordinated fashion to get good results.
 *
 * The dtkDisplay::frame() method should be overwritten to do
 * something active. Like for example a graphics display would write
 * into a graphics frame buffer.  A dymanics display would calculate
 * the next dynamical state, like turn the machine one cycle.
 */
class DTKAPI dtkDisplay : public dtkAugment
{
 public: 

  /*!
   *  \param name is copied to the dtkAugment name.
   */
  dtkDisplay(const char *name);

  /*!
   * Destructor
   */
  virtual ~dtkDisplay(void);

  //! Configure callback for the display
  /*!
   * Overwrite this method to insert a configuration callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR.
   */
  virtual int config(void);

  //! Frame callback for the display
  /*!
   * Overwrite this method to insert a display frame callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR.
   */
  virtual int frame(void);

  //! Synchronization callback for the display
  /*!
   * Overwrite this method to insert a display synchronizing callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR.
   */
  virtual int sync(void);
};
