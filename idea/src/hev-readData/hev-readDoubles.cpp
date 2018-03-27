#define DATA double

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
	    "Usage: hev-readDoubles [-d] [-i] [-N] shmName\n\n"
	    "\tshmName is the name of shared memory containing doubles\n\n"
	    "\tOptions:\n"
	    "\t\t-d\tdiscard duplicates\n"
	    "\t\t-i\tprint the initial value\n"
	    "\t\t-N\texit after printing N values\n"
	    "\t\t-u t\tsleep t microseconds each loop\n\n"
	    "\tOutput to stdout when a double changes is\n"
	    "\ta series of doubles, one for each in the segment\n\n"
	    "\tThis command is similar to dtk-readDoubles.\n"
	    "\tIt has less features, but stdout is flushed making it\n"
	    "\tusable with redirection to a pipe.\n") ;
}

void print(DATA *data, size_t size)
{
    for (unsigned int i=0; i<size/sizeof(DATA); i++)
    {
	printf("%+16.16g ",*(data+i)) ;
    }
    printf("\n") ;
}

#include "common.h"
