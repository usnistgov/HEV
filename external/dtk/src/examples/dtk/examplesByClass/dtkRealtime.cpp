// This example uses the set of functions (class) dtkRealtime_*() with
// the dtkTime class.

#include <dtk.h>

int main(void)
{
  dtkTime T;
  dtkRealtime_init(0.2); // 0.01 may be the smallest usable

  long double t = T.get();
  while(t< ((long double) 3.0))
    {
      printf("time = %Lf seconds\n",t = T.get());
      // Sleep until next 0.1 seconds.
      dtkRealtime_wait();
    }

  dtkRealtime_close();
}
