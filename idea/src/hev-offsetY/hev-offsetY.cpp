#include <stdio.h>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <signal.h>

#include <dtk.h>

#include <iris.h>

// evil global variables
bool running = true ;
bool active = true ;

char* baseShmName ;
dtkSharedMem* baseShm ;
osg::Matrix base ;
osg::Matrix oldBase ;

char* lengthShmName ;
dtkSharedMem* lengthShm ;
float length[1] ;
float oldLength[1] = {FLT_MAX} ;

char* endpointShmName ;
dtkSharedMem* endpointShm ;
osg::Matrix endpoint ;

void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO, "hev-offsetY: signal_catcher: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
    running = false ;
}

void usage()
{
    fprintf(stderr,"Usage: hev-offsetY [--usleep t] baseShm lengthShm endpointShm\n") ;
}

void update()
{
    baseShm->read(base.ptr()) ;
    lengthShm->read(length) ;

    if (oldBase != base || oldLength[0] != length[0])
    {
	oldBase = base ;
	oldLength[0] = length[0] ;

	endpoint.makeTranslate(osg::Vec3(0.f, length[0], 0.f)) ;
	endpoint.postMult(base) ;
	
	endpointShm->write(endpoint.ptr()) ;
    }
}

int main(int argc, char **argv)
{

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc != 4 && argc != 6)
    {
	usage() ;
	return 1 ;
    }

    int ticks = iris::GetUsleep() ;
    
    int c = 1 ;
    while (c<argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (iris::IsSubstring("--usleep",argv[c],3))
	{
	    c++ ;
	    if (c<argc && !iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-offsetY: invalid usleep value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else 
	{
	    usage() ;
	    return 1 ;
	}
    }

    baseShmName = argv[c] ;
    lengthShmName = argv[c+1] ;
    endpointShmName = argv[c+2] ;

    dtkMsg.add(DTKMSG_INFO, "hev-offsetY: "
	       "baseShmName = \"%s\", lengthShmName = \"%s\", endpointShmName = \"%s\"\n",
	       baseShmName, lengthShmName, endpointShmName) ;
    

    iris::Signal(signal_catcher);

    // get shared memory
    baseShm = new dtkSharedMem(16*sizeof(double), baseShmName) ;
    if (baseShm->isInvalid()) return 1 ;

    lengthShm = new dtkSharedMem(sizeof(length), lengthShmName) ;
    if (lengthShm->isInvalid()) return 1 ;


    endpointShm = new dtkSharedMem(16*sizeof(double), endpointShmName,0) ;
    if (endpointShm->isInvalid()) return 1 ;

    while (running)
    {
	update() ;
	usleep(ticks) ;
    }

    return 0 ;
}
