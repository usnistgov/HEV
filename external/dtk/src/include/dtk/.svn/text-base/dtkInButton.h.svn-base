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

/*! \class dtkInButton dtkInButton.h dtk.h
 * \brief A class used to represent 32 buttons or less.
 *
 * Each button state is stored in a bit in a u_int32_t (unsigned 32
 * bit int).
 *
 * The only errors that can occur are do to constructing with an
 * invalid dtkRecord and array over run.  So if your careful you
 * generally don't need to check for errors in this class.
 *
 * example code snippet:

 \code
  // **** For polling buttons ****

  // app is a dtkManager
  dtkInButton *button = static_cast<dtkInButton *>
    (app.get("buttons", DTKINBUTTON_TYPE));
  printf("Buttons=(%d %d %d)\n",
       button->read(0), button->read(1), button->read(2));



  // **** One of many ways to read buttons from a queue. ****

  // app is a dtkManager
  dtkRecord *record = app.record();
  dtkInButton *button = static_cast<dtkInButton *>
    (app.get("buttons", DTKINBUTTON_TYPE));

  dtkRecord_event *event;
  while((event = record->getNextEvent(button)))
      printf("Buttons=(%d %d %d)\n",
	     button->read(0,event),
	     button->read(1,event),
	     button->read(2,event));
 \endcode

 */
class DTKAPI dtkInButton : public dtkInput
{
public:

  /*!
   * Contruct a dtkInButton object.
   *
   * \param record a pointer to the dtkRecord to store the data entries
   * in. A dtkRecord may be gotten from a dtkManager.
   *
   * \param number the number of buttons. \e number must be less than or
   * equal to 32.
   *
   * \param name is the name of this dtkAugment object.
   */
  dtkInButton(dtkRecord *record, int number, const char *name);

  /*!
   * Destructor
   */
  virtual ~dtkInButton(void);

  //! Return state of all buttons
  /*!
   * Return the state of all the buttons in a u_int32_t (unsigned 32 bit int).
   *
   * \param rec If \e rec is non-NULL this will read from the queued
   * event pointed to by \e rec. dtkRecord is used to get queued
   * events.
   *
   * \return returns the state of all buttons as the state of
   * each bit in a u_int32_t (unsigned 32 bit int).
   */
  virtual u_int32_t read(const struct dtkRecord_event *rec = NULL);

  //! Return the state of one button
  /*!
   * \param button_num If \e button_num is non-NULL the value pointed
   * to by \e button_num will be set to the number of the button
   * effected.  Button numbers start at 0.
   *
   * \param rec If \e rec is non-NULL the button state and button is
   * gotten for the event in pointed to by \e rec, else if \e rec is
   * NULL the state the the button effected by the last button event
   * is returned.
   *
   * \return Returns a 0 or a 1 that indicates the state of a button.
   * There can only be one button effected for a given button event.
   */
  virtual int read(int *button_num, 
		   const struct dtkRecord_event *rec = NULL);

  //! Read the state of a given button
  /*!
   * \param button_num is the button number who's state is
   * requested. Button numbers start at 0.
   *
   * \param rec If \e rec is non-NULL the button state is
   * gotten for the event pointed to by \e rec, else if \e rec is
   * NULL the state of this button in the last button event
   * is returned.
   *
   * \return the button state of button \e button_num.
   */
  virtual int read(int button_num,
		   const struct dtkRecord_event *rec = NULL);

  /*! Get the number of buttons
   *
   * \return the number of buttons in used in this object.
   */
  inline int number(void) const { return _number; }

  //! Write a button event
  /*! 
   * Most users should not call write().  We write one button at a
   * time so that button events are ordered and can be queued. The
   * state of the button will be set for this event and all events
   * that follow that don't set this button.
   *
   * \param button_num is the the button number to write.
   *
   * \param val is the value to write to that button.  If \e val is
   * nonzero the bit for the button is set , else if \e val is zero
   * the bit for the button is unset.
   *
   * \return 0 on success and -1 on error
   */
  virtual int write(int button_num, int val);

  // Write the state of all buttons
  /*
   * This should only be called to initialize the objects records
   * or if you know that the object is not being queued.
   *
   * \param state Write the state, \e state, of all buttons into the
   * record.  This should only be used if you know that the buttons
   * will not be queued, or you are initializing the state of all
   * buttons. Whether the object is queued is generally up to the
   * reader of the buttons. It's a good idea in general not to use
   * this method except for initializing the state of all buttons.
   *
   * \return 0 on success and -1 on error
   */
  virtual int write(u_int32_t state);

private:

  int _number;
};

