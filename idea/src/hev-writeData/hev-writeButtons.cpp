/* writes buttons to DTK shared memory.

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
	    "Usage: hev-writeButtons [ -c N ] shmName [ button ... ] \n\n"
	    "\tshmName is the name of an existing shared memory file\n\n"
	    "\tbutton is one or moere buttons to be written to shmName\n"
	    "\tIf button is omitted, buttons are read from stdin\n"
	    "\tand written to shmName until EOF\n"
	    "\tif the -c option is specified, the shared memory file\n"
	    "\twill be created, sized to store N buttons\n\n") ;
}

int main(int argc, char **argv) 
{
    
    // send dtkMsg files to stderr
    dtkMsg.setFile(stderr) ;

    dtkSharedMem *shm ;
    int numButtons ;
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
	if (!iris::StringToInt(argv[2], &numButtons))
	{
	    fprintf(stderr, "hev-writeButtons: invalid size\n") ;
	    return 1 ;
	}
	char *shmName = argv[3] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(numButtons*sizeof(unsigned char), shmName) ;
	if(shm->isInvalid()) return 1 ;

	c = 4 ;
    }
    else
    {
	char *shmName = argv[1] ;
	//printf("shmName = %s\n",shmName) ;
	
	shm = new dtkSharedMem(shmName) ;
	if(shm->isInvalid()) return 1 ;

	numButtons = shm->getSize()/sizeof(unsigned char) ;
	c = 2 ;
    }
    
    //printf("shm->getSize() = %d\n",shm->getSize()) ;
    //printf("numButtons = %d\n",numButtons) ;

    unsigned char data[numButtons] ;
    //printf("size of data = %d\n",sizeof(data)) ;


    int holder ;
    // a single command line parameter, or read from stdin?
    if (argc>c)
    {
	if (numButtons != argc-c)
	{
	    dtkMsg.add(DTKMSG_ERROR, "hev-writeButtons: wrong number of buttons.  %d needed, %d supplied\n",
		       numButtons, argc-c) ;
	    return 1 ;
	}

	for (unsigned int i=0; i<argc-c; i++)
	{
	    if (!iris::StringToInt(argv[i+c], &holder) || holder<0 || holder>255)
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeButtons: invalid button \"%s\"\n",argv[i+c]) ;
		return 1 ;
	    }
	    data[i] = holder ;
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
	    if (numButtons != numbers.size())
	    {
		dtkMsg.add(DTKMSG_ERROR, "hev-writeButtons: wrong number of buttons.  %d needed, %d supplied\n",
			   numButtons, numbers.size()) ;
		return 1 ;
	    }
	    for (unsigned int i=0; i<numbers.size(); i++)
	    {
		if (!iris::StringToInt(numbers[i], &holder) || holder<0 || holder>255)
		{
		    dtkMsg.add(DTKMSG_ERROR, "hev-writeButtons: invalid button \"%s\"\n",numbers[i].c_str()) ;
		    return 1 ;
		}
		data[i] = holder ;
	    }
	    shm->write(data) ;
	}
    }
}
