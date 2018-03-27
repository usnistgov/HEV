/* reads data from DTK shared memory.  See Usage for details

   John Kelso, 9/04, kelso@nist.gov

 */

#include <dtk.h>
#include <dtk/_config.h>
#include <stdlib.h>
#include <string.h>
#ifdef DTK_ARCH_WIN32_VCPP
# include <process.h>
#else
# include <strings.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#define PROGRAM_NAME "dtk-readLine"

void signal_catcher(int sig)
{
  dtkMsg.add(DTKMSG_INFO,
	     "PID %d caught signal %d, starting exit sequence ...\n",
	     getpid(), sig);
  exit(0) ;
}

static int Usage(char *name) {
  fprintf(stdout,
	  " Usage: "PROGRAM_NAME" [-q | --queue] SHM_FILE [SIZE] \n\n"
	  "  Reads data from the DTK shared memory file named SHM_FILE\n"
	  "  and writes to STDOUT.  Assumes the entire shared memory file\n"
	  "  can be read and written as a string.\n"
	  "  if SIZE is specified, the existing shared memory file must be\n"
	  "  SIZE bytes, or will be created if it does not exist.\n"
	  " Options:\n"
	  "  -h | --help  print this uasge information\n"
	  "  -q | --queue flush the queue and perform queued reads in a loop\n") ;
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
  
  if (argc<2 || argc>4) 
    return Usage(argv[0]) ;
  
  bool queued = false ;
  char *name ;
  int size = 0 ;

  for( int i=1;i<argc;i++ )
  {
    if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
      return Usage(argv[0]);
  }

  if (argc==2) { // dtk-readLine SHM_FILE
    name = argv[1] ;
  } else if (argc==4 && // dtk-readLine -q SHM_FILE SIZE
	     (!(strcasecmp(argv[1],"-q")) || 
	      !(strcasecmp(argv[1],"--queue"))) &&
	     (isInt(argv[3],&size))) {
    name = argv[2] ;
    queued = true ;
  } else if (argc==3 && // dtk-readLine -q SHM_FILE
	     (!(strcasecmp(argv[1],"-q")) || 
	      !(strcasecmp(argv[1],"--queue")))) {
    name = argv[2] ;
    queued = true ;
  } else if (argc==3 && // dtk-readLine SHM_FILE SIZE
	     (isInt(argv[2],&size))) {
    name = argv[1] ;
  } else {
    return Usage(argv[0]) ;
  }

  dtkSharedMem *shm = NULL ;
  if (size<=0) {
    shm = new dtkSharedMem(name) ;
    if(shm->isInvalid()) 
      return 1 ;
    size = shm->getSize() ;
  } else {
    shm = new dtkSharedMem(size, name) ;
    if(shm->isInvalid()) 
      return 1 ;
  }

  // holds data to be read from shm
  char *val = (char *) dtk_malloc(size) ;
  int err ;

  if (queued) {
    signal(SIGINT, signal_catcher);
    signal(SIGTERM, signal_catcher);
#ifndef DTK_ARCH_WIN32_VCPP
    signal(SIGKILL, signal_catcher);
#endif
    
    shm->flush() ;
    while (1) {
      memset(val, 0, size);
      int ret = shm->blockingQRead(val) ;
      if (ret>0) {
	if ((err=printf("%s\n", val))<=0) { 
	  dtkMsg.add(DTKMSG_ERROR, "printf returned %d\n",err) ;
	  return 1 ;
	}
      } else if (ret<0) {
	dtkMsg.add(DTKMSG_ERROR, "shm blockingQread returned %d\n",ret) ;
	return 1 ;
      }
    } 

  } else { //single polled read
    memset(val, 0, size);
    if(shm->read(val)) {
      dtkMsg.add(DTKMSG_ERROR, "shm read returned error\n") ;
      return 1 ;
    } else {
      if ((err=printf("%s\n", val))<=0) { 
	dtkMsg.add(DTKMSG_ERROR, "printf returned %d\n",err) ;
	return 1 ;
      }
    }
  }
}
