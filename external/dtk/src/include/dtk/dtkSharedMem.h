/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2004  Lance Arsenault
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

/****** dtkSharedMem.h ******/
/****************************************************************************/
/************ Structure of a DTK shared memory file *************************/

/************** data in a dtkSharedMem file ****************************

  padding is to bring pointers to CHUNK_SIZE (currently 8 bytes in
  _private_sharedMem.h) boundaries. struct dtkSharedMem_header::flags
  tells about all the if() below.  The padding is required because we
  are managing memory so that the data structures may be accessed
  correctly in the processes virtual address space.  In some (many)
  cases the size of the padding is zero.


          item                       size
  --------------------        ---------------------------------

  struct dtkSharedMem_header     sizeof(struct dtkSharedMem_header)
  padding      

 *repeat the following dtkSharedMem_header::queue_length times {


  if(COUNTED)
    count                      sizeof(u_int64_t) = 8
    padding 

  if(TIMING)
    time stamp                 sizeof(struct timeval)
    padding 

  if(QUEUING)
    queue counter              sizeof(u_int64_t) = 8
    padding

  flags                         sizeof(u_int8_t) = 1
                                byte_order and other flags as needed
  padding


  user data                    size that user requested
  padding

 }

 * managed footer data that varies in size.
 * Remote write address lists are kept here.

*********************************************************************/

#define DTK_CONNECT   01
#define DTK_CREATE    02
#define DTK_GET       (DTK_CONNECT | DTK_CREATE)

#define DTKSHAREDMEM_NOSIZE    ((size_t) (-2))
#define DTKSHAREDMEM_FILE_MODE ((mode_t) 0666)
#define DTK_DEFAULT_QUEUELENGTH  (24)

#define DTK_UDP              0
#define DTK_TCP              1

#define DTK_DEFAULT_NETTYPE  DTK_TCP

#define DTK_ALL   ((size_t) -1)

#define DTK_CURRENT_PID      ((pid_t) -1)
#define DTK_CURRENT_OBJECT   ((pid_t) -2)
#define DTK_ALL_PID          ((pid_t) -3)

class dtkSharedMem;
typedef void (*dtkSharedMem_sigHandler_t)(dtkSharedMem *, void *);

#define DTKSHAREDMEM_CATCHER_ERROR ((dtkSharedMem_sigHandler_t) -1)


/* flags for int *dtkSharedMem_destroy(const char *file, int flags)
 */
#define DTK_RECURSE  01


struct dtkSharedMem_header
{
  u_int32_t magic_number;
  dtkRWLock rwlock; // copy of the dtkRWLock object data
  dtkConditional blocking_read_lock;

  u_int64_t write_index;
  // write counter for the circular queue write_index is incremented
  // for each event queued.  write_index is the index to the last
  // event written.

  u_int64_t counter; // write counter= number of times written and
		     // counted

  u_int32_t filter_change_count;
  // This is incremented if a r/w filter has been added or removed.
  u_int32_t filter_list_offset;
  // offset from top of file to first entries of the r/w filter list.

  u_int32_t address_change_count;
  // address_change_count is incremented every time the remote write
  // inet write list is changed.  Each dtkSharedMem object has a local
  // address_change_count in it and at the time of writing if
  // it differs from this value the dtkSharedMem object looks to see if
  // there are new remote addresses to write to or that are removed.
  u_int32_t write_list_offset; // offset from top of file to start of
  // write list.

  u_int32_t user_size;    // size of user data.
  u_int32_t file_size;    // 
  u_int16_t queue_length; // total number of events in queue

  u_int8_t flags; // nature of queuing and time stamped and/or counted
 // bit masks used are in _private_sharedMem.h.  flags must be checked
 // at every call of every method that assumes the shared memory
 // file is of a certain size and shape.
};


