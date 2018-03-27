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
/****** dtkManager.h ******/


/////////////////////// dtkManager state bit masks /////////////////////
//
// This is just a communication tool for the user and it does not !
// affect anything but dtkManager::state.  When the dtkManager object !
// is invalid DTK_ISRUNNING will be unset.  The DTK_ISRUNNING bit is !
// set in dtkManager::state at the begining of !
// dtkManager::dtkManager().

#define DTK_ISRUNNING   01
#define DTK_ICONSLEEP   02


// callback function bitmasks
// for struct dtkAugment_list::flag
// and struct dtkDisplay_list::flag

#define DTK_PRECONFIG     ((unsigned int) 0001)
#define DTK_CONFIG        ((unsigned int) 0002)
#define DTK_POSTCONFIG    ((unsigned int) 0004)
#define DTK_SYNC          ((unsigned int) 0010)
#define DTK_PREFRAME      ((unsigned int) 0020)
#define DTK_FRAME         ((unsigned int) 0040)
#define DTK_POSTFRAME     ((unsigned int) 0100)
#define DTK_RECONFIG      ((unsigned int) 0200)

#define DTK_ISDISPLAY     ((unsigned int) 0400)

#define ALL_CALLBACKS (DTK_PRECONFIG |\
		       DTK_CONFIG |\
		       DTK_POSTCONFIG |\
		       DTK_SYNC |\
		       DTK_PREFRAME |\
		       DTK_FRAME |\
		       DTK_POSTFRAME |\
		       DTK_RECONFIG )


//augment list for calling preConfig, postConfig,
//preFrame, postFrame, and reConfig.
struct dtkAugment_list
{
  dtkAugment *augment;
  struct dtkAugment_list *next;
  struct dtkAugment_list *prev;
  unsigned int flag;
  void *loaderFunc;
};

struct dtkAugment_callback
{
  dtkAugment *augment;
  struct dtkAugment_callback *next;
};

// display list for calling config, frame and sync.
struct dtkDisplay_list
{
  dtkDisplay *display;
  struct dtkDisplay_list *next;
  unsigned int flag;
};


/*! \class dtkManager dtkManager.h dtk.h
 * \brief A class to manage dtkAugment objects
 *
 * This class manages dtkAugment objects. The class provide methods to
 * add and remove dtkAugment objects, by directly adding user
 * instancated objects or loading objects from dynamic shared object
 * (DSO) files.
 *
 * Except for type=DTKDISPLAY_TYPE, dtkManager does not care what type
 * (dtkBase::getType()) it is, so you may be able to change the type
 * to what ever you like.  If you change the type, be sure to
 * dtkBase::validate() it as needed.
 */

class DTKAPI dtkManager : public dtkBase
{
public:

  dtkManager(void);

  /**
    Construct a dtkManager with the argc/argv passed to the program.  This is
    useful for augments that need access to this information on systems such 
    as Mac OS X which don't have a /proc file system.  
  */
  dtkManager( int argc, char** argv );


  virtual ~dtkManager(void);
  
  /**
    @note this may be 0.
    @return the length of argv() that was passed to the constructor.  
  */
  int argc()const;
  /**
    @note this may be 0.
    @return the command line args passed to the constructor.
  */
  char** argv()const;

  /*!
   * Set the search path to look for DSO files. It will make a copy of
   * what's passed in.  The dtkAugment DSO file that DTK installed
   * with may be found from the output of the program:
   *
   * `dtk-config --augment-dso-dir'
   *
   * \param dso_path seach path to set to.
   */
  void path(const char *dso_path);


  /*!
   * Get the search path look for DSO files.
   *
   * \return Don't write to the memory that this returned. If the
   * path was not set NULL is returned.
   */
  char *path(void) const;

  /*!
   * Users may set and unset bits in this state variable.  It's just a
   * kludgey way to share binary type state data.  There are currently
   * two bits use so far: DTK_ISRUNNING and DTK_ICONSLEEP.
   */
  u_int32_t state;

  //! Load a dtkAugment DSO file
  /*!
  * No additional DSOs will be loaded after failing to load one.  The
  * file loaded is not required to contain a dtkAugment object, so
  * that it may just run some code and than request to be unloaded
  * after just that one function call.
  *
  * \param file a NULL terminated array list of files.
  *
  * \param arg a pointer to data to pass to the DSO loader which in
  * turn may be passed to the dtkAugment constructor and so on.
  *
  * \return returns 0 on success and -1 if one DSO failed to load.
  */
  int load(const char **file, void *arg=NULL);

