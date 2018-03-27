/********************************
* SAVG-FLAG
* Created on 8/2/05
*
* This program creates a glyph
* flag model.  It takes exactly 
* three parameters, r, g, and b.
* It creates a flag of the specified
* color with a silver pole.
********************************/





#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "feed.h"

void usage();

main(argc, argv)
int argc;
char *argv[];
{

	char outString[1024];
	char tempString[1024];
	float r, b, g;



	if(argc != 4)
	{ usage();
	exit(EXIT_FAILURE);
	}

	/*  ((argv[1]>1 || argv[1] < 0) || (argv[2]>1 || argv[2] < 0) || (argv[3]>1 || argv[3] < 0))
{	usage();
	exit(EXIT_FAILURE);
	}
*/	
	else{

	r= atof(argv[1]);
	g=atof(argv[2]);
	b=atof(argv[3]);
}	
strcpy(outString, "exec savg-cylinder  | savg-rgb .5 .5 .5 | savg-scale .0333 .0333 .88 | savg-translate 0 0 .06;  savg-cone | savg-rgb .5 .5 .5 |savg-scale .0333 .0333 .06| savg-rotate 0 180 0 | savg-translate 0 0 0.06; savg-sphere | savg-rgb .5 .5 .5 |savg-scale .06 .06 .06 | savg-translate 0 0 .94; savg-cube | savg-rgb ");  

	sprintf(tempString, "%f", r);
	strcat(outString, tempString);
	strcat(outString, " ");
	sprintf(tempString, "%f", g);
        strcat(outString, tempString);
        strcat(outString, " ");
	sprintf(tempString, "%f", b);
        strcat(outString, tempString);
        strcat(outString, " | savg-scale .2333 .00333 .16666 | savg-translate .2333 0 .8;");

system(outString);

}

void usage(){
	fprintf(stderr, "\n\tERROR -Incorrect command line arguments:\n Must have r val, g val, and b val\nUSAGE: savg-flag 1 .2 0 > file.savg\n\n");
}
