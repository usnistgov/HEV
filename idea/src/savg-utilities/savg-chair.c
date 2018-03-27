

/****************************************
*Savg-Chair                             *
*					*
*Created on 7-28-05			*
*					*
* This program takes exactly		*
* one parameter, the type of chair.	*	
* The user can either choose		*
* the "conference" chair with 		*
* armrests, or the small "computer"	*
* chair with no armrests.  The user	*
* must type in either "conference" or	*
* "computer," else an error message	*
* will appear.				*
****************************************/



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();

main(argc, argv)
int argc;
char *argv[];
{
	// declarations

	char outString[2048];
	int type;

//take in arguments

	if(argc != 2){
		usage();
		exit(EXIT_FAILURE);
	}
	
	else{
	
	if(strcmp(argv[1], "computer")==0)
		type=1;
	else if(strcmp(argv[1], "conference")==0)
		type=2;
	else {
		usage();
		exit(EXIT_FAILURE);
	}
	}


if(type ==1){
	
	strcpy(outString, "exec savg-sphere | savg-scale .2834302 .232558 .0436046 | savg-rgb .3 .3 .3 |savg-translate 0 0 .5232558; savg-sphere | savg-scale .232558 .232558  .0436046 | savg-rgb .3 .3 .3 | savg-rotate 0 90 90 | savg-translate 0 .2406976 .7674418; savg-cylinder | savg-rgb 0.005 0.005 0.005 | savg-scale .0219767 .0219767 .5232558 | savg-translate 0 0 0; savg-cube | savg-scale .03488372 .0436046 .0104651 | savg-rgb .005 .005 .005 | savg-rotate 0 90 0| savg-translate 0 .232558 .5302325; savg-cube | savg-scale .03488372 .0436046 .0104651 | savg-rgb .005 .005 .005 | savg-rotate 0 0 0| savg-translate 0 .1988372 .4953488; savg-cylinder | savg-rgb 0.005 0.005 0.00500  | savg-scale .0174418 .0174418 .3139534 | savg-rotate 0 72 90 | savg-translate 0 0 .0174418; savg-cylinder | savg-rgb 0.005 0.005 0.00500 | savg-scale .0174418 .0174418 .3139534 | savg-rotate 0 144 90 | savg-translate 0 0 .0174418; savg-cylinder | savg-rgb 0.005 0.005 0.00500  | savg-scale .0174418 .0174418 .3139534 | savg-rotate 0 216 90 | savg-translate 0 0 .0174418; savg-cylinder | savg-rgb 0.005 0.005 0.00500 | savg-scale .0174418 .0174418 .3139534 | savg-rotate 0 288 90 | savg-translate 0 0 .0174418; savg-cylinder | savg-rgb 0.005 0.005 0.0050 | savg-scale .0174418 .0174418 .3139534 | savg-rotate 0 360 90 | savg-translate 0 0 .0174418;");

}

else{
		strcpy(outString, "exec savg-cylinder 50 | savg-scale .2831714 .2157496 .0539373 | savg-rgb .3 .2 .3| savg-translate 0 0 .4207118; savg-cylinder 50 | savg-scale .2880258 .2831714 .040453 | savg-rotate 0 90 90 | savg-translate 0 .2157496 .7119739;  savg-cylinder | savg-rgb 0.005 0.005 0.00500 | savg-scale .0202265 .0202265 .3883494| savg-translate 0 0 .0161812; savg-cylinder | savg-rgb 0.005 0.005 0.00500  | savg-scale .0161812 .0161812 .291263 | savg-rotate 0 72 90 | savg-translate 0 0 .0161812; savg-cylinder | savg-rgb 0.005 0.005 0.00500 | savg-scale .0161812 .0161812 .291263| savg-rotate 0 144 90 | savg-translate 0 0 .0161812; savg-cylinder | savg-rgb 0.005 0.005 0.00500  | savg-scale .0161812 .0161812 .291263 | savg-rotate 0 216 90 | savg-translate 0 0 .0161812; savg-cylinder | savg-rgb 0.005 0.005 0.00500 | savg-scale .0161812 .0161812 .291263 | savg-rotate 0 288 90 | savg-translate 0 0 .0161812; savg-cylinder | savg-rgb 0.005 0.005 0.0050 | savg-scale .0161812 .0161812 .291263| savg-rotate 0 360 90 | savg-translate 0 0 .0161812 ; savg-cylinder 25 open | savg-rgb 0.005 0.005 0.005 | savg-scale .0970873 .1618122 .040453 | savg-rotate 0 0 90 | savg-translate -.2831714 0.0582524 .5501616; savg-cylinder 25 open | savg-rgb 0.005 0.005 0.005 | savg-scale .0970873 .1618122 .040453 | savg-rotate 0 0 90 | savg-translate .2427183 0.0582561 .5501616; savg-cube | savg-scale .03236245 .040453 .0097087 | savg-rgb .005 .005 .005 | savg-rotate 0 90 0| savg-translate 0 .2157496 .4433655; savg-cube | savg-scale  .03236245 .040453 .0097087 | savg-rgb .005 .005 .005 | savg-rotate 0 0 0| savg-translate 0 .1844659 .4110031;  ");

	}

system(outString);

			

}



void usage()
{
	fprintf(stderr, "\n\tERROR - incorrect command line- \n\t\t  parameter must be either [computer] or [conference] \n\n");
}		
