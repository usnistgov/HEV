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

static void segv_handler(int signal_number) 
{
  printf("read: %d\n", *memory);
  pause();
} 

int main(void)
{
  int fd;
#if 1
  signal(SIGSEGV, segv_handler);
#else
  struct sigaction sa;

  /* Install segv_handler as the handler for SIGSEGV. */
  memset(&sa, 0, sizeof (sa));
  sa.sa_handler = &segv_handler;
  sigaction (SIGSEGV, &sa, NULL);

#endif

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

  *memory = 333;

  if(mprotect(memory, alloc_size, PROT_READ))
    {
      perror("mprotect() failed\n");
      return 1;
    }


  pause();
  
  return 0;
}
