// test program- does async reads from stdin, does writes and flushes to stdout

#include <iris/Utils.h>

static bool running = true ;

////////////////////////////////////////////////////////////////////////
// for catching the KILL, QUIT, ABRT, TERM and INT signals
static void signal_catcher(int sig)
{
    dtkMsg.add(DTKMSG_INFO, "readWrite: PID %d caught signal %d, starting exit sequence ...\n", getpid(), sig);
    running = false ;
}


////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) 
{
    
    iris::Signal(signal_catcher);

    // send messages to stderr
    dtkMsg.setFile(stderr) ;

    // for reading stdin
    std::string line ;
    
    // number of lines read
    int lines = 0 ;
    
    int ticks = iris::GetUsleep() ;

    int argsUsed = 1 ;

    while (argsUsed<argc) 
    {
	if ((argsUsed<argc) && iris::IsSubstring("--usleep",argv[argsUsed],3)) 
	{
	    argsUsed++ ;
	    if (argsUsed>=argc) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "readWrite: missing usleep parameter\n") ;
		return 1 ;
	    } 
	    else if (!(iris::StringToInt(argv[argsUsed], &ticks)) || ticks<0) 
	    {
		dtkMsg.add(DTKMSG_ERROR, "readWrite: invalid ticks parameter, \"%s\"\n",argv[argsUsed]) ;
		return 1 ;
	    } 
	    else 
	    {
		argsUsed++ ;
	    }
	    
	}
	else 
	{
	    dtkMsg.add(DTKMSG_ERROR, "readWrite: invalid option, \"%s\"\n",argv[argsUsed]) ;
	    return 1 ;
	}
    }
    
    while (running) 
    {
	bool ret ; 
	bool to ;
	while ((ret=iris::Getline(&line, float(ticks)/1000000.f, &to)))
	{
	    printf("line %d: %s\n",lines,line.c_str()) ;
	    fflush(stdout) ;
	    lines++ ;
	}
	//printf("ret = %d, to = %d\n",ret, to) ;
    }
    return 0 ;
}
