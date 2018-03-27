/* writes doubles to DTK shared memory.

John Kelso, 2/14/08, kelso@nist.gov

*/

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>
#include <iostream>
#include <string>

#include <iris.h>

void usage()
{
    fprintf(stderr,
	    "Usage: hev-writeDoubles [ -c N ] shmName [ double ... ] \n\n"
	    "\tshmName is the name of an existing shared memory file\n\n"
	    "\tdouble is one or moere doubles to be written to shmName\n"
	    "\tIf double is omitted, doubles are read from stdin\n"
	    "\tand written to shmName until EOF\n"
	    "\tif the -c option is specified, the shared memory file\n"
	    "\twill be created, sized to store N doubles\n\n") ;
}

int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    dtkSharedMem *shm ;
    int numDoubles ;
    int c ;

    if (argc<2)
    {
	usage() ;
	return 1;
    }

    if (!strcmp("-c",argv[1]))
    {
	if (argc<4)
	{
	    usage() ;
	    return 1;
	}
	if (!iris::StringToInt(argv[2], &numDoubles))
	{
	    fprintf(stderr, "hev-writeDoubles: invalid size\n") ;
	    return 1 ;
	}
	char *shmName = argv[3] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(numDoubles*sizeof(double), shmName) ;
	if(shm->isInvalid()) return 1 ;

	c = 4 ;
    }
    else
    {
	char *shmName = argv[1] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(shmName) ;
	if(shm->isInvalid()) return 1 ;

	numDoubles = shm->getSize()/sizeof(double) ;
	c = 2 ;
    }
    
    //printf("shm->getSize() = %d\n",shm->getSize()) ;
    //printf("numDoubles = %d\n",numDoubles) ;

    double data[numDoubles] ;
    //printf("size of data = %d\n",sizeof(data)) ;


    // a single command line parameter, or read from stdin?
    if (argc>c)
    {
	if (numDoubles != argc-c)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-writeDoubles: wrong number of doubles.  %d needed, %d supplied\n",
		       numDoubles, argc-c) ;
	    return 1 ;
	}
	for (unsigned int i=0; i<argc-c; i++)
	{
	    if (!iris::StringToDouble(argv[i+c], data+i))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeDoubles: invalid double \"%s\"\n",argv[i+c]) ;
		return 1 ;
	    }
	}
	shm->write(data) ;
    }
    else
    {
	std::string line ;
	std::vector<std::string> numbers ;
	while (getline(std::cin, line))
	{
	    numbers = iris::ParseString(line) ; 
	    if (numDoubles != numbers.size())
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeDoubles: wrong number of doubles.  %d needed, %d supplied\n",
			   numDoubles, numbers.size()) ;
		return 1 ;
	    }
	    for (unsigned int i=0; i<numbers.size(); i++)
	    {
		if (!iris::StringToDouble(numbers[i], data+i))
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-writeDoubles: invalid double \"%s\"\n",numbers[i].c_str()) ;
		    return 1 ;
		}
	    }
	    shm->write(data) ;
	}
    }
}