/*! \class dtkSharedMem dtkSharedMem.h dtk.h
 * \brief  Provides an interface to shared memory in DIVERSE Toolkit (DTK).
 *
 * It provides creation, reading, writing, queuing, time stamping and
 * related methods.  The environment variable DTK_SHAREDMEM_DIR may be
 * set to affect the default directory that the associated DTK shared
 * memory file is in where the full file path is not given in
 * constructing the shared memory.
 */

class DTKAPI dtkSharedMem : public dtkBase
{
 public:

  /*!
   * Construct a shared memory object the is associated with the DTK
   * shared memory file \e filename.  dtkBase::isValid() will be
   * non-zero if the created object is in a usable state and zero if
   * not.
   *
   * \param size the user size of the shared memory.
   *
   * \param filename may be a full or relative path file name of the
   * shared memory file.
   *
   * \param initBuff The data pointed to by \e initBuff will be use to
   * initialize the shared memory if the shared memory is being
   * constructed. The data pointed to by \e initBuff will be
   * overwritten with the current values in shared memory if the
   * shared memory already exists.
   *
   * \param flag set to DTK_GET to create or just connect to existing
   * shared memory, DTK_CREATE to create shared memory, and
   * DTK_CONNECT to just connect to existing shared memory.
   */
  dtkSharedMem(size_t size, const char *filename,
	       void *initBuff=NULL, int flag=DTK_GET);

  /*!
   * Construct a shared memory object the is associated with the DTK
   * shared memory file \e filename.  dtkBase::isValid() will be
   * non-zero if the created object is in a usable state and zero if
   * not.
   *
   * \param size the user size of the shared memory.
   *
   * \param filename may be a full or relative path file name of the
   * shared memory file.
   *
   * \param flag set to DTK_GET to create or just connect to existing
   * shared memory, DTK_CREATE to create shared memory, and
   * DTK_CONNECT to just connect to existing shared memory.
   */
  dtkSharedMem(size_t size, const char *filename, int flag);
  
  /*!
   * Construct a shared memory object the is associated with the
   * existing DTK shared memory file \e filename.  dtkBase::isValid()
   * will be non-zero if the created object is in a usable state and
   * zero if not.
   *
   * \param filename may be a full or relative path file name of the
   * shared memory file.
   *
   * \param do_spew If \e do_spew is non-zero than if the created
   * object is not in a usable state this will set message in the
   * dtkMsg, global dtkMessage object, else if \e do_spew zero if will
   * not set message due to the shared memory file not existing.
   */
  dtkSharedMem(const char *filename, int do_spew=1); 
  
  /*!
   * This destructor only destroys the instance of this class object.
   * It does not destroy the DTK shared memory associated with the
   * object.  Use the function dtkSharedMem_destroy() to destroy the
   * shared system resources associated with the DTK shared memory.
   */
  virtual ~dtkSharedMem(void);

  /*!
   * Get the user size of the shared memory segment.
   */
  size_t getSize(void) const;
  
  /*! 
   * Get the full path shared memory file.
   *
   * \return Returns a pointer to a string.  Do you mess with this
   * memory.
   */
  char *getName(void) const;

  /*!
   * Get the name to the shared memory, the name used in the
   * constructor, which may not be the full path file name.
   *
   * \return Returns a pointer to a string.  Do you mess with this
   * memory.
   */
  char *getShortName(void) const;
  
  /*!
   * Get the time-stamp from the last data entree that was read with
   * read() or qread().
   *
   * \param t t in a pointer to the users timeval struct that will be
   * overwritten.
   *
   * \return Returns non-zero on error and zero on success.
   */
  int getTimeStamp(struct timeval *t) const;

  /*!
   * Get the time-stamp from the last data entree that was read with
   * read() or qread().
   *
   * \return Returns the time-stamp in seconds.
   */
  long double getTimeStamp(void) const;

