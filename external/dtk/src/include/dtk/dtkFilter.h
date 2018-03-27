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

/*! \class dtkFilter dtkFilter.h dtk.h
 * \brief Provides dynamic read and/or write filter plug-ins for DTK shared memory
 *
 * dtkFilter is a base class object for codes that can be loaded from
 * dynamic shared objects (DSO) files interactively for any given
 * process without coding it into the program.  You can tell running
 * programs to load and unload these read and write filters
 * (callbacks) using method in the dtkSharedMem class.  One or more
 * dtkFilter filter can be loaded for a given DTK shared memory file,
 * process and dtkSharedMem object.  By calling
 * dtkSharedMem::stopReadFilter(), and dtkSharedMem::stopWriteFilter()
 * a particular dtkSharedMem object may stop the corresponding
 * dtkFilters from being injected from outside the programs code, but
 * dtkFilters may still be loaded in the programs code where that
 * dtkSharedMem object is instanced.  When multipule dtkFilter objects
 * or loaded the read(), and write() methods are called in the order
 * in which they where loaded.
 *
 * An inherting class should call dtkBase method validate() in it's
 * constructor if it succeeds in make a usable object.  Not calling
 * dtkBase method validate() will cause a failure and the filter will
 * be unloaded.
 */

class DTKAPI dtkFilter : public dtkBase
{
 public:

  /*!
   * read() and write() return values. These returns values instruct the
   * calling dtkSharedMem object which loaded a corresponding dtkFilter DSO
   * to take the following actions.
   */

  enum RETURN_VALUE
    {
      CONTINUE = 0,/*!< Continue calling this callback. */
      REMOVE,      /*!< Discontinue calling this callback. */
      ERROR_        /*!< Discontinue calling this callback and return an error. */
    };

  /*!
   * \param size dtkSharedMem will pass the size of the shared memory
   * in \e size in the dtkSharedMem::loadFilter() methods.
   */
  dtkFilter(size_t size);

  /*!
   * Destructor
   */
  virtual ~dtkFilter(void);

  /*!
   * getRequiredSize() is used by the loading dtkSharedMem objects
   * to see if the shared memory is the correct size for the filter
   * to work properly.
   */
  inline size_t getRequiredSize(void) { return _requiredSize; }

  /*!
   * needTimeStamp() is used by the loading dtkSharedMem objects to
   * see if the shared memory needs to be time-stamped for the filter
   * to work properly.
   */
  inline int needTimeStamp(void) { return _needTimeStamp; }

  /*!
   * needWriteCount() is used by the loading dtkSharedMem objects to
   * see if the shared memory needs to be write counted for the filter
   * to work properly.
   */
  inline int needWriteCount(void) { return _needCount; }

  /*!
   * needQueued() is used by the loading dtkSharedMem objects to see
   * if the shared memory needs to be queued for the filter to work
   * properly.
   */
  inline int needQueued(void) { return _needQueued; }

  /*!
   * needByteSwapped() is used by the loading dtkSharedMem objects to
   * see if the filter is to be automatically byte swapped into the
   * local machine byte order.
   *
   * \return Returns the array element size if the filter is to be
   * automatically byte swapped, else it returns 0.
   */
  inline size_t needByteSwapped(void) { return _byteSwapElementSize; }

  //! Read filter callback
  /*!
   * A class overwriting this method should preserve the meaning of the parameters.
   * read() is called by the reading dtkSharedMem object each time the
   * dtkSharedMem::read() methods are called.
   *
   * \param buf is a pointer to the users read buffer that contains
   * the a pointer to the unfiltered data and can be overwritten with
   * the filtered data.
   *
   * \param nbytes is the size of the buffer \e buf
   * in bytes. The lesser of \e nbytes bytes or the user size of the
   * DTK shared memory file will be written to the buffer \e buf.
   *
   * \param offset is the offset in bytes to the point from the
   * begining of the user data in shared memory to read from.
   *
   * \param diffByteOrder diffByteOrder will be set to non-zero if
   * the data in \e buf is of a different byte order as the current
   * computer.
   *
   * \param timeStamp
   * If needTimeStamp non-zero then timeStamp will be
   * called with the data time-stamp and the filter may change the
   * value returned.  If needTimeStamp() returns 0 then \e timeStamp should
   * be ignored.
   *
   * \param count If needCount() returns non-zero then count
   * will be called with the data count.  If needCount() returns 0
   * then it should be ignored.
   * \return
   * 
   * Returns dtkFilter::CONTINUE on success or dtkFilter::REMOVE if
   * this method is not over-written by the inherting filter
   * class. dtkFilter::REMOVE is returned by this base class method.
   * Passing invalid parameters to the filter can cause the filter to
   * return dtkFilter::REMOVE or dtkFilter::ERROR in order to cause
   * removal of the filter callback.
   */
  virtual int
    read(void *buf, size_t nbytes, size_t offset, int diffByteOrder,
	 struct timeval *timeStamp, u_int64_t count);

