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
/****** dtkUDPSocket.h ******/

/*! \class dtkUDPSocket dtkUDPSocket.h dtk.h
 * \brief  A simple C++ UDP sockets wrapper
 *
 */
class DTKAPI dtkUDPSocket : public dtkSocket
{
public:

  //! Creates a udp socket with or without binding it to a port
  /*!
   * If bind_port is set, this will bind to bind_port.
   */
  dtkUDPSocket(int domain, const char *bind_port=NULL);

  //! Set the remote address and port for writing
  /*!
   * This will set the remote address and port to write to.
   */
  dtkUDPSocket(const char *retmote_address_port, int domain=PF_INET);

  //! Destructor
  /*!
   * Close the socket and delete the object.
   */
  virtual ~dtkUDPSocket(void);

  //! Read data from the socket
  /*!
   * Read data from the socket with a void pointer
   */
  ssize_t read(void *data, size_t size);
  //! Write to the socket
  /*!
   * Write to the socket with a void pointer
   */
  ssize_t write(const void *data, size_t size);
  // With select() timeouts
  //!Read data from the socket
  /*!
   * Read data from the socket with timeout
   *
   * \return DTK_SOCKET_TIMEOUT if it times out, the number of bytes
   * read on successful read, and -1 on failure
   */
  ssize_t read(void *data, size_t size,
	       unsigned int seconds);
  //!Write to the socket with timeout
  /*!
   * Write to the socket with a void pointer
   */
  ssize_t write(const void *data, size_t size,
		unsigned int seconds);

  //! Set multicasting
  /*!
   * Set multicasting at a specific address.
   */
  int setMulticast(const char *address, int ttl_in);

private:
  void init(int domain, const char *binding_port);
};
