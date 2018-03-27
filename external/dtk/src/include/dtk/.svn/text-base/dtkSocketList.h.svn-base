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

//The node for the link list
struct dtkSocketList_list
{
  dtkSocket *sock;
  char *addressPort;
  struct dtkSocketList_list *next;
};

// This call is used interally to libdtk
// Skip the Doxygen.

/* \class dtkSocketList dtk.h
 * \brief  Single linked list for dtkSockets
 *
 * This is a singly linked list that handles dtkSockets You can get,
 * add, and remove sockets.  This does not handle duplicate sockets,
 * it is up to the user to make sure duplicates are not included.
 */
class DTKAPI dtkSocketList
{
 public:

  /*!
   * Create a socket list
   */
  dtkSocketList(void);

  // will delete All dtkSockets.
  /*!
   * Destroy all the sockets and the class. This will delete any
   * sockets that are in the list.
   */
  virtual ~dtkSocketList(void);

  //! Get a socket based on the address port
  /*!
   *Get a socket based on the address port
   */
  dtkSocket *get(const char *addressPort);
  
  //! Get the actual address port
  /*!
   *  Get the actual address port, do not mess with the 
   *  returned memory
   */
  char *get(const dtkSocket *s);

  //! Get the first object
  /*!
   * Returns the first dtkSocket object in the list or NULL if the
   * list is empty.
   */
  dtkSocket *get(void);
  
  //! Add a socket to the list
  /*!
   * Add a socket to the list, check for duplicates first by
   * using a get 
   */
  int add(dtkSocket *s, const char *addressName=NULL);

  //! Remove without deletion
  /*!
   * Remove without deletion of the socket
   */
  int remove(const char *addressPort);
  //! Remove a socket
  /*!
   * Remove a socket
   */
  int remove(const dtkSocket *s);

  // int delete_in_destructor;

 private:

  // stack list
  struct dtkSocketList_list *first;
};
