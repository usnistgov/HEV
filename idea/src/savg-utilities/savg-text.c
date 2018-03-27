/*
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
 * savg-cube				        *
 * Created by: Adele Peskin  		        *
 * Created on: 6-28-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-text generates a text object         *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "savgLineEvaluator.c"

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char *argv[];
{
  char line[STRING_LENGTH];
/*
 * COMMAND LINE PROCESSING
 */

  if( argc != 2) {
    usage();
    exit(EXIT_FAILURE);
  }

  strcpy(line,argv[1]);

/*
 * BODY
 */

  printf("TEXT PARTS ALL\n");
  printf("TEXT STRING \"%s\"\n",line);
}

/* Prints information on usage*/
void usage()
{
  printf("usage: savg-text 'textstring'\n");
  exit(1);
}




