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


/***************************************
****** message severity level **********
****************************************/
/*! please don't add to severity levels */
//!  DTKMSG_NONE is for DTK or DIVERSE users to shut off stuff
#define DTKMSG_NONE         0200  //! do not use in dtkMessage::add()

#define DTKMSG_FATAL        0100
#define DTKMSG_ERROR         040
#define DTKMSG_WARN          020
#define DTKMSG_WARNING      DTKMSG_WARN
#define DTKMSG_NOTICE        010
#define DTKMSG_INFO           04
#define DTKMSG_DEBUG          02

#define DTKMSG_OFF            01


#define DTKMSG_DEFAULT_LEVEL DTKMSG_NOTICE

// A Windows bug will not let this be stdout,
// so we set it to stdout in message.cpp.
#define DTKMSG_DEFAULT_FILE  NULL /* set to stdout in messager.cpp */

#define PRESTR_SIZE          128


/*! \class dtkMessage dtkMessage.h dtk.h
 * \brief  Message class for dtk
 *
 * This is a dtk message class that allows shifting , premessages
 * reset, appending, and escape sequence colors. This is mostly
 * intended to spew when an error occurs of when debuging. The idea of
 * this is taken from SGIs' OpenGL Performer pfNotify class.
 *
 * There is an globel instance of a dtkMessage in the DTK library
 * called dtkMsg which is used by most classes in the DTK library.
 * You can control the spew level when using the DTK library with the
 * environment variable DTK_SPEW which may be: fatal, error, warn,
 * notice, info, debug, or off.  So for maximum spew set environment
 * variable DTK_SPEW=debug.
 */
class DTKAPI dtkMessage
{
 public:

  /*!
   * Create a message object with a specified severity, file, and color on/off-nis.
   *
   * \param print_on_severity Tells then to print. \e print_on_severity
   * may be DTKMSG_FATAL, DTKMSG_ERROR, DTKMSG_WARN, DTKMSG_NOTICE,
   * DTKMSG_INFO, DTKMSG_DEBUG, or DTKMSG_OFF. This may be over-ridden
   * by the value of environment variable DTK_SPEW which may be set to
   * fatal, error, warn, notice, info, debug, or off.
   *
   * \param file is the file stream to print to.
   *
   * \param is_color may be set to DTKCOLOR_OFF, DTKCOLOR_ON, and
   * DTKCOLOR_AUTO.  The effect of this may be over-ridden by the
   * environment variable DTK_COLOR which may be set to off, on, and
   * auto.
   */
  dtkMessage(int print_on_severity=DTKMSG_DEFAULT_LEVEL,
	     FILE *file=DTKMSG_DEFAULT_FILE,
	     int is_color=DEFAULT_DTKCOLOR_SWITCH);

  /*! Destructor */
  virtual ~dtkMessage(void);


  /*!
   * Reset the specified severity, file, and color.
   *
   * \param print_on_severity Tells then to print. \e print_on_severity
   * may be DTKMSG_FATAL, DTKMSG_ERROR, DTKMSG_WARN, DTKMSG_NOTICE,
   * DTKMSG_INFO, DTKMSG_DEBUG, or DTKMSG_OFF. This may be over-ridden
   * by the value of environment variable DTK_SPEW which may be set to
   * fatal, error, warn, notice, info, debug, or off.
   *
   * \param file is the file stream to print to.
   *
   * \param is_color may be set to DTKCOLOR_OFF, DTKCOLOR_ON, and
   * DTKCOLOR_AUTO.  The effect of this may be over-ridden by the
   * environment variable DTK_COLOR which may be set to off, on, and
   * auto.
   */
  void reset(int print_on_severity=DTKMSG_DEFAULT_LEVEL,
	     FILE *file=DTKMSG_DEFAULT_FILE,
	     int is_color=DEFAULT_DTKCOLOR_SWITCH);

