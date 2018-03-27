/* zeros a shared memory segment, creating it if it doesn't exist

   John Kelso, 9/04, kelso@nist.gov

 */

#include <dtk.h>
#include <stdlib.h>
#include <string.h>
#ifndef DTK_ARCH_WIN32_VCPP
# include <strings.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#define PROGRAM_NAME "dtk-zeroShm"

static int Usage(char *name) {
  fprintf(stdout,
	  " Usage: "PROGRAM_NAME" SHM_FILE [ SIZE ]\n\n"
	  "  Zeros a DTK shared memory file named SHM_FILE\n"
	  "  whose size is SIZE bytes.\n"
	  "  SIZE can be give as \"-\", or omitted, if the shared\n"
	  "  memory segment already exists.\n") ;
  return 1 ;
}

int main(int argc, char **argv) {
  
  dtkMsg.setPreMessage(argv[0]) ;

  for( int i=1;i<argc;i++ )
  {
    if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
      return Usage(argv[0]);
  }

  if (argc > 3 || argc < 2) 
    return Usage(argv[0]) ;
  
  char *name = argv[1] ;

  int size ;
  bool useExistingSize = false ;
  if ((argc==2) || (!(strcmp(argv[2],"-")))) {
    useExistingSize = true ;
  } else if ((1 != sscanf(argv[2],"%d",&size) || size<1)) {
    dtkMsg.add(DTKMSG_ERROR, "Malformed SIZE\n") ;
    return 1 ;
  }

  dtkSharedMem *shm = NULL ;
  if (useExistingSize) {
    shm = new dtkSharedMem(name) ;
    if(shm->isInvalid()) 
      return 1 ;
    size = shm->getSize() ;
    //printf("using existing size of %d bytes\n",size) ;
  } else {
    shm = new dtkSharedMem(size, name) ;
    if(shm->isInvalid()) 
      return 1 ;
  }
  
  char *zero = (char *) dtk_malloc(size) ;
  memset(zero, 0, size);

  if(shm->write(zero)) 
    return 1 ;
  
  return 0 ;
}
