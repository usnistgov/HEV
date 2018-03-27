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

// The environment variable DTK_ROOT will be checked in the
// dtkConfigure constructor, and not after.


//#ifdef DTK_ARCH_WIN32_VCPP
//# define DTK_DEFAULT_ROOT_DIR  "C:\\Program Files\\DTK"
//#else /* #ifdef DTK_ARCH_WIN32_VCPP */
# ifndef DTKAPI
#   define DTKAPI
# endif
//#endif



/*! \class dtkConfigure _config.h dtk.h
 * \brief A small class to query configuration strings for a given DIVERSE Toolkit (DTK) installation.
 *
 * The values in the configuration strings depend on the environment
 * variable DTK_ROOT. If the environment variable DTK_ROOT is not set
 * than a default value is used.  This default is derived at the time
 * DTK is built from source.
 *
 * This class is a solution to making it so that DTK can configure
 * itself on the fly.  With this the DTK package may be built and then
 * installed in a different ROOT directory (PREFIX) from the one it
 * was built for by setting environment variable DTK_ROOT.
 */


class DTKAPI dtkConfigure
{
public:

  // This bintype stuff is due to IRIX having more than one binary
  // type.  Like for example IRIX N32 and IRIX 64.

  /*!
   * This is used by getString() to specify a string to query.
   */
  enum DTK_STRING 
    {
      // don't change ROOT_DIR from 0
      ROOT_DIR =         0,/*!< Get the top level DTK installation directory. */
      LINK_LIBS =        1,/*!< Get the compiler link options. */
      LINK_LIBSX =       2,/*!< Get the compiler link options with libdtkX11. */
      INCLUDE_FLAGS =    3,/*!< Get the compiler include options. */
      CFLAGS =           4,/*!< Degenerated. Use CXXFLAGS. */
      CXXFLAGS =         4,/*!< Get the general C++ compiler options. */
      AUGMENT_DSO_DIR =  5,/*!< Get the directory where dtkAugment DSOs are installed. */
      SERVICE_DSO_DIR =  6,/*!< Get the directory where dtkService DSOs are installed. */
      FILTER_DSO_DIR =   7,/*!< Get the directory where dtkFilter DSOs are installed. */
      LIB_DIR =          8,/*!< Get the directory where DTK libraries are installed. */
      SERVER =           9,/*!< Get the DTK server executable path. */
      NUM_STRINGS =     10 // this should be the last one in the enum.
    };

  /*! Constructor
   */
  dtkConfigure(void);

  
  /*! Destructor
   */
  virtual ~dtkConfigure(void);

  /*!
   * Get a configuration character string.
   *
   * \param string Which string to get.
   *
   * \return Returns a pointer to a string. The memory that this points to
   * should not be written to by the user.
   */
  char *getString(DTK_STRING string);

private:

  char *string[NUM_STRINGS];

  // flag to show that environment variable DTK_ROOT was set.
  int gotRootEnv;
};

/*! \var dtkConfig
 * A globel dtkConfigure object.
 */
extern DTKAPI dtkConfigure dtkConfig;
