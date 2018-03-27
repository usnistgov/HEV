/*
 * The DIVERSE Toolkit
 * Copyright (C) 2000 - 2003  Virginia Tech
 * 
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License (GPL) as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software, in the top level source directory in a file
 * named "COPYING.GPL"; if not, see it at:
 * http://www.gnu.org/copyleft/gpl.html or write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 */
// This is a simple example that uses the dtkDSO family of C++
// functions, which includes: dtkDSO_load() and dtkDSO_unload().  The
// program "load" will load the DSO file foo.so.  This must be linked
// to dynamic shared object (DSO), base.so, to get the base class
// interfaces to class Foo.  The code for the base class Base must be
// in a DSO (.so) file so that this can load data from it at run time,
// since at link time the data needed is not known.

// Using this method we can load any C++ code.  The dtkManager class
// loads and runs C++ DSOs (DLLs).  Using dtkManager is easyer than
// the code below, but this is more general and can be used to load
// C++ objects that have any base class that the C++ code inherits in
// the loaded DSO file.  The classes dtkManager, dtkSharedMem, and the
// program dtk-server all use this method to load C++ code from DSO
// files.  They just add more code to manage lists of DSOs, compared
// to below.


#include <stdio.h>
#include <dtk.h>

#include "base.h" // defines interface to class Base

int main(void)
{
  // Get the C++ Base loader function.  Note: declarations of function
  // pointers are butt ugly.
  Base *(*loaderFunc)(const char *) =
    (Base * (*)(const char *))
    dtkDSO_load(".", "foo");

  if(!loaderFunc) return 1; // error

  // Get the Base object which is really a Foo object that inherits
  // Base.
  Base *foo = (Base *) loaderFunc("Hello");

  // Use the loaded C++ Foo objects over-written Base method.
  foo->base();

  // Get the Base unloader function.
  int (*unloaderFunc)(Base *) = 
    (int (*)(Base *)) dtkDSO_getUnloader((void *) loaderFunc);

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
