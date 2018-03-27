#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

float timeout = 0 ;

std::string selectorString ;
iris::ShmString selector("idea/selector") ;
bool usingSelector = false ;

std::vector<std::string> onCommands ;
std::vector<std::string> offCommands ;

////////////////////////////////////////////////////////////////////////
// parse a line- return true if it has a command which had an exit status of zero
void parseLine(const std::string& line) 
{
    std::vector<std::string> vec = iris::ParseString(line) ;
    if (vec.size() < 2) return ;
    if (iris::IsSubstring("on", vec[0])) onCommands.push_back(iris::ChopFirstWord(line)) ;
    else if (iris::IsSubstring("off", vec[0])) offCommands.push_back(iris::ChopFirstWord(line)) ;
    else dtkMsg.add(DTKMSG_WARNING, "hev-shmOnOff: ignoring unregnized command \"%s\"\n",line.c_str()) ;
    return;

    
}
////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"Usage: hev-shmOnOff [--usleep t] [--selector string] shm\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;
    
    if (argc != 2 && argc != 4)
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
		dtkMsg.add(DTKMSG_ERROR, "hev-shmOnOff: invalid usleep value\n") ;
		usage() ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (iris::IsSubstring("--selector",argv[c],3))
	{
	    c++ ;
	    if (c<argc) 
	    {
		selectorString = argv[c] ;
		usingSelector = true ;
	    }
	    else 
	    {
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
    
    dtkSharedMem *shm = new dtkSharedMem(argv[c]) ;
    if (shm->isInvalid()) return 1 ;

    shm->queue(256) ;
    shm->flush() ;
    

    int shmSize = shm->getSize() ;
    char* shmData = static_cast<char*>(malloc(shmSize)) ;
    shm->read(shmData) ;
    char* zeroData = static_cast<char*>(malloc(shmSize)) ;
    memset(zeroData, 0, shmSize) ;

#if 0    
    fprintf(stderr,"init: ") ;
    for (int i=0; i<shmSize; i++)
    {
	fprintf(stderr,"%d ",shmData[i]) ;
    }
    fprintf(stderr,"\n") ;
#endif

    bool onOff = memcmp(shmData, zeroData, shmSize) ;

    if (onOff) dtkMsg.add(DTKMSG_INFO, "hev-shmOnOff: initial value of %s is ON\n",argv[1]) ;
    else dtkMsg.add(DTKMSG_INFO, "hev-shmOnOff: initial value of %s is OFF\n",argv[1]) ;

    dtkMsg.add(DTKMSG_DEBUG, "hev-shmOnOff: reading from stdin\n") ;
    std::string line ;
    while (iris::GetLine(&line, 10.f))
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-shmOnOff: line \"%s\"\n",line.c_str()) ;
	parseLine(line) ;
    }

    if (onCommands.size() == 0 && offCommands.size() == 0)
    {
	dtkMsg.add(DTKMSG_ERROR, "hev-shmOnOff: no ON or OFF comands read\n") ;	
	return 1 ;
    }
    
    for (int i=0; i<offCommands.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-shmOnOff: OFF:  %s\n",offCommands[i].c_str()) ;
    }

    for (int i=0; i<onCommands.size(); i++)
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-shmOnOff: ON:  %s\n",onCommands[i].c_str()) ;
    }


    while(1)
    {
	// want to read the buttons no matter who is selected
	while(shm->qread(shmData)>0)
	{
#if 0
	    fprintf(stderr, "new data:") ;
	    for (int i=0; i<shmSize; i++)
	    {
		fprintf(stderr,"%d ",shmData[i]) ;
	    }
	    fprintf(stderr,"\n") ;
#endif
	    bool newOnOff = memcmp(shmData, zeroData, shmSize) ;
	    
	    //fprintf(stderr,"new onOff = %d\n",newOnOff) ;

	    if (usingSelector && selector.getString() != selectorString) continue ;

	    if (onOff && !newOnOff)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "hev-shmOnOff: OFF!\n") ;
		for (int i=0; i<offCommands.size(); i++)
		{
		    printf("%s\n",offCommands[i].c_str()) ;
		}
		fflush(stdout) ;
	    }
	    else if (!onOff && newOnOff)
	    {
		dtkMsg.add(DTKMSG_DEBUG, "hev-shmOnOff: ON!\n") ;
		for (int i=0; i<onCommands.size(); i++)
		{
		    printf("%s\n",onCommands[i].c_str()) ;
		}
		fflush(stdout) ;
	    }
	    onOff = newOnOff ;
	}
	usleep(ticks) ;
    }

    return 0 ;

}
