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
/****** dtkSegAddrList.h ******/
struct dtkSegAddrList_list
{
  char *address; // address and port or just file?
  char *name; // remote shared memory file name
  struct dtkSegAddrList_list *next;
};

// Do not Doxygen this, it's an internal libdtk thing.

/* \class dtkSegAddrList dtkSegAddrList.h dtk.h
 * \brief A link list of shared memroy segments
 *
 * A basic link list in C that removes, adds, gets, and
 * returns whether or not the list is empty. This is
 */
class DTKAPI dtkSegAddrList
{
 public:

  /*!
   * Construct the list with name being the first data element
   */
  dtkSegAddrList(const char *name);

  /*!
   * Destructor
   */
  virtual ~dtkSegAddrList(void);

  /*!
   * returns remote shared memory file name.
   * return NULL if not found.
   */
  char *get(const char *address);

  //! Check to see if there are any address ports in the list
  /*!
   * Use before the first call to getNextAddressPort() in a loop.
   * returns 1 if there are any in the list.  returns 0 if not.
   */
  int start(void);

  //! Return the address port
  /*!
   * returns NULL at the end.
   * Don't mess with the memory returned.
   */
  char *getNextAddressPort(char **remoteName=NULL);

  //!Add an address to the list
  /*!
   * Add addres to the list with connection remoteName
   */
  int add(const char *address, const char *remoteName);

  //!Remove an address from the list
  /*!
   * Remove the parameter address from the list
   */
  int remove(const char *address);

  /*!
   * Clear the list
   */
  int removeAll(void);

  /*! shared memory file name that this list is for */
  char *name;

 private:

  // first in the stack and current one.
  struct dtkSegAddrList_list *first, *current;

};
