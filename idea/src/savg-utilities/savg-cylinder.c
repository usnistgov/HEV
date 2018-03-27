/********************************************************
 * savg-cylinder  					*
 * Created by: Adele Peskin on 4-07-05 			*
 * Created on: 5-10-04					*
 * Current Version: 1.00				*
 *							*
 * Description:						*
 *    gtb-cylinder represents a set of polygons based   *
 * 2 sets of points representing circles 		*
 ********************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define PI 3.141592653589793238462643

/* Prints information on usage */
void usage();

int main(argc, argv)
int argc;
char *argv[];
{
  int face = 3;
  int i;
  int npoints  = 16;    /* no. of polygons requested */
  double angle, angle2;

/*
 * COMMAND LINE PROCESSING
 */

  if (argc > 1) {
    // first argument is npoints
    npoints = atoi(argv[1]);
  }
  
  if (argc > 2) {
    if (strcasecmp(argv[2],"top")==0)
      face = 2;
    else if (strcasecmp(argv[2],"bottom")==0)
      face = 1;
    else if (strcasecmp(argv[2],"open")==0)
      face = 0;
    else {
      usage();
      exit(EXIT_FAILURE);
    }
  }
  
  if (argc > 3) {
    usage();
    exit(EXIT_FAILURE);
  }

/*
 * BODY
 */

  // draw the bottom face if requested
  if(face == 1 || face == 3) {
    printf("polygons\n");
    for (i = 0; i < npoints; ++i) {
      // find the angle in radians
      angle = (double)(i*360/npoints) * PI/180;
      printf("%f %f 0.0 0.0 0.0 -1.0\n",cos(angle),sin(angle));
    }
  }

  // draw the top face if requested
  if(face == 2 || face == 3) {
    printf("polygons\n");
    for (i = 0; i < npoints; ++i) {
      // find the angle in radians
      angle = (double)(i*360/npoints) * PI/180;
      printf("%f %f 1.0 0.0 0.0 1.0\n",cos(angle),sin(angle));
    }
  }

  // draw the middle polygons with normals
  for (i = 0; i < npoints; ++i) {
    printf("polygons\n");
    // find the angles in radians
    angle = (double)(i*360/npoints) * PI/180;
    angle2 = (double)((i+1)*360/npoints) * PI/180;
    // enter node i,i+1 at z = 0, i+1,i at z = 1.0
    printf("%f %f 0.0 %f %f 0.0\n",cos(angle),sin(angle),cos(angle),sin(angle));
    printf("%f %f 0.0 %f %f 0.0\n",cos(angle2),sin(angle2),cos(angle2),sin(angle2));
    printf("%f %f 1.0 %f %f 0.0\n",cos(angle2),sin(angle2),cos(angle2),sin(angle2));
    printf("%f %f 1.0 %f %f 0.0\n",cos(angle),sin(angle),cos(angle),sin(angle));
  }
}

/* Prints information on usage */
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: savg-cylinder [value] [top/botto/open]\n");
	exit(1);
}





