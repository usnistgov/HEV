// Example using dtkSharedMem::blockingRead() and dtk_catchSignal() //

#include <stdio.h>
#include <signal.h>
#include <dtk.h>

int main(void)
{
  int is_running = 1;
  // Catch SIGINT (cntl-c) and unset is_running flag. Helps keep
  // shared memory from getting in a deadlocked or simular hosed
  // state.
  dtk_catchSignal(SIGINT, &is_running);

  dtkSharedMem shm(sizeof(float), "foO");
  if(shm.isInvalid()) return 1; // error

  while(is_running)
    {
      float f;
      if(shm.blockingRead(&f)) return 1; // error
      if(is_running) printf("read %f\n", f);
    }

  return 0;
}