  /*!
   * Get the write counter of the shared memory.  The counter value is
   * the number of times the the shared memory is written to at the
   * time of the last read() or qread(), since the shared memory was
   * requested to be write counted.
   *
   * \return Returns the value of the counter since the shared memory
   * was requested to be counted. See writeCount().
   */
  u_int64_t getWriteCount(void) const;
  
  /*!
   * Request that the shared memory file be queued.  If the DTK shared
   * memory is queued the qread() method may be used, and past values
   * that have be written to shared memory may be read.
   *
   * \param queue_length Sets the queue length to \e queue_length if
   * it is larger that the current queue length.
   *
   * \return Returns non-zero on error, or zero on success.
   */
  int queue(int queue_length=DTK_DEFAULT_QUEUELENGTH);

  /*!
   * \return non-zero if the shared memory is queued, and zero if not.
   */
  int isQueued(void);

  /*!
   * \return non-zero if the shared memory is counted, and zero if
   * not. See writeCount().
   */
  int isCounted(void);

  /*!
   * \return non-zero if the shared memory is time stamped, and zero
   * if not.
   */
  int isTimeStamped(void);
  
  /*!
   * Request that the shared memory file keep a count of the number of
   * writes to it.  If the shared memory is queued, than keep a count
   * for each entry in the queue (queued shared memory).  The count is
   * stored in a 64 bit integer, so if we count at 10 Giga Hz the count
   * will not wrap back to zero for 58.5 years.
   *
   * \param truth If \e truth is zero than this will request that the
   * shared memory file stop keeping a write count, or else if \e
   * truth is non-zero it requests that the shared memory file keep a
   * count of the number of writes to it.
   *
   * \return Returns non-zero on error, or zero on success.
   *
   * When the shared memory is first set to being counted the current
   * count value is set to zero.  The first value that is counted will
   * have a count value of zero.  As an example consider shared memory
   * with an int in it.  We show the count value and how is changes as
   * it is written to:

  \verbatim
  action              count      value  count status    time(seconds)   
  ----------         --------    -----  ------------    -----------
  write 436          not there    436   not counted       0.0
  write 435          not there    435   not counted       0.1
  call writeCount()    0          435   not counted       0.3
  write 434            1          434   counted           0.5
  write 433            2          433   counted           0.6
  write 432            3          432   counted           0.7
  ...\endverbatim
   */
  int writeCount(int truth=1);

  /*!
   * Request that the shared memory file keep a time-stamp for writes
   * to it.  By default the time-stamp value is the current system
   * time.
   *
   * \param truth If \e truth is zero than this will request that the shared
   * memory file stop keeping a write count-stamp, or else if \e truth
   * is non-zero it requests that the shared memory file keep a
   * time-stamp.
   *
   * \return Returns non-zero on error, or zero on success.
   */
  int timeStamp(int truth=1);

  //! shared memory queue read
  /*!
   * Read the shared memory data from a queue.  Read filters will not
   * be called. After reading the queue is advanced to the next entree
   * in the queue.  This only affects the queue reading of this
   * dtkShareMem object.  If no new entrees are present, no value will
   * be read.
   *
   * \param buf A pointer to the users buffer that is over-written by
   * the current shared memory event data.
   *
   * \param nbyte The number of bytes to write to \e buf
   * from the shared memory.
   *
   * \param offset Read the data at \e offset bytes from the beginning
   * of the shared memory data.
   *
   * \return Returns the number of events that are in the queue before
   * the qread() call. So it returns 0 nothing was read.  Returns -1
   * on error.
   */
  int qRead(void *buf, size_t nbyte=DTK_ALL, size_t offset=0);
  
  
  //! shared memory queue read
  /*!
   * Read the shared memory data from a queue.  Read filters will not
   * be called. After reading the queue is NOT advanced to the next entree
   * in the queue.  This only affects the queue reading of this
   * dtkShareMem object.  If no new entrees are present, no value will
   * be read.
   *
   * \param buf A pointer to the users buffer that is over-written by
   * the current shared memory event data.
   *
   * \param nbyte The number of bytes to write to \e buf
   * from the shared memory.
   *
   * \param offset Read the data at \e offset bytes from the beginning
   * of the shared memory data.
   *
   * \return Returns the number of events that are in the queue before
   * the qread() call. So it returns 0 nothing was read.  Returns -1
   * on error.
   */
  int qPeek(void *buf, size_t nbyte=DTK_ALL, size_t offset=0);

