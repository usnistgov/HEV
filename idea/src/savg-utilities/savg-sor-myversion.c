/*
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
 * savg-sor				        *
 * Created by: Brandon M. Smith                 *
 * Original "gtb" file created by: Alex Harn	*
 * Created on: 7-1-04				*
 * Current Version: 1.00			*
 *						*
 * Description:	                                *
 *    savg-sor creates a surface of revolution	*
 * in savg format from a set of lines.          *			   
 * Its command line arguments are -x xval,      *
 * -y yval, or -z zval, to determine axis to    *
 * revolve about and number of degrees to       *
 * rotate.  It also takes -n as the number of   *
 * times the original set of lines is used in   *
 * the creation of the surface of revolution.   *
 * savg-sor prints a usage message if more than *
 * one axis is an argument in the command line. *
 * All comments in the original file are left   *
 * intact, and a comment is added to the top of *
 * the new file describing the surface of       *
 * revolution.  Z is the default axis, and the  *
 * default is 360 degrees rotation.		*   
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "savgLineEvaluator.c"

#ifndef PI
#define PI 3.141592653589793
#endif

#define ARRAY_CAPACITY (500)

/* 
 * Prints information on usage
 */
void usage();

void calculateCrossProduct(double *uVector,double *vVector,
			   double *normalVector, double vectorMagnitude);
void rotatep(double* xyzr, int axis, double angle, double* temp);
void normalizeVector(double *normalVector, double vectorMagnitude);
const double tolerance = .00001;

