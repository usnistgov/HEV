/* prints the size of shared memory segment
   
   John Kelso, 9/04, kelso@nist.gov

*/

#include <dtk.h>
#include <stdlib.h>
#include <string.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <strings.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#define PROGRAM_NAME "dtk-getSize"

static int Usage(char *name) {
  fprintf(stdout,
	  " Usage: "PROGRAM_NAME" SHM_FILE\n\n"
	  "  Prints the size of a shared memory file named SHM_FILE\n") ;
  return 1 ;
}

int main(int argc, char **argv) {
  
  dtkMsg.setPreMessage(argv[0]) ;

  for( int i=1;i<argc;i++ )
  {
    if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
      return Usage(argv[0]);
  }

  if (argc != 2) 
    return Usage(argv[0]) ;
  
  char *name = argv[1] ;
  
  int size ;

  dtkSharedMem *shm = NULL ;
  shm = new dtkSharedMem(name) ;
  if(shm->isInvalid()) 
    return 1 ;
  size = shm->getSize() ;
  
  printf("%d\n",size) ;

  return 0 ;
}
