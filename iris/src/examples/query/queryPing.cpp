// example of using a QUERY control command and getting its reponse back

// it sends the QUERY command to standard output and
// it reads the response from the fifo in argv[1]

#include <stdio.h>
#include <string>

#include <iris.h>

int main(int argc, char** argv)
{
    if (argc!=2)
    {
	fprintf(stderr,"Usage: queryPing fifoName\n") ;
	return 1 ;
    }

    iris::FifoReader fifo(argv[1])  ;
    if (!fifo.open())
    {
	fprintf(stderr,"queryPing: can't open fifo\n") ;
	return 1 ;
    }
    
    // send the QUERY to iris-viewer
    printf("QUERY %s PING ABC\n",argv[1]) ;
    // always flush after every QUERY command!
    fflush(stdout) ;

    // get the response back.  The read doesn't block
    std::string line ;
    while(1)
    {
	if (fifo.readLine(&line))
	{
	    fprintf(stderr,"queryPing: \"%s\"\n",line.c_str()) ;
	}
	// hang out a bit
	else usleep(iris::GetUsleep()) ;
    }
}
