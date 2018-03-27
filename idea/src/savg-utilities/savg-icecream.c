#include <stdio.h>
#include <string.h>
#include "feed.h"
#include <stdlib.h>

void usage();

main(argc, argv)
int argc;
char *argv[];
{

char outString[1024];

if(argc >1){
	usage();
	exit(EXIT_FAILURE);
}

strcpy(outString, "exec savg-cone 16 | savg-rotate 0 180 0  | savg-scale .1875 .1875 .75| savg-translate 0 0 .25 |savg-rgb .7 .5 0; savg-sphere | savg-rgb .9 .9 .9  | savg-translate 0 0 1.3325 | savg-scale .2142855 .2142855 .2142855; ");

system(outString);
}

void usage()
{
	fprintf(stderr, "\n\tTakes no arguments! USAGE: savg-icecream > file.savg\n\n");
}
