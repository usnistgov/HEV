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
 * Created by: Brandon M. Smith     		*
 * Created on: 7-15-04  			*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-cube generates a unit-sized cube     *
 * (1 unit by 1 unit by 1 unit) in the 1st      *
 * quadrant.  One corner is located at the      *
 * origin, while it's most distant neighbor     *
 * is located at (1, 1, 1).                     *
 * 1/25/05: changed to a 2 unit cube centered   *
 * at the origin, to resemble savg-sphere.      *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>

/* Prints information on usage*/
void usage();

int main(argc, argv)
int argc;
char *argv[];
{
  
/*
 * COMMAND LINE PROCESSING
 */

if( argc > 1)
  usage();

/*
 * BODY
 */
  // bottom face 
  printf("polygon\n");
  printf("-1  1 -1 0 0 -1\n");
  printf(" 1  1 -1 0 0 -1\n");
  printf(" 1 -1 -1 0 0 -1\n");
  printf("-1 -1 -1 0 0 -1\n");

  // top face 
  printf("polygon\n");
  printf(" 1  1 1 0 0 1\n");
  printf("-1  1 1 0 0 1\n");
  printf("-1 -1 1 0 0 1\n");
  printf(" 1 -1 1 0 0 1\n");

  // +y face
  printf("polygon\n");
  printf("-1 1  1 0 1 0\n");
  printf(" 1 1  1 0 1 0\n");
  printf(" 1 1 -1 0 1 0\n");
  printf("-1 1 -1 0 1 0\n");

  // -y face
  printf("polygon\n");
  printf("-1 -1 -1 0 -1 0\n");
  printf(" 1 -1 -1 0 -1 0\n");
  printf(" 1 -1  1 0 -1 0\n");
  printf("-1 -1  1 0 -1 0\n");

  // +x face
  printf("polygon\n");
  printf("1  1 -1 1 0 0\n");
  printf("1  1  1 1 0 0\n");
  printf("1 -1  1 1 0 0\n");
  printf("1 -1 -1 1 0 0\n");

  // -x face
  printf("polygon\n");
  printf("-1  1  1 -1 0 0\n");
  printf("-1  1 -1 -1 0 0\n");
  printf("-1 -1 -1 -1 0 0\n");
  printf("-1 -1  1 -1 0 0\n");
}

/* Prints information on usage*/
void usage()
{
  printf("usage: savg-cube [no arguments]\n");
  exit(1);
}




