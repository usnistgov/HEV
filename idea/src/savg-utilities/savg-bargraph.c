#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();

main(argc, argv)
int argc;
char *argv[];
{
  // declarations
  int rows, cols;
  float frows, fcols;
  char filename[STRING_LENGTH];
  char line[STRING_LENGTH];
  char temp[STRING_LENGTH];
  float rgb[6][3];
  float height[1000];
  int type;
  int xmarks, ymarks, zmarks;
  float fxmarks, fymarks, fzmarks;
  float xscale, yscale, zscale;
  float color, max;
  float rr,bb,gg;
  FILE* fp;
  FILE* fp_out;
  int pargc = 0;
  char **pargv = NULL;
  int i,j,k, cnt, iorf, ndim;
  float xs,ys,hs;
  rows = 1;
  cols = 1;
  max = 0;
  cnt = 0;
  ndim = 2;
  xmarks = 3;
  ymarks = 3;
  zmarks = 3;
  fxmarks = 3.0;
  fymarks = 3.0;
  fzmarks = 3.0;

  // fill in color table
  // purple
  rgb[5][0] = 0.5;
  rgb[5][1] = 0.0;
  rgb[5][2] = 0.5;

  // blue
  rgb[4][0] = 0.0;
  rgb[4][1] = 0.0;
  rgb[4][2] = 0.5;

  // green
  rgb[3][0] = 0.0;
  rgb[3][1] = 0.5;
  rgb[3][2] = 0.0;

  // yellow
  rgb[2][0] = 0.5;
  rgb[2][1] = 0.5;
  rgb[2][2] = 0.0;

  // orange
  rgb[1][0] = 0.8;
  rgb[1][1] = 0.3;
  rgb[1][2] = 0.0;

  // red
  rgb[0][0] = 0.5;
  rgb[0][1] = 0.0;
  rgb[0][2] = 0.0;

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

  // read in int or float
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  iorf = 0;
  if (atoi(pargv[0]) > 0)
    iorf = 1;

  // read in the number of dimensions
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  ndim = atoi(pargv[0]);

  // read in the number of rows and columns
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  rows = atoi(pargv[0]);
  frows = atof(pargv[0]);
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  cols = atoi(pargv[0]);
  fcols = atof(pargv[0]);

  // read in all the heights and find the max
  for (i = 0; i < rows*cols; ++i) {
    fgets(line, 250, fp);
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    height[i] = atof(pargv[0]);
    if (height[i] > max) max = height[i];
  }

  xscale = 1.0;
  yscale = 1.0;
  zscale = 1.0;
  if (rows > 0) {
    xscale = 1.0/fcols;
    yscale = 1.0/frows;
    zscale = 1.0/max;
  }

  // loop over rows
  for (i = 0; i < rows; ++i) {
    // loop over cols
    for (j = 0; j < cols; ++j) {
      // check the  next height
      if (height[cnt] > 0.0) {
	// to create the box, need x,y of start, height, color
	xs = j*xscale;
	ys = i*yscale;
	hs = height[cnt]*zscale;
	color = height[cnt]/max;
	if (color < 1.0/6.0) {
	  rr = rgb[0][0];
	  bb = rgb[0][1];
	  gg = rgb[0][2];
	}
	else if (color < 1.0/3.0) {
	  rr = rgb[1][0];
	  bb = rgb[1][1];
	  gg = rgb[1][2];
	}
	else if (color < 1.0/2.0) {
	  rr = rgb[2][0];
	  bb = rgb[2][1];
	  gg = rgb[2][2];
	}
	else if (color < 2.0/3.0) {
	  rr = rgb[3][0];
	  bb = rgb[3][1];
	  gg = rgb[3][2];
	}
	else if (color < 5.0/6.0) {
	  rr = rgb[4][0];
	  bb = rgb[4][1];
	  gg = rgb[4][2];
	}
	else {
	  rr = rgb[5][0];
	  bb = rgb[5][1];
	  gg = rgb[5][2];
	}

	// create the box

	// bottom face 
	printf("polygon\n");
	printf("%f %f 0.0 %f %f %f 1.0 0 0 -1\n",j*xscale,(i+1)*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 0 -1\n",(j+1)*xscale,(i+1)*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 0 -1\n",(j+1)*xscale,i*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 0 -1\n",j*xscale,i*yscale,rr,bb,gg);

	// top face 
	printf("polygon\n");
	printf("%f %f %f  %f %f %f 1.0 0 0 1\n",(j+1)*xscale,i*yscale,hs,rr,bb,gg);
	printf("%f %f %f  %f %f %f 1.0 0 0 1\n",(j+1)*xscale,(i+1)*yscale,hs,rr,bb,gg);
	printf("%f %f %f  %f %f %f 1.0 0 0 1\n",j*xscale,(i+1)*yscale,hs,rr,bb,gg);
	printf("%f %f %f  %f %f %f 1.0 0 0 1\n",j*xscale,i*yscale,hs,rr,bb,gg);
	
	// +x face
	printf("polygon\n");
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",(j+1)*xscale,i*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",(j+1)*xscale,(i+1)*yscale,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",(j+1)*xscale,(i+1)*yscale,hs,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",(j+1)*xscale,i*yscale,hs,rr,bb,gg);
	
	// -x face
	printf("polygon\n");
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",j*xscale,(i+1)*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",j*xscale,i*yscale,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",j*xscale,i*yscale,hs,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",j*xscale,(i+1)*yscale,hs,rr,bb,gg);
	
	// -y face
	printf("polygon\n");
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",j*xscale,i*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",(j+1)*xscale,i*yscale,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",(j+1)*xscale,i*yscale,hs,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",j*xscale,i*yscale,hs,rr,bb,gg);
	
	// +y face
	printf("polygon\n");
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",(j+1)*xscale,(i+1)*yscale,rr,bb,gg);
	printf("%f %f 0.0 %f %f %f 1.0 0 1 0\n",j*xscale,(i+1)*yscale,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",j*xscale,(i+1)*yscale,hs,rr,bb,gg);
	printf("%f %f %f %f %f %f 1.0 0 1 0\n",(j+1)*xscale,(i+1)*yscale,hs,rr,bb,gg);
      }
      ++cnt;
    }
  }

  // create the box
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
  fprintf(stderr, "\n\tERROR - incorrect command line- \n\t\tsavg-bargraph filename (# tickmarks in x,y,z (default 3)) \n\n\tFILE FORMAT:\n\t  0(int) or 1(float) data\n\t  dimensions\n\t  # of rows\n\t  # of columns\n\t  data(0,0)\n\t  data(1,0)\n\t  ...\n");
}		
