#include <iris/ShmString.h>

namespace iris
{
  ShmString::ShmString(std::string name, size_t shmSize) : _shm(NULL), _valid(false), _shmSize(0), _name("") 
  { 

    _shm = new dtkSharedMem(shmSize+1,name.c_str()) ;
    if (_shm->isValid())
      {
	_name = name ;
	_shmSize = shmSize ;
	_data = static_cast<char*>(malloc(shmSize+1)) ;
	_valid = true ;
      }
    else _shm = NULL ;
  }

  bool ShmString::setString(std::string value)
  {
    if (!_valid) return false ;
    if (value.length()>_shmSize+1) return false ;
    _shm->write(value.c_str()) ;
    return true ;
  }

  std::string ShmString::getString()
  {
    if (_valid)
      {
	_shm->read(_data) ;
	return std::string(_data) ;
      }
    else return std::string("") ;
  }

}
