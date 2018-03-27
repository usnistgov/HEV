// This is a simple example that uses the dtkDSO family of functions,
// which includes: dtkDSO_load() and dtkDSO_unload().
// The program load will load the DSO file fooDSO.so.

#include <stdio.h>
#include <dtk.h>

extern char girlfriend[128];

#include "barDSO.h" // defines interface to class Bar

class Foo : public Bar
{
public:
  Foo(const char *s);
  int bar(void);
  virtual ~Foo(void);
};


Foo::Foo(const char *s) : Bar(s)
{
  (void) printf("calling Foo::Foo(\"%s\")\n",s);
  (void) printf("girlfriend = \"%s\"\n",girlfriend);
}

Foo::~Foo(void)
{
  (void) printf("calling Foo::~Foo()\n");
}

int Foo::bar(void)
{
  return printf("calling Foo::bar()\n");
}



static Bar *loader(const char *str)
{
  return new Foo(str);
}

static int unloader(Bar *bar)
{
  delete bar;
  return 0;
}


extern "C"
{
  // C functions that return a pointer to a C++ function.  The
  // dtkDSO_*load() functions are hard coded to look for these C
  // functions.  These are not C++ functions so that the names don't
  // get mangled.

  void *__dtkDSO_loader(void)
  {
    return (void *) loader;
  }
  void *__dtkDSO_unloader(void)
  {
    return (void *) unloader;
  }
}
