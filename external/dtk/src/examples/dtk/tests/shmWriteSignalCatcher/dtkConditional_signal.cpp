#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <dtk.h>

void Sleep(unsigned int secs)
{
  unsigned int i=0;
  printf("sleeping(%d) ", secs);
  fflush(stdout);
  for(; i<secs; i++)
    {
      printf(".");
      fflush(stdout);
      sleep(1);
    }
  printf("\n");
}
    

int main(int argc, char **argv)
{

#ifdef _DTK_USE_SYSTEM_V
  //unsigned int secs = 9;
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


  while(1)
    {
      printf("<enter> to signal:  \r");
      fflush(stdout);
      getchar();
      dtkConditional_signal(&cond);
    }

#endif

  return 0;
}
