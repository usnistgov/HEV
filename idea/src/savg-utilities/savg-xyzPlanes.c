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

/* Prints information on usage */
void usage();

int main(argc, argv)
int argc;
char *argv[];
{
  printf("polygons\n");
  printf(" 1.0 -1.0 0.0\n");
  printf(" 1.0  1.0 0.0\n");
  printf("-1.0  1.0 0.0\n");
  printf("-1.0 -1.0 0.0\n");
  printf("polygons\n");
  printf(" 1.0 0.0 -1.0\n");
  printf(" 1.0 0.0  1.0\n");
  printf("-1.0 0.0  1.0\n");
  printf("-1.0 0.0 -1.0\n");
  printf("polygons\n");
  printf("0.0  1.0 -1.0\n");
  printf("0.0  1.0  1.0\n");
  printf("0.0 -1.0  1.0\n");
  printf("0.0 -1.0 -1.0\n");
}

/* Prints information on usage */
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: savg-xyzPlanes\n");
	exit(1);
}





