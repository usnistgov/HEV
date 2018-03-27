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
/****** dtkInXKeyboard.h ******/
// This is a Keyboard reading class that will work
// with more than one X server and keyboard.

// The common case is that when there is one keyboard and one X
// server.

// Each string should corrispond to a different X server connection.
// example: nameString = ":0"  It's a good idea not to include the
// screen number, as in ":0.1".

/*! \class dtkInXKeyboard dtkInXKeyboard.h dtkX11.h
 *
 * \brief dtkInXKeyboard is a dtkInput to read a X windows keyboard
 *
 * dtkInXKeyboard lets you read and write the X windows keyboard
 * presses and releases from a dtkRecord.  It uses X keys codes to
 * refer to key press and key release events.
 *
 * In common use most applications will get a pointer to a
 * dtkInXKeyboard from a dtkManager.
 */

class dtkInXKeyboard : public dtkInput
{

public:

  /*! Contruct a dtkInXKeyboard object with a given dtkRecord, \e
   * record, a NULL terminated array of Display pointers, \e xdis,
   * a pointer to a dtkManager, \e manager, and a \e name.
   * 
   */
  dtkInXKeyboard(dtkRecord *record,
		 // xdis is NULL terminated array
		 Display **xdis,
		 dtkManager *manager = NULL,
		 const char *name = "xkeyboard");
  virtual ~dtkInXKeyboard(void);

  //! Read a X keyboard event
  /*!
   * Returns the X key code that was pressed or released in \e
   * keyCode_return in the given dtkRecord_event \e event, or the
   * current dtkRecord_event (polling). The returned value is 1 if the
   * key was pressed and 0 if the key was released. \e display_number
   * and \e winID can be used to get the X display and Window that the
   * X event came from.
   */
  // returns KeyCode, winID and display_number
  // returns 1 if key is pressed
  // returns 0 if key is released
  // Poll the from the current state if event = NULL.
  int read(KeyCode *keyCode_return,
	   const struct dtkRecord_event *event = NULL,
	   int *display_number = NULL, Window *winID = NULL);

  //! Read a X keyboard event
  /*!
   * Returns whether the key with X key code \e key was pressed, 1, or
   * released, 0, in the given dtkRecord_event \e event, or the
   * current dtkRecord_event (polling).  \e display_number and
   * \e winID can be used to get the X display and Window that the X
   * event came from.
   */
  // returns winID and display_number to the pointer arguments
  // returns 1 if key is pressed
  // returns 0 if key is released
  // Poll the from the current state if event = NULL.
  // If display_number is non-NULL and *display_number is a valid value
  // than the key will be queried from the display that corresponds to
  // *display_number, else if display_number is nonNULL it will be set
  // to the display number from the event.
  int read(KeyCode key, const struct dtkRecord_event *event = NULL,
	   Window *winID = NULL, int *display_number = NULL);

  /*!
   * Get a X key code from a given key sym.  It's just a wrapper on an
   * X thing.
   */
  KeyCode getKeyCode(KeySym sym, int display_number = 0);

  /*!
   * Returns the number of X display connections used by the object.
   * This is used for systems with more than one keyboard.
   */
  int getNumberOfDisplays(void);

  /*! By calling this with a valid dtk object you will cause the
   * pressing of escape key to set DTK_ISRUNNING in dtkManager::state.
   * Call this with \e manager = NULL to unset this behavior.
   */
  void escapeUnsetsISRUNNING(dtkManager *manager = NULL);

  /*!
   * Put key events into the dtkRecord. This is not used in most
   * codes.
   */
  /******* write() is not for the dtk user *********/
  // write keys[32] from XQueryKeymap for the X11 library
  // and KeyCode from an XEvent.
  // pressed = 1 for XEvent key press.
  // pressed = 0 for XEvent key release.
  // keys points to the 32 byte key state for each X server.
  int write(const char *keys, KeyCode key, int pressed,
	    Window winID, int display_number = 0);

private:

  dtkManager *manager;

  void clean_up(void);
  size_t getEventSize(Display **dis);

  size_t event_size;
  unsigned char *write_buffer;

  int numDisplays; // May likely be the same as the number of keyboards.

  KeyCode *xkc_esc;

  Display **xdisplay;

  // offsets to data in the event buffer in the record buffer
  size_t KeyCodeOffset, pressedOffset, winIDOffset, display_numberOffset;
};




  
