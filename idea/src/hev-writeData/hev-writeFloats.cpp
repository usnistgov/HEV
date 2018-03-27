/* writes floats to DTK shared memory.

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
	    "Usage: hev-writeFloats [ -c N ] shmName [ float ... ] \n\n"
	    "\tshmName is the name of an existing shared memory file\n\n"
	    "\tfloat is one or moere floats to be written to shmName\n"
	    "\tIf float is omitted, floats are read from stdin\n"
	    "\tand written to shmName until EOF\n"
	    "\tif the -c option is specified, the shared memory file\n"
	    "\twill be created, sized to store N floats\n\n") ;
}

int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    dtkSharedMem *shm ;
    int numFloats ;
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
	if (!iris::StringToInt(argv[2], &numFloats))
	{
	    fprintf(stderr, "hev-writeFloats: invalid size\n") ;
	    return 1 ;
	}
	char *shmName = argv[3] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(numFloats*sizeof(float), shmName) ;
	if(shm->isInvalid()) return 1 ;

	c = 4 ;
    }
    else
    {
	char *shmName = argv[1] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(shmName) ;
	if(shm->isInvalid()) return 1 ;

	numFloats = shm->getSize()/sizeof(float) ;
	c = 2 ;
    }
    
    //printf("shm->getSize() = %d\n",shm->getSize()) ;
    //printf("numFloats = %d\n",numFloats) ;

    float data[numFloats] ;
    //printf("size of data = %d\n",sizeof(data)) ;


    // a single command line parameter, or read from stdin?
    if (argc>c)
    {
	if (numFloats != argc-c)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-writeFloats: wrong number of floats.  %d needed, %d supplied\n",
		       numFloats, argc-c) ;
	    return 1 ;
	}
	for (unsigned int i=0; i<argc-c; i++)
	{
	    if (!iris::StringToFloat(argv[i+c], data+i))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeFloats: invalid float \"%s\"\n",argv[i+c]) ;
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
	    if (numFloats != numbers.size())
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeFloats: wrong number of floats.  %d needed, %d supplied\n",
			   numFloats, numbers.size()) ;
		return 1 ;
	    }
	    for (unsigned int i=0; i<numbers.size(); i++)
	    {
		if (!iris::StringToFloat(numbers[i], data+i))
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-writeFloats: invalid float \"%s\"\n",numbers[i].c_str()) ;
		    return 1 ;
		}
	    }
	    shm->write(data) ;
	}
    }
}
