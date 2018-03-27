#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

static int alloc_size = sizeof(int);
static int *memory = NULL;

static void segv_handler(int signal_number) 
{
  printf ("----------------- caught SIGSEGV -- memory accessed\n");
  int prot = PROT_READ|PROT_WRITE;
  if(mprotect(memory, alloc_size, prot))
    {
      perror("mprotect() failed\n");
      exit(1);
    }

  int i;
  for(i=0;i<8;i++)
    {
      printf("read: %d\n", *memory);
      sleep(1);
    }

  exit(0);
} 

int main(void)
{
  int fd;
  signal(SIGSEGV, segv_handler);

  fd = open ("foo", O_RDWR|O_CREAT, 0666);

  if(fd < 0)
    {
      perror("open(\"foo\") failed\n");
      return 1;
    }


  memory = (int *) mmap(NULL, alloc_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

  if((void *) memory == MAP_FAILED)
    {
      perror("mmap() failed\n");
      return 1;
    }

  if(ftruncate(fd, alloc_size))
    {
      perror("ftruncate() failed\n");
      return 1;
    }

  printf("after ftruncate() and before mprotect()\n");

  *memory = 10;
  
  //int prot = PROT_READ|PROT_WRITE;
  int prot = PROT_READ;

  if(mprotect(memory, alloc_size, prot))
    {
      perror("mprotect() failed\n");
      return 1;
    }
  
  printf("after  mprotect()\n");
  
  *memory = 123;
  
  return 0;
}