  //! Load a dtkAugment DSO file
  /*!
  * No additional DSOs will be loaded after failing to load one.  The
  * file loaded is not required to contain a dtkAugment object, so
  * that it may just run some code and than request to be unloaded
  * after just that one function call.
  *
  * \param file may be one file or a colon separated list of files.
  *
  * \param arg a pointer to data to pass to the DSO loader which in
  * turn may be passed to the dtkAugment constructor and so on.
  *
  * \return returns 0 on success and -1 if one DSO failed to load.
  */
  int load(const char *file, void *arg=NULL);

  /*!
   * Add a dtkAugment object to the managers list of dtkAugment
   * objects.  This enables you to "load" dtkAugment objects that are
   * not in a DSO file.
   *
   * \param augment is a pointer to a valid dtkAugment object
   *
   * \return 0 on sucess or -1 on error
   */
  int add(dtkAugment *augment);

  //! Remove dtkAugment object by name
  /*!
   * Remove the dtkAugment object that has the name \e name.  If the
   * dtkAugment object was loaded from a file than the file will be
   * unloaded if it is the last object from that file.
   *
   * \param name the name of the dtkAugment object. This name is
   * unique, for a given dtkAugment in a given dtkManager object. The
   * name was passed to the dtkAugment constructor.
   *
   * \return 0 on success, -1 on error
   */
  int remove(const char *name);

  //! Remove dtkAugment object by object pointer
  /*!
   * Remove the dtkAugment object that is pointed to by augment.  If
   * the dtkAugment object was loaded from a file than the file will
   * be unloaded if it is the last object from that file.
   *
   * \param augment is pointer to the dtkAugment object that is to be
   * removed.
   *
   * \return 0 on success, -1 on error
   */
  int remove(dtkAugment *augment);

  //! Return the next dtkAugment object in the list
  /*!
   * If \e augment is NULL a pointer to the first dtkAugment object is
   * returned, else if \e augment is non-NULL a pointer to the next
   * dtkAugment object is returned.  If \e augment is a pointer to the
   * last dtkAugment object than NULL is returned.
   *
   * \param augment a pointer to a dtkAugment object
   *
   * \param class_type if \e class_type is not DTK_ANY_TYPE than
   * the returned pointer will be a pointer to the next dtkAugment object
   * of the type \e class_type. If there is none than NULL is returned.
   *
   * \return returns a dtkAugment object from the list of managed
   * dtkAugment objects.
   */
  dtkAugment *getNext(const dtkAugment *augment = NULL,
		      u_int32_t class_type = DTK_ANY_TYPE) const;

  //! Return the first dtkAugment object in the list
  /*!
   * Same as getNext(NULL, class_type).
   *
   * \param class_type if \e class_type is not DTK_ANY_TYPE than
   * the returned pointer will be a pointer to the first dtkAugment object
   * of the type \e class_type. If there is none than NULL is returned.
   *
   * \return returns a dtkAugment object from the list of managed
   * dtkAugment objects.
   */
  dtkAugment *getNext(u_int32_t class_type) const;

  //! Get a dtkAugment object by name
  /*!
   * Returns a dtkAugment object pointer from the list with the name
   * \e name.
   *
   * \param name is the name of the dtkAugment object
   *
   * \param class_type If \e class_type is not of type DTK_ANY_TYPE
   * than the returned dtkAugment object must be of type \e class_type
   * or else NULL will be returned.
   *
   * \return returns a dtkAugment object from the list of managed
   * dtkAugment objects.
   */
  dtkAugment *get(const char *name,
		  u_int32_t class_type = DTK_ANY_TYPE) const;


  #ifdef __APPLE__
     #ifdef check
       #undef check
     #endif
  #endif
  //! Check a dtkAugment object by name
  /*!
   * Returns a dtkAugment object pointer from the list with the name
   * \e name.  This is the same as get() without the spew due to not
   * finding the object.
   *
   * \param name is the name of the dtkAugment object
   *
   * \param class_type If \e class_type is not of type DTK_ANY_TYPE
   * than the returned dtkAugment object must be of type \e class_type
   * or else NULL will be returned.
   *
   * \return returns a dtkAugment object from the list of managed
   * dtkAugment objects.
   */
  dtkAugment *check(const char *name,
		    u_int32_t class_type = DTK_ANY_TYPE) const;

  //! Check a dtkAugment object by object pointer
  /*!
   * This checks if the dtkAugment is in the list.
   *
   * \param augment is a pointer to the dtkAugment that you're
   * checking.
   *
   * \return returns a dtkAugment object from the list of managed
   * dtkAugment objects, which is also \e augment.  Returns NULL if it
   * is not found in the list.
   */
  dtkAugment *check(const dtkAugment *augment) const;

