#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include "feed.h"
#include "savgLineEvaluator.c"

void usage();

main(argc, argv)
int argc;
char *argv[];
{
  // declarations
  int numlines;
  int ptline;
  int npts;
  char filename[STRING_LENGTH];
  char line[STRING_LENGTH];
  char temp[STRING_LENGTH];
  char plottitle[100];
  char label1[100], label2[100], label3[100], label4[100], label5[100];
  char label6[100], label7[100], label8[100], label9[100], label10[100];
  char xlabel[100], ylabel[100], zlabel[100];
  float rgb[10][3];
  int colors[10];
  // define z and z[line][pt on line]
  float z[25][1000];
  float xval, yval;
  int type;
  float xscale, yscale, zscale, color;
  float maxz;
  int xmarks, ymarks, zmarks;
  float fxmarks, fymarks, fzmarks;
  float rr,bb,gg;
  FILE* fp;
  FILE* fp_out;
  int pargc = 0;
  char **pargv = NULL;
  int i,j,k, cnt;
  maxz = -100000.0;
  cnt = 0;
  xmarks = 3;
  ymarks = 3;
  zmarks = 3;
  fxmarks = 3.0;
  fymarks = 3.0;
  fzmarks = 3.0;

  // fill in color table
  // salmon
  rgb[9][0] = 1.0;
  rgb[9][1] = 0.63;
  rgb[9][2] = 0.5;

  // dark green
  rgb[8][0] = 0.4;
  rgb[8][1] = 0.55;
  rgb[8][2] = 0.25;

  // pink
  rgb[7][0] = 1.0;
  rgb[7][1] = 0.7;
  rgb[7][2] = 0.77;

  // cyan
  rgb[6][0] = 0.0;
  rgb[6][1] = 0.5;
  rgb[6][2] = 0.5;

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

  // first read in a title for the plot
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  if (pargc == 1)
    strcpy(plottitle,pargv[0]);
  else {
    // put title words together in quotes
    strcpy(plottitle," ' ");
    for (i = 0; i < pargc; ++i) {
      strcat(plottitle,pargv[i]);
      strcat(plottitle," ");
    }
    strcat(plottitle," ' ");
  }

  // read in the number of lines and points per line
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  numlines = atoi(pargv[0]);
  //printf("read in number of lines: %d\n",numlines);
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  ptline = atoi(pargv[0]);
  //printf("points per line: %d\n",ptline);

  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  // read in line labels
  if (numlines > 0)
    strcpy(label1,pargv[0]);
  if (numlines > 1)
    strcpy(label2,pargv[1]);
  if (numlines > 2)
    strcpy(label3,pargv[2]);
  if (numlines > 3)
    strcpy(label4,pargv[3]);
  if (numlines > 4)
    strcpy(label5,pargv[4]);
  if (numlines > 5)
    strcpy(label6,pargv[5]);
  if (numlines > 6)
    strcpy(label7,pargv[6]);
  if (numlines > 7)
    strcpy(label8,pargv[7]);
  if (numlines > 8)
    strcpy(label9,pargv[8]);
  if (numlines > 9)
    strcpy(label10,pargv[9]);
  //printf("labels: %s %s %s\n",label1,label2,label3);

  // read in labels for the axes
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  strcpy(xlabel,pargv[0]);
  strcpy(ylabel,pargv[1]);
  strcpy(zlabel,pargv[2]);

  // read in colors
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);
  for (i = 0; i < numlines; ++i) {
    colors[i] = atoi(pargv[i]);
  }

  // read in the data, then scale
  for (i = 0; i < numlines; ++i) {
    for (j = 0; j < ptline; ++j) {
      fgets(line, 250, fp);
      dpf_charsToArgcArgv(line, &pargc, &pargv);
      z[i][j] = atof(pargv[0]);
      if (z[i][j] > maxz) maxz = z[i][j];
    }
  }

  //printf("maxs: %f %f %f\n",maxz);

  // scale
  xscale = 1.0/(float)(ptline-1);
  yscale = 1.0/(float)(numlines-1);
  if (maxz > 0.0)
    zscale = 1.0/maxz;

  // scale all the points
  // read in the data, then scale
  for (i = 0; i < numlines; ++i) {
    for (j = 0; j < ptline; ++j) {
      z[i][j] = z[i][j]*zscale;
    }
  }

  maxz = maxz*zscale;
  //printf("xscale, yscale, zscale: %f %f %f\n",xscale,yscale,zscale);

  // loop over points,creating line segments
  for (i = 0; i < numlines; ++i) {
    yval = (float)(i) * yscale;
    // assign a color based on the line number
    rr = rgb[colors[i]][0];
    bb = rgb[colors[i]][1];
    gg = rgb[colors[i]][2];

    for (j = 0; j < ptline-1; ++j) {
      xval = (float)(j) * xscale;
      printf("lines\n");
      printf("%f %f %f %f %f %f 1.0\n",xval,yval,z[i][j],rr,bb,gg);
      xval = (float)(j+1) * xscale;
      printf("%f %f %f %f %f %f 1.0\n",xval,yval,z[i][j+1],rr,bb,gg);
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
  printf("TEXT STRING %s SXYZ 0.05 0.02 0.05 XYZ 1.3 -0.05 0.0 RGBA .5 1 1 1\n",xlabel);
  printf("TEXT STRING %s SXYZ 0.05 0.02 0.05 XYZ -0.25 1.3 0.0 RGBA .5 1 1 1\n",ylabel);
  printf("TEXT STRING %s SXYZ 0.05 0.02 0.05 XYZ -0.25 0.0 1.3 RGBA .5 1 1 1\n",zlabel);

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
    sprintf(temp, "%5.1f", i*fxmarks);
    printf("TEXT STRING %s SXYZ 0.07 0.02 0.07 XYZ %f -0.1 0.0\n",temp,i*fxmarks*xscale-0.05);
  }

  for (i = 0; i < numlines; ++i) {
    float ii = (float)(i);
    if (i == 0)
      strcpy(temp, label1);
    if (i == 1)
      strcpy(temp, label2);
    if (i == 2)
      strcpy(temp, label3);
    if (i == 3)
      strcpy(temp, label4);
    if (i == 4)
      strcpy(temp, label5);
    if (i == 5)
      strcpy(temp, label6);
    if (i == 6)
      strcpy(temp, label7);
    if (i == 7)
      strcpy(temp, label8);
    if (i == 8)
      strcpy(temp, label9);
    if (i == 9)
      strcpy(temp, label10);
    printf("TEXT STRING %s SXYZ 0.07 0.02 0.07 XYZ -0.25 %f 0.0\n",temp,ii*yscale);
  }

  fzmarks = fzmarks/zscale; 
  for (i = 1; i < zmarks; ++i) {
    float ii = (float)(i) * fzmarks;
    if (zscale > 20.0) {
      if (fzmarks < 0.001) {
	sprintf(temp, "%2.1e", ii);
	printf("TEXT STRING %s SXYZ 0.07 0.02 0.07 XYZ -0.05 0.0 %f\n",temp,i*fzmarks*zscale);
      }
      else {
	sprintf(temp, "%5.2f", ii);
	printf("TEXT STRING %s SXYZ 0.07 0.02 0.07 XYZ -0.05 0.0 %f\n",temp,i*fzmarks*zscale);
      }
    }
    else {
      sprintf(temp, "%5.1f", i*fzmarks);
      printf("TEXT STRING %s SXYZ 0.07 0.02 0.07 XYZ -0.15 0.0 %f\n",temp,i*fzmarks*zscale);
    }
  }

  // put the title on the top
  printf("TEXT STRING %s SXYZ 0.1 0.02 0.1 XYZ 0.0 0.1 1.2 RGBA 0 0 1 1\n",plottitle);
}

void usage()
{
  fprintf(stderr, "\n\tERROR - incorrect command line- \n\t\tsavg-lineplot filename (# tickmarks in x,y,z (default 3)) \n\n\tFILE FORMAT:\n\t  plot title\n\t  # of lines\n\t  # of points per line\n\t  line labels, ex.:compA compB compC compD\n\t  axis labels, ex:. time component volume\n\t  colors: 0=red 1=orange 2=yellow 3=green 4=blue 5=purple\n\t          6=cyan 7=pink 8=dk green 9=salmon, ex:. 0 1 2 3\n\t  dataline1pt1\n\t  dataline1pt2\n\t  ...\n\t  dataline2pt1\n\t  dataline2pt2\n\t  ...\n");
}		