  //! Write filter callback
  /*!
   * A class overwriting this method should preserve the meaning of the
   * parameters.
   *
   * \param buf is a pointer to a copy of the users write buffer that contains
   * the a pointer to the unfiltered data and can be overwritten with
   * the filtered data.
   *
   * \param nbytes is the size of the buffer \e buf
   * in bytes. The lesser of \e nbytes bytes or the user size of the
   * DTK shared memory file will be written to the buffer \e buf.
   *
   * \param offset is the offset in bytes to the point from the
   * begining of the user data in shared memory to read from.
   *
   * \param timeStamp
   * If needTimeStamp non-zero then timeStamp will be
   * called with the data time-stamp and the filter may change the
   * value returned.  If needTimeStamp() returns 0 then \e timeStamp should
   * be ignored.
   *
   * \return Returns dtkFilter::CONTINUE on success or
   * dtkFilter::REMOVE if this method is not over-written by the
   * inherting filter class. dtkFilter::REMOVE is returned by this
   * base class method.  Passing invalid parameters to the filter can
   * cause the filter to return dtkFilter::REMOVE or dtkFilter::ERROR
   * in order to cause removal of the filter callback.
   */
  virtual int
    write(void *buf, size_t nbytes, size_t offset, struct timeval *timeStamp);


 protected:

  //! Set the required shared memory user size for this filter
  /*!
   * If a inheriting filter requires a particular shared memory user
   * size this method should be called by the inheriting filter
   * constructor.  If setRequiredSize() is called and the shared
   * memory user size is not the same as the requested size, then the
   * filter will fail to used by a dtkSharedMem object.
   */
  inline void setRequiredSize(size_t size) { _requiredSize = size; }

  /*!
   * Request that the dtkSharedMem object that uses an object built from
   * this class send time-stamps when calling the read() and write().
   *
   * \param t If \e t is 0 this will request that the dtkSharedMem
   * object that uses an object built from this class not send
   * time-stamps when calling the read() and write(). This is the
   * default.
   */
  inline void requestTimeStamp(int t=1) { _needTimeStamp = t; }

  /*!
   * Request that the dtkSharedMem object that uses an object built from
   * this class send write counts when calling the read() and write().
   *
   * \param t If \e t is 0 this will request that the dtkSharedMem
   * object that uses an object built from this class not send
   * write counts when calling the read() and write(). This is the
   * default.
   */
  inline void requestWriteCount(int t=1) { _needCount = t; }

  /*!
   * Request that the dtkSharedMem object that uses an object built
   * from this be queued.  This will cause the dtkSharedMem::read()
   * methods to call dtkFilter::read() for each for each data entry in
   * the queue.  This should only be used for read filters.
   *
   * \param t If \e t is 0 this will request that the dtkSharedMem
   * object that uses an object built from this class to just used
   * polled data. This is the default.
   */
  inline void requestQueued(int t=1) { _needQueued = t; }

  /*!
   * Request that the dtkSharedMem object that uses an object built
   * pass the data in the write() and read() methods that is in the
   * machines byte order.  This can only be used if the data is a
   * array of fixed sized elements.
   *
   * \param size \e size is the size of an array element.
   */
  inline void requestAutoByteSwap(size_t size) { _byteSwapElementSize = size; }

  //! Lets the filter get the size of the users shared memory.
  /*
   * \return Returns the size that was pasted into the constructor,
   * which should be the size of the users shared memory.
   */
  inline size_t getSize(void) { return _shmSize; }

 private:

  int _needTimeStamp;

  int _needCount;

  int _needQueued;

  // shared memory user size
  size_t _shmSize;

  // size needed by filter
  size_t _requiredSize;

  size_t _byteSwapElementSize;
};
