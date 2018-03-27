// class Bar is the base class for class Foo in fooDSO.C.  It does not
// have to be pure virtual.  The example program load links with this.
#include <stdio.h>

#include "barDSO.h"


Bar::Bar(const char *s)
{
  printf("calling Bar::Bar(\"%s\")\n", s);
}

Bar::~Bar(void)
{
  printf("calling Bar::~Bar()\n");
}

