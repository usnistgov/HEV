/*

  an existing DTK shared memory file is passed in
  
  in a loop:

  1) compares the current shared memory file to its contents the last loop,
     and if they are different, writes the new contents as binary data to
     stdout

  2) reads binary data from stdin the size of the shared memory file,
     compares it to a DTK shared memory file, and if it different updates
     the shared memory file and the stored contents from the last frame

  other options to control the loop speed, nor read or not write

  dtkmsg debug option to write the data to stderr

 */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <dtk.h>
#include <iris/Utils.h>

bool running = true ;
////////////////////////////////////////////////////////////////////////
void cleanup(int)
{
    running = false ;
    iris::Signal(SIG_DFL) ;
}
////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"usage: iris-readStdinUpdateShmWriteStdout [--usleep t] [--noread] [--nowrite] fileName\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc < 2 || argc > 6)
    {
	usage() ;
	return 1 ;
    }

    // call this when exiting
    iris::Signal(cleanup) ;

    int ticks = iris::GetUsleep() ;
    bool noread = false ;
    bool nowrite = false ;

    int c = 1 ;
    while (c<argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (iris::IsSubstring("--usleep",argv[c],4))
	{
	    c++ ;
	    if (c<argc && !iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-readStdinUpdateShmWriteStdout: invalid usleep value\n") ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (iris::IsSubstring("--noread",argv[c],5))
	{
	    c++ ;
	    noread = true ;
	}
	else if (iris::IsSubstring("--nowrite",argv[c],5))
	{
	    c++ ;
	    nowrite = true ;
	}
	else
	{
	    usage() ;
	    return 1 ;
	}
    }

    if (c+1 != argc)
    {
	usage() ;
	return 1 ;
    }

    char* shmName = argv[c] ;

    dtkSharedMem* shm = new dtkSharedMem(shmName) ;
    if (shm->isInvalid()) return 1 ;
    
    size_t shmSize = shm->getSize() ;
    
    dtkMsg.add(DTKMSG_INFO, "iris-readStdinUpdateShmWriteStdout: using shared memory file \"%s\", size %d\n",shmName, shmSize) ;
    
    // use to see if shared memory changed
    char lastShmData[shmSize] ;

    // bool always write the first one
    bool first = true ;

    // reads in shm data
    char shmData[shmSize] ;
    
    // reads in stdin
    char inData[shmSize] ;

    // make stdin read non blocking, and don't write stdout until it's possible
    fd_set rfds;
    fd_set wfds;
    // no wait, just poll
    struct timespec tv;
    int retval ;

    while (running)
    {
	
	shm->read(shmData) ;
	if (first || memcmp(lastShmData,shmData,shmSize))
	{
	    first = false ;
	    if (dtkMsg.isSeverity(DTKMSG_DEBUG))
	    {
		dtkMsg.add(DTKMSG_DEBUG, "iris-readStdinUpdateShmWriteStdout: shared memory file \"%s\" externally updated\n", shmName) ;
	    }
	    memcpy(lastShmData,shmData,shmSize) ;
	    if (!nowrite)
	    {
		FD_ZERO(&wfds);
		FD_SET(1, &wfds);
		tv.tv_sec = 0 ;
		tv.tv_nsec = 0 ;
		retval = pselect(2, NULL, &wfds, NULL, &tv, NULL);
		if (retval>0)
		{
		    if (fwrite(shmData, shmSize, 1, stdout) != 1) dtkMsg.add(DTKMSG_DEBUG, "iris-readStdinUpdateShmWriteStdout: error writing %d bytes of data to stdout\n",shmSize) ;
		    fflush(stdout) ;
		}
		else if (retval < 0)
		{
		    dtkMsg.add(DTKMSG_ERROR, "iris-readStdinUpdateShmWriteStdout: %s\n",strerror_r(errno,NULL,0)) ;
		}
	    }
	}
	else if (!noread)
	{
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
	    tv.tv_sec = 0 ;
	    tv.tv_nsec = 0 ;
	    retval = pselect(1, &rfds, NULL, NULL, &tv, NULL);
	    if (retval > 0)
	    {
		if (fread(inData, shmSize, 1, stdin)<0) dtkMsg.add(DTKMSG_DEBUG, "iris-readStdinUpdateShmWriteStdout: error reading %d bytes of data from stdin\n",shmSize) ;
		else if (memcmp(inData,shmData,shmSize)) 
		{
		    shm->write(inData) ;
		    memcpy(lastShmData,shmData,shmSize) ;
		    if (dtkMsg.isSeverity(DTKMSG_DEBUG))
		    {
			dtkMsg.add(DTKMSG_DEBUG, "iris-readStdinUpdateShmWriteStdout: shared memory file \"%s\" updated from stdin\n", shmName) ;
		    }
		}
	    }
	    else if (retval < 0)
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-readStdinUpdateShmWriteStdout: %s\n",strerror_r(errno,NULL,0)) ;
	    }
	}
	
	

	usleep(ticks) ;
    }
    return 0 ;
}
