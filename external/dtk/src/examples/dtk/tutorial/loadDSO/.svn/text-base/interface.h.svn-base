// These are the base classes for class Implementation in
// implement.cpp.

class A
{
public:
  A(const char *s)
  {
    printf("calling A::A(\"%s\")\n", s);
  }

  virtual int taskA(void)
  {
    return printf("calling A::taskA()\n");
  }

  virtual ~A(void)
  {
    printf("calling A::~A()\n");
  }

};

class B
{
  public:
  B(const char *s)
  {
    printf("calling B::B(\"%s\")\n", s);
  }

  virtual int taskB(void) = 0;
  virtual ~B(void)
  {
    printf("calling B::~B()\n");
  }

};

