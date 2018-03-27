/* writes ints to DTK shared memory.

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
	    "Usage: hev-writeInts [ -c N ] shmName [ int ... ] \n\n"
	    "\tshmName is the name of an existing shared memory file\n\n"
	    "\tint is one or moere ints to be written to shmName\n"
	    "\tIf int is omitted, ints are read from stdin\n"
	    "\tand written to shmName until EOF\n"
	    "\tif the -c option is specified, the shared memory file\n"
	    "\twill be created, sized to store N ints\n\n") ;
}

int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    dtkSharedMem *shm ;
    int numInts ;
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
	if (!iris::StringToInt(argv[2], &numInts))
	{
	    fprintf(stderr, "hev-writeInts: invalid size\n") ;
	    return 1 ;
	}
	char *shmName = argv[3] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(numInts*sizeof(int), shmName) ;
	if(shm->isInvalid()) return 1 ;

	c = 4 ;
    }
    else
    {
	char *shmName = argv[1] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(shmName) ;
	if(shm->isInvalid()) return 1 ;

	c = 2 ;
    }
    
    numInts = shm->getSize()/sizeof(int) ;
    //printf("shm->getSize() = %d\n",shm->getSize()) ;
    //printf("numInts = %d\n",numInts) ;

    int data[numInts] ;
    //printf("size of data = %d\n",sizeof(data)) ;


    // a single command line parameter, or read from stdin?
    if (argc>c)
    {
	if (numInts != argc-c)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-writeInts: wrong number of ints.  %d needed, %d supplied\n",
		       numInts, argc-c) ;
	    return 1 ;
	}
	for (unsigned int i=0; i<argc-c; i++)
	{
	    if (!iris::StringToInt(argv[i+c], data+i))
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeInts: invalid int \"%s\"\n",argv[i+c]) ;
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
	    if (numInts != numbers.size())
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeInts: wrong number of ints.  %d needed, %d supplied\n",
			   numInts, numbers.size()) ;
		return 1 ;
	    }
	    for (unsigned int i=0; i<numbers.size(); i++)
	    {
		if (!iris::StringToInt(numbers[i], data+i))
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-writeInts: invalid int \"%s\"\n",numbers[i].c_str()) ;
		    return 1 ;
		}
	    }
	    shm->write(data) ;
	}
    }
}
