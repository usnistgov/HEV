/* writes strings to DTK shared memory.

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
	    "Usage: hev-writeStrings [ -c N ] shmName [ string ] \n\n"
	    "\tshmName is the name of an existing shared memory file\n\n"
	    "\tstring is a text strings to be written to shmName\n"
	    "\tIf string is omitted, strings are read from stdin\n"
	    "\tand written to shmName until EOF\n"
	    "\tif the -c option is specified, the shared memory file\n"
	    "\twill be created, sized to store N characters\n\n") ;
}

int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    dtkSharedMem *shm ;
    int numChars ;
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
	if (!iris::StringToInt(argv[2], &numChars))
	{
	    fprintf(stderr, "hev-writeStrings: invalid size\n") ;
	    return 1 ;
	}
	char *shmName = argv[3] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(numChars*sizeof(char), shmName) ;
	if(shm->isInvalid()) return 1 ;

	c = 4 ;
    }
    else
    {
	char *shmName = argv[1] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(shmName) ;
	if(shm->isInvalid()) return 1 ;

	numChars = shm->getSize()/sizeof(char) ;
	c = 2 ;
    }
    
    // a single command line parameter, or read from stdin?
    std::string data ;
    if (argc>c)
    {
	for (unsigned int i=c; i<argc; i++)
	{
	    data += std::string(argv[i]) ;
	    if (i != (argc-1)) data += " " ;
	}
	if (data.size()+1>numChars)
	{
	    fprintf(stderr, "hev-writeStings: text string too long\n") ;
	    return 1 ;
	}
	shm->write(data.c_str()) ;
    }
    else
    {
	while (getline(std::cin, data))
	{
	    if (data.size()+1>numChars)
	    {
		fprintf(stderr, "hev-writeStings: text string too long\n") ;
		return 1 ;
	    }
	    shm->write(data.c_str()) ;
	}
    }
}
