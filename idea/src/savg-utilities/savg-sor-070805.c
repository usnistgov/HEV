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
#include "rotate.h"
#include "savgLineEvaluator.c"

#ifndef PI
#define PI 3.1415927
#endif

#define ARRAY_CAPACITY (500)

/* 
 * Prints information on usage
 */
void usage();

/*
 * Returns the distance of the points from the given axis
 */
double distanceFromAxis(int axis, double x, double y, double z);

/*
 * Stores polygons corresponding to one line being revolved around
 * circle
 */
void storePolygon( int index, double *fromPointAVec3, double *toPointAVec3, double *lineArgbaVec4, double *toPointBVec3, double *fromPointBVec3, double *lineBrgbaVec4);

/*
 * Print the contents of polygon store
 */
void printRevolutionOfLine();

/*
 * Calculates the normals for the ends of the first and last polygon
 * in a revolution if not connected (ie not 360 degree rotation)
 */
void calculateAndStoreEdgeNormals(int beginningPolygon, int endingPolygon);

/*
 * Sent are the indices of the adjacent polygons (stored in polygonStore array)
 * Calculates the average normals for the two points on the adjacent line and
 * stores them into the polygonStore array.
 */
void calculateAndStoreAveragedNormals(int polygon1, int polygon2);

/*
 * Sent are the u and v vectors.
 * Returns a pointer to an array containing the resultant
 * vector of u x v.
 */
double * calculateCrossProduct(double *uVector, double *vVector);

/* 
 * Stores polygons generated from one line as it's revolved.
 * [size of array][point number, starting bottom left going counter-clockwise][vertex, rgba, normal]
 */
double ***polygonStore;

const double tolerance = .00001;

/*
 * Determines axis of rotation: X_AXIS, Y_AXIS, Z_AXIS
 */
enum ax axis;

int endOfArrayIndex;
int maxArraySize;

