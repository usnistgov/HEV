#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

float timeout = 0 ;

////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-clicksToToggle [--usleep t] clickShm toggleShm\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;
    
    if (argc != 3 && argc != 5)
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
		dtkMsg.add(DTKMSG_ERROR, "hev-clicksToToggle: invalid usleep value\n") ;
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
    
    dtkSharedMem *clickShm = new dtkSharedMem(argv[c]) ;
    if (clickShm->isInvalid()) return 1 ;

    clickShm->queue(256) ;
    clickShm->flush() ;
    
    int clickShmSize = clickShm->getSize() ;
    // [0] = off state, [1] = on state
    char* clickShmDataOff = static_cast<char*>(malloc(clickShmSize)) ;
    char* clickShmDataOn = static_cast<char*>(malloc(clickShmSize)) ;
    char* clickShmData[2] ;
    clickShmData[0] = clickShmDataOff ;
    clickShmData[1] = clickShmDataOn ;
    clickShm->read(clickShmData[0]) ;
    // count clicks
    int clicks = 0 ;

    dtkSharedMem *toggleShm = new dtkSharedMem(clickShmSize,argv[c+1]) ;
    if (toggleShm->isInvalid()) return 1 ;

    // initialize toggle output with initial state of input
    toggleShm->write(clickShmData[0]) ;

#if 0   
    fprintf(stderr,"init: ") ;
    for (int i=0; i<clickShmSize; i++)
    {
	fprintf(stderr,"%d ",clickShmData[0][i]) ;
    }
    fprintf(stderr,"\n") ;
#endif

    
    // keep track if toggled on or off
    clicks++ ;
    bool toggleOn = false ;

    while(1)
    {
	while(clickShm->qread(clickShmData[clicks])>0)
	{
#if 0
	    fprintf(stderr, "new data: clicks = %d: ",clicks) ;
	    for (int i=0; i<clickShmSize; i++)
	    {
		fprintf(stderr,"  %d ",clickShmData[clicks][i]) ;
	    }
	    fprintf(stderr,"\n") ;
#endif
	    clicks = (clicks+1)%2 ;
	    // received second click and not on
	    if (clicks == 1)
	    {
		if (toggleOn)
		{
#if 0
		    fprintf(stderr,"toggle off, writing ") ;
		    for (int i=0; i<clickShmSize; i++)
		    {
			fprintf(stderr,"  %d ",clickShmData[0][i]) ;
		    }
		    fprintf(stderr,"\n") ;
#endif
		    toggleShm->write(clickShmData[0]) ;
		}
		else
		{
#if 0
		    fprintf(stderr,"toggle on, writing ") ;
		    for (int i=0; i<clickShmSize; i++)
		    {
			fprintf(stderr,"  %d ",clickShmData[1][i]) ;
		    }
		    fprintf(stderr,"\n") ;
#endif
		    toggleShm->write(clickShmData[1]) ;
		}
		toggleOn = !toggleOn ;
	    }

	}
	usleep(ticks) ;
    }

    return 0 ;

}
