#define DATA char

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
	    "Usage: hev-readString [-d] [-i] [-N] shmName\n\n"
	    "\tshmName is the name of shared memory file\n\n"
	    "\tOptions:\n"
	    "\t\t-d\tdiscard duplicates\n"
	    "\t\t-i\tprint the initial value\n"
	    "\t\t-N\texit after printing N values\n\n"
	    "\t\t-u t\tsleep t microseconds each loop\n"
	    "\tOutput to stdout when a string changes state.\n\n"
	    "\tThis command is similar to dtk-readStrings.\n"
	    "\tIt has less features, but stdout is flushed making it\n"
	    "\tusable with redirection to a pipe.\n") ;
}

void print(DATA *data, size_t size)
{
    printf("%s\n",data) ;
}

#include "common.h"
