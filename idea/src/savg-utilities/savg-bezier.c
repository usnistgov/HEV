/********************************************************
 * savg-bezier 					        *
 * Created by: Adele Peskin                             *
 * Created on: 5-02-05					*
 * Current Version: 1.00				*
 *							*
 * Description:						*
 *    input 16 points and compute a bezier surface      *
 ********************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "savgLineEvaluator.c"

/* Prints information on usage */
void usage();
#define maxPerRow (100)

int main(argc, argv)
int argc;
char *argv[];
{
  int numpt = 0; // number of defining points = 16
  int numvert = 11; // number of vertices in each row
  double delta = 1.0/((double)(numvert-1));

  // store xyz and normals at each vertex
  double xv[maxPerRow][maxPerRow];
  double yv[maxPerRow][maxPerRow];
  double zv[maxPerRow][maxPerRow];
  double xn[maxPerRow][maxPerRow];
  double yn[maxPerRow][maxPerRow];
  double zn[maxPerRow][maxPerRow];
  double a,b,c,d;
  double tanga[3],tangc[3];
  double magnitude;

  char line[STRING_LENGTH];
  char filename[STRING_LENGTH];
  FILE* fp;
  double pt[16][3];
  int i,j;
  int pargc = 0;
  char **pargv  = NULL;

  // there should be 1 argument, the file name
  if (argc != 2) {
    usage();
    exit(EXIT_FAILURE);
  }

  // the first parameter is the root file name
  strcpy(filename, argv[1]);

  // open the file and read the 16 points
  fp = fopen(filename, "r");

  if (!fp)
    exit(EXIT_FAILURE);

  // read until you have all the data
  // read the incoming file
  while (fgets(line, 250, fp) && numpt < 16) {
    // parse each line and reprint to either one or both new files
    dpf_charsToArgcArgv(line, &pargc, &pargv);

    if (pargc > 2) {
      pt[numpt][0] = (double)atof(pargv[0]);
      pt[numpt][1] = (double)atof(pargv[1]);
      pt[numpt][2] = (double)atof(pargv[2]);
      numpt = numpt + 1;
    }
  }

  // computes points and normals
  for (i = 0; i < numvert; ++i) {
    a = 1.0 - (double)i * delta;
    b = 1.0 - a;
    for (j = 0; j < numvert; ++j) {
      c = 1.0 - (double)j * delta;;
      d = 1.0 - c;

      // first get the coordinates at this point
      xv[i][j] = pt[0][0]*a*a*a*c*c*c      + pt[1][0]*3.0*a*a*a*c*c*d + 
	         pt[2][0]*3.0*a*a*a*c*d*d  + pt[3][0]*a*a*a*d*d*d +
	         pt[4][0]*3.0*a*a*b*c*c*c  + pt[5][0]*9.0*a*a*b*c*c*d + 
	         pt[6][0]*9.0*a*a*b*c*d*d  + pt[7][0]*3.0*a*a*b*d*d*d +
	         pt[8][0]*3.0*a*b*b*c*c*c  + pt[9][0]*9.0*a*b*b*c*c*d + 
	         pt[10][0]*9.0*a*b*b*c*d*d + pt[11][0]*3.0*a*b*b*d*d*d +
	         pt[12][0]*b*b*b*c*c*c     + pt[13][0]*3.0*b*b*b*c*c*d + 
	         pt[14][0]*3.0*b*b*b*c*d*d + pt[15][0]*b*b*b*d*d*d;

      yv[i][j] = pt[0][1]*a*a*a*c*c*c      + pt[1][1]*3.0*a*a*a*c*c*d + 
	         pt[2][1]*3.0*a*a*a*c*d*d  + pt[3][1]*a*a*a*d*d*d +
	         pt[4][1]*3.0*a*a*b*c*c*c  + pt[5][1]*9.0*a*a*b*c*c*d + 
	         pt[6][1]*9.0*a*a*b*c*d*d  + pt[7][1]*3.0*a*a*b*d*d*d +
	         pt[8][1]*3.0*a*b*b*c*c*c  + pt[9][1]*9.0*a*b*b*c*c*d + 
	         pt[10][1]*9.0*a*b*b*c*d*d + pt[11][1]*3.0*a*b*b*d*d*d +
	         pt[12][1]*b*b*b*c*c*c     + pt[13][1]*3.0*b*b*b*c*c*d + 
	         pt[14][1]*3.0*b*b*b*c*d*d + pt[15][1]*b*b*b*d*d*d;

      zv[i][j] = pt[0][2]*a*a*a*c*c*c      + pt[1][2]*3.0*a*a*a*c*c*d + 
	         pt[2][2]*3.0*a*a*a*c*d*d  + pt[3][2]*a*a*a*d*d*d +
	         pt[4][2]*3.0*a*a*b*c*c*c  + pt[5][2]*9.0*a*a*b*c*c*d + 
	         pt[6][2]*9.0*a*a*b*c*d*d  + pt[7][2]*3.0*a*a*b*d*d*d +
	         pt[8][2]*3.0*a*b*b*c*c*c  + pt[9][2]*9.0*a*b*b*c*c*d + 
	         pt[10][2]*9.0*a*b*b*c*d*d + pt[11][2]*3.0*a*b*b*d*d*d +
	         pt[12][2]*b*b*b*c*c*c     + pt[13][2]*3.0*b*b*b*c*c*d + 
	         pt[14][2]*3.0*b*b*b*c*d*d + pt[15][2]*b*b*b*d*d*d;

      // get the tangent vectors
      tanga[0] = pt[0][0]*3.0*a*a*c*c*c   + pt[1][0]*9.0*a*a*c*c*d + 
	         pt[2][0]*9.0*a*a*c*d*d   + pt[3][0]*3.0*a*a*d*d*d +
	         pt[4][0]*3.0*(2.0*a-3.0*a*a)*c*c*c + 
	         pt[5][0]*9.0*(2.0*a-3.0*a*a)*c*c*d + 
	         pt[6][0]*9.0*(2.0*a-3.0*a*a)*c*d*d + 
	         pt[7][0]*3.0*(2.0*a-3.0*a*a)*d*d*d +
	         pt[8][0]*3.0*(1.0-4.0*a+3.0*a*a)*c*c*c  + 
	         pt[9][0]*9.0*(1.0-4.0*a+3.0*a*a)*c*c*d + 
	         pt[10][0]*9.0*(1.0-4.0*a+3.0*a*a)*c*d*d + 
	         pt[11][0]*3.0*(1.0-4.0*a+3.0*a*a)*d*d*d +
	         pt[12][0]*3.0*(2.0*a-1.0-a*a)*c*c*c + 
	         pt[13][0]*9.0*(2.0*a-1.0-a*a)*c*c*d + 
	         pt[14][0]*9.0*(2.0*a-1.0-a*a)*c*d*d + 
	         pt[15][0]*3.0*(2.0*a-1.0-a*a)*d*d*d;

      tanga[1] = pt[0][1]*3.0*a*a*c*c*c   + pt[1][1]*9.0*a*a*c*c*d + 
	         pt[2][1]*9.0*a*a*c*d*d   + pt[3][1]*3.0*a*a*d*d*d +
	         pt[4][1]*3.0*(2.0*a-3.0*a*a)*c*c*c + 
	         pt[5][1]*9.0*(2.0*a-3.0*a*a)*c*c*d + 
	         pt[6][1]*9.0*(2.0*a-3.0*a*a)*c*d*d + 
	         pt[7][1]*3.0*(2.0*a-3.0*a*a)*d*d*d +
	         pt[8][1]*3.0*(1.0-4.0*a+3.0*a*a)*c*c*c  + 
	         pt[9][1]*9.0*(1.0-4.0*a+3.0*a*a)*c*c*d + 
	         pt[10][1]*9.0*(1.0-4.0*a+3.0*a*a)*c*d*d + 
	         pt[11][1]*3.0*(1.0-4.0*a+3.0*a*a)*d*d*d +
	         pt[12][1]*3.0*(2.0*a-1.0-a*a)*c*c*c + 
	         pt[13][1]*9.0*(2.0*a-1.0-a*a)*c*c*d + 
	         pt[14][1]*9.0*(2.0*a-1.0-a*a)*c*d*d + 
	         pt[15][1]*3.0*(2.0*a-1.0-a*a)*d*d*d;

      tanga[2] = pt[0][2]*3.0*a*a*c*c*c   + pt[1][2]*9.0*a*a*c*c*d + 
	         pt[2][2]*9.0*a*a*c*d*d   + pt[3][2]*3.0*a*a*d*d*d +
	         pt[4][2]*3.0*(2.0*a-3.0*a*a)*c*c*c + 
	         pt[5][2]*9.0*(2.0*a-3.0*a*a)*c*c*d + 
	         pt[6][2]*9.0*(2.0*a-3.0*a*a)*c*d*d + 
	         pt[7][2]*3.0*(2.0*a-3.0*a*a)*d*d*d +
	         pt[8][2]*3.0*(1.0-4.0*a+3.0*a*a)*c*c*c  + 
	         pt[9][2]*9.0*(1.0-4.0*a+3.0*a*a)*c*c*d + 
	         pt[10][2]*9.0*(1.0-4.0*a+3.0*a*a)*c*d*d + 
	         pt[11][2]*3.0*(1.0-4.0*a+3.0*a*a)*d*d*d +
	         pt[12][2]*3.0*(2.0*a-1.0-a*a)*c*c*c + 
	         pt[13][2]*9.0*(2.0*a-1.0-a*a)*c*c*d + 
	         pt[14][2]*9.0*(2.0*a-1.0-a*a)*c*d*d + 
	         pt[15][2]*3.0*(2.0*a-1.0-a*a)*d*d*d;

      tangc[0] = pt[0][0]*3.0*a*a*a*c*c + 
	         pt[1][0]*3.0*a*a*a*(2.0*c-3.0*c*c) + 
	         pt[2][0]*3.0*a*a*a*(1.0-4.0*c+3.0*c*c) + 
	         pt[3][0]*3.0*a*a*a*(2.0*c-1.0-c*c) +

	         pt[4][0]*9.0*a*a*b*c*c + 
	         pt[5][0]*9.0*a*a*b*(2.0*c-3.0*c*c) + 
	         pt[6][0]*9.0*a*a*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[7][0]*9.0*a*a*b*(2.0*c-1.0-c*c) +

	         pt[8][0]*9.0*a*b*b*c*c + 
	         pt[9][0]*9.0*a*b*b*(2.0*c-3.0*c*c) + 
	         pt[10][0]*9.0*a*b*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[11][0]*9.0*a*b*b*(2.0*c-1.0-c*c) +

	         pt[12][0]*3.0*b*b*b*c*c +
	         pt[13][0]*3.0*b*b*b*(2.0*c-3.0*c*c) + 
	         pt[14][0]*3.0*b*b*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[11][0]*9.0*a*b*b*(2.0*c-1.0-c*c) +
	         pt[15][0]*3.0*b*b*b*(2.0*c-1.0-c*c);

      tangc[1] = pt[0][1]*3.0*a*a*a*c*c + 
	         pt[1][1]*3.0*a*a*a*(2.0*c-3.0*c*c) + 
	         pt[2][1]*3.0*a*a*a*(1.0-4.0*c+3.0*c*c) + 
	         pt[3][1]*3.0*a*a*a*(2.0*c-1.0-c*c) +

	         pt[4][1]*9.0*a*a*b*c*c + 
	         pt[5][1]*9.0*a*a*b*(2.0*c-3.0*c*c) + 
	         pt[6][1]*9.0*a*a*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[7][1]*9.0*a*a*b*(2.0*c-1.0-c*c) +

	         pt[8][1]*9.0*a*b*b*c*c + 
	         pt[9][1]*9.0*a*b*b*(2.0*c-3.0*c*c) + 
	         pt[10][1]*9.0*a*b*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[11][1]*9.0*a*b*b*(2.0*c-1.0-c*c) +

	         pt[12][1]*3.0*b*b*b*c*c +
	         pt[13][1]*3.0*b*b*b*(2.0*c-3.0*c*c) + 
	         pt[14][1]*3.0*b*b*b*(1.0-4.0*c+3.0*c*c) +
	         pt[15][1]*3.0*b*b*b*(2.0*c-1.0-c*c);

      tangc[2] = pt[0][2]*3.0*a*a*a*c*c + 
	         pt[1][2]*3.0*a*a*a*(2.0*c-3.0*c*c) + 
	         pt[2][2]*3.0*a*a*a*(1.0-4.0*c+3.0*c*c) + 
	         pt[3][2]*3.0*a*a*a*(2.0*c-1.0-c*c) +

	         pt[4][2]*9.0*a*a*b*c*c + 
	         pt[5][2]*9.0*a*a*b*(2.0*c-3.0*c*c) + 
	         pt[6][2]*9.0*a*a*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[7][2]*9.0*a*a*b*(2.0*c-1.0-c*c) +

	         pt[8][2]*9.0*a*b*b*c*c + 
	         pt[9][2]*9.0*a*b*b*(2.0*c-3.0*c*c) + 
	         pt[10][2]*9.0*a*b*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[11][2]*9.0*a*b*b*(2.0*c-1.0-c*c) +

	         pt[12][2]*3.0*b*b*b*c*c +
	         pt[13][2]*3.0*b*b*b*(2.0*c-3.0*c*c) +
	         pt[14][2]*3.0*b*b*b*(1.0-4.0*c+3.0*c*c) + 
	         pt[15][2]*3.0*b*b*b*(2.0*c-1.0-c*c);

      // cross the tangent vectors to get the normal vectors
      xn[i][j] = (tanga[1]*tangc[2]) - (tangc[1]*tanga[2]);
      yn[i][j] = (tangc[0]*tanga[2]) - (tanga[0]*tangc[2]);
      zn[i][j] = (tanga[0]*tangc[1]) - (tangc[0]*tanga[1]);
      
      // normalize the vectors
      magnitude = sqrt(xn[i][j]*xn[i][j] +
		       yn[i][j]*yn[i][j] +
		       zn[i][j]*zn[i][j]);
      if (magnitude > 1.0e-6) {
	xn[i][j] = xn[i][j]/magnitude;
	yn[i][j] = yn[i][j]/magnitude;
	zn[i][j] = zn[i][j]/magnitude;
      }
    }
  }

  // now put together the polygons
  for (i = 0; i < numvert-1; ++i) {
    for (j = 0; j < numvert-1; ++j) {
      printf("polygons\n");
      printf("%f %f %f %f %f %f\n",xv[i][j],yv[i][j],zv[i][j],
	                           xn[i][j],yn[i][j],zn[i][j]);
      printf("%f %f %f %f %f %f\n",xv[i+1][j],yv[i+1][j],zv[i+1][j],
	                           xn[i+1][j],yn[i+1][j],zn[i+1][j]);
      printf("%f %f %f %f %f %f\n",xv[i][j+1],yv[i][j+1],zv[i][j+1],
	                           xn[i][j+1],yn[i][j+1],zn[i][j+1]);

      printf("polygons\n");
      printf("%f %f %f %f %f %f\n",xv[i+1][j],yv[i+1][j],zv[i+1][j],
	                           xn[i+1][j],yn[i+1][j],zn[i+1][j]);
      printf("%f %f %f %f %f %f\n",xv[i+1][j+1],yv[i+1][j+1],zv[i+1][j+1],
	                           xn[i+1][j+1],yn[i+1][j+1],zn[i+1][j+1]);
      printf("%f %f %f %f %f %f\n",xv[i][j+1],yv[i][j+1],zv[i][j+1],
	                           xn[i][j+1],yn[i][j+1],zn[i][j+1]);
    }
  }
}

/* Prints information on usage */
void usage()
{
	fprintf(stderr,"ERROR: Incorrect command line arguments\n");
	fprintf(stderr,"\tUsage: savg-bezier filename\n");
	exit(1);
}