int main(argc, argv)
int argc;
char *argv[];
{
  char line[STRING_LENGTH];
  int i;
  int numberOfLineCopies;
  double x,y,z;
  double angleOfRotation;
  double angleInRadians;
  double angleIncrement;
  double currentAngle;
  double firstPointAVec3[3],
         firstPointBVec3[3],
         fromPointAVec3[3],
         fromPointBVec3[3],
         toPointAVec3[3],
         toPointBVec3[3];
  double lineArgbaVec4[4],
         lineBrgbaVec4[4];
  int keyWordLineHasRGBA;
  int pargc = 0;
  char **pargv  = NULL;

/*
 * INITIALIZATION
 */

  numberOfLineCopies = 16;
  angleOfRotation = 360;
  axis = Z_AXIS;		/* default is z-axis */
  maxArraySize = ARRAY_CAPACITY;
  polygonStore = (double ***) realloc(polygonStore, sizeof(double*)*maxArraySize);

/*
 * COMMAND LINE PROCESSING
 */

  if (argc == 1) {
    usage();
    exit(EXIT_FAILURE);
  }

  if (strcasecmp(argv[1],"x")==0)
    axis = X_AXIS;
  else if (strcasecmp(argv[1],"y")==0)
    axis = Y_AXIS;
  else if (strcasecmp(argv[1],"z")==0)
    axis = Z_AXIS;
  else {
    usage();
    exit(EXIT_FAILURE);
  }
   
  if (argc > 2)
    numberOfLineCopies = atoi(argv[2]);
  
  if (argc == 4)
    angleOfRotation = atof(argv[3]);

  if (argc > 4) {
    usage();
    exit(EXIT_FAILURE);
  }
  // printf("n = %d %f\n",numberOfLineCopies,angleOfRotation);

/*
 * BODY
 */
  angleIncrement = angleOfRotation / numberOfLineCopies;

  if(!getLine(line))
    return 0;

  while( jumpToNearestLineGeometry(line,pargv,pargc) ) {
    //printf("line at line geom: %s\n",line);
    // see how many parameters to know if rgba info is on keyword line
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    currentAngle = 0;
    if (pargc == 5 || pargc == 6) {
      keyWordLineHasRGBA = 1;

      lineArgbaVec4[1] = lineBrgbaVec4[0] = atof(pargv[pargc-4]);
      lineArgbaVec4[1] = lineBrgbaVec4[1] = atof(pargv[pargc-3]);
      lineArgbaVec4[2] = lineBrgbaVec4[2] = atof(pargv[pargc-2]);
      lineArgbaVec4[3] = lineBrgbaVec4[3] = atof(pargv[pargc-1]);
    }
    else {
      keyWordLineHasRGBA = 0;
    }
    
    // get the next line
    if(!getLine(line))
      return 0;
    //printf("next line after line keyword: %s\n",line);

    while( isBlankLine(line) || containsComment(line) ) {
      printf("check for blank\n");
      if(!getLine(line))
	return 0;
      dpf_charsToArgcArgv(line, &pargc, &pargv);
    }

    dpf_charsToArgcArgv(line, &pargc, &pargv);
    fromPointAVec3[0] = firstPointAVec3[0] = atof(pargv[0]);
    fromPointAVec3[1] = firstPointAVec3[1] = atof(pargv[1]);
    fromPointAVec3[2] = firstPointAVec3[2] = atof(pargv[2]);

    if( !keyWordLineHasRGBA ) {
      dpf_charsToArgcArgv(line, &pargc, &pargv);
      if (pargc >= 7) {
	lineArgbaVec4[0] = atof(pargv[3]);
	lineArgbaVec4[1] = atof(pargv[4]);
	lineArgbaVec4[2] = atof(pargv[5]);
	lineArgbaVec4[3] = atof(pargv[6]);
      }
      else {
	lineArgbaVec4[0] =
          lineArgbaVec4[1] =
          lineArgbaVec4[2] =
          lineArgbaVec4[3] = 1;
      }
    }

    while(isCorrectDataLine(line,pargv,pargc)) {
      // printf("next data line: %s\n",line);
      dpf_charsToArgcArgv(line, &pargc, &pargv);

      fromPointBVec3[0] = firstPointBVec3[0] = atof(pargv[0]);
      fromPointBVec3[1] = firstPointBVec3[1] = atof(pargv[1]);
      fromPointBVec3[2] = firstPointBVec3[2] = atof(pargv[2]);
      printf("got xyz: %f %f %f\n",fromPointBVec3[0],fromPointBVec3[1],fromPointBVec3[2]);

      // pull out rgba from line if there (xyzrgba or xyzrgbannn)
      if( pargc == 7 || pargc == 10) {
	lineBrgbaVec4[0] = atof(pargv[3]);
	lineBrgbaVec4[1] = atof(pargv[4]);
	lineBrgbaVec4[2] = atof(pargv[5]);
	lineBrgbaVec4[3] = atof(pargv[6]);
      }
      else {
	lineBrgbaVec4[0] =
	  lineBrgbaVec4[1] =
	  lineBrgbaVec4[2] =
	  lineBrgbaVec4[3] = 1;
      }

      currentAngle = angleIncrement;
      
      i = 0;
      while( fabs(currentAngle) < fabs(angleOfRotation) + tolerance ) {
	printf("current angle: %f\n",currentAngle);
	x = firstPointAVec3[0];
	y = firstPointAVec3[1];
	z = firstPointAVec3[2];
	
	angleInRadians = currentAngle * (PI / 180);

	printf("pointavec 1: %f %f %f\n",x,y,z);	
	rotate(&x, &y, &z, axis, angleInRadians);
	printf("pointavec 2: %f %f %f\n",x,y,z);
	toPointAVec3[0] = x;
	toPointAVec3[1] = y;
	toPointAVec3[2] = z;
	
	x = firstPointBVec3[0];
	y = firstPointBVec3[1];
	z = firstPointBVec3[2];
	
	printf("pointbvec 1: %f %f %f\n",x,y,z);
	rotate(&x, &y, &z, axis, angleInRadians);
	printf("pointbvec 2: %f %f %f\n",x,y,z);
	
	toPointBVec3[0] = x;
	toPointBVec3[1] = y;
	toPointBVec3[2] = z;	 
	
	storePolygon(i, fromPointAVec3, toPointAVec3, lineArgbaVec4, toPointBVec3, fromPointBVec3, lineBrgbaVec4);	  
	
	fromPointAVec3[0] = toPointAVec3[0];
	fromPointAVec3[1] = toPointAVec3[1];
	fromPointAVec3[2] = toPointAVec3[2];
	fromPointBVec3[0] = toPointBVec3[0];
	fromPointBVec3[1] = toPointBVec3[1];
	fromPointBVec3[2] = toPointBVec3[2];
	
	currentAngle += angleIncrement;
	i++;
      }
      
      i--;
      
      endOfArrayIndex = i;
      
      if( fabs(angleOfRotation) <= (360 + tolerance) && fabs(angleOfRotation) >= (360 - tolerance) ) {
	// First and last lines in revolution are the same (360 degrees)
	
	// Calculate and store the normals for the adjacent sides of the
	// first and last polygons.
	printf("pointa\n");
	calculateAndStoreAveragedNormals(endOfArrayIndex, 0);
      }
      else {
	// Make seperate normals for the first and last line
	printf("pointb\n");
	calculateAndStoreEdgeNormals(0, endOfArrayIndex);
      }
      
      for(i = 0 ; i < endOfArrayIndex; i++)
	{
	  calculateAndStoreAveragedNormals(i, i+1);
	}
      
      printRevolutionOfLine(polygonStore);
      
      while( isBlankLine(line) || containsComment(line) ) {
	if(!getLine(line))
	  return 0;
      }
      
      fromPointAVec3[0] = firstPointAVec3[0] = firstPointBVec3[0];
      fromPointAVec3[1] = firstPointAVec3[1] = firstPointBVec3[1];
      fromPointAVec3[2] = firstPointAVec3[2] = firstPointBVec3[2];
      
      lineArgbaVec4[0] = lineBrgbaVec4[0];
      lineArgbaVec4[1] = lineBrgbaVec4[1];
      lineArgbaVec4[2] = lineBrgbaVec4[2];
      lineArgbaVec4[3] = lineBrgbaVec4[3];

      if(!getLine(line))
	  return 0;
      
      } //end while
  }

  return 0;
}	


