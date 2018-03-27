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
 * savg-sphere				        *
 * Created by: Adele Peskin     		*
 * Modified by: Brandon M. Smith on 16 July '04 *
 * Created on: 4-13-04  			*
 * Current Version: 2.00			*
 *						*
 * Description:	                                *
 *    savg-sphere creates a sphere          	*
 * in savg format from a set of x,y,z points.   *			   
 * Its command line arguments are -n nval, and  *
 * -r rval,  where nval is the requested number *
 * of polygons, and rval is the                 *
 * radius of the sphere. At each level, the     *
 * edges of the current triangles are bisected  *
 * and each new point is pushed to the surface. *   
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>

#ifndef PI
#define PI 3.1415927
#endif

#define MAX 1000

/* Prints information on usage*/
void usage();
int findRows(int poly, int *nVerts);
void printPoly(float* xp, float* yp, float* zp, float r, int n1, int n2, int n3);
int main(argc, argv)
int argc;
char *argv[];
{
  int i,j;
  float r;	/* radius of sphere */
  int poly;     /* no. of polygons requested */
  int tris ;    /* number of triangles in the input file */
  int pts;	/* number of points in the input file */
  int rows;     /* rows of points */
  // float xp[MAX],yp[MAX],zp[MAX];
  float *xp, *yp, *zp;
  int cnt = 0;

  float delta_theta, delta_phi;
  int first,last,sets,first2,last2,first3,last3,first4,last4;
  float theta, phi;
  int nVerts;
/*
 * INITIALIZATION
 */

  poly = 128; /* default */
  r=1.0; /* default */
  pts = 6;
  //tris = 8;
  rows = 2;
  
/*
 * COMMAND LINE PROCESSING
 */

  if (argc > 1) {
    // first argument is npoints
    poly = atoi(argv[1]);
  }

  if (argc > 2) {
    usage();
    exit(EXIT_FAILURE);
  }
  
  /* calculate total number of points and triangles */
  // first find the number of rows based on the polygon request
  rows = findRows(poly, &nVerts);
  tris = 8 * rows * rows;
  nVerts *= 8;

  if  ( (xp = (float *) malloc (nVerts * sizeof (float))) == NULL)
	{
	fprintf (stderr, 
	  "savg-sphere: Unable to allocate memory for %d points.\n", nVerts);
	exit (EXIT_FAILURE);
	}


  if  ( (yp = (float *) malloc (nVerts * sizeof (float))) == NULL)
	{
	fprintf (stderr, 
	  "savg-sphere: Unable to allocate memory for %d points.\n", nVerts);
	exit (EXIT_FAILURE);
	}


  if  ( (zp = (float *) malloc (nVerts * sizeof (float))) == NULL)
	{
	fprintf (stderr, 
	  "savg-sphere: Unable to allocate memory for %d points.\n", nVerts);
	exit (EXIT_FAILURE);
	}


	


  printf("# savg-sphere: requested %d polygons, providing %d \n",poly,tris);

  // pts = number of points in each half sphere
  pts = 0;
  for (i=1; i<rows+2; ++i) {
    pts = pts + 1+ 4*(i-1);
  }

  // redefine rows as rows of points
  rows = rows + 1;

  /* print points for theta: 0-2*pi; phi:0-pi/2 */
  /* calculate points and project each point to the sphere */
  delta_phi = (PI/2.0)/(rows-1);
  
  /* start with top point */
  theta = 0.0;
  phi = 0.0;
  xp[cnt] = sin(phi)*cos(theta);
  yp[cnt] = sin(phi)*sin(theta);
  zp[cnt] = cos(phi);
  ++cnt;
  
  
  delta_phi = (PI/2.0)/(rows-1);
  for (i = 2; i<rows+1; ++i) {
    phi = (i-1)*delta_phi;
    delta_theta = (PI/2.0)/(i-1);
    for (j = 0; j < 1+(4*(i-1)); ++j) {

      if (cnt >= nVerts)
	{
	fprintf (stderr, 
	  "savg-sphere: Exceeded maximum number of vertices (%d).\n", nVerts);
	exit (EXIT_FAILURE);
	}

      theta = j*delta_theta;
      xp[cnt] = sin(phi)*cos(theta);
      yp[cnt] = sin(phi)*sin(theta);
      zp[cnt] = cos(phi);
      ++cnt;
    }
  }

  /* print points for theta: 0-2*pi; phi:pi-pi/2 */
  /* calculate points and project each point to the sphere */
  delta_phi = (PI/2.0)/(rows-1);
  
  /* start with top point */
  theta = 0.0;
  phi = PI;

  if (cnt >= nVerts)
	{
	fprintf (stderr, 
	  "savg-sphere: Exceeded maximum number of vertices (%d).\n", nVerts);
	exit (EXIT_FAILURE);
	}

  xp[cnt] = sin(phi)*cos(theta);
  yp[cnt] = sin(phi)*sin(theta);
  zp[cnt] = cos(phi);
  ++cnt;
  
  delta_phi = (PI/2.0)/(rows-1);
  for (i = 2; i<rows+1; ++i) {
    phi = PI-(i-1)*delta_phi;
    delta_theta = (PI/2.0)/(i-1);
    for (j = 0; j < 1+(4*(i-1)); ++j) {
      theta = j*delta_theta;

      if (cnt >= nVerts)
	{
	fprintf (stderr, 
	  "savg-sphere: Exceeded maximum number of vertices (%d).\n", nVerts);
	exit (EXIT_FAILURE);
	}

      xp[cnt] = sin(phi)*cos(theta);
      yp[cnt] = sin(phi)*sin(theta);
      zp[cnt] = cos(phi);
      ++cnt;
    }
  }

  // print out the triangles for positive z side, then repeat for 
  // negative z side
  first = 1;
  first2 = 1;
  first3 = 1;
  first4 = 1;
  for (i=1; i<rows; ++i) {
    /* triangles for the first quadrant */
    last = first;
    first = first+1+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, first, first+1, last);
    sets = i-1;
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last+j,first+1+j,last+1+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, first+1+j,first+2+j,last+1+j);     
    }

    /* triangles for the second quadrant*/
    last2 = first2;
    first2 = first2+2+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, first2,first2+1,last2);
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last2+j,first2+1+j,last2+1+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, first2+1+j,first2+2+j,last2+1+j);
    }

    /* triangles for the third quadrant*/
    last3 = first3;
    first3 = first3+3+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, first3,first3+1,last3);
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last3+j,first3+1+j,last3+1+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, first3+1+j,first3+2+j,last3+1+j);
    }

    /* triangles for the fourth quadrant*/
    last4 = first4;
    first4 = first4+4+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, first4,first4+1,last4);
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last4+j,first4+1+j,last4+1+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, first4+1+j,first4+2+j,last4+1+j);
    }
  }
    
  // flip the triangles for negative z side
  first = pts+1;
  first2 = pts+1;
  first3 = pts+1;
  first4 = pts+1;
  for (i=1; i<rows; ++i) {
    /* triangles for the first quadrant */
    last = first;
    first = first+1+4*(i-1);
    /* do the first triangle in the set */
      printPoly(xp, yp, zp, r, last,first+1,first);
    sets = i-1;
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last+1+j,first+1+j,last+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, last+1+j,first+2+j,first+1+j);     
    }

    /* triangles for the second quadrant*/
    last2 = first2;
    first2 = first2+2+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, last2,first2+1,first2);
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last2+1+j,first2+1+j,last2+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, last2+1+j,first2+2+j,first2+1+j);
    }

    /* triangles for the third quadrant*/
    last3 = first3;
    first3 = first3+3+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, last3,first3+1,first3);
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last3+1+j,first3+1+j,last3+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, last3+1+j,first3+2+j,first3+1+j);
    }

    /* triangles for the fourth quadrant*/
    last4 = first4;
    first4 = first4+4+4*(i-1);
    /* do the first triangle in the set */
    printPoly(xp, yp, zp, r, last4,first4+1,first4);
    for (j=0; j<sets; ++j) {
      /* top element */
      printPoly(xp, yp, zp, r, last4+1+j,first4+1+j,last4+j);

      /* bottom element */
      printPoly(xp, yp, zp, r, last4+1+j,first4+2+j,first4+1+j);
    }
  }

  return 0;
}	

