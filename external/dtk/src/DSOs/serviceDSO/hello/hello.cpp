#include <stdio.h>
#include <string.h>
#include <dtk.h>
#include <dtk/dtkService.h>
#ifdef DTK_ARCH_IRIX // for bzero()
# include <bstring.h>
#endif

#define SHM_SIZE 256

class Hello : public dtkService
{
public:
  Hello(const char *name);
  int serve(void);
private:
  dtkSharedMem shm;
};


Hello::Hello(const char *name):
  shm(SHM_SIZE, (name && name[0])?name:"hello")
{
  if(shm.isInvalid()) return; // failure, fd not set

  printf(" type here ==> "); fflush(stdout);
#ifdef DTK_ARCH_WIN32_VCPP
  fd = stdin;
#else
  fd = STDIN_FILENO; // success, fd set
#endif
}


int Hello::serve(void)
{
  char str[SHM_SIZE];
  memset(str, 0, SHM_SIZE);

  if(fgets(str,SHM_SIZE,stdin) != NULL)
    if(shm.write(str)) return DTKSERVICE_ERROR;

  // Print the next prompt.
  printf(" type here ==> "); fflush(stdout);

  return DTKSERVICE_CONTINUE;
}



static dtkService *dtkDSO_loader(const char *sharedMem_name)
{
  return new Hello(sharedMem_name);
}

static int dtkDSO_unloader(dtkService *hello)
{
  delete hello;
  // Just add a new line for the last type here ==> prompt
  printf("\n");
  return DTKDSO_UNLOAD_CONTINUE; // success
}

