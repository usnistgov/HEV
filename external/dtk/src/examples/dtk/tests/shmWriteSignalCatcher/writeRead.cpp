#include <dtk.h>

int main(void)
{
  dtkSharedMem shm(sizeof(int), "foo");
  int count = 0;

  while(count<1000000)
    {
      shm.write(&count);
      shm.read(&count);
      count++;
    }

  return 0;
}
