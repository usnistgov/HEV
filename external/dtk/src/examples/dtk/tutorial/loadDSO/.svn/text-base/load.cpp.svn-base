// This is a simple example that uses the dtkDSO family of functions,
// which includes: dtkDSO_load() and dtkDSO_unload(), which are just
// wrappers dlopen() and family of functions (or LoadLibrary() on
// Windows).  The program "load" will load the DSO file implement.so.
// The code for the base classes A and B must be in a DSO (.so) file
// so that this can load data from it at run time, since at link time
// the data needed is not known. So load.cpp is compiled into a shared
// object and then linked.

// Using this method we can load any C++ code.  The dtkManager class
// loads and runs C++ DSOs with a particular interface, so dtkManager
// is easyer to use than dtkDSO_load() in code below.


#include <stdio.h>
#include <dtk.h>

// interface.h defines interface to class A and class B which are base
// classes for the Implementation
#include "interface.h"



int main(void)
{
  // Get the C++ A loader function.
  A *(*loaderFunc)(const char *) =
    (A * (*)(const char *))
    dtkDSO_load(".", "implement.so");

  if(!loaderFunc) return 1; // error

  // Get the A object.
  A *objectA = loaderFunc("Hello");

  // Use the loaded C++ object.
  objectA->taskA();

  // See what we loaded has also implimented B
  B *objectB = dynamic_cast<B *>(objectA);
  if(objectB)
    objectB->taskB();

  // Get the A C++ unloader function.
  int (*unloaderFunc)(A *) = 
    (int (*)(A *))
    dtkDSO_getUnloader((void *) loaderFunc);

  if(!unloaderFunc)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "failed to get "
		 "unloader function.\n");
      dtkDSO_unload((void *) loaderFunc);
      return 1; // error
    }

  // Call the unloader function.
  if(unloaderFunc(objectA) == -1)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "unloading DSO file failed: "
		 "unloader function failed.\n");
      dtkDSO_unload((void *) loaderFunc);
      return 1; // error
    }

  // dtkDSO_unload() returns -1 on error or 0 on success
  return dtkDSO_unload((void *) loaderFunc); 
}
