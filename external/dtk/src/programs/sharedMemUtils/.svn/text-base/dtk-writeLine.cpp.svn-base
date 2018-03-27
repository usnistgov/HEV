/* writes data to DTK shared memory.  See Usage for details

   John Kelso, 5/04, kelso@nist.gov

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
#include <iostream>
#include <string>

#define PROGRAM_NAME "dtk-writeLine"

static int Usage(char *name) {
  fprintf(stdout,
	 " Usage: "PROGRAM_NAME" SHM_FILE [SIZE] [data]\n\n"
	 "  Write data to the DTK shared memory file named SHM_FILE\n"
	 "  whose size is SIZE bytes.\n"
	 "  SIZE can be give as \"-\" if the shared memory segment\n"
	 "  already exists, or omitted if data are from stdin.\n"
	 "  Data can be specified on the command line, or if none\n"
	 "  are given, via stdin.\n") ;
  return 1 ;
}

// simple wrapper function to see if a string is an integer
bool isInt(char *s, int *i) {
  char *e ;
  long int l = strtol(s, &e, 10) ;
  if (s+strlen(s) == e) {
    *i = (int) l ;
    return true ;
  } else {
    return false ;
  }
}

int main(int argc, char **argv) {

  dtkMsg.setPreMessage(argv[0]) ;

  for( int i=1;i<argc;i++ )
  {
    if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
      return Usage(argv[0]);
  }

  if (argc<2 || argc>4) 
    return Usage(argv[0]) ;
  
  char *name = argv[1] ;

  int size = 0 ;
  bool useExistingSize = false ;
  if (argc==2) {
    useExistingSize = true ;
  } else if (!(strcmp(argv[2],"-"))) {
    useExistingSize = true ;
  } else if (!isInt(argv[2],&size) || size<1) {
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
  
  if (argc==4) { // read from command line if supplied
    int datalen = strlen(argv[3])+1 ;
    if (datalen > size) {
    dtkMsg.add(DTKMSG_ERROR,
	      "data size, %d, is bigger than "
	      "shared memory size, %d\n",
	       datalen, size) ;
      return 1 ;
    }
    if(shm->write(argv[3])) 
      return 1 ;
    
  } else {      // read from stdin
    std::string line ;
    while (getline(std::cin, line)>0) {
      int datalen = strlen(line.c_str())+1 ;
      if (datalen > size) {
	dtkMsg.add(DTKMSG_ERROR,
		   "data size, %d, is bigger than "
		   "shared memory size, %d\n",
		   datalen, size) ;
	return 1 ;
      }
      //printf("%s\n", line) ;
      if(shm->write(line.c_str())) 
	return 1 ; 
    }
  }
  return 0 ;
}
