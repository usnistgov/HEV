#include <stdio.h>
#include <unistd.h>
#include <dtk.h>


int main(int argc, char **argv)
{
  int i = 0;
  dtkSharedMem shm(sizeof(int), "foo_int", &i);

  if(shm.isInvalid()) return 1; // error

  printf("Background this and then run `./runSecond'.\n");

  while(i < 7)
    {
      sleep(1);
      if(shm.read(&i)) return 1; // error
      if(i>1)
	printf("%s     read foo_int=%d\n", argv[0], i);
    }

  printf("%s exiting.\n",argv[0]);

  return 0;
}
