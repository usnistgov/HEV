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

/*! \class dtkInput dtkInput.h dtk.h
 * \brief Reads and writes entries from and to a dtkRecord
 *
 * An object of this class is tied to a record keeping object called
 * dtkRecord.  A dtkRecord will manage the memory that this dtkInput
 * class object will use.
 *
 * This class can do a polling read on its own and with the aid of
 * dtkRecord or dtkDequeuer can read queued events.  dtkInput in
 * intended for building higher level input device reading/writing
 * classes. dtkInput is a base class to dtkInButton, dtkInSelector,
 * dtkInValuator, and dtkInLocator.
 *
 * This class is a friend of dtkRecord.  In place of a lot of gutless
 * methods to hook into the private data of dtkRecord we let this
 * class change the dtkRecord data its self.
 *
 * The only errors that can occur are due to constructing with an
 * invalid dtkRecord and array over runs.  So if your careful you
 * generally don't need to check for errors in this class.
 */

class DTKAPI dtkInput : public dtkAugment
{
public:

  /*! Contruct a dtkInput object with a given dtkRecord.
   *
   * \param record a pointer to the dtkRecord to store the data
   * entries in.
   *
   * \param size is the size, in bytes, of the entries of this dtkInput
   * device.
   *
   * \param name is the name of the dtkAugment that is passed to the
   * dtkAugment constructor.
   */
  dtkInput(dtkRecord *record, size_t size, const char *name = NULL);

  /*! Destructor
   */
  virtual ~dtkInput(void);

  /*! 
   * \return 1 if data is queued else return 0
   */
  int isQueued(void);

  //! Tell the object whether to queue or not
  /*!
   * dtkInput object start out by default being not queued. It
   * generally a good idea to let the reader of the object decide if
   * the object should be queued. If you don't read from a queue,
   * don't queue it!
   *
   * \param truth being non-zero will cause this to be queued.  If \e
   * truth is 0 than this to not be queued. 
   */
  void queue(int truth = 1);

protected:

  //! Reads an entry
  /*!
   * \param rec If \e rec is NULL this reads this objects entry from last
   * (newest) event record (it polls).
   *
   * \return This returns a pointer to the data in the event.  The
   * pointer returned should not be written to by the user.  If \e rec
   * is non-NULL this will return an event record from that pointed to
   * by \e rec. A derived class may want to overwrite (wrap) this so
   * it returns a more approprate data type like for example
   * dtkInButton, dtkInSelector, dtkInValuator, and dtkInLocator.
   */
  void *readRaw(const struct dtkRecord_event *rec = NULL);

  //! Write an entry
  /*!
   * The data is written to the current entry with all the other
   * dtkInput events and if the device is queued than the entry with
   * all the other dtkInput events is duplicated to the next queued
   * entry. This enables you to poll the state of all devices in a
   * given queue entry.
   *
   * \param buffer is a pointer to the data to be written into the entry.
   * \e buffer must be at least as large as the entrys in this device.
   * 
   */
  int writeRaw(const void *buffer);

private:

  // Set _isQueued to make this input cause things to be queued.  By
  // default it's causing a queued.
  int _isQueued;
  void clean_up(void);
  int sync_account_with_record(void);

  // Entry position offset from the start of an event.  This is
  // maintained by the dtkRecord that manages the event queue.  It
  // may change if a dtkInput, client, is added to the dtkRecord.
  size_t offset;

  // entry size
  size_t size;

  int change_count;

  int queue_length;

  dtkRecord *record;
};
