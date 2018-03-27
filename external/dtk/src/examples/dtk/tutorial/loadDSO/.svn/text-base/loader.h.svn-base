// This file is included in the loader part of a loadable DSO file
// that uses the class A as the base class and interface to loaded
// DSOs.


static A *loader(const char *str);
static int unloader(A *i);

extern "C"
{
  // C loader functions that return a pointer to a C++ loader
  // function.  The dtkDSO_load() functions are hard coded to look for
  // these C functions.  These are not C++ functions so that the names
  // don't get mangled. We use these C functions to get pointers to
  // the above C++ functions that we than get C++ class objects from
  // them.  And so we can load any C++ objects that define an external
  // interface.

  void *__dtkDSO_loader(void)
  {
    return (void *) loader;
  }
  void *__dtkDSO_unloader(void)
  {
    return (void *) unloader;
  }
}
