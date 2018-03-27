
/****************************************
* Savg-Computer				*
* Created on : 7/29/05			*	
*  					*	
* Updated for Rainbow on:		*
*   2/15/06 				*
*					*
* Description:				*
*					*
* This program take up to one		*
* parameter and contructs a 		*
* glyph computer.  The parameter	*
* is the name of the computer. 		*
* The user must type in any name	*
* he wants displayed, (usually		*
* the computer name), which MUST	*
* BE IN QUOTATIONS.  			*
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
	char name[10];
	char tempString[1024];

   // Take in argument

	if(argc > 2){
		usage();
		exit(EXIT_FAILURE);
	}
	
        if(argc ==2)
	   strcpy(name, argv[1]);
	// Make the computer

	strcpy(outString, "exec savg-cube | savg-scale .5 .0833333 .4166666 | savg-rgb .4 .4 .4 | savg-rotate 0 -5 0 ;  savg-cube | savg-rgb .005 .005 .005 | savg-scale .416666 .33333 0.0066666 | savg-rotate 0 85 0 | savg-translate 0 -.08466666 0; savg-cube | savg-scale .4 .333333 .166666 | savg-rgb .4 .4 .4 | savg-rotate 0 85 0 | savg-translate 0 .25 0; savg-cube | savg-rgb .4 .4 .4 | savg-scale .5 .1666666 .02083333 |savg-rotate 0 0 0 | savg-translate 0 -.2533333 -.44;");
	
	if(argc==2){
	  strcat(outString, " savg-text ");
	  sprintf(tempString, "%s", name);
	  strcat(outString, tempString);
	  strcat(outString, " | savg-scale .2 .2 .15 | savg-rgb ");
	  if((strcmp(name, "Visage") ==0) || (strcmp(name, "visage") ==0) 
		|| (strcmp(name, "Rainbow") ==0) || (strcmp(name, "Rainbow:0.0") ==0)
		|| (strcmp(name, "rainbow") ==0))

	    strcat(outString, ".8 0 0 ");
	  else strcat(outString, " 0 .2 .8 ");
	  strcat(outString, " | savg-translate -.4 -.0866666 0;");
	}
	system(outString);

}

void usage()
{
	fprintf(stderr, "\nERROR - Incorrect command line - \n\tTakes up to one parameter [computer name] <---- MUST BE IN QUOTATIONS\n");
} 
