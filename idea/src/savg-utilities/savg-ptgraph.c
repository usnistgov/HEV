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
 * savg-ptgraph 				*
 * Created by: Adele Peskin                     *
 * Created on: 8-15-06				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *    reads in a savg file and a node number 	*
 * and creates a visual connectivity net	*
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

#define ARRAY_CAPACITY (10000)
#define PI 3.141592653589793238462643

double storePoint[10000][3];
int connectivity[10000][50];
int trilist[50];
FILE* fp;
FILE* fp2;

/* Prints information on usage*/
void usage();
int newPoint(double tmpx, double tmpy, double tmpz, 
             int ntot);
void printInfo(int ntot, int nel, int selected);
double findPitch(double d1, double d2, double d3);
double findHeading(double d1, double d2, double d3);
void printPt(int pt, double heading, double pitch, double scaling, 
	     double tx, double ty, double tz);
void addOctahedron(double x, double y, double z);

int main(argc, argv)
int argc;
char** argv;
{
/*
 * INITIALIZATION
 */
	double x = 0, y = 0, z = 0; /* translation values */
	double tmpx = 0, tmpy = 0, tmpz = 0; /* storage for each point */
	char line[STRING_LENGTH];
	char templine[STRING_LENGTH];
	char assembleNewLine[STRING_LENGTH] = "";
	char tempString[20] = ""; 
	int pargc = 0;
	char **pargv  = NULL;
	int i,j,jj,jjj;
	char tempval[10];
	int ntot = 0;
	int conntot = 0;
	int nel = 0;
	int nnext = 0;
	int isit = 0;
	int selected = 1;
/*
 * COMMAND LINE PROCESSING
  if(argc != 4) { 
    usage();
    exit(EXIT_FAILURE);
  }
 */

/*
 * BODY
 */
	if(!getLine(line))
	  return 0;

	fp = fopen(argv[1], "w");
	fp2 = fopen(argv[2], "w");
	selected = atoi(argv[3]);
	//printf("selected node is: %d\n",selected);
	
        // jump to the nearest key word while not at EOF
	while(jumpToNearestKeyWord(line,pargv,pargc))
	{
	  dpf_charsToArgcArgv(line, &pargc, &pargv);
	  //printf("keyword line: %s pargc %d\n",line,pargc);
	  nnext = 0;
	  connectivity[nel][0] = 0;
	  if( !getLine(line) )
	    return 0;

	  while(isCorrectDataLine(line,pargv,pargc) || isBlankLine(line)) {
	    if(!isBlankLine(line)) {

	      dpf_charsToArgcArgv(line, &pargc, &pargv);

	      if (strcasecmp(pargv[0],"text")==0) {
	      }

	      else {
		tmpx = x + atof(pargv[0]); 
		tmpy = y + atof(pargv[1]);
		tmpz = z + atof(pargv[2]);
		// see if this is a new point
		isit = newPoint(tmpx,tmpy,tmpz,ntot);

		if (isit < 0) {
		  storePoint[ntot][0] = tmpx;
		  storePoint[ntot][1] = tmpy;
		  storePoint[ntot][2] = tmpz;
		  // add point number to element list
		  connectivity[nel][nnext+1] = ntot;
		  // keep track of nodes per element in first entry
		  connectivity[nel][0] = connectivity[nel][0] + 1;
		  ntot = ntot + 1;
		  nnext = nnext + 1;
		}
		else {
		  connectivity[nel][nnext+1] = isit;
		  // keep track of nodes per element in first entry
		  connectivity[nel][0] = connectivity[nel][0] + 1;
		  nnext = nnext + 1;
		}
	      }
	    }
	    if(!getLine(line)) {
	      // process the last element
	      nel = nel + 1;
	      // print information
	      
	      // print out file with node numbers and coordinates
	      printInfo(ntot, nel, selected);
	      return 0;
	    }
	  }
	  // process next polygon
	  nel = nel + 1;
	}

	// print out file with node numbers and coordinates
	printInfo(ntot, nel, selected);

        return 0;
}

/* Prints information on usage*/
void usage()
{
	fprintf(stderr,
	"ERROR: Incorrect command line arguments\n"
	"\n"
	"USAGE:\n"
	"\tsavg-ptgraph nodesfilename savgfilename nodenumber\n"
	"\tfilenames and node number are required.\n");
}

