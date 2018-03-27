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
#include <stdlib.h>
#include <math.h>
#define PI 3.141592653589793238462643

/* Prints information on usage */
void usage();
void usage2();

int main(argc, argv)
int argc;
char *argv[];
{
  int i;
  int n = 6;
  float xx[100];
  float yy[100];
  float zz[100];
  float norm[110];
  double partInRadians;
  
/*
 * COMMAND LINE PROCESSING
 */

  // savg-arrow has 1 optional parameter, n, number of polygons
  if (argc == 2)
    n = atoi(argv[1]) + 1;

  else if (argc > 2) {
    usage();
    exit(EXIT_FAILURE);
  }

/*
 * BODY
 */


  partInRadians = 2.0 * PI / (n-1);
  // first create the points

  // n points radius = .1, z = 0 for stem
  for (i=0; i<n; ++i) {
    xx[i] = 0.1*cos(i*partInRadians);
    yy[i] = 0.1*sin(i*partInRadians);
    zz[i] = 0.0;
    norm[i] = sqrt(xx[i]*xx[i]+yy[i]*yy[i]);
  }
  // n points radius = .1, z = 0.75 for stem
  for (i=0; i<n; ++i) {
    xx[i+n] = 0.1*cos(i*partInRadians);
    yy[i+n] = 0.1*sin(i*partInRadians);
    zz[i+n] = 0.75;
    norm[i+n] = norm[i];
  }
  // n points radius = .2, z = 0.75 for head
  for (i=0; i<6; ++i) {
    xx[i+2*n] = 0.2*cos(i*partInRadians);
    yy[i+2*n] = 0.2*sin(i*partInRadians);
    zz[i+2*n] = 0.75;
    norm[i+2*n] = sqrt(xx[i+2*n]*xx[i+2*n]+yy[i+2*n]*yy[i+2*n]);
  }
  // 1 point for the top
  xx[3*n] = 0.0;
  yy[3*n] = 0.0;
  zz[3*n] = 1.0;
  norm[3*n] = 1.0;

  // print the polygons for the stem
  for (i=0; i<n-1; ++i) {
    printf("polygon\n");
    printf("%f %f %f  %f %f 0.0\n",xx[i],yy[i],zz[i],
         xx[i]/norm[i],yy[i]/norm[i]);
    printf("%f %f %f  %f %f 0.0\n",xx[i+1],yy[i+1],zz[i+1],
         xx[i+1]/norm[i+1],yy[i+1]/norm[i+1]);
    printf("%f %f %f  %f %f 0.0\n",xx[n+i+1],yy[n+i+1],
	   zz[n+i+1],xx[n+i+1]/norm[n+i+1],yy[n+i+1]/norm[n+i+1]);
    printf("%f %f %f  %f %f 0.0\n",xx[n+i],yy[n+i],
	   zz[n+i],xx[n+i]/norm[n+i],yy[n+i]/norm[n+i]);
  }

  // print top and bottom of stem
  printf("polygon\n");
  for (i = 0; i < n; ++i) {
    printf("%f %f %f  0.0 0.0 -1.0\n",xx[i],yy[i],zz[i]);
  }
  printf("polygon\n");
  for (i = n; i < 2*n; ++i) {
    printf("%f %f %f  0.0 0.0 1.0\n",xx[i],yy[i],zz[i]);
  }

  // print the bottom of the head
  printf("polygon\n");
  for (i = 0; i < n; ++i) {
    printf("%f %f %f  0.0 0.0 -1.0\n",xx[i+2*n],yy[i+2*n],zz[i+2*n]);
  }

  // print the polygons for the head
  for (i=0; i<n-1; ++i) {
    printf("polygon\n");
    printf("%f %f %f  %f %f 0.0\n",xx[i+2*n],yy[i+2*n],
	   zz[i+2*n],xx[i+2*n]/norm[i+2*n],yy[i+2*n]/norm[i+2*n]);
    printf("%f %f %f  %f %f 0.0\n",xx[i+1+2*n],
	   yy[i+1+2*n],zz[i+1+2*n],
	   xx[i+1+2*n]/norm[i+1+2*n],yy[i+1+2*n]/norm[i+1+2*n]);
    printf("%f %f %f  0.0 0.0 1.0\n",xx[3*n],yy[3*n],
	   zz[3*n]);
  }
}

/* Prints information on usage */
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: savg-cylinder [value]\n");
	exit(1);
}





