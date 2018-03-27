// simply reads from the indicated fifo and writes to stdout
// this program is responsible for creating the fifo.

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>



static void
usage ()
	{
	fprintf (stderr, "Usage:   hev-readFifo lineLen fifoName\n");
	}  // end of usage

int main (int argc, char **argv)
	{


	// Get lineLength and fifoName from command line arguments

  	if (argc == 1)
        	{
        	usage ();
        	exit (0);
        	}
  	else if (argc != 3)
        	{
        	fprintf (stderr, 
			"hev-readFifo: Bad command line argument count.\n");
        	usage ();
        	exit (-1);
        	}


  	int lineLength;

  	if (sscanf (argv[1], "%d", &lineLength) != 1)
        	{
        	fprintf (stderr, 
			"hev-readFifo: Bad line length parameter: \"%s\".\n",
                	argv[1]);
		usage ();
        	exit (-1);
        	}

  	char *fifoName = argv[2];


	// Note that in hev-readFifo we can block on reading the fifo, 
	// but in other applications it might be advantageous to do 
	// non-blocking reads.  See comments below if you want to 
	// implement with non-blocking reads.

	int fd;
	fd = open(fifoName, O_RDONLY ) ;

	// non-blocking form of open:
	//     fd = open(fifoName, O_RDONLY | O_NONBLOCK) ;

        if (fd == -1)
		{
		fprintf (stderr, 
			"hev-readFifo: Unable to open input FIFO \"%s\".\n", 
			fifoName);
		exit (-1);
		}


	// Allocate buffer for reading from fifo
	char *buf;
	if ( (buf = (char *) malloc (lineLength+1)) == NULL )
		{
		fprintf (stderr, 
		 "hev-readFifo: Unable to allocate memory for input buffer.\n");
		exit (-1);
		}


	int nBytes;

	while (1)
		{
		nBytes = read(fd, buf, lineLength);
		// printf ("nBytes = %d\n", nBytes);

		// When doing non-blocking reads, nBytes could
		// be -1 when there is no error.  I think that
		// nBytes == -1 when there is no process trying
		// to write to the fifo.

		if (nBytes > 0)
    			{
			fprintf (stdout, "%s\n", buf);
			fflush (stdout);
			}
		else if (nBytes < -1)
			{
			perror ("hev-readFifo: Read error ");
			close (fd);
			exit (-1);
			}
#if 0
		// If we were doing non-blocking reads, we might
		// want to sleep for .01 sec (or some other interval)
		// before trying again
		usleep (10000);
#endif
		}
			

 
	// note that there is no way to get here...

	close (fd);
 	exit (0); 

	}  // end of main 
