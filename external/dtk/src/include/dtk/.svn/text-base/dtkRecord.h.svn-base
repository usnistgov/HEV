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

class dtkInput;
class dtkDequeuer;


#define DTKRECORD_DEFAULT_LENGTH ((size_t) 48)


//! struct dtkRecord_event is the event that is queued.

struct dtkRecord_event
{
  // A pointer to the dtkInput entry that put() (queued) the last
  // record which generates a event record of the state of all
  // entries.
  dtkInput *obj;
 
  // This is a circular event buffer.
  struct dtkRecord_event *next;
 
  // Points to the data of this event record.
  unsigned char *buffer;
};

//! a "next = NULL" terminated list
 
struct dtkRecord_account
{
  // The input object with the account. There can be only one account
  // for a given object.
  dtkInput *obj;
 
  // entry size
  size_t size;
 
  // entry position offset from the start of an event
  size_t offset;
 
  struct dtkRecord_account *next;
};

/*! \class dtkRecord dtkRecord.h dtk.h
 * \brief  This class to handle event records
 *
 * A dtkRecord is a class object that manages records for the dtkInput
 * objects.  There is one dtkRecord in a dtkManager class object.
 * Each record contains the record of all the dtkInput entries.  Each
 * dtkInput entry is a segment in all events in the records.
 *
 * There are many entries in an event.  There are many events (record
 * events) in a Record.
 *
 * None of the methods in this class will block the calling process.
 * This uses the current process to maintain the queue, so that there
 * will be no new events incured in the users event processing loop
 * unless the user put them there in the users event processing loop.
 * This is faster that large memory coping.
 *
 * This class uses handles to pass pointers around.  The returned
 * handles pointer to the pointers that point to the data managed by
 * this class.
 */

class DTKAPI dtkRecord :public dtkBase
{
public:

  /*!
   * Construct the record
   */
  dtkRecord(size_t queue_lenght = DTKRECORD_DEFAULT_LENGTH);

  /*! Destructor */
  virtual ~dtkRecord(void);
 
  //! Dequeue the next event
  /*!
   * Get the next event in the queue, and dequeue the queue. Returns the
   * number of events in the queue before this call.  The pointers
   * returned should not be written to by the user. */
  int getNextEvent(struct dtkRecord_event **event_return = NULL,
          dtkInput **input_return = NULL);
 
  //!Get the next event for a dtkInput object 
  /*! This one the same as the above but skips events that are not from
   * the dtkInput input.  input is a pointer to the device who's data
   * your looking for.  Returns NULL if no events are found. */
  struct dtkRecord_event *getNextEvent(dtkInput *input);
 
  //! Get the last dtkRecord_event from the last object that caused
  //! a dequeue
  /*!Gets the dtkRecord_event from the last thing dequeued in this
   * object.*/
  struct dtkRecord_event *getEvent(void);
 
  //! Get the dtkInput object that last sent data
  /* gets the dtkInput that caused the event from the last thing
  * dequeued in this object. */
  dtkInput *getInput(void);

  //! Get the next even with dequeueing it
  /*! 
   * Get the current event which is not queued yet, but will be
   * soon. This does not dequeue the queue. Returns the number of
   * events in the queue before this call (which is the same as
   * after). The pointers returned should not be written to by the
   * user. */
  struct dtkRecord_event *poll(void);
 
  //! debug print
  /*! debug print */
  void print(FILE *file=NULL); // default is stdout

  friend class dtkInput;
  friend class dtkDequeuer;
 
private:
 
  // remove account.  This is automatically called by
  // dtkInput::~dtkInput().
  int remove(dtkInput *input);
 
  int add(dtkInput *input, size_t event_size);
  int remove(struct dtkRecord_account *account);
  void clean_up(void);
  void reset_event_buffer_size(void);
 
  // list of dtkInput accounts
  struct dtkRecord_account *account;

// incrumented every time an input account is added or removed, so
  // that the record stucture has changed.
  int change_count;
 
  // number of events that there are in the listed link of events
  int queue_length;
 
  // size of one event
  size_t event_size;
 
  // The memory to put the events in.
  unsigned char *event_buffer;
 
  // The last event not added yet added.  This event is written to by
  // writers that that do not queue. When this does get queued this
  // will go to the next event record in the list and all the data in
  // this will be copied to the "next" current_event.  current_event
  // is where poll reads from and the newest data is written to here
  // until it gets queued.
  struct dtkRecord_event *current_event;
  // next event to be returned from getNextEvent()
  struct dtkRecord_event *next_event;
 
  // incrumented every time a record is queued
  // and deincrument every time a record is dequeued
  int queue_count;
 
  // incrumented every time a record is queued
  int queue_add_count;
 
  // pointer to the memory for the event list.
  struct dtkRecord_event *list;
};

