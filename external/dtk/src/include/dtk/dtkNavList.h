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
/****** dtkNavList.h ******/
/*! \class dtkNavList dtkNavList.h dtk.h
 * \brief Manages dtkNav objects
 *
 * This class manages a list of dtkNav naviagtion objects.
 */


// Used to hold a list of all dtkNav objects that dtkNavList
// manages.
struct dtkNavList_navList
{
  dtkNav *nav;
  struct dtkNavList_navList *next;
};


class DTKAPI dtkNavList : public dtkAugment
{
 public: 
  //! Constructor
  dtkNavList(void);

  //! Destructor
  virtual ~dtkNavList(void);

  /*!
   * \e add() adds the dtkNav to the managed list and puts pfDCS into
   * the scene tranversal for navigation, but does not activate it.
   */
  int add(dtkNav *nav);

  /*!
   * \e removeNav() removes the nav from the list but does not
   * deactivate it.
   */
  int remove(dtkNav *nav);
  
  /*!
   * \e switch_() sets a specific dtkNav object to be the current
   * navigation \e nav = NULL means NO Navigation will be enabled.
   */
  int switch_(dtkNav *nav = NULL);
  
  /*!
   * \e current() returns a pointer to the current navigation, or
   * NULL if no navigation.
   */
  inline dtkNav *current(void) { return currentNav;}

  /*!
   * \e activateNav() activates the current nav that is in the scene.
   * \e activateNav() will cause the navigation to start calls to its
   * \e preFrame() and \e postFrame() call-backs.
   */
  int activate(void);

  /*!
   * \e deactivateNav() activates the current nav that is in the scene.
   * \e deactivateNav() will cause the navigation to stop calls to its
   * \e preFrame() and \e postFrame() call-backs.
   */
  int deactivate(void);
  
  /*!
   * query the navList.
   */
  inline dtkNav *first(void) { return (navList? navList->nav: NULL);}

  /*!
   * query the navList.
   */
  dtkNav *next(dtkNav *nav);

 private:

  // a list of dtkNav ojbects
  struct dtkNavList_navList *navList;

  // current connected navigation- can be NULL if no navigation.
  dtkNav *currentNav;

};