  //! Add a callback for a given dtkAugment
  /*!
   * 
   * Adds the call-back to the list so that it does get called in the
   * next dtkManager cycle.  There will be no error do to the callback
   * already being added.
   *
   * \param augment pointer to the dtkAugment object whos callback is
   * to be added.
   *
   * \param when specifies the particular dtkAugment callback method
   * (member function) that is to be added. \e when is a bit mask
   * and can be the bits DTK_PRECONFIG, DTK_CONFIG, DTK_POSTCONFIG,
   * DTK_SYNC, DTK_PREFRAME, DTK_FRAME, DTK_POSTFRAME, and
   * DTK_RECONFIG and any bit-wise ORed combination of these. Like for
   * example: addCallback(augment, DTK_PREFRAME|DTK_POSTFRAME).
   *
   * \return 0 on success, -1 on error
   */
  int addCallback(const dtkAugment *augment, unsigned int when);
 
  //! Remove a callback for a given dtkAugment
  /*!
   * Removes the call-back from the list so that it does not get
   * called in the next dtkManager cycle.  There will be no error do to the
   * callback already being removed.
   *
   * \param augment pointer to the dtkAugment object whos callback is
   * to be removed.
   *
   * \param when specifies the particular dtkAugment callback method
   * (member function) that is to be removed. \e when is a bit mask
   * and can be the bits DTK_PRECONFIG, DTK_CONFIG, DTK_POSTCONFIG,
   * DTK_SYNC, DTK_PREFRAME, DTK_FRAME, DTK_POSTFRAME, and
   * DTK_RECONFIG and any bit-wise ORed combination of these. Like for
   * example: removeCallback(augment, DTK_PREFRAME|DTK_POSTFRAME).
   *
   * \return 0 on success, -1 on error
   */
  int removeCallback(const dtkAugment *augment, unsigned int when);

  /*!
   * Debug printing
   *
   * \param file the file to print to. If \e file is NULL
   * this will print to stdout.
   */
  void print(FILE *file=NULL) const; // default is stdout

  /*!
   * This calls all the managed dtkAugment objects
   * dtkAugment::preConfig() callbacks.
   *
   * \returns the sum of the number of times that
   * dtkAugment::preConfig() returns dtkAugment::ERROR.
   */
  virtual int preConfig(void);

  /*!
   * This calls all the managed dtkAugment objects
   * dtkDisplay::config() callbacks. A dtkDisplay object is also a
   * dtkAugment object. dtkDisplay inherits dtkAugment. dtkAugment
   * does not have a config() method. This will call
   * dtkManager::preConfig() if it has not been called before this and
   * this is the start of a new configuration cycle. A configuration cycle
   * is the sequence of preConfig(), config(), and postConfig().
   *
   * \returns the sum of the number of times that dtkDisplay::config()
   * returns dtkAugment::ERROR plus the return value of the
   * preConfig() if that is called.  It returns 0 if no error occurred
   * in any callback.
   */
  virtual int config(void); 

  /*!
   * This calls all the managed dtkAugment objects
   * dtkAugment::postConfig() callbacks. This will call
   * dtkManager::config() if it has not been called before this and
   * this is the start of a new configuration cycle. A configuration cycle
   * is the sequence of preConfig(), config(), and postConfig().
   *
   * \returns the sum of the number of times that any of the callbacks
   * return dtkAugment::ERROR. It returns 0 if no error occurred
   * in any callback.
   */
  virtual int postConfig(void);

  /*!
   * This calls all the managed dtkAugment objects dtkDisplay::sync()
   * callbacks. A dtkDisplay object is also a dtkAugment
   * object. dtkDisplay inherits dtkAugment. dtkAugment does not have
   * a sync() method. This will call dtkManager::postConfig() if it
   * has not been called before this and this is the start of a new
   * configuration cycle. A configuration cycle is the sequence of
   * preConfig(), config(), and postConfig().
   *
   * \returns the sum of the number of times that any of the callbacks
   * return dtkAugment::ERROR. It returns 0 if no error occurred
   * in any callback.
   */
  virtual int  sync(void);

  /*!
   * This calls all the managed dtkAugment objects dtkDisplay::frame()
   * callbacks. A dtkDisplay object is also a dtkAugment
   * object. dtkDisplay inherits dtkAugment. dtkAugment does not have
   * a sync() method. This will call dtkManager::postConfig() if it
   * has not been called before this and this is the start of a new
   * configuration cycle. A configuration cycle is the sequence of
   * preConfig(), config(), and postConfig(). If dtkManager::sync()
   * has not been called before the last call to dtkManager::frame()
   * this will dtkManager::sync().
   *
   *
   * \returns the sum of the number of times that any of the callbacks
   * return dtkAugment::ERROR. It returns 0 if no error occurred
   * in any callback.
   */
  virtual int  frame(void); 

