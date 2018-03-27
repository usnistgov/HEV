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
 * savg-cone				        *
 * Rewritten by: Adele Peskin   		*
 * Created on: 4-30-10 		        	*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-square generates a unit-sized        *
 * square centered at 0.                        *
 ************************************************/

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
  int i;
/*
 * COMMAND LINE PROCESSING
 */

  if (argc > 1) {
    usage();
    exit(EXIT_FAILURE);
  }

/*
 * BODY
 */

  printf("polygons\n");
  printf("0.5 0.0 0.5 1.0 1.0 1.0 1.0\n");
  printf("-0.5 0.0 0.5 1.0 1.0 1.0 1.0\n");
  printf("-0.5 0.0 -0.5 1.0 1.0 1.0 1.0\n");
  printf("0.5 0.0 -0.5 1.0 1.0 1.0 1.0\n");
}

/* Prints information on usage*/
void usage()
{
  fprintf(stderr, "ERROR: Incorrect command line arguments\n");
  fprintf(stderr, "usage: savg-square\n");
  exit(1);
}

