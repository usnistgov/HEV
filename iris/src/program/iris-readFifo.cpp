// read a single line from the fifo and print it to stdout
#include <sys/types.h>
#include <signal.h>
#include <dtk.h>
#include <iris/FifoReader.h>
#include <iris/Utils.h>

bool running = true ;
////////////////////////////////////////////////////////////////////////
void cleanup(int)
{
    running = false ;
    iris::Signal(SIG_DFL) ;
}
////////////////////////////////////////////////////////////////////////
void usage()
{
    fprintf(stderr,"usage: iris-readFifo [--usleep t] [--unlink] [--once] fifoName\n") ;
}

////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    if (argc < 2 || argc > 6)
    {
	usage() ;
	return 1 ;
    }

    // call this when exiting
    iris::Signal(cleanup) ;

    int ticks = iris::GetUsleep() ;
    bool unlinkFifo = false ;
    bool once = false ;

    int c = 1 ;
    while (c<argc && argv[c][0] == '-' && argv[c][1] == '-')
    {
	if (iris::IsSubstring("--usleep",argv[c],4))
	{
	    c++ ;
	    if (c<argc && !iris::StringToInt(argv[c],&ticks)) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "iris-readFifo: invalid usleep value\n") ;
		return 1 ; 
	    }
	    c++ ;
	}
	else if (iris::IsSubstring("--unlink",argv[c],4))
	{
	    c++ ;
	    unlinkFifo = true ;
	}
	else if (iris::IsSubstring("--once",argv[c],3))
	{
	    c++ ;
	    once = true ;
	}
	else
	{
	    usage() ;
	    return 1 ;
	}
    }
    iris::FifoReader fifo  ;
    fifo.setName(argv[c]) ;
    if (!fifo.open())
    {
	dtkMsg.add(DTKMSG_ERROR, "iris-readFifo: can't open fifo\n") ;
	return 1 ;
    }
    fifo.unlinkOnExit(unlinkFifo) ;
    std::string line ;
    while (running)
      {
	if (fifo.readLine(&line))
	{
	    printf("%s\n",line.c_str()) ;
	    if (once) running = false ;
	}
	else usleep(ticks) ;
      }
    return 0 ;
}
