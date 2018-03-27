#include <stdio.h>
#include <unistd.h>
#include <dtk.h>


int main(int argc, char **argv)
{
  {
    int i=0;
    dtkSharedMem shm(sizeof(int), "foo_int",  &i);
    if(shm.isInvalid()) return 1;// error
    
    while(i < 8)
      {
	sleep(1);
	printf("%s setting foo_int=%d\n", argv[0], ++i);
	if(shm.write(&i)) return 1; // error
      }
    sleep(1);
  }

  dtkSharedMem_destroy("foo_int");
  printf("%s exiting.\n",argv[0]);

  return 0;
}