int main(argc, argv)
int argc;
char *argv[];
{
  int axis = 1;
  char line[STRING_LENGTH];
  int i,j;
  int numpts = 16;
  double angleOfRotation = 360.0;
  double angleIncrement;
  double angle;
  int pargc = 0;
  char **pargv  = NULL;
  int count = 0;
  char keyrgbaline[100];
  int maxArraySize = ARRAY_CAPACITY;
  double x[maxArraySize];
  double y[maxArraySize];
  double z[maxArraySize];
  float r[maxArraySize];
  float g[maxArraySize];
  float b[maxArraySize];
  float a[maxArraySize];
  int rgba[maxArraySize];
  int keyWordLineHasRGBA = 0;
  double x1[maxArraySize];
  double y1[maxArraySize];
  double z1[maxArraySize];
  double x2[maxArraySize];
  double y2[maxArraySize];
  double z2[maxArraySize];
  double x3[maxArraySize];
  double y3[maxArraySize];
  double z3[maxArraySize];
  double xm1[maxArraySize];
  double ym1[maxArraySize];
  double zm1[maxArraySize];
  double normal1[3];
  double normal2[3];
  double normal3[3];
  double normal4[3];
  double normalAvg[3];
  double uVector[3];
  double vVector[3];
  double xyzr[3];
  double temp[3];
  double vectorMagnitude;
  int cnt;
  int numpolys;
  int full = 1;
/* 
 * INITIALIZATION
 */
  axis = 3;		/* default is z-axis */

/*
 * COMMAND LINE PROCESSING
 */

  if (argc == 1) {
    usage();
    exit(EXIT_FAILURE);
  }

  if (strcasecmp(argv[1],"x")==0)
    axis = 1;
  else if (strcasecmp(argv[1],"y")==0)
    axis = 2;
  else if (strcasecmp(argv[1],"z")==0)
    axis = 3;
  else {
    usage();
    exit(EXIT_FAILURE);
  }
   
  if (argc == 3)
    numpts = atoi(argv[2]);
  
  if (argc == 4)
    angleOfRotation = atof(argv[3]);

  if (argc > 4) {
    usage();
    exit(EXIT_FAILURE);
  }

  // BODY
  // set an extra row of polygons if last and first rows connect
  numpolys = numpts - 1;
  if (angleOfRotation > 359.99)
    numpolys = numpts;
  else
    full = 0;

  if(!getLine(line))
    return 0;

  // write out file until you get to a line
  while( jumpToNearestLineGeometry(line,pargv,pargc) ) {
    // found a line

    dpf_charsToArgcArgv(line, &pargc, &pargv);
    if (pargc == 5 ) {
      // RGBA can be found on key word line
      keyWordLineHasRGBA = 1;
      strcpy(keyrgbaline,"polygons ");
      strcat(keyrgbaline,pargv[1]);
      strcat(keyrgbaline," ");
      strcat(keyrgbaline,pargv[2]);
      strcat(keyrgbaline," ");
      strcat(keyrgbaline,pargv[3]);
      strcat(keyrgbaline," ");
      strcat(keyrgbaline,pargv[4]);
    }
    else
      keyWordLineHasRGBA = 0;

    // now collect data lines under the keyword tag    
    if( !getLine(line) )
      return 0;

    count = 0;
    while( pargc > 0 && !isKeyWord(line, pargv, pargc) ) { 
      dpf_charsToArgcArgv(line, &pargc, &pargv);
      if (pargc == 3 || pargc == 6 || pargc == 7 || pargc == 10) {
	// collect xyz, rgba
	x[count] = (double)atof(pargv[0]);
	y[count] = (double)atof(pargv[1]);
	z[count] = (double)atof(pargv[2]);

	if (pargc > 6) {
	  r[count] = atof(pargv[3]);
	  g[count] = atof(pargv[4]);
	  b[count] = atof(pargv[5]);
	  a[count] = atof(pargv[6]);
	  rgba[count] = 1;
	}
	else
	  rgba[count] = 0;
      }
      count = count + 1;   
      if( !getLine(line) ) {
	strcpy(line,"");
	pargc = 0;
      }
    }

    // process the surface of revolution
    for (j = 0; j < numpts+1; ++j) {
      // find the angle in radians
      angle = (double)(j*angleIncrement) * PI/180;
      // rotate first 2 rows to get started (need 3 rows to find norms)
      xyzr[0] = x[0];
      xyzr[1] = y[0];
      xyzr[2] = z[0]; 
      rotatep(xyzr,axis,angle,temp);
      x2[j] = xyzr[0];
      y2[j] = xyzr[1];
      z2[j] = xyzr[2];

      xyzr[0] = x[1];
      xyzr[1] = y[1];
      xyzr[2] = z[1]; 
      rotatep(xyzr,axis,angle,temp);
      x3[j] = xyzr[0];
      y3[j] = xyzr[1];
      z3[j] = xyzr[2];
    }

    // form count-1 rows of polygons
    for (i = 0; i < count-1; ++i) {

      // move x2,y2,z2 to x1,y,z1 and x3,y3,z3 to x2,y2,z2
      for (j = 0; j < numpts+1; ++j) {
	xm1[j] = x1[j];
	ym1[j] = y1[j];
	zm1[j] = z1[j];
	x1[j] = x2[j];
	y1[j] = y2[j];
	z1[j] = z2[j];
	x2[j] = x3[j];
	y2[j] = y3[j];
	z2[j] = z3[j];
      }
 
      // calculate rotated coordinates for x3
      if (i < count-2) {
	for (j = 0; j < numpts+1; ++j) {
	  // find the angle in radians
	  angle = (double)(j*angleOfRotation/numpts) * PI/180;
	  xyzr[0] = x[i+2];
	  xyzr[1] = y[i+2];
	  xyzr[2] = z[i+2];
	  rotatep(xyzr,axis,angle,temp);
	  x3[j] = xyzr[0];
	  y3[j] = xyzr[1];
	  z3[j] = xyzr[2];
	}
      }

      // form the polygons
      for (j = 0; j < numpolys; ++j) {
	if (keyWordLineHasRGBA == 0)
	  printf("polygons\n");
	else {
	  printf("%s\n",keyrgbaline);
	}
	
	// point1: bottom left:  normal: 1-2 X 1-4
	cnt = 1;
	uVector[0] = x2[j+1] - x1[j+1];
	uVector[1] = y2[j+1] - y1[j+1];
	uVector[2] = z2[j+1] - z1[j+1];
	vVector[0] = x1[j] - x1[j+1];
	vVector[1] = y1[j] - y1[j+1];
	vVector[2] = z1[j] - z1[j+1];
	calculateCrossProduct(uVector, vVector, normal1, vectorMagnitude);
	if (j != (numpts-1)) {
	  // average with the top left of the next one down: 4-1 X 4-3
	  uVector[0] = x1[j+2] - x1[j+1];
	  uVector[1] = y1[j+2] - y1[j+1];
	  uVector[2] = z1[j+2] - z1[j+1];
	  vVector[0] = x2[j+1] - x1[j+1];
	  vVector[1] = y2[j+1] - y1[j+1];
	  vVector[2] = z2[j+1] - z1[j+1];
	  calculateCrossProduct(uVector, vVector, normal2, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal2[0] = 0.0;
	  normal2[1] = 0.0;
	  normal2[2] = 0.0;
	}
	if (i != 0) {
	  // find the normal of on the left: normal: 2-3 X 2-1
	  uVector[0] = x1[j] - x1[j+1];
	  uVector[1] = y1[j] - y1[j+1];
	  uVector[2] = z1[j] - z1[j+1];
	  vVector[0] = xm1[j+1] - x1[j+1];
	  vVector[1] = ym1[j+1] - y1[j+1];
	  vVector[2] = zm1[j+1] - z1[j+1];
	  calculateCrossProduct(uVector, vVector, normal3, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal3[0] = 0.0;
	  normal3[1] = 0.0;
	  normal3[2] = 0.0;
	}
	if (j != (numpts-1) &&  i != 0) {
	  // find the normal on the left and below: point 3: 3-4 X 3-2
	  uVector[0] = xm1[j+1] - x1[j+1];
	  uVector[1] = ym1[j+1] - y1[j+1];
	  uVector[2] = zm1[j+1] - z1[j+1];
	  vVector[0] = x1[j+2] - x1[j+1];
	  vVector[1] = y1[j+2] - y1[j+1];
	  vVector[2] = z1[j+2] - z1[j+1];
	  calculateCrossProduct(uVector, vVector, normal4, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal4[0] = 0.0;
	  normal4[1] = 0.0;
	  normal4[2] = 0.0;
	}

	// find the normal of the polygon on the left and below
	normalAvg[0]=(normal1[0]+normal2[0]+normal3[0]+normal4[0])/((double)cnt);
	normalAvg[1]=(normal1[1]+normal2[1]+normal3[1]+normal4[1])/((double)cnt);
	normalAvg[2]=(normal1[2]+normal2[2]+normal3[2]+normal4[2])/((double)cnt);
	normalizeVector(normalAvg, vectorMagnitude);
	if (rgba[i] == 1)
	  printf("%f %f %f %f %f %f %f %f %f %f\n",x1[j+1],y1[j+1],z1[j+1],r[i],g[i],b[i],a[i],normalAvg[0],normalAvg[1],normalAvg[2]);
	else
	  printf("%f %f %f %f %f %f\n",x1[j+1],y1[j+1],z1[j+1],normalAvg[0],normalAvg[1],normalAvg[2]);

	// point2: bottom right: normal: 2-3 X 2-1
	cnt = 1;
	uVector[0] = x2[j] - x2[j+1];
	uVector[1] = y2[j] - y2[j+1];
	uVector[2] = z2[j] - z2[j+1];
	vVector[0] = x1[j+1] - x2[j+1];
	vVector[1] = y1[j+1] - y2[j+1];
	vVector[2] = z1[j+1] - z2[j+1];
	calculateCrossProduct(uVector, vVector, normal1, vectorMagnitude);
	if (j != (numpts-1)) {
	  // average with the top right of the next one down: 3-4 X 3-2
	  uVector[0] = x1[j+1] - x2[j+1];
	  uVector[1] = y1[j+1] - y2[j+1];
	  uVector[2] = z1[j+1] - z2[j+1];
	  vVector[0] = x2[j+2] - x2[j+1];
	  vVector[1] = y2[j+2] - y2[j+1];
	  vVector[2] = z2[j+2] - z2[j+1];
	  calculateCrossProduct(uVector, vVector, normal2, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal2[0] = 0.0;
	  normal2[1] = 0.0;
	  normal2[2] = 0.0;
	}
	if (i != (count-1)) {
	  // find the normal of on the right : normal:1-2 X 1-4 
	  uVector[0] = x3[j+1] - x2[j+1];
	  uVector[1] = y3[j+1] - y2[j+1];
	  uVector[2] = z3[j+1] - z2[j+1];
	  vVector[0] = x2[j] - x2[j+1];
	  vVector[1] = y2[j] - y2[j+1];
	  vVector[2] = z2[j] - z2[j+1];
	  calculateCrossProduct(uVector, vVector, normal3, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal3[0] = 0.0;
	  normal3[1] = 0.0;
	  normal3[2] = 0.0;
	}
	if (i != (count-1) && j !=(numpts-1)) {
	  // find the normal of on the right below: normal:4-1 X 4-3
	  uVector[0] = x2[j+2] - x2[j+1];
	  uVector[1] = y2[j+2] - y2[j+1];
	  uVector[2] = z2[j+2] - z2[j+1];
	  vVector[0] = x3[j+1] - x2[j+1];
	  vVector[1] = y3[j+1] - y2[j+1];
	  vVector[2] = z3[j+1] - z2[j+1];
	  calculateCrossProduct(uVector, vVector, normal4, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal4[0] = 0.0;
	  normal4[1] = 0.0;
	  normal4[2] = 0.0;
	}
	// find the normal of the polygon on the left and below
	normalAvg[0]=(normal1[0]+normal2[0]+normal3[0]+normal4[0])/((double)cnt);
	normalAvg[1]=(normal1[1]+normal2[1]+normal3[1]+normal4[1])/((double)cnt);
	normalAvg[2]=(normal1[2]+normal2[2]+normal3[2]+normal4[2])/((double)cnt);
	normalizeVector(normalAvg, vectorMagnitude);
	if (rgba[i] == 1)
	  printf("%f %f %f %f %f %f %f %f %f %f\n",x2[j+1],y2[j+1],z2[j+1],r[i+1],g[i+1],b[i+1],a[i+1],normalAvg[0],normalAvg[1],normalAvg[2]);
	else
	  printf("%f %f %f %f %f %f\n",x2[j+1],y2[j+1],z2[j+1],normalAvg[0],normalAvg[1],normalAvg[2]);


	// point3: top right:    normal: 3-4 X 3-2
	cnt = 1;
	uVector[0] = x1[j] - x2[j];
	uVector[1] = y1[j] - y2[j];
	uVector[2] = z1[j] - z2[j];
	vVector[0] = x2[j+1] - x2[j];
	vVector[1] = y2[j+1] - y2[j];
	vVector[2] = z2[j+1] - z2[j];
	calculateCrossProduct(uVector, vVector, normal1, vectorMagnitude);
	if (j != 0) {
	  // average with the one above: 2-3 X 2-1
	  uVector[0] = x2[j-1] - x2[j];
	  uVector[1] = y2[j-1] - y2[j];
	  uVector[2] = z2[j-1] - z2[j];
	  vVector[0] = x1[j] - x2[j];
	  vVector[1] = y1[j] - y2[j];
	  vVector[2] = z1[j] - z2[j];
	  calculateCrossProduct(uVector, vVector, normal2, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal2[0] = 0.0;
	  normal2[1] = 0.0;
	  normal2[2] = 0.0;
	}
	if (j != 0 && i != (count-1)) {
	  // average with the one above and to the right: 1-2 X 1-4
	  uVector[0] = x3[j] - x2[j];
	  uVector[1] = y3[j] - y2[j];
	  uVector[2] = z3[j] - z2[j];
	  vVector[0] = x2[j-1] - x2[j];
	  vVector[1] = y2[j-1] - y2[j];
	  vVector[2] = z2[j-1] - z2[j];
	  calculateCrossProduct(uVector, vVector, normal3, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal3[0] = 0.0;
	  normal3[1] = 0.0;
	  normal3[2] = 0.0;
	}
	if (i != (count-1)) {
	  // average with the one to the right: 4-1 X 4-3
	  uVector[0] = x2[j+1] - x2[j];
	  uVector[1] = y2[j+1] - y2[j];
	  uVector[2] = z2[j+1] - z2[j];
	  vVector[0] = x3[j] - x2[j];
	  vVector[1] = y3[j] - y2[j];
	  vVector[2] = z3[j] - z2[j];
	  calculateCrossProduct(uVector, vVector, normal4, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal4[0] = 0.0;
	  normal4[1] = 0.0;
	  normal4[2] = 0.0;
	}
	// find the normal of the polygon on the left and below
	normalAvg[0]=(normal1[0]+normal2[0]+normal3[0]+normal4[0])/((double)cnt);
	normalAvg[1]=(normal1[1]+normal2[1]+normal3[1]+normal4[1])/((double)cnt);
	normalAvg[2]=(normal1[2]+normal2[2]+normal3[2]+normal4[2])/((double)cnt);
	normalizeVector(normalAvg, vectorMagnitude);
	if (rgba[i] == 1)
	  printf("%f %f %f %f %f %f %f %f %f %f\n",x2[j],y2[j],z2[j],r[i+1],g[i+1],b[i+1],a[i+1],normalAvg[0],normalAvg[1],normalAvg[2]);
	else
	  printf("%f %f %f %f %f %f\n",x2[j],y2[j],z2[j],normalAvg[0],normalAvg[1],normalAvg[2]);
	

	// point4: top left:     normal: 4-1 X 4-3
	cnt = 1;
	uVector[0] = x1[j+1] - x1[j];
	uVector[1] = y1[j+1] - y1[j];
	uVector[2] = z1[j+1] - z1[j];
	vVector[0] = x2[j] - x1[j];
	vVector[1] = y2[j] - y1[j];
	vVector[2] = z2[j] - z1[j];
	calculateCrossProduct(uVector, vVector, normal1, vectorMagnitude);
	if (j != 0) {
	  // average with the one above: 1-2 X 1-4
	  uVector[0] = x2[j] - x1[j];
	  uVector[1] = y2[j] - y1[j];
	  uVector[2] = z2[j] - z1[j];
	  vVector[0] = x1[j-1] - x1[j];
	  vVector[1] = y1[j-1] - y1[j];
	  vVector[2] = z1[j-1] - z1[j];
	  calculateCrossProduct(uVector, vVector, normal2, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal2[0] = 0.0;
	  normal2[1] = 0.0;
	  normal2[2] = 0.0;
	}
	if (j != 0 && i != 0) {
	  // average with the one above and to the left:2-3 X 2-1
	  uVector[0] = x2[j-1] - x1[j];
	  uVector[1] = y2[j-1] - y1[j];
	  uVector[2] = z2[j-1] - z1[j];
	  vVector[0] = xm1[j] - x1[j];
	  vVector[1] = ym1[j] - y1[j];
	  vVector[2] = zm1[j] - z1[j];
	  calculateCrossProduct(uVector, vVector, normal3, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal3[0] = 0.0;
	  normal3[1] = 0.0;
	  normal3[2] = 0.0;
	}
	if (i != 0) {
	  // average with the one to the left: 3-4 X 3-2
	  uVector[0] = xm1[j] - x1[j];
	  uVector[1] = ym1[j] - y1[j];
	  uVector[2] = zm1[j] - z1[j];
	  vVector[0] = x1[j+1] - x1[j];
	  vVector[1] = y1[j+1] - y1[j];
	  vVector[2] = z1[j+1] - z1[j];
	  calculateCrossProduct(uVector, vVector, normal4, vectorMagnitude);
	  cnt = cnt + 1;
	}
	else {
	  normal4[0] = 0.0;
	  normal4[1] = 0.0;
	  normal4[2] = 0.0;
	}
	// find the normal of the polygon on the left and below
	normalAvg[0]=(normal1[0]+normal2[0]+normal3[0]+normal4[0])/((double)cnt);
	normalAvg[1]=(normal1[1]+normal2[1]+normal3[1]+normal4[1])/((double)cnt);
	normalAvg[2]=(normal1[2]+normal2[2]+normal3[2]+normal4[2])/((double)cnt);
	normalizeVector(normalAvg, vectorMagnitude);
	if (rgba[i] == 1)
	  printf("%f %f %f %f %f %f %f %f %f %f\n",x1[j],y1[j],z1[j],r[i],g[i],b[i],a[i],normalAvg[0],normalAvg[1],normalAvg[2]);
	else
	  printf("%f %f %f %f %f %f\n",x1[j],y1[j],z1[j],normalAvg[0],normalAvg[1],normalAvg[2]);
      }
    }
  }
}


/* Prints information on usage*/
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: gtb-sor x/y/z [n] [degrees]\n");
	exit(1);
}

void calculateCrossProduct(double *uVector, double *vVector, double *normalVector, double vectorMagnitude)
{
  /* Calculate the cross product */
  normalVector[0] = (uVector[1]) * (vVector[2]) - (uVector[2]) * (vVector[1]);
  normalVector[1] = (uVector[2]) * (vVector[0]) - (uVector[0]) * (vVector[2]);
  normalVector[2] = (uVector[0]) * (vVector[1]) - (uVector[1]) * (vVector[0]);

  vectorMagnitude = sqrt( normalVector[0]*normalVector[0] + 
			  normalVector[1]*normalVector[1] + 
			  normalVector[2]*normalVector[2]);

  if(fabs(vectorMagnitude) > 0.0)
  {
    normalVector[0] = normalVector[0] / vectorMagnitude;
    normalVector[1] = normalVector[1] / vectorMagnitude;
    normalVector[2] = normalVector[2] / vectorMagnitude;
  }
}

void rotatep(double* xyzr, int axis, double angle, double* temp) {
  temp[0] = xyzr[0];
  temp[1] = xyzr[1];
  temp[2] = xyzr[2];

  if (axis == 1) {
    xyzr[0] = temp[0];
    xyzr[1] = cos(angle)*(temp[1])-sin(angle)*(temp[2]);
    xyzr[2] = sin(angle)*(temp[1])+cos(angle)*(temp[2]);
  }
  else if (axis == 2) {
    xyzr[0] = cos(angle)*(temp[0])+sin(angle)*(temp[2]);
    xyzr[1] = temp[1];
    xyzr[2] = -sin(angle)*(temp[0])+cos(angle)*(temp[2]);
  }
  else if (axis == 3) {
    xyzr[0] = cos(angle)*(temp[0])-sin(angle)*(temp[1]);
    xyzr[1] = sin(angle)*(temp[0])+sin(angle)*(temp[1]);
    xyzr[2] = temp[2];
  }
}

void normalizeVector(double *normalVector, double vectorMagnitude)
{
  vectorMagnitude = sqrt( normalVector[0]*normalVector[0] + 
			  normalVector[1]*normalVector[1] + 
			  normalVector[2]*normalVector[2]);

  if(fabs(vectorMagnitude) > 0.0)
  {
    normalVector[0] = normalVector[0] / vectorMagnitude;
    normalVector[1] = normalVector[1] / vectorMagnitude;
    normalVector[2] = normalVector[2] / vectorMagnitude;
  }
}