double distanceFromAxis(int axis, double x, double y, double z)
     /*
      * Returns the distance that the given point (given by x, y, z) is from
      * the given axis.
      */
{
  if(axis == X_AXIS)
    {
      return (double) sqrt( z*z + y*y );
    }
  else if(axis == Y_AXIS)
    {
      return (double) sqrt( x*x + z*z );
    }
  else
    /* axis == Z_AXIS */
    {
      return (double) sqrt( x*x + y*y );
    }
}


void storePolygon(int index, double *fromPointAVec3, double *toPointAVec3, double *lineArgbaVec4, double *toPointBVec3, double *fromPointBVec3, double *lineBrgbaVec4)
{
  if(index >= maxArraySize)
    {
      maxArraySize += ARRAY_CAPACITY;
      polygonStore = (double ***) realloc(polygonStore, sizeof(double*)*maxArraySize );
    }
  
  polygonStore[index] = (double **) malloc(sizeof(double)*4);
  
  polygonStore[index][0] = (double *) malloc(sizeof(double)*10);
  polygonStore[index][0][0] = fromPointAVec3[0];
  polygonStore[index][0][1] = fromPointAVec3[1];
  polygonStore[index][0][2] = fromPointAVec3[2];
  polygonStore[index][0][3] = lineArgbaVec4[0];
  polygonStore[index][0][4] = lineArgbaVec4[1];
  polygonStore[index][0][5] = lineArgbaVec4[2];
  polygonStore[index][0][6] = lineArgbaVec4[3];
  
  polygonStore[index][1] = (double *) malloc(sizeof(double)*10);
  polygonStore[index][1][0] = toPointAVec3[0];
  polygonStore[index][1][1] = toPointAVec3[1];
  polygonStore[index][1][2] = toPointAVec3[2];
  polygonStore[index][1][3] = lineArgbaVec4[0];
  polygonStore[index][1][4] = lineArgbaVec4[1];
  polygonStore[index][1][5] = lineArgbaVec4[2];
  polygonStore[index][1][6] = lineArgbaVec4[3];

  polygonStore[index][2] = (double *) malloc(sizeof(double)*10);
  polygonStore[index][2][0] = toPointBVec3[0];
  polygonStore[index][2][1] = toPointBVec3[1];
  polygonStore[index][2][2] = toPointBVec3[2];
  polygonStore[index][2][3] = lineBrgbaVec4[0];
  polygonStore[index][2][4] = lineBrgbaVec4[1];
  polygonStore[index][2][5] = lineBrgbaVec4[2];
  polygonStore[index][2][6] = lineBrgbaVec4[3];

  polygonStore[index][3] = (double *) malloc(sizeof(double)*10);
  polygonStore[index][3][0] = fromPointBVec3[0];
  polygonStore[index][3][1] = fromPointBVec3[1];
  polygonStore[index][3][2] = fromPointBVec3[2];
  polygonStore[index][3][3] = lineBrgbaVec4[0];
  polygonStore[index][3][4] = lineBrgbaVec4[1];
  polygonStore[index][3][5] = lineBrgbaVec4[2];
  polygonStore[index][3][6] = lineBrgbaVec4[3];
}


