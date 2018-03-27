/* reads DATA from DTK shared memory.

Original: John Kelso, 1/16/08, kelso@nist.gov

Minor modifications and and rename to hev-readButtons
by Steve Satterfield, steve@nist.gov

Ported to IRIS 4/11 by kelso

*/
#include <iris.h>

int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    if (argc<2 || argc>7)
    {
	usage() ;
	return 1;
    }
    
    bool discardDups = false ;
    bool printInitValue = false ;
    int howMany = -1 ; // this means go forever
    char *shmName ;
    int ticks = iris::GetUsleep() ;

    unsigned int i=1 ;
    while (i<argc-1)
    {
	if (!strcmp(argv[i],"-d"))
	{
	    discardDups=true ;
	    //printf("discardDups=true\n") ;
	}
	else if (!strcmp(argv[i],"-i"))
	{
	    printInitValue=true ;
	    //printf("printInitValue=true\n") ;
	}
	else if (!strcmp(argv[i],"-u"))
	{
	    i++ ;
	    if (!iris::StringToInt(argv[i],&ticks) || ticks<0) 
	    {
		usage() ;
		return 1 ; 
	    }	
	}
	else if (!strncmp(argv[i],"-",1))
	{
	    if (iris::StringToInt(&(argv[i][1]), &howMany))
	    {
		if (howMany<=0)
		{
		    usage() ;
		    return 1 ;
		}
		//printf("howMany = %d\n",howMany) ;
	    }
	    else
	    {
		usage() ;
		return 1 ;
	    }
	}
	else
	{
	    usage() ;
	    return 1 ;
	}
	i++ ;
    }

    shmName = argv[argc-1] ;
    //printf("shmName = %s\n",shmName) ;
    
    dtkSharedMem *shm = new dtkSharedMem(shmName, DTK_CONNECT) ;
    if(shm->isInvalid()) return 1 ;

    size_t size = shm->getSize() ;

    DATA *data = (DATA*) malloc(size) ;
    DATA *oldData ;
    if (discardDups) oldData = (DATA*) malloc(size) ;
    
    if (printInitValue)
    {
	shm->read(data) ;
	print(data, size) ;
	if (howMany>0) howMany-- ;
	if (discardDups) memcpy(oldData, data, size) ;
    }
    fflush(stdout) ;
	
    shm->queue(256) ;

    // you might want to play with this and 
    // see if you want to flush old data or not
    shm->flush() ;
    
    while (howMany==-1 || howMany>0)
    {
	while (shm->qread(data)>0)
	{
	    if (howMany>0) howMany-- ;
	    if (!discardDups || memcmp(oldData, data, size))
	    {
		print(data, size) ;
		fflush(stdout) ;
		if (discardDups) memcpy(oldData, data, size) ;
	    }
	}
	usleep(ticks) ;
    }
}
