#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

int main()
{  
  sem_t *sem = sem_open("semaphore0", O_CREAT, 0644, 0);
  if (sem == SEM_FAILED)
    {
      perror("sem_open() failed");
    }
  
  return 0;
}
