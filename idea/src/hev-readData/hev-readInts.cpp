#define DATA int

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <dtk.h>

void usage()
{
    fprintf(stderr,
	    "Usage: hev-readInts [-d] [-i] [-N] shmName\n\n"
	    "\tshmName is the name of shared memory containing ints\n\n"
	    "\tOptions:\n"
	    "\t\t-d\tdiscard duplicates\n"
	    "\t\t-i\tprint the initial value\n"
	    "\t\t-N\texit after printing N values\n"
	    "\t\t-u t\tsleep t microseconds each loop\n\n"
	    "\tOutput to stdout when an int changes state is\n"
	    "\ta series of ints, one for each in the segment\n\n"
	    "\tThis command is similar to dtk-readInts.\n"
	    "\tIt has less features, but stdout is flushed making it\n"
	    "\tusable with redirection to a pipe.\n") ;
}

void print(DATA *data, size_t size)
{
    for (unsigned int i=0; i<size/sizeof(DATA); i++)
    {
	printf("%+8d ",*(data+i)) ;
    }
    printf("\n") ;
}

#include "common.h"
