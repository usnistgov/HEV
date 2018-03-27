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

/*! \class dtkReadBuffer dtkReadBuffer.h dtk.h
 * \brief Calls your callback parser using a circular buffer
 *
 * read(2) wrapper that calls your callback parser function using a
 * circular buffer.  The callback parser function must return the
 * size, in bytes, of the data that was parsed, and zero if nothing
 * was found.
 */
class dtkReadBuffer : public dtkBase
{
 public:

  /*!
   * Make the read buffer 
   */
  dtkReadBuffer(int fileDescriptor, size_t minRecordSize, size_t bufferSize,
		size_t (*parseFunc)(unsigned char *buffer, size_t size));
  /*!
   * Destructor
   */
  virtual ~dtkReadBuffer(void);

  //! Read from the buffer
  /*!
   * Return information from the buffer (user circular reading)
   */
  int read(void);

 private:

  unsigned char *buffer; // read buffer
  // ptr points to the unprocessed data
  // end points to next place to read to in the read buffer
  unsigned char *ptr, *end;
  size_t bufferSize, minRecordSize;
  int fd; // file describer

  // callback is called any time there's data that is at least at big
  // as minRecordSize
  size_t (*callback)(unsigned char *buffer, size_t size);
};
