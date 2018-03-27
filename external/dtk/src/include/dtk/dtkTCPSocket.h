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
/****** dtkTCPSocket.h ******/
/* BACKLOG is the number of waiting connections that the OS will keep
 * for you, before it refuses connections.  Or something like that.
 * See man pages (man listen) for details. */

#define  DTKTCPSOCKET_DEFAULT_BACKLOG 6


/*! \class dtkTCPSocket dtkTCPSocket.h dtk.h
 * \brief A simple C++ TCP sockets wrapper
 */
class  DTKAPI dtkTCPSocket : public dtkSocket
{
 public:  

  //! Makes a binded socket
  /*!
   * This makes a binded socket.
   * domain = PF_INET or PF_UNIX
   */
  dtkTCPSocket(int domain, const char *binding_port,
	       int backlog=DTKTCPSOCKET_DEFAULT_BACKLOG);

  //! Makes a connected socket
  /*!
   * Makes a connected TCP socket
   */
  dtkTCPSocket(const char *address_port, int domain=PF_INET);

  //! Create an unbinded / un-connected socket
  /*!
   * This makes a unbinded and un-connected socket.
   */
  dtkTCPSocket(int domain=PF_INET);

  // Not for users
  /*
   * This is to accept connections. Look in dtkTCPSocket::accept().
   * This is not for users.
   */
  dtkTCPSocket(int domain, int fd,
               struct sockaddr *a, int addr_length);

  //!Destructor
  /*!
   * Closes the socket and deletes the object.
   */
  virtual ~dtkTCPSocket(void);

  //! Wrapper for select and accept
  /*!
   * wrapper on select() then accept().
   */
  dtkSocket *accept(unsigned int timeOutSeconds);
  
  //! Wrapper for  accept
  /*!
   * wrapper on just accept()
   * Use this if you call select() before.
   */
  dtkSocket *accept(void);
  
  //! Read data from the socket
  /*!
   * Read data from the socket via a void pointer
   *
   * \return the number of bytes read on successful read, and -1 on
   * failure
   */
  ssize_t read(void *data, size_t size);

  //!Write data to the socket
  /*!
   * Write data to the socket via a void pointer
   */
  ssize_t write(const void *data, size_t size);
  
  // With select() timeout, returns DTK_SOCKET_TIMEOUT if it times out.
  //! Read with a timeout
  /*!
   * Read data with a void pointer using a time out
   *
   * \return DTK_SOCKET_TIMEOUT if it times out, the number of bytes
   * read on successful read, and -1 on failure
   */
  ssize_t read(void *data, size_t size,
	       unsigned int seconds);

 private:
  void init(int domain, const char *binding_port, int backlog);
};
