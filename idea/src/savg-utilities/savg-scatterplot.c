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
  int npts;
  char filename[STRING_LENGTH];
  char line[STRING_LENGTH];
  char temp[STRING_LENGTH];
  float rgb[6][3];
  float x[100000], y[100000], z[100000];
  int type;
  float xscale, yscale, zscale, color;
  float minx, miny, minz;
  float maxx, maxy, maxz;
  int xmarks, ymarks, zmarks;
  float fxmarks, fymarks, fzmarks;
  float rr,bb,gg;
  FILE* fp;
  FILE* fp_out;
  int pargc = 0;
  char **pargv = NULL;
  int i,j,k, cnt, ndim;
  float delta;
  maxx = -100000.0;
  maxy = -100000.0;
  maxz = -100000.0;
  minx = 100000.0;
  miny = 100000.0;
  minz = 100000.0;
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
  //printf("opened %s\n",filename);

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

  // read in the number of dimensions
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  ndim = atoi(pargv[0]);
  //printf("read in number of dims: %d\n",ndim);

  // read in the number of points
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  npts = atoi(pargv[0]);
  //printf("npts: %d\n",npts);
  // read in the data, then scale
  for (i = 0; i < npts; ++i) {
    fgets(line, 250, fp);
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    x[i] = atof(pargv[0]);
    y[i] = atof(pargv[1]);
    if (ndim == 3)
      z[i] = atof(pargv[2]);
    else {
      // if only 2-d, put y value into z, make y = 0
      z[i] = y[i];
      y[i] = 0.0;
    }
    if (x[i] > maxx) maxx = x[i];
    if (y[i] > maxy) maxy = y[i];
    if (z[i] > maxz) maxz = z[i];
    if (x[i] < minx) minx = x[i];
    if (y[i] < miny) miny = y[i];
    if (z[i] < minz) minz = z[i];
  }
  
  // translate all points so the corner of the box is at (0,0,0)
  for (i = 0; i < npts; ++i) {
    x[i] = x[i] - minx;
    y[i] = y[i] - miny;
    z[i] = z[i] - minz;
  }
  //printf("maxs: %f %f %f\n",maxx,maxy,maxz);
  maxx = maxx - minx;
  maxy = maxy - miny;
  maxz = maxz - minz;
  //printf("maxs: %f %f %f\n",maxx,maxy,maxz);

  // scale
  xscale = 1.0;
  yscale = 1.0;
  zscale = 1.0;
  if (maxx > 0.0)
    xscale = 1.0/maxx;
  if (maxy > 0.0)
    yscale = 1.0/maxy;
  if (maxz > 0.0)
    zscale = 1.0/maxz;

  // choose a point size; delta is the distance from the point to any side
  delta = 0.01;
  //printf("xscale: %f\n",xscale);
  //printf("yscale: %f\n",yscale);
  //printf("zscale: %f\n",zscale);

  // scale all the points
  for (i = 0; i < npts; ++i) {
    x[i] = x[i]*xscale;
    y[i] = y[i]*yscale;
    z[i] = z[i]*zscale;
  }
  maxx = maxx*xscale;
  maxy = maxy*yscale;
  maxz = maxz*zscale;

  // loop over points
  for (i = 0; i < npts; ++i) {
  //for (i = 0; i < 3; ++i) {
    // assign a color based on the z value
    color = z[i]/maxz;
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
    //printf("color: %f z: %f maxz: %f\n",color,z[i],maxz);
    
    // create a small box at the point
    // bottom face 
    printf("polygon\n");
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]-delta,y[i]+delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]+delta,y[i]+delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]+delta,y[i]-delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]-delta,y[i]-delta,z[i]-delta,rr,bb,gg);
    
    // top face 
    printf("polygon\n");
    printf("%f %f %f %f %f %f 1.0 0 0 1\n",x[i]+delta,y[i]+delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 1\n",x[i]-delta,y[i]+delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 1\n",x[i]-delta,y[i]-delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 1\n",x[i]+delta,y[i]-delta,z[i]+delta,rr,bb,gg);
    
    // +x face
    printf("polygon\n");
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]+delta,y[i]+delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]+delta,y[i]+delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]+delta,y[i]-delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 0 -1\n",x[i]+delta,y[i]-delta,z[i]-delta,rr,bb,gg);
    
    // -x face
    printf("polygon\n");
    printf("%f %f %f %f %f %f -1.0 0 0 -1\n",x[i]-delta,y[i]+delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f -1.0 0 0 -1\n",x[i]-delta,y[i]+delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f -1.0 0 0 -1\n",x[i]-delta,y[i]-delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f -1.0 0 0 -1\n",x[i]-delta,y[i]-delta,z[i]+delta,rr,bb,gg);
    
    // +y face
    printf("polygon\n");
    printf("%f %f %f %f %f %f 1.0 0 1 0\n",x[i]+delta,y[i]+delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 1 0\n",x[i]-delta,y[i]+delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 1 0\n",x[i]-delta,y[i]+delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 1 0\n",x[i]+delta,y[i]+delta,z[i]+delta,rr,bb,gg);
    
    // -y face
    printf("polygon\n");
    printf("%f %f %f %f %f %f 1.0 0 -1 0\n",x[i]-delta,y[i]-delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 -1 0\n",x[i]+delta,y[i]-delta,z[i]-delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 -1 0\n",x[i]+delta,y[i]-delta,z[i]+delta,rr,bb,gg);
    printf("%f %f %f %f %f %f 1.0 0 -1 0\n",x[i]-delta,y[i]-delta,z[i]+delta,rr,bb,gg);
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
      if (fxmarks < 0.001) {
	sprintf(temp, "%2.1e", ii);
	printf("TEXT STRING %s SXYZ 0.05 0.02 0.1 XYZ %f -0.1 0.0\n",temp,i*fxmarks*xscale-0.05);
      }
      else {
	sprintf(temp, "%5.2f", ii);
	printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ %f -0.1 0.0\n",temp,i*fxmarks*xscale-0.05);
      }
    }
    else {
      sprintf(temp, "%5.1f", i*fxmarks);
      printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ %f -0.1 0.0\n",temp,i*fxmarks*xscale-0.05);
    }
  }

  fymarks = fymarks/yscale;
  for (i = 0; i < ymarks; ++i) {
    float ii = (float)(i) * fymarks;
    if (xscale > 20.0) {
      if (fxmarks < 0.001) {
	sprintf(temp, "%2.1e", ii);
	printf("TEXT STRING %s SXYZ 0.05 0.02 0.1 XYZ -0.12 %f 0.0\n",temp,i*fymarks*yscale);
      }
      else {
	sprintf(temp, "%5.2f", ii);
	printf("TEXT STRING %s SXYZ 0.05 0.02 0.1 XYZ -0.12 %f 0.0\n",temp,i*fymarks*yscale);
      }
    }
    else {
      sprintf(temp, "%5.1f", i*fymarks);
      printf("TEXT STRING %s SXYZ 0.1 XYZ -0.12 %f 0.0\n",temp,i*fymarks*yscale);
    }
  }

  fzmarks = fzmarks/zscale; 
  for (i = 1; i < zmarks; ++i) {
    float ii = (float)(i) * fzmarks;
    if (xscale > 20.0) {
      if (fxmarks < 0.001) {
	sprintf(temp, "%2.1e", ii);
	printf("TEXT STRING %s SXYZ 0.05 0.02 0.1 XYZ -0.05 0.0 %f\n",temp,i*fzmarks*zscale);
      }
      else {
	sprintf(temp, "%5.2f", ii);
	printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ -0.05 0.0 %f\n",temp,i*fzmarks*zscale);
      }
    }
    else {
      sprintf(temp, "%5.1f", i*fzmarks);
      printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ -0.05 0.0 %f\n",temp,i*fzmarks*zscale);
    }
  }
}

void usage()
{
  fprintf(stderr, "\n\tERROR - incorrect command line- \n\t\tsavg-scatterplot filename (# tickmarks in x,y,z (default 3)) \n\n\tFILE FORMAT:\n\t  # of dimensions\n\t  # of points\n\t  data0(x,y(,z))\n\t  data1(x,y(,z))\n\t  ...\n");
}		
