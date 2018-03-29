#ifndef __IRIS_SHM_STRING__
#define __IRIS_SHM_STRING__

#include <dtk.h>

namespace iris
{

  /**
     \brief The %ShmString class stores and returns a std::string value in dtk shared memory.
  */
  
  class ShmString
  {
  public:    
    /**
       \brief create a ShmString object.

       \param name is the name of the dtk shared memory file.

       \param shmSize is the maximum string size allowed
    */
    ShmString(std::string name, size_t shmSize=1023) ;

    /**
       \brief write the string to shared memory
    */
    bool setString(std::string value) ;

    /**
      \brief get the string from shared memory
    */ 
    std::string getString() ;

    /**
      \brief get the maximum string size
    */ 
    size_t getSize() { return _shmSize ; } ;
    /**
      \brief get the name of the dtk shared memory file
    */ 
    std::string getShmName() { return _name ; } ;
    /**
      \brief get a pointer to the dtk shared memory file
    */ 
    dtkSharedMem* getShm() { return _shm ; } ;
    /**
       \brief return true if the object can be used to store/retrieve data
    */
    bool isValid() { return _valid ; } ;
    /**
       \brief return true if the object can NOT be used to store/retrieve data
    */
    bool isInvalid()  { return !_valid ; } ;
  private:
    dtkSharedMem* _shm ;
    char* _data ;
    std::string _name ;
    size_t _shmSize ;
    bool _valid ;
  } ;
} 
#endif
