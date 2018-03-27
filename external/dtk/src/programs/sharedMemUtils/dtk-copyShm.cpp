/* copies data between DTK shared memory files.  See Usage for details

   John Kelso, 5/04, kelso@nist.gov

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
#ifdef DTK_ARCH_WIN32_VCPP
# include <winsock2.h> // for struct timeval
# define timespec timeval
#else
# include <time.h>
#endif

#define PROGRAM_NAME "dtk-copyShm"

static int Usage(char *name) {
  fprintf(stdout,
	  " Usage: "PROGRAM_NAME" [ -t TIME ] FROM_SHM_FILE TO_SHM_FILE \n\n"
	  "  Copy data from the shared memory file named FROM_SHM_FILE\n"
	  "  to the shared memory file named TO_SHM_FILE, creating\n"
	  "  TO_SHM_FILE if it doesn't exist.\n"
	  "  If both FROM_SHM_FILE and TO_SHM_FILE exist, they must\n"
	  "  both be the same size.\n"
	  "  If the -t option is given, copies occur in a loop, every\n"
	  "  TIME seconds.  TIME is a non-negative floating point number\n"
	  "  After the initial copy, copies are only done if FROM_SHM_FILE\n"
	  "  has been written to.  Queued reads are performed, so no new\n"
	  "  data will be lost, but data queued before "PROGRAM_NAME" is run\n"
	  "  will not be copied.\n") ;
  return 1 ;
}

int main(int argc, char **argv) {

#define DEBUG 
//#define POLL

#ifdef POLL
  const bool poll = true ;
#else
  const bool poll = false ;
#endif

#ifdef DEBUG
  const bool debug = true ;
#else
  const bool debug = false ;
#endif

  dtkMsg.setPreMessage(argv[0]) ;
  
  for( int i=1;i<argc;i++ )
  {
    if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
      return Usage(argv[0]);
  }

  if (argc!=3 && argc!=5) 
    return Usage(argv[0]) ;
  
  // true if looping
  bool loop ;
  
  // loop time
  float ltime ;
  
  // names of shm segments
  char *fromName, *toName ;
  
  // parse parameters
  if (!(strcmp(argv[1],"-t"))) {
    loop = true ;
    if ((1 != sscanf(argv[2],"%f",&ltime) || ltime<0.f)) {
      dtkMsg.add(DTKMSG_ERROR, "Invalid loop time, %s\n",argv[2]) ;
      return 1 ;
    }
    fromName = strdup(argv[3]) ;
    toName = strdup(argv[4]) ;
  } else {
    loop = false ;
    fromName = strdup(argv[1]) ;
    toName = strdup(argv[2]) ;
  }
  
  if (debug) {
    printf("copying \"%s\" to \"%s\", ",fromName, toName) ;
    if (loop)
      printf("looping every %f seconds\n",ltime) ;
    else
      printf("just once\n") ;
  }
  
  // get "from" buffer, assume it already exists, so don't pass a size
  dtkSharedMem *fromShm ;
  fromShm = new dtkSharedMem(fromName) ;
  if (fromShm->isInvalid()) 
    return 1 ;
  int size = fromShm->getSize() ;
  
  if (debug)
    printf("from is %d bytes in size\n",size) ;
  
  // get "to" buffer- pass size so it'll work whether or not it already exists
  dtkSharedMem *toShm ;
  toShm = new dtkSharedMem(size, toName) ;
  if (toShm->isInvalid()) 
    return 1 ;
  
  if (fromShm->writeCount())
    return 1 ;
  if (!poll) {
    if (debug)
      printf("turning on queueing\n") ;
    if (fromShm->queue())
      return 1 ;
  }
  
  // stores data to be copied
  char *buffer = (char *) dtk_malloc(size) ;
  
  // holds write counts when polling
  u_int64_t oldWriteCount ;
  u_int64_t writeCount ;

  if (fromShm->read(buffer)) 
    return 1 ;
  // get write count after reading new data
  oldWriteCount = fromShm->getWriteCount() ;
  writeCount = oldWriteCount ;
  if (debug)
    printf("initial write count = %d\n", (int)oldWriteCount) ;
  
  // special code for a new shared memory segment!  If the segment is new
  // and hasn't had writeCount turned on yet, the write count won't be
  // incremented after the first write, as the writeCount value of 0 is the
  // same for when it's not on as for after the first write.  A future
  // release of DTK will have a isWriteCounted() method and we can be a
  // little smarter about this
  if (oldWriteCount == 0) {
    if (debug)
      printf("writing back data since writeCount == 0\n") ;
    fromShm->write(buffer) ;
  }
  
  // holds sleep time
  timespec snooze ;
  if (ltime>0.f) {
    snooze.tv_sec = (time_t) ltime ;
#ifdef DTK_ARCH_WIN32_VCPP
    snooze.tv_usec = (long) ((ltime - (int) ltime)*1000000.f) ;
#else
    snooze.tv_nsec = (long) ((ltime - (int) ltime)*1000000000.f) ;
#endif
  }
  
  while (1) {
    if (poll) {
      if (fromShm->read(buffer)) 
	return 1 ;
      writeCount = fromShm->getWriteCount() ;
      if (debug)
	printf("new write count = %d\n", (int)writeCount) ;
      
      if (writeCount != oldWriteCount) {
	if (debug)
	  printf("copying\n") ;
	
	if (toShm->write(buffer))
	  return 1 ;
	oldWriteCount = writeCount ;
      }
    } else {  // queueing
      int n ;
      while ((n=fromShm->qread(buffer))>0) {
	if (debug)
	  printf("%d items to copy\n",n) ;
	
	if (toShm->write(buffer))
	  return 1 ;
      }
    }
    // bail if not looping
    if (!loop) 
      break ;
    if (ltime>0.f)
#ifdef DTK_ARCH_WIN32_VCPP
      Sleep(snooze.tv_usec/1000.0f);
#else
      nanosleep(&snooze,NULL);
#endif

    if (debug)
      printf("checking\n") ;
    
  }
  return 0 ;
}