  /*! For compatibility with old code. qread() is deprecated.
   * Use qRead() instead.
   */
  inline int qread(void *buf, size_t nbyte=DTK_ALL, size_t offset=0)
    {
      return qRead(buf, nbyte, offset);
    }

  //! blocking shared memory queue read
  /*!
   * Read the shared memory data from a queue.  Read filters will not
   * be called. After reading the queue is advanced to the next entree
   * in the queue.  This only affects the queue reading of this
   * dtkShareMem object.  This blocks (suspends the current process)
   * until a new entree can be read.  The different between a
   * blockingQRead() and blockRead() is that the shared memory data is
   * read from a queue for blockingQRead() and blockingRead() just
   * gets the last value that was written.
   *
   * \param buf A pointer to the users buffer that is over-written by
   * the current shared memory event data.
   *
   * \param nbyte The number of bytes to write to \e buf
   * from the shared memory.
   *
   * \param offset Read the data at \e offset bytes from the beginning
   * of the shared memory data.
   *
   * \return Returns the number of events that are in the queue before
   * the qread() call.  So it returns 0 nothing was read.  Returns -1
   * on error.
   *
   * \include dtk/tests/blockingRead/blockingQRead.cpp
   */
  int blockingQRead(void *buf, size_t nbyte=DTK_ALL, size_t offset=0);

  //! shared memory read
  /*!
   * Read the shared memory data from the last thing written. We call
   * this polling the current data as opposed to reading from a queue
   * as in qread().  If one or more reading dtkFilter objects are
   * loaded the read filter, dtkFilter::read() methods will be called
   * in the order that they have be loaded.
   *
   * \param buf A pointer to the users buffer that is over-written by
   * the current shared memory event data.
   *
   * \param nbytes The number of bytes to write to \e buf from the
   * shared memory.
   *
   * \param offset Read the data at \e offset bytes from the beginning
   * of the shared memory data.
   *
   * \return Returns -1 on error and zero on success.
   */
  int read(void *buf, size_t nbytes=DTK_ALL, size_t offset=0);

  //! blocking shared memory read
  /*!
   * Read the shared memory data from the next thing that will be
   * written.  If one or more reading dtkFilter objects are loaded the
   * read filter, dtkFilter::read() methods will be called in the
   * order that they have be loaded.  In order to determine when new
   * data is present in shared memory the shared memory will be write
   * counted.  See writeCount(). This will block (suspends the current
   * process) until a new data can be read.  This will not block if the
   * last write count read is not equal to the write count that is in
   * the current shared memory.
   *
   * \param buf A pointer to the users buffer that is over-written by
   * the current shared memory event data.
   *
   * \param nbytes The number of bytes to write to \e buf from the
   * shared memory.
   *
   * \param offset Read the data at \e offset bytes from the beginning
   * of the shared memory data.
   *
   * \return Returns -1 on error and zero on success.
   *
   * \include dtk/tests/blockingRead/blockingRead.cpp
   */
  int blockingRead(void *buf, size_t nbytes=DTK_ALL, size_t offset=0);

  //! write to shared memory
  /*!
   * Write into shared memory.  If there are network connections in
   * the DTK shared memory write list, this call will write to the
   * network, remote shared memory.  See
   * dtkClient::connectRemoteSharedMem() or the programs
   * dtk-connectRemoteSharedMem.
   *
   * \param ptr pointer to data to write to shared memory
   *
   * \param timeStamp pointer to the time-stamp that the
   * user is setting for this write operation
   *
   * \param nbytes number of bytes to write from \e ptr
   *
   * \param offset the offset in bytes from the start of the entree in
   * shared memory to write to
   *
   * \return return -1 on error and 0 on success
   */
  int write(const void *ptr, const struct timeval *timeStamp,
	    size_t nbytes=DTK_ALL, size_t offset=0);

