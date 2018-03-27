// The following example, dtkSharedMem/read.cpp, opens and reads the
// DTK shared memory file "foobar" which is the size of one
// float. First we include two standard header files and the DTK
// header file.

#include <stdio.h>
#include <dtk.h>
#ifndef DTK_ARCH_WIN32_VCPP
#  include <unistd.h>
#endif

int main(void)
{
  // Now we get a DTK shared memory object and initialize it with the
  // value from x if the DTK shared memory file, "foobar", does not
  // exist, or read the current value in x if the DTK shared memory
  // file, "foobar", exists.

  float x = 0.0f;
  dtkSharedMem shm(sizeof(float), "foobar", &x);

  // We check of an error. DTK does not throw exceptions. The
  // dtkSharedMem constructor can fail for many reasons, like if the
  // DTK shared memory file "foobar" exists and is not the requested
  // size.

  if(shm.isInvalid()) return 1; // error

  // We enter the main loop. Reading from DTK shared memory and than
  // write to standard output. If this program reads a float from
  // shared memory larger than or equal to 0.9 this program will exit.

  while(x < 0.9f)
    {
      if(shm.read(&x)) return 1; // error
      printf("%g\n", x);
      sleep(1);
    }
  return 0;
}
