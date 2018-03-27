#include <stdio.h>
#include <dtk.h>

#include <sys/types.h>
#include <unistd.h>

char spin_char(void)
{
  const char str[5] = "|/-\\";
  static int i=0;
  i++;
  i %= 4;
  return str[i];
}

void callback(dtkSharedMem *shm, void *ptr)
{
  float f;
  shm->read(&f);
  printf("we read %f\n", f);
  // sleep until next signal
  pause();
}

int main(void)
{
  printf("pid= %d\n", getpid());

  dtkSharedMem shm("foO");
  if(shm.isInvalid()) return 1; // error

// DTK_WITH_SETWRITESIGNALCATCHER would be defined in dtkSharedMem.h
#ifdef DTK_WITH_SETWRITESIGNALCATCHER

  shm.setWriteSignalCatcher(callback);
  pause();

#else
  
  printf("NOT using dtkSharedMem::setWriteSignalCatcher()\n\n");
  while(1)
    {
      float f;
      shm.read(&f);
      printf("%c read: %f     \r",spin_char(), f);
      fflush(stdout);
      usleep(100000);
    }

#endif

  return 0;
}
