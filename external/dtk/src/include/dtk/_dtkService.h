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
#include <vector>

/****** dtkService.h ******/
/* This is the virtual base class for DIVERSE toolkit (DTK) DSO loaded
 * services.  Build serial and network interfaces with it.  */

// dtkService::serve() return values

// Tell the DTK server that all is happy.
#define DTKSERVICE_CONTINUE   0

// Tell the server to unload the current service.
#define DTKSERVICE_UNLOAD     3

// Tell the server that a fatal error has accorded, which will cause
// the DTK server to exit.
#define DTKSERVICE_ERROR     -1

/*! \class dtkService _dtkService.h dtk.h
 *
 *  \brief Virtual base class for DSO loaded DTK services.
 *
 * This pure virtual base class is used to construct DTK dynamic
 * shared object (DSO) loadable services.  An object of this class can
 * be created in a running DTK server by being loaded as a DSO in a
 * DTK server, <b>dtk-server</b>.
 *
 * A derived classes constructor and over written
 * <b>dtkService::serve</b> method are required.  All other
 * <b>dtkService</b> virtual methods are not required to be over
 * written to make a usable DTK server loadable DSO service.
 */

class DTKAPI dtkService
{
public:
  //! Constructor.
  dtkService( const char* arg = 0 );

  //! Destructor.
  virtual ~dtkService(void);

  //! Initialize the service
  /*!
   * The init function should be used instead of the constructor
   * to perform initialization of the service. The same return values
   * used for the serve() function should be returned from this function.
   * DTKSERVICE_CONTINUE should be returned if the initialization is successful.
   * DTKSERVICE_ERROR should be returned if an error occurs.
   * DTKSERVICE_UNLOAD should be returned under circumstances where an error did
   * not occur but the service should not continue.
   */
  virtual int init(void);
  //! File descriptor to trigger the service.
  /*!
   * When this object is created by being loaded by a DTK server,
   * <b>dtk-server</b>, \e fd is used by a call to <b>select(2)</b> by
   * <b>dtk-server</b> which in turn calls the corresponding
   * <b>dtkService::serve</b> method.  \e fd is used by the DTK server
   * to determine if the service is valid, by checking to see if it is
   * set to a valid file descriptor.  \e fd starts with the invalid
   * value of -1.
   */
#ifdef DTK_ARCH_WIN32_VCPP
  HANDLE fd;
  std::vector<HANDLE> fd_vector;
  std::vector<HANDLE> fd_set_vector;
#else
  int fd; // An opened file descriptor to trigger the service.
  std::vector<int> fd_vector;
  std::vector<int> fd_set_vector;
#endif
  //! Debugging print.
  /*!
   * print() can be used for debugging print stuff.
   */
  virtual void print(FILE *file);
  //! The service action is to serve.
  /*!
   * When this object is created by being loaded by a DTK server,
   * <b>dtk-server</b>, \e fd is used by a call to <b>select(2)</b> by
   * <b>dtk-server</b> which in turn calls this serve() method.
   *
   * This serve() is typically used to call <b>read(2)</b>,
   * parse data read, and then write to DTK shared memory.
   * DTK client programs read the service
   * data from the DTK shared memory.
   */
  virtual int serve(void) = 0;

#ifdef DTK_ARCH_WIN32_VCPP
  //! Stop the service thread
  /*!
   * Windoz does not have a general way to stop a thread that is
   * waiting on a blocking read.  So the thread that is unloading this
   * service will call this stop() to do it for it.  The particular
   * dtkService should overwrite this method to cause the blocking
   * read() is the serve() to return, or else the thread that is
   * unloading this service will call TerminateThread() on the
   * service, which can make trouble.  In UNIX this is not needed.
   */
  virtual void stop(void);
#endif

  //! Reset.
  /*!
   * This method is called to reset the service.
   */
  virtual int reset(void);
  //! Version string for this service.
  /*!
   * The derived class may set this version string as desired.  The
   * default returned value is a pointer to a static string that is
   * the DTK version number string as defined in
   * <pre>
   *  `dtk-config --root`/include/dtk/_config.h
   * </pre> as DTK_VERSION.
   */
  virtual char *version(void);

  const char* getArg() { return m_arg; };
private:
	char* m_arg;
};
