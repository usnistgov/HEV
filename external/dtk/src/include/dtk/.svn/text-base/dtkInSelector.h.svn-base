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

/*! \class dtkInSelector dtkInSelector.h dtk.h
 * \brief A dtkInValuator that is a selector
 *
 *  A class object to handle read and writing of an array of int32_t
 * (32 bit ints) into an event records, dtkRecord.  This the input to
 * the record for this object is not queue by default. Call
 * dtkInput::queue() to queue it.
 *
 * The only errors that can occur are do to constructing with an
 * invalid dtkRecord and array over run.  So if you're careful you
 * generally don't need to check for errors in this class.
 *
 * example code snippet:
  \code
  // **** For polling selectors ****

  // app is a dtkManager
  dtkInSelector *selector = static_cast<dtkInSelector *>
    (app.get("selector", DTKINBUTTON_TYPE));
  printf("selector=(%f %f %f)\n",
       selector->read(0), selector->read(1), selector->read(2));


  // **** One of many ways to read selectors from a queue. ****

  // app is a dtkManager
  dtkRecord *record = app.record();
  dtkInSelector *selector = static_cast<dtkInSelector *>
    (app.get("selector", DTKINSELECTOR_TYPE));

  dtkRecord_event *event;
  while((event = record->getNextEvent(selector)))
      printf("Selectors=(%d %d %d)\n",
	     selector->read(0,event),
	     selector->read(1,event),
	     selector->read(2,event));
 \endcode
 */
class DTKAPI dtkInSelector : public dtkInput
{
public:

  /*!
   * Contruct a dtkInSelector object.
   *
   * \param record a pointer to the dtkRecord to store the data entries
   * in. A dtkRecord may be gotten from a dtkManager.
   *
   * \param number the number of selectors.
   *
   * \param name is the name of this dtkAugment object.
   */

  dtkInSelector(dtkRecord *record, int number, const char *name);

  /*!
   * Destructor
   */
  virtual ~dtkInSelector(void);

  ///! Return state of all selectors
  /*!
   * Return the state of all the selectors.
   *
   * \param x is an array to receive the selector values.
   *
   * \param rec If \e rec is non-NULL this will read from the queued
   * event pointed to by \e rec. dtkRecord is used to get queued
   * events.
   *
   * \return o on success or -1 on error.
   */
  virtual int read(int32_t *x, const struct dtkRecord_event *rec = NULL);

  /*!
   * This gets the data with coping it to another buffer. So it's much
   * faster than the other reads, but we're only talk'n one memcpy
   * here.
   *
   * \param rec If \e rec is NULL this will read the last values in
   * the record queue, else if \e rec is non-NULL this will read from
   * the queue in \e rec.
   *
   * \return an array of int32_t that is the data for the selectors or
   * NULL on error.
   */
  virtual int32_t *read(const struct dtkRecord_event *rec = NULL);

  //! Read the state of a given selector
  /*!
   * \param index is the selector number who's state is
   * requested. Selector indexes start at 0.
   *
   * \param rec If \e rec is non-NULL the selector state is
   * gotten for the event pointed to by \e rec, else if \e rec is
   * NULL the state of this selector in the last selector event
   * is returned.
   *
   * \return the selector state of selector \e index.
   */
  virtual int32_t read(int index, const struct dtkRecord_event *rec = NULL);

  /*!
   * \return the number of selectors
   */
  int number(void) const;

  /*!
   * Write an entry into the current event in the record.  If this
   * device is queued this current event will be copied the event
   * write pointer will be moved to the next event in the record.
   *
   * \param state Write the state, \e state, of all selectors into the
   * record.
   *
   * \return 0 on success and -1 on error
   */
  virtual int write(const int32_t *state);

private:

  int _number;
};