void calculateAndStoreEdgeNormals(int polygon1, int polygon2)
{
  double *normal1Vec3;
  double *normal2Vec3;
  double uVector[3];
  double vVector[3];

  if( distanceFromAxis(axis, polygonStore[polygon1][0][0], polygonStore[polygon1][0][1], polygonStore[polygon1][0][2]) < tolerance )
  /*
   * Point A of polygon1 is on axis
   */
  {
    printf("calculate n\n");
    uVector[0] = polygonStore[polygon1][2][0] - polygonStore[polygon1][0][0];
    uVector[1] = polygonStore[polygon1][2][1] - polygonStore[polygon1][0][1];  
    uVector[2] = polygonStore[polygon1][2][2] - polygonStore[polygon1][0][2];  

    vVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][0][0];
    vVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][0][1];  
    vVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][0][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon1][0][7] = normal2Vec3[0];
    polygonStore[polygon1][0][8] = normal2Vec3[1];
    polygonStore[polygon1][0][9] = normal2Vec3[2];

    uVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][3][0];
    uVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][3][1];  
    uVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][3][2];  

    vVector[0] = polygonStore[polygon1][2][0] - polygonStore[polygon1][3][0];
    vVector[1] = polygonStore[polygon1][2][1] - polygonStore[polygon1][3][1];  
    vVector[2] = polygonStore[polygon1][2][2] - polygonStore[polygon1][3][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon1][3][7] = normal2Vec3[0];
    polygonStore[polygon1][3][8] = normal2Vec3[1];
    polygonStore[polygon1][3][9] = normal2Vec3[2];


    uVector[0] = polygonStore[polygon2][2][0] - polygonStore[polygon2][1][0];
    uVector[1] = polygonStore[polygon2][2][1] - polygonStore[polygon2][1][1];  
    uVector[2] = polygonStore[polygon2][2][2] - polygonStore[polygon2][1][2];  

    vVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][1][0];
    vVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][1][1];  
    vVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][1][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon2][1][7] = normal1Vec3[0];
    polygonStore[polygon2][1][8] = normal1Vec3[1];
    polygonStore[polygon2][1][9] = normal1Vec3[2];

    uVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][2][0];
    uVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][2][1];  
    uVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][2][2];  

    vVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][2][0];
    vVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][2][1];  
    vVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][2][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon2][2][7] = normal1Vec3[0];
    polygonStore[polygon2][2][8] = normal1Vec3[1];
    polygonStore[polygon2][2][9] = normal1Vec3[2];
  }
  else if( distanceFromAxis(axis, polygonStore[polygon1][3][0], polygonStore[polygon1][3][1], polygonStore[polygon1][3][2]) < tolerance )
  /*
   * Point B of polygon1 is on axis
   */
  {
    printf("on axis\n");
    uVector[0] = polygonStore[polygon1][1][0] - polygonStore[polygon1][0][0];
    uVector[1] = polygonStore[polygon1][1][1] - polygonStore[polygon1][0][1];  
    uVector[2] = polygonStore[polygon1][1][2] - polygonStore[polygon1][0][2];  

    vVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][0][0];
    vVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][0][1];  
    vVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][0][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon1][0][7] = normal2Vec3[0];
    polygonStore[polygon1][0][8] = normal2Vec3[1];
    polygonStore[polygon1][0][9] = normal2Vec3[2];

    uVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][3][0];
    uVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][3][1];  
    uVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][3][2];  

    vVector[0] = polygonStore[polygon1][1][0] - polygonStore[polygon1][3][0];
    vVector[1] = polygonStore[polygon1][1][1] - polygonStore[polygon1][3][1];  
    vVector[2] = polygonStore[polygon1][1][2] - polygonStore[polygon1][3][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon1][3][7] = normal2Vec3[0];
    polygonStore[polygon1][3][8] = normal2Vec3[1];
    polygonStore[polygon1][3][9] = normal2Vec3[2];


    uVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][1][0];
    uVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][1][1];  
    uVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][1][2];  

    vVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][1][0];
    vVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][1][1];  
    vVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][1][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon2][1][7] = normal1Vec3[0];
    polygonStore[polygon2][1][8] = normal1Vec3[1];
    polygonStore[polygon2][1][9] = normal1Vec3[2];

    uVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][2][0];
    uVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][2][1];  
    uVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][2][2];  

    vVector[0] = polygonStore[polygon2][1][0] - polygonStore[polygon2][2][0];
    vVector[1] = polygonStore[polygon2][1][1] - polygonStore[polygon2][2][1];  
    vVector[2] = polygonStore[polygon2][1][2] - polygonStore[polygon2][2][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon2][2][7] = normal1Vec3[0];
    polygonStore[polygon2][2][8] = normal1Vec3[1];
    polygonStore[polygon2][2][9] = normal1Vec3[2];
  }
  else
    /*
     * No sides are touching the axis
     */
  {
    uVector[0] = polygonStore[polygon1][1][0] - polygonStore[polygon1][0][0];
    uVector[1] = polygonStore[polygon1][1][1] - polygonStore[polygon1][0][1];  
    uVector[2] = polygonStore[polygon1][1][2] - polygonStore[polygon1][0][2];  

    vVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][0][0];
    vVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][0][1];  
    vVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][0][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon1][0][7] = normal2Vec3[0];
    polygonStore[polygon1][0][8] = normal2Vec3[1];
    polygonStore[polygon1][0][9] = normal2Vec3[2];

    uVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][3][0];
    uVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][3][1];  
    uVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][3][2];  

    vVector[0] = polygonStore[polygon1][2][0] - polygonStore[polygon1][3][0];
    vVector[1] = polygonStore[polygon1][2][1] - polygonStore[polygon1][3][1];  
    vVector[2] = polygonStore[polygon1][2][2] - polygonStore[polygon1][3][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon1][3][7] = normal2Vec3[0];
    polygonStore[polygon1][3][8] = normal2Vec3[1];
    polygonStore[polygon1][3][9] = normal2Vec3[2];


    uVector[0] = polygonStore[polygon2][2][0] - polygonStore[polygon2][1][0];
    uVector[1] = polygonStore[polygon2][2][1] - polygonStore[polygon2][1][1];  
    uVector[2] = polygonStore[polygon2][2][2] - polygonStore[polygon2][1][2];  

    vVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][1][0];
    vVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][1][1];  
    vVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][1][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon2][1][7] = normal1Vec3[0];
    polygonStore[polygon2][1][8] = normal1Vec3[1];
    polygonStore[polygon2][1][9] = normal1Vec3[2];

    uVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][2][0];
    uVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][2][1];  
    uVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][2][2];  

    vVector[0] = polygonStore[polygon2][1][0] - polygonStore[polygon2][2][0];
    vVector[1] = polygonStore[polygon2][1][1] - polygonStore[polygon2][2][1];  
    vVector[2] = polygonStore[polygon2][1][2] - polygonStore[polygon2][2][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    polygonStore[polygon2][2][7] = normal1Vec3[0];
    polygonStore[polygon2][2][8] = normal1Vec3[1];
    polygonStore[polygon2][2][9] = normal1Vec3[2];
  }
}

