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

/*! \class dtkInLocator dtkInLocator.h dtk.h
 * \brief  A dtkInValuator with 6 float values
 *
 * dtkInLocator is a dtkInValuator with 6 float values. The 6 values
 * may be: x, y, z, heading, pitch, and roll.
 * 
 * The only errors that can occur are do to constructing with an
 * invalid dtkRecord and array over run.  So if your careful you
 * generally don't need to check for errors in this class.
 */
class DTKAPI dtkInLocator : public dtkInValuator
{
 public:

/*!
 * \param record a pointer to the dtkRecord to store the data entrees
 * in.
 *
 * \param name the name of this dtkAugment object.
 */
  dtkInLocator(dtkRecord *record, const char *name);

  /*!
   * Destructor
   */
  virtual ~dtkInLocator(void);

};
