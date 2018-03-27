/****************************************
* SAVG-Dualtetra			*
* Created on 8/5/05			*
*					*
* This program takes no paramters	*
* and creates a glyph model of two	*
* juxtaposed tetrahedrons and an 	*
* interposed cube.  If arguments	*
* are present, an error message 	*
* will appear.				*
****************************************/		

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define PI 3.141592653589793238462643

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char *argv[];
{

	char outString[1024];
	
	if(argc >1){
	usage();
	exit(EXIT_FAILURE);
}

	strcpy(outString, "exec savg-tetra | savg-rgb 0 1 0 | savg-scale .8163305 .8163305 .8163305 | savg-translate 0 0 -.1665314; savg-tetra | savg-rgb 1 0 0 | savg-scale .8163305 .8163305 .8163305 | savg-rotate 0 180 0 | savg-translate 0 0 .1665314; savg-cube | savg-rgb 0 0 1| savg-scale .2040826 .2040826 .2040826 | savg-rotate 0 0 45 | savg-rotate  0 35 0; "); 
	
	system(outString);

}

void usage()
{
	fprintf(stderr, "\n\tError: Takes no arguments! Usage: savg-dualtetra > file.savg\n\n");
}

