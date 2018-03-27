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

/*! \class dtkBase dtkBase.h dtk.h
 *  \brief A common DTK base class.
 *
 *  dtkBase is the lowest level base class for many DTK classes. It
 *  just provides methods that allow the the user to avoid using C++
 *  exceptions, and dynamic_cast, which seem to be a good thing to
 *  avoid on some systems. It provides:
 *   - valid class object checking
 *   - class object typing
 *
 * Not all DTK classes inherit dtkBase as the name might imply.  The
 * code for this class is dumb-ass simple.
 *
 * Example code using class dtkBase:
 * \include examples/dtk/examplesByClass/dtkBase.cpp
 */

class DTKAPI dtkBase
{
public:

  /*!
   * The constructor may be used to set the DTK class type.  This
   * class object starts out in a invalid state.
   * 
   * \param type_number is the DTK class type number.  All DTK classes
   * that inherit dtkBase have a unique class type number which is
   * listed in types.h.
   */
  dtkBase(u_int32_t type_number=DTKBASE_TYPE);

  /*! destructor
   */
  virtual ~dtkBase(void);

  /*!
   * Check if the drived class object is invalid
   *
   * \return Returns non-zero if the drived class object is invalid,
   * or returns zero if the drived class object is valid.
   */
  int isInvalid(void) const;

  /*!
   * Check if the drived class object is valid
   *
   * \param type If \e type is not DTK_ANY_TYPE than this
   * will check if this is a valid class of type \e type.
   *
   * \return Returns non-zero if the drived class object is valid,
   * or returns zero if the drived class object is invalid.
   */
  int isValid(u_int32_t type=DTK_ANY_TYPE) const;

  /*!
   * \return the DTK class type number.
   */
  u_int32_t getType(void) const;

protected:

  /*!
   * \param type_number Sets the DTK class type number to \e
   * type_number. This will also invalidate the class object.  You may
   * call validate() to validate it after.
   */
  void setType(u_int32_t type_number);

  /*!
   * Set this class object to be invalid.
   */
  void invalidate(void);

  /*!
   * Set this class object to be valid.
   */
  void validate(void);

private:

  u_int32_t magic_number;
  u_int32_t type;
  // magic_number == type  if it's valid.
};
