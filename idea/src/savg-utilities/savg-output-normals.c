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
 * savg-rm-normals				*
 * Created by: Adele Peskin     		*
 * Created on: 3-31-05  			*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    savg-rm-normals removes normal values     *
 * from all lines that contain them.            *
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "savgLineEvaluator.c"

#define PI 3.141592653589793238462643

/* Prints information on usage*/
void usage();
float findPitch(float d1, float d2, float d3);
float findHeading(float d1, float d2, float d3);

int main(argc, argv)
int argc;
char *argv[];
{
/*
 * INITIALIZATION
 */
	char line[STRING_LENGTH]; 
	int pargc = 0;
	char **pargv  = NULL;
	char assembleNewLine[STRING_LENGTH] = "";
	int isTextured = 0;
	int textureDim = 0;
	int next = 0;
	//char filename[STRING_LENGTH];
	//FILE* fp;
	float x1,y1,z1,x2,y2,z2;
	float normx,normy,normz;
	float arrowtop[3];
	float arrowleft[3];
	float arrowright[3];
	float p11[3],p21[3],p31[3];
	float p12[3],p22[3],p32[3];
	float heading,pitch;
	float scale,r,g,b;
	scale = 1.0;
	r = 0.0;
	g = 0.0;
	b = 1.0;
/*
 * COMMAND LINE PROCESSING
 */
        // argument list can be 1, 2 (+scale factor), 4 (+rgb), 5(+scale,rgb)
	if( argc != 1 && argc != 2 && argc != 4 && argc != 5) {
	  usage();
	  exit(EXIT_FAILURE);
	}
	if (argc == 2 || argc == 5) {
	  scale = atof(argv[1]);
	}
	if (argc > 2) {
	  r = atof(argv[argc-3]);
	  g = atof(argv[argc-2]);
	  b = atof(argv[argc-1]);
	}
	//printf("scale r g b %f %f %f %f\n",scale,r,g,b);

	arrowtop[0] = 0.0;
	arrowtop[1] = 0.0;
	arrowtop[2] = 1.0 * scale;
	arrowleft[0] = 0.1 * scale;
	arrowleft[1] = 0.0;
	arrowleft[2] = 0.9 * scale;
	arrowright[0] = -0.1 * scale;
	arrowright[1] = 0.0;
	arrowright[2] = 0.9 * scale;
/*
 * BODY
 */ 
	if( !getLine(line) )
	  return 0;

        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line, pargv, pargc)) {
	  // see how many parameters to know if rgba info is on keyword line
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  isTextured = 0;

	  if (pargc > 1) {
	    if (strcasecmp("2DTEXTURE",pargv[1])==0) {
	      //printf("got istextured\n");
	      isTextured = 1;
	      textureDim = 2;
	    }
	    if (strcasecmp("3DTEXTURE",pargv[1])==0) {
	      isTextured = 1;
	      textureDim = 3;
	    }
	  }
	  //printf("%s\n", line);

	  if( !getLine(line) )
	    return 0;
	  
	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      // see whats in the line and then update the alpha value
	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      next = 0;
	      // if not text data
	      if (strcasecmp(pargv[0],"text")!=0) {
		// see if this line has normals in it
		if (pargc == 6 || pargc == 10 || pargc == 12) {
		  // get the xyz coordinates
		  x1 = atof(pargv[0]);
		  y1 = atof(pargv[1]);
		  z1 = atof(pargv[2]);

		  // get the normal values
		  normx = scale * (float)(atof(pargv[pargc-3]));
		  normy = scale * (float)(atof(pargv[pargc-2]));
		  normz = scale * (float)(atof(pargv[pargc-1]));
		  // printf("norm: %f %f %f\n",normx,normy,normz);

		  // get the second point
		  x2 = x1 + normx;
		  y2 = y1 + normy;
		  z2 = z1 + normz;
		  //printf("x1: %f %f %f\n",x1,y1,z1);
		  //printf("x2: %f %f %f\n",x2,y2,z2);
		  // create the arrow - find angles of rotation
		  pitch = findPitch(normx, normy, normz);
		  heading = findHeading(normx, normy, normz);
		  //printf("pitch: %f headimg: %f\n",pitch,heading);

		  // take point from the standard arrow and rotate
		  // change pitch of 3 points
		  //printf("arrowleft %f %f %f\n",arrowleft[0],arrowleft[1],arrowleft[2]);
		  p11[0] = arrowtop[0];
		  p11[1] = arrowtop[1]*cos(pitch)-arrowtop[2]*sin(pitch);
		  p11[2] = arrowtop[2]*cos(pitch)+arrowtop[1]*sin(pitch);

		  p21[0] = arrowleft[0];
		  p21[1] = arrowleft[1]*cos(pitch)-arrowleft[2]*sin(pitch);
		  p21[2] = arrowleft[2]*cos(pitch)+arrowleft[1]*sin(pitch);

		  p31[0] = arrowright[0];
		  p31[1] = arrowright[1]*cos(pitch)-arrowright[2]*sin(pitch);
		  p31[2] = arrowright[2]*cos(pitch)+arrowright[1]*sin(pitch);

		  // change heading
		  p12[0] = p11[0]*cos(heading)-p11[1]*sin(heading);
		  p12[1] = p11[1]*cos(heading)+p11[0]*sin(heading);
		  p12[2] = p11[2];

		  p22[0] = p21[0]*cos(heading)-p21[1]*sin(heading);
		  p22[1] = p21[1]*cos(heading)+p21[0]*sin(heading);
		  p22[2] = p21[2];

		  p32[0] = p31[0]*cos(heading)-p31[1]*sin(heading);
		  p32[1] = p31[1]*cos(heading)+p31[0]*sin(heading);
		  p32[2] = p31[2];

		  // translate all 3 points to the start of the arrow
		  p11[0] = p12[0] + x1;
		  p11[1] = p12[1] + y1;
		  p11[2] = p12[2] + z1;
		  //printf("line %f %f %f to %f %f %f\n",x1,y1,z1,p11[0],p11[1],p11[2]);

		  p21[0] = p22[0] + x1;
		  p21[1] = p22[1] + y1;
		  p21[2] = p22[2] + z1;

		  p31[0] = p32[0] + x1;
		  p31[1] = p32[1] + y1;
		  p31[2] = p32[2] + z1;

		  // print out the arrow
		  printf("lines\n");
		  printf("%f %f %f %f %f %f 1\n",x1,y1,z1,r,g,b);
		  printf("%f %f %f %f %f %f 1\n",p11[0],p11[1],p11[2],r,g,b);
		  printf("lines\n");
		  printf("%f %f %f %f %f %f 1\n",p11[0],p11[1],p11[2],r,g,b);
		  printf("%f %f %f %f %f %f 1\n",p21[0],p21[1],p21[2],r,g,b);
		  printf("lines\n");
		  printf("%f %f %f %f %f %f 1\n",p11[0],p11[1],p11[2],r,g,b);
		  printf("%f %f %f %f %f %f 1\n",p31[0],p31[1],p31[2],r,g,b);
		}
	      }
	    }

	    if(!getLine(line))
	      return 0;
	  }
	}
	return 0;
}

