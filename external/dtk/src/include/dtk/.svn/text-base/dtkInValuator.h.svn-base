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

/*! \class dtkInValuator dtkInValuator.h dtk.h
 * \brief  A dtkInValuator that is a valuator
 *
 * A class object to handle read and writing of an array of floats
 * into an event records, dtkRecord. The only errors that can occur
 * are do to constructing with an invalid dtkRecord and array over
 * runs.  So if you're careful you generally don't need to check for
 * errors in this class.
 *
 * example code snippet:
  \code
  // **** For polling valuators ****

  // app is a dtkManager
  dtkInValuator *valuator = static_cast<dtkInValuator *>
    (app.get("valuator", DTKINBUTTON_TYPE));
  printf("valuator=(%f %f %f)\n",
       valuator->read(0), valuator->read(1), valuator->read(2));


  // **** One of many ways to read valuators from a queue. ****

  // app is a dtkManager
  dtkRecord *record = app.record();
  dtkInValuator *valuator = static_cast<dtkInValuator *>
    (app.get("valuator", DTKINVALUATOR_TYPE));

  dtkRecord_event *event;
  while((event = record->getNextEvent(valuator)))
      printf("Valuators=(%f %f %f)\n",
	     valuator->read(0,event),
	     valuator->read(1,event),
	     valuator->read(2,event));
 \endcode

 */

class DTKAPI dtkInValuator : public dtkInput
{
public:

/*!
 * Construct a dtkInValuator.
 *
 * \param record a pointer to the dtkRecord to store the data entries
 * in.
 *
 * \param number the number of floats in an event
 *
 * \param name the name of this dtkAugment object.
 */
  dtkInValuator(dtkRecord *record, int number, const char *name);

  /*!
   * Destructor
   */
  virtual ~dtkInValuator(void);

  /*!
   * Polling or dequeuing read.
   *
   * \param x must be an array of floats large enough to get all the
   * values.
   *
   * \param rec If \e rec is NULL this will read the last values in
   * the record queue, else if \e rec is non-NULL this will read from
   * the queue in \e rec.
   *
   * \return 0 on success or -1 on error.
   */
  virtual int read(float *x, const struct dtkRecord_event *rec = NULL);

  /*!
   * This gets the data with coping it to another buffer. So it's much
   * faster than the other reads, but we're only talk'n one memcpy
   * here.
   *
   * \param rec If \e rec is NULL this will read the last values in
   * the record queue, else if \e rec is non-NULL this will read from
   * the queue in \e rec.
   *
   * \return an array of floats that is the data for the valuators or
   * NULL on error.
   */
  virtual float *read(const struct dtkRecord_event *rec = NULL);

  /*!
   * This returns a given value.
   *
   * \param index is which particular float in the valuator you'd like
   * to read. \e index starts counting at 0.
   *
   * \param rec If \e rec is NULL this will read the last values in
   * the record queue, else if \e rec is non-NULL this will read from
   * the queue in \e rec.
   *
   * \return the float value in the array at index \e index or zero on
   * error.
   */
  virtual float read(int index, const struct dtkRecord_event *rec = NULL);

  /*!
   * \return the number of floats in this valuator.
   */
  int number(void) const;

  /*!
   * Write an entry into the current event in the record.  If this
   * device is queued this current event will be copied the event
   * write pointer will be moved to the next event in the record.
   *
   * \param x is the array of floats to write to the valuator entry.
   *
   * \return 0 on success or -1 on error.
   */
  virtual int write(const float *x);

private:

  int _number;
};
