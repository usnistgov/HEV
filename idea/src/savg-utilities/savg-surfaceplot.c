#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();
void find_normal(int i, int j, float* normal, float* val, int rows, int cols);
void crossProduct(float *uVector, float *vVector, float *normalVector);

main(argc, argv)
int argc;
char *argv[];
{
  // declarations
  int npts, rows, cols;
  float frows, fcols;
  char filename[STRING_LENGTH];
  char line[STRING_LENGTH];
  char temp[STRING_LENGTH];
  float val[10000];
  float normal[3];
  float rr,gg,bb;
  float min, max, range;
  int xmarks, ymarks, zmarks;
  float fxmarks, fymarks, fzmarks;
  float xscale, yscale, zscale;
  float color;
  FILE* fp;
  FILE* fp_out;
  int pargc = 0;
  char **pargv = NULL;
  int i,j,k, cnt, ndim;
  float delta, value;
  max = -10000.0;
  min = 10000.0;
  cnt = 0;
  ndim = 2;
  xmarks = 3;
  ymarks = 3;
  zmarks = 3;
  fxmarks = 3.0;
  fymarks = 3.0;
  fzmarks = 3.0;

  // take in arguments

  if(argc != 2 && argc != 5){
    usage();
    exit(EXIT_FAILURE);
  }

  // the first parameter is the data file name
  strcpy(filename, argv[1]);

  // open the file and read the header
  fp = fopen(filename, "r");

  if (!fp)
    exit(EXIT_FAILURE);

  // read in number of tick marks if given
  if (argc == 5) {
    xmarks = atoi(argv[2]);
    ymarks = atoi(argv[3]);
    zmarks = atoi(argv[4]);
    fxmarks = atof(argv[2]);
    fymarks = atof(argv[3]);
    fzmarks = atof(argv[4]);
  }

  // read in the number of rows and columns
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  rows = atoi(pargv[0]);
  frows = atof(pargv[0]);
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  cols = atoi(pargv[0]);
  fcols = atof(pargv[0]);
  npts = rows * cols;

  // read in the data, then scale
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      fgets(line, 250, fp);
      dpf_charsToArgcArgv(line, &pargc, &pargv);
      val[i*cols+j] = atof(pargv[0]);
      if (val[i*cols+j] > max) max = val[i*cols+j];
      if (val[i*cols+j] < min) min = val[i*cols+j];
    }
  }

  xscale = 1.0;
  yscale = 1.0;
  zscale = 1.0;
  if (rows > 0) {
    xscale = 1.0/(fcols - 1.0);
    yscale = 1.0/(frows - 1.0);
    zscale = 1.0/max;
  }
  //printf("xscale: %f yscale: %f zscale: %f\n",xscale, yscale, zscale);

  // scale all the points
  for (i = 0; i < rows; ++i) {
    for (j = 0; j < cols; ++j) {
      val[i*cols+j] = val[i*cols+j]*zscale;
      //printf(" %f ",val[i*cols+j]);
    }
  }
  min = min*zscale;
  max = max*zscale;
  //printf("\n");

  // loop over points
  for (i = 1; i < rows; ++i) {
    for (j = 1; j < cols; ++j) {
      // define a polygon with this point as the right top one
      // average the 4 values for the color
      k = (i-1)*cols + (j-1);
      value = 0.25 * (val[k] + val[k+1] + val[k+cols] + val[k+cols+1])/max;
      if (value < 0.16667) {
	// red
	//printf("red: value: %f\n",value);
	rr = 0.5;
	bb = 0.0;
	gg = 0.0;
      }
      if (value < 0.33333) {
	// orange
	//printf("red: value: %f\n",value);
	rr = 0.8;
	bb = 0.3;
	gg = 0.0;
      }
      if (value < 0.5) {
	// yellow
	//printf("red: value: %f\n",value);
	rr = 0.5;
	bb = 0.5;
	gg = 0.0;
      }
      else if (value < 0.66667) {
	// green
	//printf("green: value: %f\n",value);
	rr = 0.0;
	bb = 0.5;
	gg = 0.0;
      }
      else if (value < 0.83333) {
	// blue
	//printf("blue: value: %f\n",value);
	rr = 0.0;
	bb = 0.0;
	gg = 0.5;
      }
      else {
	// purple
	//printf("purple: value: %f\n",value);
	rr = 0.5;
	bb = 0.0;
	gg = 0.5;
      }

      printf("polygons\n");

      // first point is i-1,j-1
      // compute the normal at this point
      find_normal(j-1, i-1, normal, val, rows, cols);
      printf("%f %f %f %f %f %f 1.0 %f %f %f\n",(j-1)*xscale,(i-1)*yscale,val[k],
	     rr,bb,gg,normal[0],normal[1],normal[2]);
      // second point is i,j-1
      find_normal(j, i-1, normal, val, rows, cols);
      printf("%f %f %f %f %f %f 1.0 %f %f %f\n",j*xscale,(i-1)*yscale,val[k+1],
	     rr,bb,gg,normal[0],normal[1],normal[2]);
      // third point is i,j
      find_normal(j, i, normal, val, rows, cols);
      printf("%f %f %f %f %f %f 1.0 %f %f %f\n",j*xscale,i*yscale,val[k+cols+1],
	     rr,bb,gg,normal[0],normal[1],normal[2]);
      // fourth point is i-1,j
      find_normal(j-1, i, normal, val, rows, cols);
      printf("%f %f %f %f %f %f 1.0 %f %f %f\n",(j-1)*xscale,i*yscale,val[k+cols],
	     rr,bb,gg,normal[0],normal[1],normal[2]);
    }
  }

  // create the wireframe
  // bottom
  printf("lines\n");
  printf("0.0 0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("1.0 0.0 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("1.0 0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("1.0 1.0 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("1.0 1.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 1.0 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("0.0 1.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 0.0 0.0 1.0 1.0 1.0 1.0\n");

  // top
  printf("lines\n");
  printf("0.0 0.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("1.0 0.0 1.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("1.0 0.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("1.0 1.0 1.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("1.0 1.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 1.0 1.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("0.0 1.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 0.0 1.0 1.0 1.0 1.0 1.0\n");

  // vertical lines
  printf("lines\n");
  printf("0.0 0.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 0.0 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("1.0 0.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("1.0 0.0 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("0.0 1.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 1.0 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("1.0 1.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("1.0 1.0 0.0 1.0 1.0 1.0 1.0\n");

  // put axes labels on the +x, +y, +z lines
  // put axes labels on the +x, +y, +z lines
  
  // make lines = 0.25 units on each axis
  printf("lines\n");
  printf("1.0  0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("1.25 0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("lines\n");
  printf("0.0 1.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 1.25 0.0 1.0 1.0 1.0 1.0\n");
  printf("lines\n");
  printf("0.0 0.0 1.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 0.0 1.25 1.0 1.0 1.0 1.0\n");

  // put arrowheads at the end of each line
  printf("lines\n");
  printf("1.25 0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("1.15 0.1 0.0 1.0 1.0 1.0 1.0\n");
  printf("lines\n");
  printf("1.25  0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("1.15 -0.1 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("0.0 1.25 0.0 1.0 1.0 1.0 1.0\n");
  printf("0.1 1.15 0.0 1.0 1.0 1.0 1.0\n");
  printf("lines\n");
  printf("0.0 1.25 0.0 1.0 1.0 1.0 1.0\n");
  printf("-0.1 1.15 0.0 1.0 1.0 1.0 1.0\n");

  printf("lines\n");
  printf("0.0 0.0 1.25 1.0 1.0 1.0 1.0\n");
  printf("0.1 0.0 1.15 1.0 1.0 1.0 1.0\n");
  printf("lines\n");
  printf("0.0 0.0 1.25 1.0 1.0 1.0 1.0\n");
  printf("-0.1 0.0 1.15 1.0 1.0 1.0 1.0\n");

  // x, y, z labels
  printf("TEXT STRING %s SXYZ 0.2 0.04 0.2 XYZ 1.3 -0.05 0.0\n","X");
  printf("TEXT STRING %s SXYZ 0.2 0.04 0.2 XYZ -0.05 1.3 0.0\n","Y");
  printf("TEXT STRING %s SXYZ 0.2 0.04 0.2 XYZ -0.05 0.0 1.3\n","Z");

  // put tick marks along axes

  // put one at (0,0,0)
  printf("lines\n");
  printf("0.0  0.0 0.0 1.0 1.0 1.0 1.0\n");
  printf("0.0 -0.1 0.0 1.0 1.0 1.0 1.0\n");

  // put in xmarks-1 along x axis
  fxmarks = 1.0/(fxmarks-1.0);
  //printf("fxmarks: %f\n",fxmarks);
  for (i = 0; i < xmarks; ++i) {
    printf("lines\n");
    printf("%f  0.0 0.0 1.0 1.0 1.0 1.0\n",i*fxmarks);
    printf("%f -0.1 0.0 1.0 1.0 1.0 1.0\n",i*fxmarks);
    //printf("x tick: %f\n",i*fxmarks);
  }

  fymarks = 1.0/(fymarks-1.0);
  //printf("fymarks: %f\n",fymarks);
  for (i = 0; i < ymarks; ++i) {
    printf("lines\n");
    printf("0.0  %f 0.0 1.0 1.0 1.0 1.0\n",i*fymarks);
    printf("-0.1 %f 0.0 1.0 1.0 1.0 1.0\n",i*fymarks);
    //printf("y tick: %f\n",i*fymarks);
  }

  fzmarks = 1.0/(fzmarks-1.0);
  //printf("fzmarks: %f\n",fzmarks);
  for (i = 1; i < zmarks; ++i) {
    printf("lines\n");
    printf("0.0  0.0 %f 1.0 1.0 1.0 1.0\n",i*fzmarks);
    printf("-0.1 0.0 %f 1.0 1.0 1.0 1.0\n",i*fzmarks);
    //printf("z tick: %f maxz: %f\n",i*fzmarks,maxz);
  }

    // add text to the tick marks
  fxmarks = fxmarks/xscale;
  for (i = 0; i < xmarks; ++i) {
    float ii = (float)(i) * fxmarks;
    if (xscale > 20.0) {
      sprintf(temp, "%7.5f", ii);
    }
    else
      sprintf(temp, "%5.1f", i*fxmarks);
    printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ %f -0.1 0.0\n",temp,i*fxmarks*xscale-0.05);
  }

  fymarks = fymarks/yscale;
  for (i = 0; i < ymarks; ++i) {
    float ii = (float)(i) * fymarks;
    if (yscale > 20.0) {
      sprintf(temp, "%7.5f", ii);
    }
    else
      sprintf(temp, "%5.1f", i*fymarks);
    printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ -0.12 %f 0.0\n",temp,i*fymarks*yscale);
  }

  fzmarks = fzmarks/zscale; 
  for (i = 1; i < zmarks; ++i) {
    float ii = (float)(i) * fzmarks;
    if (zscale > 20.0) {
      sprintf(temp, "%7.5f", ii);
    }
    else
      sprintf(temp, "%5.1f", i*fzmarks);
    printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ -0.05 0.0 %f\n",temp,i*fzmarks*zscale);
  }
}

void usage()
{
  fprintf(stderr, "\n\tERROR - incorrect command line- \n\t\tsavg-surfaceplot filename (# tickmarks in x,y,z (default 3)) \n\n\tFILE FORMAT:\n\t  # of rows\n\t  # of columns\n\t  data(0,0)\n\t  data(1,0)\n\t  ...\n");
}		

void find_normal(int i, int j, float* normal, float* val, int rows, int cols) {
  //printf("val 2,3: %f\n",val[2]);
  int k;
  float u[3], v[3], w[3];
  float fi,fj,fk;
  fi = 1.0*i;
  fj = 1.0*j;
  fk = 0.0;
  k = i*cols + j;
  normal[0] = 0.0;
  normal[1] = 0.0;
  normal[2] = 0.0;

  // find contribution from bottom left
  if(i > 0 && j > 0) {
    // 2-3 x 2-1
    u[0] = -1.0;
    u[1] = 0.0;
    u[2] = val[k-1] - val[k];
    v[0] = 0.0;
    v[1] = -1.0;
    v[2] = val[k-cols] - val[k];
    crossProduct(u, v, normal); 
    fk = fk + 1.0;
  }
  //if (k == 0) printf("norm 1: %f %f %f\n",normal[0],normal[1],normal[2]);
  // find contribution from bottom right
  if(i < (rows-1) && j > 0) {
    // 3-0 x 3-1
    u[0] = 0.0;
    u[1] = -1.0;
    u[2] = val[k-cols] - val[k];
    v[0] = 1.0;
    v[1] = 0.0;
    v[2] = val[k+1] - val[k];
    crossProduct(u, v, w);
    normal[0] = normal[0] + w[0];
    normal[1] = normal[1] + w[1];
    normal[2] = normal[2] + w[2]; 
    fk = fk + 1.0;
  }
  //if (k == 0) printf("norm 2: %f %f %f\n",normal[0],normal[1],normal[2]);
  // find contribution from top left
  if(i > 0 && j < (cols-1)) {
    // 0-1 x 0-3
    u[0] = 1.0;
    u[1] = 0.0;
    u[2] = val[k-1] - val[k];
    v[0] = 0.0;
    v[1] = 1.0;
    v[2] = val[k+cols] - val[k];
    crossProduct(u, v, w);
    normal[0] = normal[0] + w[0];
    normal[1] = normal[1] + w[1];
    normal[2] = normal[2] + w[2]; 
    fk = fk + 1.0;
  }
  //if (k == 0) printf("norm 3: %f %f %f\n",normal[0],normal[1],normal[2]);
  // find contribution from top right
  if(i < (rows-1) && j < (cols-1)) {
    // 1-2 x 1-0
    u[0] = 0.0;
    u[1] = 1.0;
    u[2] = val[k+cols] - val[k];
    v[0] = -1.0;
    v[1] = 0.0;
    v[2] = val[k-1] - val[k];
    crossProduct(u, v, w);
    normal[0] = normal[0] + w[0];
    normal[1] = normal[1] + w[1];
    normal[2] = normal[2] + w[2]; 
    fk = fk + 1.0;
  }
  //if (k == 0) printf("norm 4: %f %f %f\n",normal[0],normal[1],normal[2]);

  // average
  normal[0] = normal[0]/fk;
  normal[1] = normal[1]/fk;
  normal[2] = normal[2]/fk;
  //if (k == 0) printf("norm 5: %f %f %f\n",normal[0],normal[1],normal[2]);
}

void crossProduct(float *uVector, float *vVector, float *normalVector) {
  float vectorMagnitude;
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