  /*!
   * Set the contents of the pre-message text.
   *
   * \param format is a format as in printf(3).
   */
  void setPreMessage(const char *format, ...);

  /*!
   * \param count Add \e count spaces to the current number of spaces
   * that is printed before each message.
   */
  void shift(int count = 2);

  /*!
   * \param count Remove \e count spaces to the current number of spaces
   * that is printed before each message.
   */
  void unshift(int count = 2);

  /*!
   * Set the current number of spaces that is printed before each
   * message to be zero.
   */
  void resetShift(void);

  //! off() will turn off spew no matter what.
  /*!
   * off() and on() methods are helpful if your using failure of a DTK
   * class method as normal mode of operation and you would like it
   * not to spew.  Otherwise use the environment DTK_SPEW=off to turn
   * off spew as a application user.
   *
   */
  void off(void);

  /*! on() will turn spew back it was like before off() was called.
   */
  void on(void);

  /*! Add a message. The message will print depending on the value of
   * \e severity compared to the current spew level.
   *
   * \param severity may be DTKMSG_FATAL, DTKMSG_ERROR, DTKMSG_WARN,
   * DTKMSG_NOTICE, DTKMSG_INFO, DTKMSG_DEBUG.  If \e severity is
   * greater than or equal to the current spew level this will print.
   *
   * \param is_sys_err If \e is_sys_err is non-zero the current system
   * error number and string will be prepended to the printed output.
   *
   * \param returnVal is returned for this call.
   *
   * \param format is a format as in printf(3).
   *
   * \return \e returnVal
   */
  int   add(int severity, int is_sys_err,
	    int returnVal, const char *format, ...);

  /*! Add a message. The message will print depending on the value of
   * \e severity compared to the current spew level.
   *
   * \param severity may be DTKMSG_FATAL, DTKMSG_ERROR, DTKMSG_WARN,
   * DTKMSG_NOTICE, DTKMSG_INFO, DTKMSG_DEBUG.  If \e severity is
   * greater than or equal to the current spew level this will print.
   *
   * \param is_sys_err If \e is_sys_err is non-zero the current system
   * error number and string will be prepended to the printed output.
   *
   * \param format is a format as in printf(3).
   */
  void  add(int severity, int is_sys_err, const char *format, ...);


  /*! Add a message. The message will print depending on the value of
   * \e severity compared to the current spew level.
   *
   * \param severity may be DTKMSG_FATAL, DTKMSG_ERROR, DTKMSG_WARN,
   * DTKMSG_NOTICE, DTKMSG_INFO, DTKMSG_DEBUG.  If \e severity is
   * greater than or equal to the current spew level this will print.
   *
   * \param format is a format as in printf(3).
   */
  void  add(int severity, const char *format, ...);

  /*!  Append to the last message.
   *
   * \param format is a format as in printf(3).
   */
  void  append(const char *format, ...);

  /*! Get the last system errno from the last call to add() value that
   * was non-zero.
   */
  int getSysErrno(void);

  /*! Return the file pointer that is printed to.
   */
  inline FILE *file(void){return _file;};

  /*! Set the file pointer that is printed to.
   */
  void setFile(FILE *file=stdout);

  /*!
   * \param severity level to check
   *
   * \return non-zero if the severity level is greater than or equal
   * to \e severity.  return 0 otherwise.
   */
  inline int isSeverity(int severity){return (print_on_severity<=severity);};

  /*!
   * A escape string color object to make pretty colorful terminal text.
   */
  dtkColor color;

private:

  void vadd(int severity, int is_sys_err, const char *format, va_list args);
  void setDefaultPreString(void);
  int shiftCount;
  int last_severity;
  int print_on_severity;
  char preMessage_str[PRESTR_SIZE];
  int sysErrno; // The last system errno value that was non-zero.
  FILE *_file;

  int _off;
};

// a globally declared DTK message object

extern DTKAPI dtkMessage dtkMsg;
