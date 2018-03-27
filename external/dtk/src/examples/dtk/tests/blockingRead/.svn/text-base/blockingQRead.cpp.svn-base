// Example using dtkSharedMem::blockingQRead() and dtk_catchSignal() //

#include <stdio.h>
#include <signal.h>
#include <dtk.h>

int main(void)
{
  int is_running = 1;
  // Catch signals (cntl-c) and unset is_running flag. Helps keep
  // shared memory from getting in a deadlocked or simular hosed
  // state.
#ifdef DTK_ARCH_WIN32_VCPP
  const int signals[4] = { SIGINT, SIGTERM, 0 };
#else
  const int signals[4] = { SIGINT, SIGTERM, SIGQUIT, 0 };
#endif
  dtk_catchSignal(signals, &is_running);

  dtkSharedMem shm(sizeof(float), "foO");
  if(shm.isInvalid()) return 1; // error

  while(is_running)
    {
      float f;
      if(shm.blockingQRead(&f) < 0) return 1; // error
      if(is_running) printf("read %f\n", f);
    }

  return 0;
}
