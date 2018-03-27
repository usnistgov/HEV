// Example code using class dtkBase

/* Since the class dtkBase doesn't do much by itself we show it's use
 * with another class.  dtkBase is used in place of exceptions.
 * dtkBase also provides simple class typing.
 */

#include <stdio.h>
#include <dtk.h>

class Foo : public dtkBase
{
public:
  Foo(void);
};

Foo::Foo(void) : dtkBase(0x1234ab)
{
  validate();
}

int main(void)
{
  Foo foo;

  if(foo.isValid()) // isvalid() is from dtkBase
    printf("foo is valid.\n");
  else
    printf("foo is invalid.\n");
  return 0;
}
