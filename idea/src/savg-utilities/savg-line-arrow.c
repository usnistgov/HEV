/********************************************************
 * savg-arrow  					        *
 * Created by: Adele Peskin                             *
 * Created on: 12-27-04					*
 * Current Version: 1.00				*
 *							*
 * Description:						*
 *    gtb-arrow represents a cylinder with              *
 *    a prism on top                                    *
 ********************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define PI 3.141592653589793238462643

/* Prints information on usage */
void usage();
void usage2();

int main(argc, argv)
int argc;
char *argv[];
{
  printf("lines\n");
  printf("0.0 0.0 0.0\n");
  printf("0.0 0.0 1.0\n");
  printf("lines\n");
  printf("0.0 0.0 1.0\n");
  printf("0.1 0.0 0.9\n");
  printf("lines\n");
  printf(" 0.0 0.0 1.0\n");
  printf("-0.1 0.0 0.9\n");
}

/* Prints information on usage */
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: savg-line-arrow\n");
	exit(1);
}





