// This is a simple example that uses the dtkDSO family of functions,
// which includes: dtkDSO_load() and dtkDSO_unload().
// The program `load' will load the DSO file implement.so.

#include <stdio.h>
#include <dtk.h>

#include "interface.h"
#include "loader.h"

class Implementation : public A, public B
{
public:
  Implementation(const char *s);
  int taskA(void);
  int taskB(void);
  virtual ~Implementation(void);
};


Implementation::Implementation(const char *s) :
  A(s), B(s)
{
  (void) printf("calling Implementation::Implementation(\"%s\")\n",s);

}

Implementation::~Implementation(void)
{
  (void) printf("calling Implementation::~Implementation()\n");
}

int Implementation::taskA(void)
{
  return printf("calling Implementation::taskA()\n");
}

int Implementation::taskB(void)
{
  return printf("calling Implementation::taskB()\n");
}


// C++ loader and unloader functions.

static A *loader(const char *str)
{
  return new Implementation(str);
}

static int unloader(A *i)
{
  delete i;
  return 0;
}