float findPitch(float d1, float d2, float d3) {
 float theta ;
 double ratio ;
 if (fabs(d3) < 1.0e-6) {
   theta = 90.0;
 }
 else {
   ratio = sqrt(d1*d1 +d2*d2)/d3;
   if (fabs(ratio)>1.0e-6) {
     theta = (180.0/PI)*atan(ratio);
   }
   else
     theta = 0.0;
   if (d3 < 1.0e-6)
     theta = 180.0 - fabs(theta);
 }

 // theta is degrees, return in radians
 theta = (PI/180.0)*theta;
 return theta;
}

float findHeading(float d1, float d2, float d3) {
  float theta ;
  float ratio ;
  // if the y value is zero: return 90
  if (fabs(d2) < 1.0e-6) {
    // either +90 or -90
    theta = 90.0;
    if (d1 < -1.0e-6)
      theta = -90.0;
  }
  else {
    ratio = (d1/d2);
    if (fabs(ratio) < 1.0e-6)
      theta = 0.0;
    else
      theta = (180.0/PI)*atan(ratio);
  }
  if (d2 > -1.0e-6)
    theta = 180.0 - theta;
  else
    theta = -theta;

  // theta is degrees, return in radians
  theta = (PI/180.0)*theta;
  return theta;
}

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tsavg-output-normals (scale factor) (rgb values)\n");
}