int newPoint(double tmpx, double tmpy, double tmpz, 
	     int ntot) {
  int isit = -1;
  int i;
  if (ntot > 0) {
    for (i = 0; i < ntot; ++i ) {
      if (fabs(tmpx - storePoint[i][0]) < 0.00001 &
	  fabs(tmpy - storePoint[i][1]) < 0.00001 &
	  fabs(tmpz - storePoint[i][2]) < 0.00001) {
	isit = i;
	//printf("found: %d\n",i);
	break;
      }
    }
  }
  return isit;
}

void printInfo(int ntot, int nel, int selected) {
  int i,j,jj,jjj,k;
  int nn;
  int onlist;
  int prev,nxt;
  int pnum, pt;
  double dir[3],hpr[3],xyz[3],distance;

  // find graph for specified nodes; store in trilist
  onlist = 0;
  fprintf(fp, "%d ",selected);
  // loop through all element connectivities
  nn = 0;
  for (j = 0; j < nel; ++j) {
    pnum = connectivity[j][0];
    // does this polygon have node i in it
    onlist = -1;
    for (jj = 0; jj < connectivity[j][0]; ++jj) {
      if (connectivity[j][jj+1] == selected) onlist = jj;
    }

    if (onlist > -1) {
      // find next and previous nodes
      if (onlist > 0) 
	prev = connectivity[j][onlist];
      else
	prev = connectivity[j][pnum];
      if (onlist < pnum-1)
	nxt = connectivity[j][onlist+2];
      else
	nxt = connectivity[j][1];

      // add prev and nxt to trilist
      onlist = -1;
      for (jj = 0; jj < nn; ++jj ) {
	if (trilist[jj] == prev) onlist = 1;
      }
      if (onlist == -1) {
	trilist[nn] = prev;
	nn = nn + 1;
      }
      onlist = -1;
      for (jj = 0; jj < nn; ++jj ) {
	if (trilist[jj] == nxt) onlist = 1;
      }
      if (onlist == -1) {
	trilist[nn] = nxt;
	nn = nn + 1;
      }
    }

  }

  for (k = 0; k < nn; ++k ) {
    fprintf(fp, " %d ",trilist[k]);
  }
  fprintf(fp, "\n");
  fclose(fp);

  // create a savg file with these nn lines, each is a cylinder
  for (k = 0; k < nn; ++k ) {
    // find the direction
    onlist = trilist[k];
    dir[0] = storePoint[onlist][0] - storePoint[selected][0];
    dir[1] = storePoint[onlist][1] - storePoint[selected][1];
    dir[2] = storePoint[onlist][2] - storePoint[selected][2];

    // find the hpr
    //printf("direction: %f %f %f\n",dir[0],dir[1],dir[2]);
    hpr[1] = findPitch(dir[0],dir[1],dir[2]);
    hpr[0] = findHeading(dir[0],dir[1],dir[2]);
    hpr[2] = 0.0;
    //printf("heading and pitch: %f %f\n",hpr[0],hpr[1]);
    distance = sqrt(dir[0]*dir[0] +dir[1]*dir[1] + dir[2]*dir[2]);

    // create 8 points for box, rotated, scaled, translated
    
    // polygon 1 = 0,1,2,3
    fprintf(fp2, "polygons\n");
    pt = 0;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 1;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 2;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	     storePoint[selected][1],storePoint[selected][2]);
    pt = 3;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    
    // polygon 2 = 1,4,5,2
    fprintf(fp2, "polygons\n");
    pt = 1;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 4;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 5;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	     storePoint[selected][1],storePoint[selected][2]);
    pt = 2;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);

    // polygon 3 = 4,6,7,5
    fprintf(fp2, "polygons\n");
    pt = 4;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 6;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 7;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	     storePoint[selected][1],storePoint[selected][2]);
    pt = 5;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);

    // polygon 4 = 6,0,3,7
    fprintf(fp2, "polygons\n");
    pt = 6;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 0;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
    pt = 3;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	     storePoint[selected][1],storePoint[selected][2]);
    pt = 7;
    printPt(pt,hpr[0],hpr[1],distance,storePoint[selected][0],
	    storePoint[selected][1],storePoint[selected][2]);
  }

  addOctahedron(storePoint[selected][0],storePoint[selected][1],
		storePoint[selected][2]);
}

double findPitch(double d1, double d2, double d3) {
 double theta ;
 double ratio ;
 if (fabs(d3) < 1.0e-6) {
   theta = 90.0;
 }
 else {
   ratio = sqrt(d1*d1 +d2*d2)/d3;
   theta = (180.0/PI)*atan(ratio);
   //printf("ratio: %f  theta: %f  atan: %f\n",ratio,theta,atan(ratio));
   if (d3 < 0.0)
     theta = 180.0 - fabs(theta);
 }

 return theta;
}

