// This is a simple example that uses the dtkDSO family of functions,
// which includes: dtkDSO_load() and dtkDSO_unload().
// The program "load" will load the DSO file fooDSO.so.  This must be
// linked to dynamic shared object (DSO), barDSO.so, to get the base
// class interfaces to class Foo.  The code for the base class Bar
// must be in a DSO (.so) file so that this can load data from it at
// run time, since at link time the data needed is not known.

// Using this method we can load any C++ code.  The dtkManager class
// loads and runs C++ DSOs.  Using dtkManager is easyer than the code
// below.


#include <stdio.h>
#include <dtk.h>

char girlfriend[128] = {"Betty"};

// defines interface to class Bar which is a base class for Foo
#include "barDSO.h" 

int main(void)
{
  // Get the C++ Bar loader function.
  Bar *(*loaderFunc)(const char *) =
    (Bar * (*)(const char *))
    dtkDSO_load(".", "fooDSO.so");

  if(!loaderFunc) return 1; // error

  // Get the Bar object.
  Bar *foo = (Bar *) loaderFunc("Hello");

  // Use the loaded C++ object.
  foo->bar();

  // Get the Bar unloader function.
  int (*unloaderFunc)(Bar *) = 
    (int (*)(Bar *)) dtkDSO_getUnloader((void *) loaderFunc);

  if(!unloaderFunc)
    {
      dtkMsg.add(DTKMSG_WARN,
		 "failed to get "
		 "unloader function.\n");
      dtkDSO_unload((void *) loaderFunc);
      return 1; // error
    }

  // Call the unloader function.
  if(unloaderFunc(foo) == -1)
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
