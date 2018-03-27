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
/****** dtkSocket.h ******/

/*! \class dtkSocket dtkSocket.h dtk.h
 * \brief  Virtual base class for UDP/TCP sockets
 *
 * The class is a virtual base class for dtkTCPSockets and dtkUDPSockets 
 * MULTICAST_MIN "233.0.0.0"
 * MULTICAST_MAX "239.255.255.255"
 */
#define DTK_SOCKET_TIMEOUT ((ssize_t) -4)

/* I assume that the windows (ifdef DTK_ARCH_WIN32_VCPP) sockets
 * only support PF_INET domain sockets.
 */


class DTKAPI dtkSocket : public dtkBase
{
public:

  /*! Consturctor */
  dtkSocket(void);

  /*! Destructor */
  virtual ~dtkSocket(void);

  //! Get method for the file describtor
  /*!
   * Returns the file descriptor for the class
   */
#ifdef DTK_ARCH_WIN32_VCPP
  inline SOCKET
#else
    inline int
#endif
     getFileDescriptor(void) { return fd;};
  //! Debug function
  /*!
   * Print debug information to the file pointer file.
   */
  void print(FILE *file=NULL); // default is stdout

  //! Read from a socket 
  /*!
   * Read data from a socket
   */
  virtual ssize_t read(void *data, size_t size) = 0;

  //! Write to a socket 
  /*!
   *  Write data to a socket
   */
  virtual ssize_t write(const void *data, size_t size) = 0;

  //! Read data with a timeout
  /*!
   * Read data with a void pointer with timeout.
   * With select() timeout, returns DTK_SOCKET_TIMEOUT if it times out.
   *
   */
  virtual ssize_t read(void *data, size_t size,
		       unsigned int seconds) = 0;

  //! Set the remote address and port
  /*!
   *
   *  set remote address and port.  This works for a TCP or UDP client-like socket
   */
  int setRemote(const char *address_port);

  //! Connect to the remote address
  /*!
   * connect to the RemoteAddress. using connect()
   */
  int connect(void); //

  //! Basic accept wrapper 
  /*!
   *  UDP has no connection, TCP overwrites it
   */
  virtual dtkSocket *accept(void);  // wrapper on accept()

  //! Set the socket to block reads
  /*!
   * Allows for blocking of reads on the socket
   */
  int setBlocking(void);    // blocking reads

  //! Set the socket to have non blocking reads
  /*!
   *Set the socket to have non blocking reads
   */
  int setNonblocking(void); // nonblocking reads

  //! Return whether or not the socket has data
  /*!
   * Return whether or not the socket has data.
   * This is a wrapper of the select() function.
   */
  int haveData(void); // wrapper on select()

  //! Get the remote address and port
  /*!
   * Return a character pointer with the remote address and port 
   * Returns a pointer to static memory
   */
  char *getRemote(void);
  
  //! Set the multicast address 
  /*!
   * Set the multicast address 
   */
  virtual int setMulticast(const char *address, int ttl);
protected:
  // For Adding the print() method above to the dtkErr object (error queue).
  // void printToErr(int severity, int error_number); // Some day.
#ifdef DTK_ARCH_WIN32_VCPP
	SOCKET fd;
#else
  int fd;
#endif
  int is_connected;
  int domain;// PF_INET or PF_UNIX
  int Bind(const char *local_address);
  void remove_object(void);
  union
  {
    /* internet domain address, defined in
     * /usr/include/socketisocket/in.h */
    // stored in here in network byte order
    struct sockaddr_in i;
    /* UNIX domain address, defined in 
     * /usr/include/sys/un.h */
#ifndef DTK_ARCH_WIN32_VCPP
    struct sockaddr_un u;
#endif
  } remote_addressPort, bind_addressPort;

#ifdef DTK_ARCH_WIN32_VCPP
  // on windows there is a globel WSA socket initialization
  // and cleanup for a given process, so we need to initialize
  // and cleanup just once per process, so we need this counter.
  static int number_of_objects;
  static struct WSAData wsaData;
#endif

};
