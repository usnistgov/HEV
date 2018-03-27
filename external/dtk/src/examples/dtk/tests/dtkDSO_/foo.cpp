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
// This is a simple example that uses the dtkDSO family of functions,
// which includes: dtkDSO_load() and dtkDSO_unload().
// The program dtkDSO_load will load the DSO (DLL) file foo.so (.dll).

#include <stdio.h>
#include <dtk.h>

#include "base.h" // defines interface to class Base

class Foo : public Base
{
public:
  Foo(const char *s);
  int base(void);
  virtual ~Foo(void);
};


Foo::Foo(const char *s) : Base(s)
{
  (void) printf("calling Foo::Foo(\"%s\")\n",s);
}

Foo::~Foo(void)
{
  (void) printf("calling Foo::~Foo()\n");
}

int Foo::base(void)
{
  return printf("calling Foo::base()\n");
}



static Base *loader(const char *str)
{
  return new Foo(str);
}

static int unloader(Base *base)
{
  delete base;
  return 0;
}


extern "C"
{
  // C functions that return a pointer to a C++ function.  The
  // dtkDSO_*load() functions are hard coded to look for these C
  // functions.  There are not C++ functions so that the names don't
  // get mangled.
#ifdef DTK_ARCH_WIN32_VCPP
  extern __declspec(dllexport) 
#endif
	  void *__dtkDSO_loader(void)
  {
    return (void *) loader;
  }
#ifdef DTK_ARCH_WIN32_VCPP
  extern __declspec(dllexport)
#endif
	  void *__dtkDSO_unloader(void)
  {
    return (void *) unloader;
  }
}
