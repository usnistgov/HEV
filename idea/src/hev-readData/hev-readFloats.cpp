#define DATA float

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
	    "Usage: hev-readFloats [-d] [-i] [-N] shmName\n\n"
	    "\tshmName is the name of shared memory containing floats\n\n"
	    "\tOptions:\n"
	    "\t\t-d\tdiscard duplicates\n"
	    "\t\t-i\tprint the initial value\n"
	    "\t\t-N\texit after printing N values\n\n"
	    "\tOutput to stdout when a float changes is\n"
	    "\ta series of floats, one for each in the segment\n"
	    "\t\t-u t\tsleep t microseconds each loop\n\n"
	    "\tThis command is similar to dtk-readFloats.\n"
	    "\tIt has less features, but stdout is flushed making it\n"
	    "\tusable with redirection to a pipe.\n") ;
}

void print(DATA *data, size_t size)
{
    for (unsigned int i=0; i<size/sizeof(DATA); i++)
    {
	printf("%+g ",*(data+i)) ;
    }
    printf("\n") ;
}

#include "common.h"
