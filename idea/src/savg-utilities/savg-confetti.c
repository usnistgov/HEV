 
/*
 *	Glyph Toolbox
 *
 *       Scientific Applications & Visualization Group
 *            SAVG web page: http://math.nist.gov/mcsd/savg/
 *       Mathematical & Computational Science Division
 *            MCSD web page: http://math.nist.gov/mcsd/
 *       National Institute of Standards and Technology
 *            NIST web page: http://www.nist.gov/
 *
 *
 *  This software was developed at the National Institute of Standards
 *  and Technology by employees of the Federal Government in the course
 *  of their official duties. Pursuant to title 17 Section 105 of the
 *  United States Code this software is not subject to copyright
 *  protection and is in the public domain.  isolate is an experimental
 *  system.  NIST assumes no responsibility whatsoever for its use by
 *  other parties, and makes no guarantees, expressed or implied, about
 *  its quality, reliability, or any other characteristic.
 *
 *  We would appreciate acknowledgement if the software is used.
 *
 */


/************************************************
 * savg-confetti                                *
 * Based on gtb-confetti by Jessica Chang       *
 * Created on: 7-27-05                          *
 * Current Version: 2.00                        *
 *                                              *
 * Description:                                 *
 *      This command reads in exactly one file. *
 * This file must contain the 3 coordinates     *
 * for each point along with the values         * 
 * for r, g, b, and a.   If this                *
 * command is used incorrectly or the given     *
 * files do not exist, an error/usage message   *
 * will print.                                  *
 ************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();
void file_error(char* filename);

main(argc, argv)
int argc;
char *argv[];     
{

/* DECLARATIONS */
        float a;                /* counter */
        int j, nlines=0;
	char pointsfile[1000];
	char tester[100];
	char object[256];
	float newX, newY, newZ;
	float r, g, b;


FILE *PTS;
char outString[2048];
char tempString[2048];

/*
 * READ IN ARGUMENTS
 */   

printf("#SAVG-CONFETTI\n");

	if(argc != 2){  //makes sure there is exactly 2 args.
		usage();
		exit(EXIT_FAILURE);
	}

        else    strcpy(pointsfile, argv[1]);

        if((PTS = fopen(pointsfile, "r")) == NULL){  //if the file is blank, terminate
                file_error(pointsfile);
                exit(EXIT_FAILURE);
        }

	
	while(fgets(tester, 100, PTS) != NULL) //find number of lines
	      nlines++;
	nlines -=1;
       rewind(PTS);
	fscanf(PTS, "%s", object); //scan for the savg object

strcpy(outString, "exec ");

for(j=0; j< (nlines-1); j++)
{
          fscanf(PTS,"%f %f %f %f %f %f %f\n", &newX, &newY, &newZ, &r, &g, &b, &a); //scan for x,y,z,r,g,b,a
       	  
	  sprintf(tempString, "%s", object);
	  strcat(outString, tempString);
	  strcat(outString, " | savg-translate ");
	  sprintf(tempString, "%f", newX);
	  strcat(outString, tempString);
	  strcat(outString, " ");
	  sprintf(tempString, "%f", newY);
	  strcat(outString, tempString);
	  strcat(outString, " ");
	  sprintf(tempString, "%f", newZ);
	  strcat(outString, tempString);
	  strcat(outString, " | savg-rgb ");
	  sprintf(tempString, "%f", r);
	  strcat(outString, tempString);
	  strcat(outString, " ");
	  sprintf(tempString, "%f", g);
	  strcat(outString, tempString);
	  strcat(outString, " ");
	  sprintf(tempString, "%f", b);
	  strcat(outString, tempString);
	  strcat(outString, " | savg-alpha ");
	  sprintf(tempString, "%f", a);
	  strcat(outString, tempString); 
	strcat(outString, "; ");
	}

system(outString);
fclose(PTS);

}


void file_error(filename)
char *filename;
{
        fprintf(stderr, "ERROR: Unable to open file: %s \n"
        "Make sure that file exists and is not protected\n", filename);
}

void usage(){
        fprintf(stderr, "Incorrent command line\n"
        "usage: savg-confetti data.txt > file.savg\n");
}