  /*!
   * Write into shared memory.
   *
   * \param ptr pointer to data to write to shared memory
   *
   * \param timeStamp pointer to the time-stamp, in seconds, that the
   * user is setting for this write operation
   *
   * \param nbytes number of bytes to write from \e ptr
   *
   * \param offset the offset in bytes from the start of the entree in
   * shared memory to write to
   *
   * \return return -1 on error and 0 on success
   */
  int write(const void *ptr, long double timeStamp,
	    size_t nbytes=DTK_ALL, size_t offset=0);

  /*!
   * Write into shared memory.
   *
   * \param ptr pointer to data to write to shared memory
   *
   * \param nbytes number of bytes to write from \e buf
   *
   * \param offset the offset in bytes from the start of the entree in
   * shared memory to write to
   *
   * \return return -1 on error and 0 on success
   */
  int write(const void *ptr, size_t nbytes=DTK_ALL, size_t offset=0);
  
  /* -- used by the DTK server only --
   *
   * Write into shared memory without pushing the data across the
   * network.  This is used by the DTK server, dtk-server, to get data
   * from the network to the local shared memory.
   *
   * \return returns -1 on error and 0 on success
   */
  int writeLocal(const void *ptr, int net_byte_order);

  /*!
   * Acquire the write lock. This will block until no other process
   * holds the write or read lock for this shared memory file.
   *
   * \param timeStamp pointer to the time-stamp that the
   * user is setting for this write operation
   *
   * \return returns -1 on error and 0 on success
   */
  int wlock(const struct timeval *timeStamp=NULL);

  /*!
   * Release the write lock.
   *
   * \param isLocal set this to non-zero to cause this not to push the
   * data across the network. This should be set only by the DTK
   * server.
   *
   * \return returns -1 on error and 0 on success
   */
  int wunlock(int isLocal=0);

  /*!
   * Acquire the read lock. This will block until no other process
   * holds a write lock for this shared memory file.
   *
   * \return returns -1 on error and 0 on success
   */
  int rlock(void);

  /*!
   * Release the read lock.
   *
   * \return returns -1 on error and 0 on success
   */
  int runlock(void);
  
  /*!
   * Flush all entrees from the shared memory queue. This just flushes the
   * entrees for this dtkShareMem object.
   *
   * \return returns -1 on error and 0 on success
   */
  int flush(void);
  
  /*!
   * See if the byte order of the data last read, with read(),
   * blockingRead(), qRead(), or blockingQRead() is of a different
   * byte order than your current machines.
   *
   * \returns Returns non-zero if the last thing read was a different
   * byte order than this machine, else it returns zero.
   */
  int differentByteOrder(void) const;
  
  /* -- used by the DTK server only --
   * Add address addr from the write list in both shared memory, 
   * and in the local dtkSegAddrList object, addrList.
   *
   * \return returns -1 on error and 0 on success
   */
  int addToWriteList(const char *addr, const char *name=0);
  
  /* -- used by the DTK server only --
   * Remove address addr from the write list in both shared memory, 
   * and in the local dtkSegAddrList object, addrList.
   */
  int removeFromWriteList(const char *addr);
  
  /* -- used by the DTK server only --
   * DTK server function for returning the list of addresses.
   */
  dtkSegAddrList *getSegAddrList(void);
  
  /*!
   * Set the networking protocol used to push the shared memory across
   * the network.
   *
   * \param type use DTK_UDP for IP/UDP and DTK_TCP for IP/TCP
   *
   * \return returns -1 on error and 0 on success
   */
  int setNetworkType(int type=DTK_UDP);