  /*!
   * Remove all of the dtkAugment objects.  If any dtkAugment object
   * was loaded from a file than the file will be unloaded if it is
   * the last object from that file.
   */
  void removeAll(void);

  /*!
   * \return Returns non-zero if dtkManager::preConfig() has
   * ever been called, otherwise it returns zero.
   */
  inline int isPreConfiged(void) { return (_isPreConfiged>0 ||
					   _isConfiged!=0 ||
					   _isPostConfiged!=0); }

  /*!
   * return Returns non-zero if dtkManager::config() has
   * ever been called, otherwise it returns zero.
   */
  inline int isConfiged(void) { return (_isConfiged>0 ||
					_isPostConfiged!=0); }

 
  /*!
   * return Returns non-zero if dtkManager::config() has
   * ever been called, otherwise it returns zero.
   */
  inline int isPostConfiged(void) { return (_isPostConfiged>0); }

  /*!
   * return Returns non-zero if dtkManager::frame() has
   * ever been called, otherwise it returns zero.
   */
  inline int isFramed(void){ return (_isFramed>0); }

  /*!
   * Get the dtkRecord object.
   *
   * \return Returns a pointer to a dtkRecord that can be used for
   * keeping records from various dtkInput objects.
   */
  dtkRecord *record(size_t queue_lenght = DTKRECORD_DEFAULT_LENGTH);

  /*
   * preFrame() and postFrame() should not be called by the API user,
   * unless they are making a display API, like for example DPF
   * (http://www.diverse.vt.edu/DPF/).
   *
   * preFrame() is automatically called by frame().
   *
   * \return magic_num is just a way to keep you from calling this.
   * This beats using class friends.
   */
  int  preFrame(unsigned int magic_num);

  /*
   * preFrame() and postFrame() should not be called by the API user,
   * unless they are making a display API, like for example DPF
   * (http://www.diverse.vt.edu/DPF/).
   *
   * postFrame() is automatically called by frame().
   *
   * \return magic_num is just a way to keep you from calling this.
   * This beats using class friends.
   */
  int postFrame(unsigned int magic_num);

  /*
   * Make it so that the preFrame() and postFrame() do not
   * automatically get called by frame(). Do not use this unless you
   * are making a display API, like for example DPF
   * (http://www.diverse.vt.edu/DPF/).
   *
   * \return magic_num is just a way to keep you from calling this.
   * This beats using class friends.
   */
  void setPublicPrePostFrames(unsigned int magic_num);

  /*
   * Make it so that the sync() does not automatically get called by
   * frame(). Do not use this unless you are making a display API,
   * like for example DPF (http://www.diverse.vt.edu/DPF/).
   *
   * \return magic_num is just a way to keep you from calling this.
   * This beats using class friends.
   */
  void setNoSyncForcing(unsigned int magic_num);

  /*
   * Change the state of the program so that it will exit
   */
  inline void exit()
  {
    state &= ~(DTK_ISRUNNING);
  };

private:

  int  preFrame(void);
  int postFrame(void);

  void _addCallback(struct dtkAugment_list *al,
		    struct dtkAugment_callback **cb_list);
  void _removeCallback(const dtkAugment *a,
		       struct dtkAugment_callback **cb_list);

  // check if name is in the augment list
  // return 1 if it is else return 0
  int check_augment(const char *name);

  // returns 0 on success or -1 on error.
  // This method is to load DSO files.
  int _load(const char *file, void *arg);

  // returns 0 on success or -1 on error.
  // This method is to unload DSO files.
  int _unload(dtkAugment *a, void *p);

  int remove(struct dtkAugment_list *l);

  // get and allocate a valid name that is not in the list
  char *get_name(const char *name);

  void clean_up(void);

  // These _is* flags are 1 if the method in question has finished
  // being called, 0 if the method in question has not been called,
  // and -1 if the method in question is being called now.
  int _isSynced;
  int _isPreConfiged;
  int _isConfiged;
  int _isPostConfiged;
  int _isFramed;

  int _forceSync;
  int _userCallsPrePostFrame;

  char *DSOPath;

  int       _argc;
  char**    _argv;

  struct dtkAugment_list *augment_list_first;
  struct dtkAugment_list *augment_list_last;

  struct dtkAugment_callback *preFrame_list;
  struct dtkAugment_callback *postFrame_list;

  struct dtkDisplay_list *display_list;

  dtkRecord *_record;
};