double findHeading(double d1, double d2, double d3) {
  double theta ;
  double ratio ;
  // if the z value is zero: return 90
  if (fabs(d2) < 1.0e-6) {
    // either +90 or -90
    theta = 90.0;
    if (d1 < 0.0)
      theta = -90.0;
  }
  else {
    ratio = (d1/d2);
    theta = (180.0/PI)*atan(ratio);
    if (d2 > 0.0)
      theta = 180.0 - theta;
    else
      theta = -theta;
  }
  
  return theta;
}

void printPt(int pt, double heading, double pitch, double scaling, 
	     double tx, double ty, double tz) {
  double box[8][3];
  double newbox[8][3];
  double x1,y1,z1,x2,y2,z2;

  // initial points of unscaled box
  box[0][0]= -0.01;
  box[1][0]=  0.01;
  box[2][0]=  0.01;
  box[3][0]= -0.01;
  box[4][0]=  0.01;
  box[5][0]=  0.01;
  box[6][0]= -0.01;
  box[7][0]= -0.01;

  box[0][1]=  0.01;
  box[1][1]=  0.01;
  box[2][1]=  0.01;
  box[3][1]=  0.01;
  box[4][1]= -0.01;
  box[5][1]= -0.01;
  box[6][1]= -0.01;
  box[7][1]= -0.01;

  box[0][2]=  0.0;
  box[1][2]=  0.0;
  box[2][2]=  1.0;
  box[3][2]=  1.0;
  box[4][2]=  0.0;
  box[5][2]=  1.0;
  box[6][2]=  0.0;
  box[7][2]=  1.0;

  x1 = box[pt][0]*1.0;
  y1 = box[pt][1]*1.0;
  z1 = box[pt][2]*scaling;

  // pitch
  x2 = x1;
  y2 = y1 * cos(pitch*PI/180.0) - z1 * sin(pitch*PI/180.0);
  z2 = z1 * cos(pitch*PI/180.0) + y1 * sin(pitch*PI/180.0);

  // heading
  x1 = x2;
  y1 = y2;
  z1 = z2;
  x2 = x1 * cos(heading*PI/180.0) - y1 * sin(heading*PI/180.0);
  y2 = y1 * cos(heading*PI/180.0) + x1 * sin(heading*PI/180.0);
  z2 = z1;

  // translate
  x2 = x2 + tx;
  y2 = y2 + ty;
  z2 = z2 + tz;
  fprintf(fp2, "%f %f %f 0.5 0.5 1.0 1.0\n",x2,y2,z2);
}

void addOctahedron(double xx, double yy, double zz) {
  // add a red octahedron at the selected point
  // create one and then translate to the point
  double x1,x2,x3,x4,x5,x6;
  double y1,y2,y3,y4,y5,y6;
  double z1,z2,z3,z4,z5,z6;

  x1 = 0.03;
  y1 = 0.0;
  z1 = 0.0;

  x2 = 0.0;
  y2 = 0.03;
  z2 = 0.0;

  x3 = 0.0;
  y3 = 0.0;
  z3 = 0.03;

  x4 = -0.03;
  y4 = 0.0;
  z4 = 0.0;

  x5 = 0.0;
  y5 = -0.03;
  z5 = 0.0;

  x6 = 0.0;
  y6 = 0.0;
  z6 = -0.03;

  // top
  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x1+xx,y1+yy,z1+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x2+xx,y2+yy,z2+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x3+xx,y3+yy,z3+zz);

  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x2+xx,y2+yy,z2+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x4+xx,y4+yy,z4+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x3+xx,y3+yy,z3+zz);

  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x4+xx,y4+yy,z4+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x5+xx,y5+yy,z5+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x3+xx,y3+yy,z3+zz);

  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x5+xx,y5+yy,z5+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x1+xx,y1+yy,z1+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x3+xx,y3+yy,z3+zz);


  // bottom
  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x6+xx,y6+yy,z6+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x2+xx,y2+yy,z2+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x1+xx,y1+yy,z1+zz);

  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x6+xx,y6+yy,z6+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x4+xx,y4+yy,z4+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x2+xx,y2+yy,z2+zz);

  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x6+xx,y6+yy,z6+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x5+xx,y5+yy,z5+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x4+xx,y4+yy,z4+zz);

  fprintf(fp2, "polygons\n");
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x6+xx,y6+yy,z6+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x1+xx,y1+yy,z1+zz);
  fprintf(fp2, "%f %f %f 1 0 0 1\n",x5+xx,y5+yy,z5+zz);
}