void calculateAndStoreAveragedNormals(int polygon1, int polygon2)
{
  double *normal1Vec3;
  double *normal2Vec3;
  double uVector[3];
  double vVector[3];

  if( distanceFromAxis(axis, polygonStore[polygon1][0][0], polygonStore[polygon1][0][1], polygonStore[polygon1][0][2]) < tolerance ) {
    // Point A of polygon1 is on axis
    printf("pointc\n");
    uVector[0] = polygonStore[polygon1][2][0] - polygonStore[polygon1][1][0];
    uVector[1] = polygonStore[polygon1][2][1] - polygonStore[polygon1][1][1];  
    uVector[2] = polygonStore[polygon1][2][2] - polygonStore[polygon1][1][2];  

    vVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][1][0];
    vVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][1][1];  
    vVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][1][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    uVector[0] = polygonStore[polygon2][2][0] - polygonStore[polygon2][0][0];
    uVector[1] = polygonStore[polygon2][2][1] - polygonStore[polygon2][0][1];  
    uVector[2] = polygonStore[polygon2][2][2] - polygonStore[polygon2][0][2];  

    vVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][0][0];
    vVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][0][1];  
    vVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][0][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    /*
     * Average the two normals and store them into the proper location
     */
    polygonStore[polygon1][1][7] = polygonStore[polygon2][0][7] = (normal1Vec3[0] + normal2Vec3[0]) / 2;
    polygonStore[polygon1][1][8] = polygonStore[polygon2][0][8] = (normal1Vec3[1] + normal2Vec3[1]) / 2;
    polygonStore[polygon1][1][9] = polygonStore[polygon2][0][9] = (normal1Vec3[2] + normal2Vec3[2]) / 2;


    uVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][2][0];
    uVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][2][1];  
    uVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][2][2];  

    vVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][2][0];
    vVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][2][1];  
    vVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][2][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    uVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][3][0];
    uVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][3][1];  
    uVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][3][2];  

    vVector[0] = polygonStore[polygon2][2][0] - polygonStore[polygon2][3][0];
    vVector[1] = polygonStore[polygon2][2][1] - polygonStore[polygon2][3][1];  
    vVector[2] = polygonStore[polygon2][2][2] - polygonStore[polygon2][3][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    /*
     * Average the two normals and store them into the proper location
     */
    polygonStore[polygon1][2][7] = polygonStore[polygon2][3][7] = (normal1Vec3[0] + normal2Vec3[0]) / 2;
    polygonStore[polygon1][2][8] = polygonStore[polygon2][3][8] = (normal1Vec3[1] + normal2Vec3[1]) / 2;
    polygonStore[polygon1][2][9] = polygonStore[polygon2][3][9] = (normal1Vec3[2] + normal2Vec3[2]) / 2;
  }
  else if( distanceFromAxis(axis, polygonStore[polygon1][3][0], polygonStore[polygon1][3][1], polygonStore[polygon1][3][2]) < tolerance ) {
    // Point B of polygon1 is on axis
    printf("pointd\n");
    uVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][1][0];
    uVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][1][1];  
    uVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][1][2];  

    vVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][1][0];
    vVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][1][1];  
    vVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][1][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    uVector[0] = polygonStore[polygon2][1][0] - polygonStore[polygon2][0][0];
    uVector[1] = polygonStore[polygon2][1][1] - polygonStore[polygon2][0][1];  
    uVector[2] = polygonStore[polygon2][1][2] - polygonStore[polygon2][0][2];  

    vVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][0][0];
    vVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][0][1];  
    vVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][0][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    /*
     * Average the two normals and store them into the proper location
     */
    polygonStore[polygon1][1][7] = polygonStore[polygon2][0][7] = (normal1Vec3[0] + normal2Vec3[0]) / 2;
    polygonStore[polygon1][1][8] = polygonStore[polygon2][0][8] = (normal1Vec3[1] + normal2Vec3[1]) / 2;
    polygonStore[polygon1][1][9] = polygonStore[polygon2][0][9] = (normal1Vec3[2] + normal2Vec3[2]) / 2;


    uVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][2][0];
    uVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][2][1];  
    uVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][2][2];  

    vVector[0] = polygonStore[polygon1][1][0] - polygonStore[polygon1][2][0];
    vVector[1] = polygonStore[polygon1][1][1] - polygonStore[polygon1][2][1];  
    vVector[2] = polygonStore[polygon1][1][2] - polygonStore[polygon1][2][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    uVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][3][0];
    uVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][3][1];  
    uVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][3][2];  

    vVector[0] = polygonStore[polygon2][1][0] - polygonStore[polygon2][3][0];
    vVector[1] = polygonStore[polygon2][1][1] - polygonStore[polygon2][3][1];  
    vVector[2] = polygonStore[polygon2][1][2] - polygonStore[polygon2][3][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    /*
     * Average the two normals and store them into the proper location
     */
    polygonStore[polygon1][2][7] = polygonStore[polygon2][3][7] = (normal1Vec3[0] + normal2Vec3[0]) / 2;
    polygonStore[polygon1][2][8] = polygonStore[polygon2][3][8] = (normal1Vec3[1] + normal2Vec3[1]) / 2;
    polygonStore[polygon1][2][9] = polygonStore[polygon2][3][9] = (normal1Vec3[2] + normal2Vec3[2]) / 2;
  }
  else {
    // No sides are touching the axis
  printf("pointe\n");
    uVector[0] = polygonStore[polygon1][2][0] - polygonStore[polygon1][1][0];
    uVector[1] = polygonStore[polygon1][2][1] - polygonStore[polygon1][1][1];  
    uVector[2] = polygonStore[polygon1][2][2] - polygonStore[polygon1][1][2];  

    vVector[0] = polygonStore[polygon1][0][0] - polygonStore[polygon1][1][0];
    vVector[1] = polygonStore[polygon1][0][1] - polygonStore[polygon1][1][1];  
    vVector[2] = polygonStore[polygon1][0][2] - polygonStore[polygon1][1][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    uVector[0] = polygonStore[polygon2][1][0] - polygonStore[polygon2][0][0];
    uVector[1] = polygonStore[polygon2][1][1] - polygonStore[polygon2][0][1];  
    uVector[2] = polygonStore[polygon2][1][2] - polygonStore[polygon2][0][2];  

    vVector[0] = polygonStore[polygon2][3][0] - polygonStore[polygon2][0][0];
    vVector[1] = polygonStore[polygon2][3][1] - polygonStore[polygon2][0][1];  
    vVector[2] = polygonStore[polygon2][3][2] - polygonStore[polygon2][0][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    /*
     * Average the two normals and store them into the proper location
     */
    polygonStore[polygon1][1][7] = polygonStore[polygon2][0][7] = (normal1Vec3[0] + normal2Vec3[0]) / 2;
    polygonStore[polygon1][1][8] = polygonStore[polygon2][0][8] = (normal1Vec3[1] + normal2Vec3[1]) / 2;
    polygonStore[polygon1][1][9] = polygonStore[polygon2][0][9] = (normal1Vec3[2] + normal2Vec3[2]) / 2;


    uVector[0] = polygonStore[polygon1][3][0] - polygonStore[polygon1][2][0];
    uVector[1] = polygonStore[polygon1][3][1] - polygonStore[polygon1][2][1];  
    uVector[2] = polygonStore[polygon1][3][2] - polygonStore[polygon1][2][2];  

    vVector[0] = polygonStore[polygon1][1][0] - polygonStore[polygon1][2][0];
    vVector[1] = polygonStore[polygon1][1][1] - polygonStore[polygon1][2][1];  
    vVector[2] = polygonStore[polygon1][1][2] - polygonStore[polygon1][2][2];    

    normal1Vec3 = calculateCrossProduct(uVector, vVector);

    uVector[0] = polygonStore[polygon2][0][0] - polygonStore[polygon2][3][0];
    uVector[1] = polygonStore[polygon2][0][1] - polygonStore[polygon2][3][1];  
    uVector[2] = polygonStore[polygon2][0][2] - polygonStore[polygon2][3][2];  

    vVector[0] = polygonStore[polygon2][2][0] - polygonStore[polygon2][3][0];
    vVector[1] = polygonStore[polygon2][2][1] - polygonStore[polygon2][3][1];  
    vVector[2] = polygonStore[polygon2][2][2] - polygonStore[polygon2][3][2];     

    normal2Vec3 = calculateCrossProduct(uVector, vVector);

    /*
     * Average the two normals and store them into the proper location
     */
    polygonStore[polygon1][2][7] = polygonStore[polygon2][3][7] = (normal1Vec3[0] + normal2Vec3[0]) / 2;
    polygonStore[polygon1][2][8] = polygonStore[polygon2][3][8] = (normal1Vec3[1] + normal2Vec3[1]) / 2;
    polygonStore[polygon1][2][9] = polygonStore[polygon2][3][9] = (normal1Vec3[2] + normal2Vec3[2]) / 2;
  }
}


double * calculateCrossProduct(double *uVector, double *vVector)
{
  double newX, newY, newZ;
  double normalVector[3];
  double vectorMagnitude;

  /* Calculate the cross product */
  newX = (uVector[1]) * (vVector[2]) - (uVector[2]) * (vVector[1]);
  newY = (uVector[2]) * (vVector[0]) - (uVector[0]) * (vVector[2]);
  newZ = (uVector[0]) * (vVector[1]) - (uVector[1]) * (vVector[0]);

  vectorMagnitude = sqrt( newX*newX + newY*newY + newZ*newZ );

  if(vectorMagnitude != 0)
  {
    normalVector[0] = newX / vectorMagnitude;
    normalVector[1] = newY / vectorMagnitude;
    normalVector[2] = newZ / vectorMagnitude;
  }

  return normalVector;
}


/*
 * Print the contents of polygon store
 */
void printRevolutionOfLine(double ***polygonStore)
{
  int i;
  int combineTrisIntoOnePoly;

  if( distanceFromAxis(axis, polygonStore[0][0][0], polygonStore[0][0][1], polygonStore[0][0][2]) < tolerance )
  /*
   * At the beginning of surface points meet axis of rotation -- generate triangles
   */
  {
    /*
     * Test to see if the rotation of the line makes one big polygon.
     * When this is the case, make one polygon instead of a series of adjacent
     * triangles.  This would be the case, for example, if you rotated a line
     * from (0 0 0) to (0 1 0) around the z-axis.  It makes a plane!
     */

    if( (fabs(polygonStore[0][0][2] - polygonStore[0][2][2]) < tolerance &&
	 fabs(polygonStore[0][0][2] - polygonStore[0][3][2]) < tolerance)   ||
        (fabs(polygonStore[0][0][1] - polygonStore[0][2][1]) < tolerance &&
	 fabs(polygonStore[0][0][1] - polygonStore[0][3][1]) < tolerance)   ||
        (fabs(polygonStore[0][0][0] - polygonStore[0][2][0]) < tolerance &&
	 fabs(polygonStore[0][0][0] - polygonStore[0][3][0]) < tolerance) )
    {
      combineTrisIntoOnePoly = 1;
    }
    else
    {
      combineTrisIntoOnePoly = 0;
    }

    if(combineTrisIntoOnePoly)
    {
      i = 0;
      printf("polygons\n");
      printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][3][0],
	                                        polygonStore[i][3][1],
                                                polygonStore[i][3][2],
	                                        polygonStore[i][3][3],
                                                polygonStore[i][3][4],
                                                polygonStore[i][3][5],
                                                polygonStore[i][3][6],
                                                polygonStore[i][3][7],
                                                polygonStore[i][3][8],
                                                polygonStore[i][3][9]);

      for( ; i < endOfArrayIndex; i++)
      {
        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][2][0],
	                                          polygonStore[i][2][1],
                                                  polygonStore[i][2][2],
	                                          polygonStore[i][2][3],
                                                  polygonStore[i][2][4],
                                                  polygonStore[i][2][5],
                                                  polygonStore[i][2][6],
                                                  polygonStore[i][2][7],
                                                  polygonStore[i][2][8],
                                                  polygonStore[i][2][9]);
      }
    }
    else
    {
      for(i = 0; i <= endOfArrayIndex; i++)
      {
        printf("polygons\n");
        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][0][0],
	                                          polygonStore[i][0][1],
                                                  polygonStore[i][0][2],
	                                          polygonStore[i][0][3],
                                                  polygonStore[i][0][4],
                                                  polygonStore[i][0][5],
                                                  polygonStore[i][0][6],
                                                  polygonStore[i][0][7],
                                                  polygonStore[i][0][8],
                                                  polygonStore[i][0][9]);

        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][2][0],
	                                          polygonStore[i][2][1],
                                                  polygonStore[i][2][2],
	                                          polygonStore[i][2][3],
                                                  polygonStore[i][2][4],
                                                  polygonStore[i][2][5],
                                                  polygonStore[i][2][6],
                                                  polygonStore[i][2][7],
                                                  polygonStore[i][2][8],
                                                  polygonStore[i][2][9]);

        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][3][0],
	                                          polygonStore[i][3][1],
                                                  polygonStore[i][3][2],
	                                          polygonStore[i][3][3],
                                                  polygonStore[i][3][4],
                                                  polygonStore[i][3][5],
                                                  polygonStore[i][3][6],
                                                  polygonStore[i][3][7],
                                                  polygonStore[i][3][8],
                                                  polygonStore[i][3][9]);
      }
    }
  }
  else if( distanceFromAxis(axis, polygonStore[endOfArrayIndex][3][0], polygonStore[endOfArrayIndex][3][1], polygonStore[endOfArrayIndex][3][2]) < tolerance )
  /*
   * At the end of surface points meet axis of rotation -- generate triangles
   */
  {
    /*
     * Test to see if the rotation of the line makes one big polygon.
     * When this is the case, make one polygon instead of a series of adjacent
     * triangles.  This would be the case, for example, if you rotated a line
     * from (0 0 0) to (0 1 0) around the z-axis.  It makes a plane!
     */

    if( (fabs(polygonStore[0][3][2] - polygonStore[0][0][2]) < tolerance &&
	 fabs(polygonStore[0][3][2] - polygonStore[0][1][2]) < tolerance)   ||
        (fabs(polygonStore[0][3][1] - polygonStore[0][0][1]) < tolerance &&
	 fabs(polygonStore[0][3][1] - polygonStore[0][1][1]) < tolerance)   ||
        (fabs(polygonStore[0][3][0] - polygonStore[0][0][0]) < tolerance &&
	 fabs(polygonStore[0][3][0] - polygonStore[0][1][0]) < tolerance) )
    {
      combineTrisIntoOnePoly = 1;
    }
    else
    {
      combineTrisIntoOnePoly = 0;
    }

    if(combineTrisIntoOnePoly)
    {
      i = 0;
      printf("polygons\n");
      printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][0][0],
	                                        polygonStore[i][0][1],
                                                polygonStore[i][0][2],
	                                        polygonStore[i][0][3],
                                                polygonStore[i][0][4],
                                                polygonStore[i][0][5],
                                                polygonStore[i][0][6],
                                                polygonStore[i][0][7],
                                                polygonStore[i][0][8],
                                                polygonStore[i][0][9]);

      for( ; i < endOfArrayIndex; i++)
      {
        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][1][0],
	                                          polygonStore[i][1][1],
                                                  polygonStore[i][1][2],
	                                          polygonStore[i][1][3],
                                                  polygonStore[i][1][4],
                                                  polygonStore[i][1][5],
                                                  polygonStore[i][1][6],
                                                  polygonStore[i][1][7],
                                                  polygonStore[i][1][8],
                                                  polygonStore[i][1][9]);
      }
    }
    else
    {
      for(i = 0; i <= endOfArrayIndex; i++)
      {
        printf("polygons\n");
        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][0][0],
	                                          polygonStore[i][0][1],
                                                  polygonStore[i][0][2],
	                                          polygonStore[i][0][3],
                                                  polygonStore[i][0][4],
                                                  polygonStore[i][0][5],
                                                  polygonStore[i][0][6],
                                                  polygonStore[i][0][7],
                                                  polygonStore[i][0][8],
                                                  polygonStore[i][0][9]);

        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][1][0],
	                                          polygonStore[i][1][1],
                                                  polygonStore[i][1][2],
	                                          polygonStore[i][1][3],
                                                  polygonStore[i][1][4],
                                                  polygonStore[i][1][5],
                                                  polygonStore[i][1][6],
                                                  polygonStore[i][1][7],
                                                  polygonStore[i][1][8],
                                                  polygonStore[i][1][9]);

        printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][3][0],
	                                          polygonStore[i][3][1],
                                                  polygonStore[i][3][2],
	                                          polygonStore[i][3][3],
                                                  polygonStore[i][3][4],
                                                  polygonStore[i][3][5],
                                                  polygonStore[i][3][6],
                                                  polygonStore[i][3][7],
                                                  polygonStore[i][3][8],
                                                  polygonStore[i][3][9]);
      }
    }
  }
  else
  /*
   * Generate 4-sided polygons
   */
  {
    for(i = 0; i <= endOfArrayIndex; i++)
    {
      printf("polygons\n");
      printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][0][0],
	                                        polygonStore[i][0][1],
                                                polygonStore[i][0][2],
	                                        polygonStore[i][0][3],
                                                polygonStore[i][0][4],
                                                polygonStore[i][0][5],
                                                polygonStore[i][0][6],
                                                polygonStore[i][0][7],
                                                polygonStore[i][0][8],
                                                polygonStore[i][0][9]);

      printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][1][0],
	                                        polygonStore[i][1][1],
                                                polygonStore[i][1][2],
	                                        polygonStore[i][1][3],
                                                polygonStore[i][1][4],
                                                polygonStore[i][1][5],
                                                polygonStore[i][1][6],
                                                polygonStore[i][1][7],
                                                polygonStore[i][1][8],
                                                polygonStore[i][1][9]);

      printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][2][0],
	                                        polygonStore[i][2][1],
                                                polygonStore[i][2][2],
	                                        polygonStore[i][2][3],
                                                polygonStore[i][2][4],
                                                polygonStore[i][2][5],
                                                polygonStore[i][2][6],
                                                polygonStore[i][2][7],
                                                polygonStore[i][2][8],
                                                polygonStore[i][2][9]);

      printf("%f %f %f %f %f %f %f %f %f %f\n", polygonStore[i][3][0],
	                                        polygonStore[i][3][1],
                                                polygonStore[i][3][2],
	                                        polygonStore[i][3][3],
                                                polygonStore[i][3][4],
                                                polygonStore[i][3][5],
                                                polygonStore[i][3][6],
                                                polygonStore[i][3][7],
                                                polygonStore[i][3][8],
                                                polygonStore[i][3][9]);
    }// end for loop
  } // end else
}

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: gtb-sor x/y/z [n] [degrees]\n");
	exit(1);
}






