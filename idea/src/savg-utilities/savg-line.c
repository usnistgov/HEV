#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void usage();


main(argc, argv)
int argc;
char *argv[];
{

	if(argc >1){
		usage();
		exit(EXIT_FAILURE);
	}

	
	printf("lines\n");
	printf("0 0 0\n");
	printf("1 0 0\n");

	return (0);
}



void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
        fprintf(stderr,"\tUsage: savg-line [no arguments]\n");
}
