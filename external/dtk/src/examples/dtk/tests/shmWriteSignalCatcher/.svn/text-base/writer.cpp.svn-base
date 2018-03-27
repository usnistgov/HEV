#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

static int alloc_size;
static int *memory;

int main(void)
{
  int fd;

  //alloc_size = sizeof(int);
  alloc_size = getpagesize();

  //fd = open ("foo", O_RDWR|O_CREAT, 0666);
  fd = shm_open("foo", O_RDWR|O_CREAT, 0666);

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
 
  int i=0;

  while(1)
    {
      sleep(1);
      *memory = ++i;
      printf("wrote: %d\n", i);
    }
  
  return 0;
}