  /*!
   * Query the network protocol being used.
   *
   * \return returns DTK_UDP for IP/UDP and DTK_TCP for IP/TCP
   */
  int getNetworkType(void);
  
  //! Set up automatic byte swapping
  /*!
   * Sets up this shared memory object to byte swap all elements in an
   * array as it is read from shared memory.  The users returned
   * buffer is byte swapped to the machines byte order, if the shared
   * memory came from a machine of a different byte order.  This can
   * only be done if the shared memory data that is read is being
   * interpreted as elements of an array, like for example an array of
   * floats.  To unset automatic byte swapping set \e element_size to
   * zero.
   *
   * \param element_size the size of array element that is to be byte
   * swapped if needed.
   */
  void setAutomaticByteSwapping(size_t element_size);

  //! Load a shared memory read/write filter from a DSO file
  /*!
   * Set this DTK shared memory file to use the dtkFilter (filter)
   * object read(), and/or write() methods found in the dynamic shared
   * object (DSO) file \e dso_file when calling the read() method in
   * the dtkSharedMem objects in the process with PID \e pid.
   *
   * \param dso_file the name of the DSO filter file to load.  The
   * environment variable DTK_FILTER_PATH and then the installed path
   * will be used if the full path file name is not given.
   *
   * \param argc Pass \e argc to the DSO loader function.
   *
   * \param argv Pass \e argv to the DSO loader function.
   *
   * \param pid
   * Load the filter into process with PID \e pid.  Setting \e pid to
   * the macro DTK_CURRENT_PID will cause the filter to get loaded
   * into just the dtkSharedMem objects that are in the current
   * process. Setting \e pid to DTK_CURRENT_OBJECT will cause the
   * filter to get loaded into just the calling dtkSharedMem object.
   * Setting \e pid to DTK_ALL_PID will cause the filter to get loaded
   * into all current and future dtkSharedMem objects that connect to
   * the shared memory file.
   *
   * \param name All filters require a name so that the filter may be
   * uniquely identified when the filter is removed. If name is NULL a
   * name will be generated. The generated name will be the same as \e
   * dso_file if it is unique for this shared memory file.
   *
   * \return returns -1 on error and 0 on success
   */
  int loadFilter(const char *dso_file, int argc=0,
		 const char **argv=NULL, pid_t pid=DTK_ALL_PID,
		 const char *name=NULL);

  //! Load a shared memory read/write filter from a DSO file with no arguments
  /*!
   * Set this DTK shared memory file to use the dtkFilter (filter)
   * object read(), and/or write() methods found in the dynamic shared
   * object (DSO) file \e dso_file when calling the read() method in
   * the dtkSharedMem objects in the process with PID \e pid.
   *
   * \param dso_file the name of the DSO filter file to load.  The
   * environment variable DTK_FILTER_PATH and then the installed path
   * will be used if the full path file name is not given.
   *
   * \param pid
   * Load the filter into process with PID \e pid.  Setting \e pid to
   * the macro DTK_CURRENT_PID will cause the filter to get loaded
   * into just the dtkSharedMem objects that are in the current
   * process. Setting \e pid to DTK_CURRENT_OBJECT will cause the
   * filter to get loaded into just the calling dtkSharedMem object.
   * Setting \e pid to DTK_ALL_PID will cause the filter to get loaded
   * into all current and future dtkSharedMem objects that connect to
   * the  shared memory file.
   *
   * \param name All filters require a name so that the filter may be
   * uniquely identified when the filter is removed. If name is NULL a
   * name will be generated. The generated name will be the same as \e
   * dso_file if it is unique for this shared memory file.
   *
   * \return returns -1 on error and 0 on success
   */
  int loadFilter(const char *dso_file, pid_t pid,
		 const char *name=NULL);
  //! Load a shared memory read/write filter from a DSO file for all processes
  /*!
   * Set this DTK shared memory file to use the dtkFilter (filter)
   * object read(), and/or write() methods found in the dynamic shared
   * object (DSO) file \e dso_file when calling the read() method in
   * the dtkSharedMem objects in the process with PID \e pid.
   *
   * \param dso_file the name of the DSO filter file to load.  The
   * environment variable DTK_FILTER_PATH and then the installed path
   * will be used if the full path file name is not given.
   *
   * \param argc Pass \e argc to the DSO loader function.
   *
   * \param argv Pass \e argv to the DSO loader function.
   *
   * \param name All filters require a name so that the filter may be
   * uniquely identified when the filter is removed. If name is NULL a
   * name will be generated. The generated name will be the same as \e
   * dso_file if it is unique for this shared memory file.
   *
   * \return returns -1 on error and 0 on success
   */
  int loadFilter(const char *dso_file, int argc,
		 const char **argv, const char *name);

