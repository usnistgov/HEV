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
 * Created on: 4-06-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-cone creates a unit-sized cone       *
 * (radius of one unit) in savg format.         *
 * The cone is centered at the                  *
 * origin. It's only command line argument is   *
 * -n nval, which indicades the number of       *
 * of the original set of lines to copy when    *
 * generating the cone.  savg-cone creates a    *
 * temp SAVG file containing two lines, which   *
 * forms the profile of the cone.  It then uses *
 * savg-sor to generate the cone based on this  *
 * profile.                                     *  
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
  int npoints = 16;    /* no. of polygons requested */
  int face;
  double newX, newY, angle, angle2, midAngle;
  double s2I, ca, sa, ca2, sa2, cam, sam;
  int revisedNormals;

  revisedNormals = 0;
  face = 1;  // closed cone
  

/*
 * COMMAND LINE PROCESSING
 */

  if (argc > 1) {
    // first parameter is number of polygons
    npoints = atoi(argv[1]);
  }

  if (argc > 2) {
    // next parameter is open
    if (strcasecmp(argv[2],"open")==0) {
      face = 0;
    }
    else if (strcasecmp(argv[2],"closed")==0) {
      face = 1;
    }
    else {
      usage();
      exit(EXIT_FAILURE);
    }
  }

  if (argc == 4) {
    if (strcasecmp(argv[3],"revisedNormals")==0) {
      revisedNormals = 1;
    }
    else if (strcasecmp(argv[3],"originalNormals")==0) {
      revisedNormals = 0;
    }
    else {
      usage();
      exit(EXIT_FAILURE);
    }

  }

  
  if (argc > 4) {
    usage();
    exit(EXIT_FAILURE);
  }
  
/*
 * BODY
 */

  // first generate the face polygon if requested
  if( face == 1) {
    printf("polygons\n");
    for (i = 0; i < npoints; ++i) {
      // find the angle in radians
      angle = (double)(i*360/npoints) * PI/180;
      if (revisedNormals)
		{
		angle *= -1;
		}
      newX = cos(angle);
      newY = sin(angle);
      printf("%f %f 0.0 0.0 0.0 -1.0\n",newX,newY);
    }
  }
  
  s2I = 1.0 / sqrt (2.0);

  // now create the (number of npoints) triangles and their normals
  for (i = 0; i < npoints; ++i) {
    printf("polygons\n");
    // find the angle in radians
    angle = (double)(i*360/npoints) * PI/180;
    angle2 = (double)((i+1)*360/npoints) * PI/180;
    midAngle = (double)((i+0.5)*360.0/npoints) * PI/180;
    if (revisedNormals)
		{
		angle *= -1;
		angle2 *= -1;
		midAngle *= -1;
		}


    ca = cos (angle);
    sa = sin (angle);

    ca2 = cos (angle2);
    sa2 = sin (angle2);

    cam = cos (midAngle);
    sam = sin (midAngle);

    // enter node i,i+1,top
    if (revisedNormals)
	{
    	printf("%f %f 0 %f %f %f\n",  ca2, sa2,  ca2*s2I, sa2*s2I, s2I);
    	printf("%f %f 0 %f %f %f\n",  ca,  sa,   ca *s2I, sa *s2I, s2I);
    	printf("0 0 1 %f %f %f\n",               cam*s2I, sam*s2I, s2I);
	}
    else
	{
	// the old way
    	printf("%f %f 0.0 %f %f 0.0\n",
		cos(angle),sin(angle),cos(angle),sin(angle));
    	printf("%f %f 0.0 %f %f 0.0\n",
		cos(angle2),sin(angle2),cos(angle2),sin(angle2));
    	printf("0.0 0.0 1.0 0.0 0.0 1.0\n");
	}
  }
}

/* Prints information on usage*/
void usage()
{
  fprintf(stderr,"ERROR: Incorrect command line arguments\n");
  fprintf(stderr, "usage: savg-cone [value] [open] [revisedNormals]\n");
  exit(1);
}


