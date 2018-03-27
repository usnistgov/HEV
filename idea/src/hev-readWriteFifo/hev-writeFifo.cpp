// simply reads from stdin and writes to the indicated fifo 
// this program is responsible for creating the fifo.

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#define MIN(x,y)   (((x)<(y))?(x):(y))


static void
usage ()
	{
	fprintf (stderr, "Usage:   hev-writeFifo lineLen fifoName\n");
	}  // end of usage


int main (int argc, char **argv)
{

  if (argc == 1)
	{
	usage ();
	exit (0);
	}
  else if (argc != 3)
	{
	fprintf (stderr, "hev-writeFifo: Bad command line argument count.\n");
	usage ();
	exit (-1);
	}


  // Get the lineLength and fifoName from the command line args.

  int lineLength;

  if (sscanf (argv[1], "%d", &lineLength) != 1)
	{
	fprintf (stderr, "hev-writeFifo: Bad line length parameter: \"%s\".\n",
		argv[1]);
	usage ();
	exit (-1);
	}

  char *fifoName = argv[2];


  // allocate a buffer with size lineLength
  char *lineBuf;
  if ( (lineBuf = (char *) malloc (lineLength+1)) == NULL )
                {
                fprintf (stderr,
                 	"hev-writeFifo: "
			"Unable to allocate memory for output buffer.\n");
                exit (-1);
                }



	
  // See if the fifo already exists
  struct stat buf ;
  int ret = stat(fifoName, &buf) ;
  // does file exist but isn't a fifo?
  if (ret == 0)
    {
      if (! (S_ISFIFO(buf.st_mode)))
	{
	  fprintf(stderr,
		"hev-writeFifo: file %s exists, but isn't a fifo.  Exiting.\n",
		  fifoName) ;
	  exit (-1) ;
	}
    }
  else
    {
      // If it doesn't exist, then create it as a fifo
      ret = mkfifo(fifoName, 0777);
      if (ret != 0)
	{
	  perror("hev-writeFifo, mkfifo: ");
	  exit (-1) ;
	}
    }
  
  // Open the fifo just like any other file
  int fd = open(fifoName, O_WRONLY) ;


  // loop over input lines from stdin
  std::string line ;
  while (getline(std::cin, line))
    {


	// I'm still not that comfortable with C++ strings so I'll do
	// it in C style

	// move it into an array of char
	strncpy (lineBuf, line.c_str(), MIN(lineLength, line.size()));
	lineBuf[line.size()] = 0;


	int len = strlen (lineBuf);
	if (len > lineLength)
		{
		// truncate, if necessary
		lineBuf[lineLength-1] = 0;
		}
	else
		{
		// pad if necessary
		for (int i = len; i < lineLength; i++)
			{
			lineBuf[i] = 0;
			}
		}


	// write the array of characters to the fifo
	write(fd, lineBuf, lineLength) ;

    }  // end of loop over getline

  // close the fifo
  close (fd);

  exit (0);

}  // end of main 
