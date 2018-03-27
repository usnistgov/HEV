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
 */

/*! \class dtkAugment dtkAugment.h dtk.h 
 * \brief The cyclic callback augmentation class for DTK
 *
 * dtkAugment provides methods that are callbacks that the dtkManager
 * calls. The dtkManager class manages dtkAugment objects.  A
 * dtkManager object calls the list of callbacks dtkAugment methods:
 * preConfig(), postConfig(), preFrame(), and postFrame() when the
 * corresponding dtkManager method, dtkManager::preConfig(),
 * dtkManager::postConfig(), dtkManager::preFrame(), and
 * dtkManager::postFrame() is called. dtkManager typically manages
 * many dtkAugment objects.  By default, in this base class,
 * preConfig(), postConfig(), preFrame(), and postFrame() remove
 * themselves from the dtkManager callback list by returning
 * dtkAugment::REMOVE_CALLBACK.
 *
 * \include examples/dtk/examplesByClass/dtkAugment/dtkAugment.cpp
 */
class DTKAPI dtkAugment : public dtkBase
{
public:

  /*!
   * return values that are can be returned by the dtkAugment callback
   * methods preConfig(), postConfig(), preFrame(), or postFrame() and
   * the dtkDisplay methods config(), frame(), and sync(). The
   * callbacks are called by the corresponding dtkManager and the
   * different return values cause the dtkManager to take actions.
   */
  enum RETURN_VALUE
    {
      CONTINUE = 0,   /*!< Continue calling this callback. */
      REMOVE_CALLBACK,/*!< Discontinue calling this callback. */
      REMOVE_OBJECT,  /*!< Unload this dtkAugment object. */
      /* Windoz will not allow us to use ERROR */
      ERROR_          /*!< Discontinue calling this callback and return an error. */
    };

  /*!
   * dtkAugment constructor will not validate the object.  The derived
   * class constructor must call validate() in order to validate the
   * object to let the dtkManager object that is managing the
   * dtkAugment know that the construction was successful in making a
   * valid object, or else the dtkManager will remove the dtkAugment
   * object assuming that there was an error.
   *
   * \param name The name of a dtkAugment is set in its
   * constuctor. The name must be unique in the list of dtkAugments
   * that a given dtkManager manages.  
   */
  dtkAugment(const char *name = NULL);

  /*! destructor
   */
  virtual ~dtkAugment(void);

  /*! 
   * This is called by the managing dtkManager object before the
   * dtkManager::config() method.  Overwrite this method to insert a
   * post-configuation callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR_.
   */
  virtual int  preConfig(void);

  /*! 
   * This is called by the managing dtkManager object after the
   * dtkManager::config() method.  Overwrite this method to insert a
   * post-configuation callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR_.
   */
  virtual int postConfig(void);

  /*! 
   * This is called by the managing dtkManager object before the
   * dtkManager::frame() method. The dtkManager::frame() is typically
   * called in a cyclic manner.  Overwrite this method to insert a
   * pre-frame callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR_.
   */
  virtual int  preFrame(void);

  /*! 
   * This is called by the managing dtkManager object after the
   * dtkManager::frame() method. The dtkManager::frame() is typically
   * called in a cyclic manner.Overwrite this method to insert a
   * post-frame callback.
   *
   * \return REMOVE_CALLBACK. The overwriting method should return
   * CONTINUE, REMOVE_CALLBACK, REMOVE_OBJECT or ERROR_.
   */
  virtual int postFrame(void);

  /*!
   * Get the description string that is the description of that
   * this dtkAugment object does for you. 
   *
   * \return Returns a pointer to the description string. Do
   * not write to the memory this points to.
   */
  char *getDescription(void) const;

  /*!
   * Get the name of this dtkAugment.  All names are unique for all
   * dtkAugments that are managed by a given dtkManager.
   *
   * \return A pointer to the character string that contains the
   * name. Do not write to the memory this points to.
   */
  char *getName(void) const;

  /*!
   * Set the description of this dtkAugment using the a printf() like
   * format.
   *
   * \param format The printf like format string. See man printf(2).
   */
  void setDescription(const char *format, ...);

  /*!
   * Set the description using a va_list format. See man stdarg(3).
   *
   * \param format The printf like format string. See man printf(2).
   *
   * \param args variable a argument list. See man stdarg(3).
   */
  void vsetDescription(const char *format, va_list args);

private:

  char *description;
  char *name;
};

// For compatablity to older versions of DTK.  Do not use in new code.

#define DTK_CONTINUE           (dtkAugment::CONTINUE)
#define DTK_REMOVE_CALLBACK    (dtkAugment::REMOVE_CALLBACK)
#define DTK_REMOVE_OBJECT      (dtkAugment::REMOVE_OBJECT)
#define DTK_CALLBACK_ERROR     (dtkAugment::ERROR_)
