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

/*! \class dtkDequeuer dtkDequeuer.h dtk.h
 * \brief Reads events from a dtkRecord
 *
 * A dtkDequeuer provides read access to a dtkRecord that is
 * independent of the reading by other objects.  It provides seperate
 * pointers to the records held by a dtkRecord object. The actions of
 * a given dtkDequeuer do not effect other dtkDequeuers.
 */

class DTKAPI dtkDequeuer : public dtkBase
{
public:

  /*!
   * \param record is a pointer to the dtkRecord object that this
   * object will read events from.
   */
  dtkDequeuer(dtkRecord *record);

  /*! destructor
   */
  virtual ~dtkDequeuer(void);

  //! Dequeue the next event
  /*!
   * Get the next event in the queue, and dequeue the queue.  The
   * pointers returned should not be written to by the user.
   *
   * \param event_return If \e event_return is non-NULL \e
   * event_return will be set to point to the event record.
   *
   * \param input_return If \e input_return is non-NULL it will be set
   * to the dtkInput that caused the event record to be generated.
   *
   * \return Returns the number of events in the queue before this
   * call.
   */
  int getNextEvent(struct dtkRecord_event **event_return = NULL,
		   dtkInput **input_return = NULL);

  //! Dequeue the next event that is from a given dtkInput
  /*!
   * Get the next event record and skip events that are not from the
   * dtkInput \e input.
   *
   * \param input is a pointer to the device who's
   * data your looking for.
   *
   * \return Returns a pointer to the event record or NULL if no
   * events are found that are from dtkInput \e input.
   */
  struct dtkRecord_event *getNextEvent(dtkInput *input);

  /*!
   * \return Returns a pointer to the last event record
   * do to a call from getNextEvent().
   */
  struct dtkRecord_event *getEvent(void);

  /*!
   * \return Returns a pointer to the dtkInput that was associated
   * with the last call from getNextEvent().
   */
  dtkInput *getInput(void);

private:

  void sync_with_record(void);

  dtkRecord *record;

  // next event to be returned from getNextEvent()
  struct dtkRecord_event *next_event;

  // incremented every time a record is queued
  // and decremented every time a record is dequeued
  int queue_count;

  // incremented every time a record is queued
  int queue_add_count;

  int change_count;
};

