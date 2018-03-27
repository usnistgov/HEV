#define DATA unsigned char

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
	    "Usage: hev-readButtons [-d] [-i] [-N] [-u t] shmName\n\n"
	    "\tshmName is the name of shared memory, typically 'buttons'\n\n"
	    "\tOptions:\n"
	    "\t\t-d\tdiscard duplicates\n"
	    "\t\t-i\tprint the initial value\n"
	    "\t\t-N\texit after printing N values\n"
	    "\t\t-u t\tsleep t microseconds each loop\n\n"
	    "\tOutput to stdout when a button changes state is\n"
	    "\tits integer value followed by its binary representation.\n"
	    "\tIf there are multiple buttons, all the integers are written\n"
	    "\tfirst, followed by all of the binary values.\n"
	    "\tExample:  '5 00001010', indicates left and right buttons are pressed.\n\n"
	    "\tThis command is similar to dtk-readButtons.\n"
	    "\tIt has less features, but stdout is flushed making it\n"
	    "\tusable with redirection to a pipe.\n") ;
}

std::string binary(DATA d)
{
    std::string s ;
    DATA m = 1 ;
    for (unsigned int i=0; i<sizeof(DATA)*8; i++)
    {
	if (d & m) s = "1" + s ;
	else s = "0" + s ;
	m = m<<1 ;
    }
    return s ;
}
void print(DATA *data, size_t size)
{
    for (unsigned int i=0; i<size; i++)
    {
	printf("%d ",*(data+i)) ;
    }
    for (unsigned int i=0; i<size; i++)
    {
	std::string b = binary(*(data+i)) ;
	
	printf("%s ",b.c_str()) ;
    }
    printf("\n") ;
}

#include "common.h"