  //! Unload a read/write filter
  /*!
   * \param name is the name of the filter to unload.  This filter
   * will be unloaded for all dtkSharedMem objects in all processes on
   * a given system.
   *
   * \return 0 on success and -1 on error.
   */
  int unloadFilter(const char *name);

  /*!
   * Print information about the filters that are loaded.
   *
   * \param file File to print to.  If \e file is NULL than this prints
   * to stdout.
   *
   * \return 0 on success and -1 on error.
   */
  int printFilters(FILE *file=NULL);

  /*!
   * Stop the use of read filters into this dtkSharedMem object.
   */
  void stopReadFilters(void);

  /*!
   * Stops the use write filters into this dtkSharedMem object.
   */
  void stopWriteFilters(void);

  /*!
   * Stops the use of read and write filters with this dtkSharedMem object.
   */
  void stopFilters(void);

  /*!
   * A tool for to help with debugging.
   *
   * \param file File to print to.  If \e file is NULL than this prints
   * to stdout.
   *
   * \return 0 on success and -1 on error.
   */
  int printWriteList(FILE *file=NULL); // defaults to stdout

  //! print debugging tool
  /*!
   * Print stuff related to the memory.
   *
   * \param file File to print to.  If \e file is NULL than this
   * prints to stdout.
   *
   * \return 0 on success and -1 on error.
   */
  int _memPrint(FILE *file=NULL); // defaults to stdout

 private:

  /* checkRemoveSockets() checks for the case when all sharedMem
   * objects have been deleted, so that another sockets that are open
   * will be deleted too.  In sharedMem_network.cpp.
   */
  void checkRemoveSockets(void);
  // initialized to 0 in sharedMem_network.cpp
  static int number_of_dtkSharedMem_objs;

#ifdef DTK_ARCH_WIN32_VCPP
  HANDLE fileMappingHandle;
  HANDLE fd;
#else
  int fd;
#endif

  /*******************************************************************
   ****************** BEGIN pointers to Shared Memory ****************
   *******************************************************************/
  // Between these two C style comments is all the pointers in this
  // class that point to shared memory.  If the shared memory is
  // remapped these pointers need to be made valid pointers again.

  // rwlock points to shared memory on UNIX and local memory on Windoz.
  struct dtkRWLock *rwlock; // used for dtkRWLock in this class
  unsigned char *top; // points to the top of the user entree data
  unsigned char *ptr; // dummy pointer, used in write() and read()
		      // methods

  struct dtkSharedMem_header *header; // points to the beginning of
				      // the shared memory file
  /*******************************************************************
   ******************** END pointers to Shared Memory ****************
   *******************************************************************/

  u_int64_t read_index;
  char *name, *fullPath;
  // file_size is the whole file size which is all mapped the memory.
  size_t user_size, file_size;

  // This will be non-zero if auto byte swapping is set up
  size_t byte_swap_size;

  // address_change_count is compared to the corresponding one in
  // shared memory.
  u_int32_t filter_change_count;
  u_int32_t address_change_count;
  u_int8_t flags;

