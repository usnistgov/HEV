#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

#include <limits.h>    /* for PAGESIZE */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

int
main(void)
{
  char *p;
  char c;
  
  /* Allocate a buffer; it will have the default
     protection of PROT_READ|PROT_WRITE. */
  p = (char *) malloc(1024+PAGESIZE-1);
  if (!p) {
    perror("Couldn't malloc(1024)");
    exit(errno);
  }
  
  /* Align to a multiple of PAGESIZE, assumed to be a power of two */
  p = (char *)(((size_t) p + PAGESIZE-1) & ~(PAGESIZE-1));
  
  c = p[666];         /* Read; ok */
  p[666] = 42;        /* Write; ok */
  
#if 1
  /* Mark the buffer read-only. */
  if (mprotect(p, 1024, PROT_READ)) {
    perror("Couldn't mprotect");
    exit(errno);
  }
#endif  

  c = p[666];         /* Read; ok */
  p[666] = 42;        /* Write; program dies on SIGSEGV */
  
  return 0;
}

