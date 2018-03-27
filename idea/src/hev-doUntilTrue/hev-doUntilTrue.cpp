#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <iris.h>

float timeout = 0 ;

////////////////////////////////////////////////////////////////////////
// parse a line- return true if it has a command which had an exit status of zero
bool parseLine(const std::string& line) 
{
    std::vector<std::string> vec = iris::ParseString(line) ;
    if (vec.size() == 0) return false ;
    dtkMsg.add(DTKMSG_INFO, "hev-doUntilTrue, trying: %s\n",line.c_str()) ;
    int ret = system(line.c_str()) ;
    if (ret ==0) return true ;
    else return false ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // send dtk messages to stdout
    dtkMsg.setFile(stderr) ;

    // now look at stdin, but don't hang if there isn't any data

    std::string line ;
    bool timeout ;
    while (iris::GetLine(&line, 10.f, &timeout))
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-doUntilTrue: line \"%s\"\n",line.c_str()) ;
	bool ret = parseLine(line) ;
	if (ret) return 0 ;
    }
    if (timeout)
    {
	dtkMsg.add(DTKMSG_DEBUG, "hev-doUntilTrue: no data read from stdin\n") ;	
	return 1 ;
    }
    dtkMsg.add(DTKMSG_DEBUG, "hev-doUntilTrue: successfully finished reading stdin without successfully running any commands\n") ;
    return 1 ;
}