  int rwlockState;
  u_int32_t entry_size;
  u_int16_t queue_length;

  // set at each qread() if flags & COUNTING
  u_int64_t counter;

  // set at each qread() if flags & TIMING
  struct timeval timeVal;

  // set at each qread() if flags & QUEUING
  u_int64_t queue_count;
  
  // different_byte_order is 1 if machine and shared mem data are
  // different byte order and 0 if they are the same byte order.
  int different_byte_order;

  dtkSegAddrList *addrList;

  FilterList *filterList;

  //void (*)(dtkSharedMem *shm) writeSignalCatcher;

  // Setting do_connect_spew will enable error spew with dtkMsg.
  void init(size_t size, const char *filename, void *initBuff,
	    int flag, int do_connect_spew=1);
  void cleanup(void);
  int _write(const void *ptr, const struct timeval *timeStamp=0,
	     size_t nbyte=0, size_t offset=0);

  // recreate() uses flags and resize/recreate this.  Get a write lock
  // before and after calling this.
  int recreate(void);

  // Get a write lock before and after calling this too.
  int reconnect(void);

  // This is to manage memory at the after the user entry data, where
  // the write lists are kept.  You must lock the memory before
  // calling _alloc() and _free().
  char *_alloc(size_t size);
  int _free(void *buf);

  // must be locked before calling and check address_change_count
  // before.
  void sync_addrList(void);

  int _write_network(void);
  void realloc_net_data_buf(void);
  int get_createOrConnect_type(size_t size, const char *filename,
			       void *initBuff,
			       int flag, int do_connect_spew);

  void create(size_t size, const char *filename, void *initBuff,
	      int flag, int do_connect_spew);

  void connect(size_t size, const char *filename, void *initBuff,
	       int flag, int do_connect_spew);

  int connect_mapFile(size_t size, int resize_flag=0);

  // get a wlock before calling this.
  int _writeCount(int truth);
  // get a wlock before calling this.
  int _queue(int queue_length=DTK_DEFAULT_QUEUELENGTH);

  int _addFilter(dtkFilter *fltr, void *loaderFunction);
  int _unloadFilter(void *loaderFunction, dtkFilter *fltr);
  int _loadFilter(const char *dso_file, int argc,
		  const char **argv,
		  dtkFilter **filter,
		  dtkFilter *(**loaderFunction)(size_t, int, const char **));
  int unloadAndLoad_filterList(void);
  int cleanUpFilters(void);

  // wlock before calling.
  void sync_filterList(void);
 
  // Called by read() if there is one or more read filters loaded.
  int filter_read(void *buf, size_t bytes, size_t offset);
  int _read(void *buf, size_t bytes, size_t offset);
  void cycle_filtersRead(void *buf, size_t nbyte, size_t offset);
  void *cycle_filtersWrite(const void *buf, struct timeval *timeStamp,
			   size_t nbyte, size_t offset);

  size_t net_data_buf_size;
  char *net_data_buf;
  char byteOrderChar;
  size_t sizeStringLength;
  char *filterDSOPath;
  int have_blocking_read_lock;

  unsigned int filter_flags;
};

/** \fn char *dtkSharedMem_getFullPathName(const char *name)
 * \brief Get the full path file name of a shared memory file
 *
 * Don't mess with the memory returned.
 *
 * \return a pointer to a string in static memory.
 * \return NULL on failure
 * on error.
 */
extern DTKAPI char *dtkSharedMem_getFullPathName(const char *name);


/** \fn int dtkSharedMem_destroy(const char *file_name, int flags)
 * \brief Destroy a shared memory file
 *
 * Removes all system resources with a given shared memory file \e
 * file_name.  Can do it recursively in a given directory by setting
 * the \e DTK_RECURSE bit in \e flags.
 *
 * \return 0 on success
 * \return nonzero on failure
 */
extern DTKAPI int dtkSharedMem_destroy(const char *file_name, int flags=0);
