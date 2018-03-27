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

/* Define the color "off" switch */
#define DTKCOLOR_OFF  0
/* Define the color "on" switch */
#define DTKCOLOR_ON   1
/* Define the "auto color" switch */
#define DTKCOLOR_AUTO 2

/* Define a default color switch */
#ifdef DTK_ARCH_WIN32_VCPP
// There is no "real" Windoz terminal. Windoz sucks.
#  define DEFAULT_DTKCOLOR_SWITCH DTKCOLOR_OFF 
#else
#  define DEFAULT_DTKCOLOR_SWITCH DTKCOLOR_AUTO
#endif

/*! \class dtkColor dtkColor.h dtk.h
 * \brief Escape color codes
 *
 * This class provides escape color strings that most UNIX like
 * terminals interpret by displaying colorized text.  The environment
 * varable DTK_COLOR can be set to "on", "off", and "auto" to cause
 * the escape color strings to be set or not.  If DTK_COLOR=on the
 * escape color strings will be set, else if DTK_COLOR=off the escape
 * color strings will all be set to NULL, else if DTK_COLOR=auto the
 * strings will be set the assocated terminal of the process is a tty
 * terminal (like with an xterm).  The default case is auto.
 *
 * The follow example shows how to use dtkColor:
 * \include examples/dtk/examplesByClass/dtkColor.cpp
 */
class DTKAPI dtkColor
{
 public:

  /*!
   * \param file \e file is the output file stream that will be
   * checked to see if the color strings are turned on or not. 
   *
   * \param off_on If \e off_on is not DTKCOLOR_OFF the default state
   * of the color string will be non-NULL.  The environment varable
   * DTK_COLOR value will override this default, and \e off_on. Usable
   * values for \e off_on are DTKCOLOR_OFF, DTKCOLOR_ON, and
   * DTKCOLOR_AUTO.  If the value of off_on is DTKCOLOR_AUTO than if
   * \e file is a pointer to a file that is a tty terminal than the
   * color will be turned on, else it will be turned off.  Usable
   * values for the environment varable DTK_COLOR are on, off and
   * auto.
   */
  dtkColor(FILE *file=NULL, int off_on=DEFAULT_DTKCOLOR_SWITCH);

  
  /*!
   * destructor.
   */
  virtual ~dtkColor(void);

  /*!
   * Change the values given in the constructor.
   *
   *\param file \e file is the output file stream that will be
   * checked to see if the color strings are turned on or not. 
   *
   * \param off_on If \e off_on is not DTKCOLOR_OFF the default state
   * of the color string will be non-NULL.  The environment varable
   * DTK_COLOR value will override this default, and \e off_on. Usable
   * values for \e off_on are DTKCOLOR_OFF, DTKCOLOR_ON, and
   * DTKCOLOR_AUTO.  If the value of off_on is DTKCOLOR_AUTO than if
   * \e file is a pointer to a file that is a tty terminal than the
   * color will be turned on, else it will be turned off.  Usable
   * values for the environment varable DTK_COLOR are on, off and
   * auto.
   */
  void reset(FILE *file=NULL, int off_on=DEFAULT_DTKCOLOR_SWITCH);

  /*!
   * \return 1 if the color strings are set, or 0 otherwise
   */
  int isOn(void);

  /*!
   * Color escape strings that made be used to colorize text.
   */
  char
    end[6], /*!< end of color */
    bld[6], /*!< bold */
    und[6], /*!< underline */
    rev[6], /*!< reverse video */
    red[6], /*!< red */
    grn[6], /*!< green */
    yel[6], /*!< yellow */
    blu[6], /*!< blue */
    vil[6], /*!< violet */
    tur[6], /*!< turquoise */
    rred[6], /*!< reverse red */
    rgrn[6], /*!< reverse green */
    ryel[6], /*!< reverse yellow */
    rblu[6], /*!< reverse blue */
    rvil[6], /*!< reverse violet */
    rtur[6]; /*!< reverse turquoise */

 private:

  void make_color_chars(FILE *file, int off_on);
};
