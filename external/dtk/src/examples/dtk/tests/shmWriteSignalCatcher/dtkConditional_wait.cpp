#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dtk.h>

int main(int argc, char **argv)
{

#ifdef _DTK_USE_SYSTEM_V
  int pid = (int) getpid();
  dtkConditional cond;

  if(argc > 2)
    {
      cond.key = atoi(argv[1]);
      cond.id  = atoi(argv[2]);
      cond.magic_number = DTKCONDITIONAL_TYPE; 
      if(dtkConditional_connect(&cond)) return 1;
    }
  else
    {
      if(dtkConditional_create(&cond)) return 1;
      printf("got key id = %d %d\n",cond.key, cond.id);
    }


  int i = 0;
  while(1)
    {
      printf("%d waiting(%d)      \r", pid, i++);
      fflush(stdout);
      dtkConditional_wait(&cond);
    }

#endif

  return 0;
}