/* Prints information on usage*/
void usage()
{
  fprintf(stderr, "ERROR: Incorrect command line arguments\n");
  fprintf(stderr, "usage: savg-sphere [value]\n");
  exit(1);
}

/* find the number of subdivision rows of triangles based on user input */
int findRows(int poly, int *nVerts) {
  int max = 20;
  int rows = 4;
  int current_pol = 8;
  int i;
  
  // this is the default - return with default = 4 rows

  if (poly != 128) {
    rows = 1;
    // while (current_pol < poly && rows <= max) {
    while (current_pol < poly) {
      ++rows;
      current_pol = 8 * rows * rows;
    }
  }

  if (current_pol > poly) rows = rows - 1;
  if (rows == 0)
    rows = 1;

  *nVerts = 1;
  for (i = 1; i <= rows; i++)
	{
	*nVerts += (i+1);
	}
  return rows;
}

void printPoly(float* xp, float* yp, float* zp, float r,
	       int n1, int n2, int n3) {
  printf("polygon\n");
  printf("%f %f %f %f %f %f\n",r*xp[n1-1],r*yp[n1-1],
	 r*zp[n1-1],xp[n1-1],yp[n1-1],zp[n1-1]);
  printf("%f %f %f %f %f %f\n",r*xp[n2-1],r*yp[n2-1],
	 r*zp[n2-1],xp[n2-1],yp[n2-1],zp[n2-1]);
  printf("%f %f %f %f %f %f\n",r*xp[n3-1],r*yp[n3-1],
	 r*zp[n3-1],xp[n3-1],yp[n3-1],zp[n3-1]);
}
