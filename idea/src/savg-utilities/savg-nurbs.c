/********************************************************
 * savg-nurbs					        *
 * Created by: Adele Peskin                             *
 * Created on: 5-11-05					*
 * Current Version: 1.00				*
 *							*
 * Description:						*
 *    compute a nurbs surface                           *
 *    (non-uniform rational bspline surface)            *
 ********************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "savgLineEvaluator.c"

/* Prints information on usage */
void usage();

void rbasis(int order, double t, int npt, double* knot, 
	    double* wt, double* basis, double* basis0,
	    double* basis1, int i, int k, double d, double e);

#define maxPerRow (100)

int main(argc, argv)
int argc;
char *argv[];
{
  int nptu = 0; // number of defining points in u direction
  int nptv = 0; // number of defining points in v direction
  int numvert = 11; // number of vertices in each row
  double delta_u, delta_v;

  // store xyz and normals at each vertex
  double xv[maxPerRow][maxPerRow];
  double yv[maxPerRow][maxPerRow];
  double zv[maxPerRow][maxPerRow];
  double xn[maxPerRow][maxPerRow];
  double yn[maxPerRow][maxPerRow];
  double zn[maxPerRow][maxPerRow];
  double a,b,c,d,e;
  double tanga[3],tangc[3];
  double magnitude;
  int order = 3; // order of basis functions
  int nu_order;
  int nv_order;
  double knotu[maxPerRow];
  double knotv[maxPerRow];
  double basis0[maxPerRow];
  double basis1[maxPerRow];
  double basisu[maxPerRow];
  double basisv[maxPerRow];
  double temp[maxPerRow];
  double wt[maxPerRow];

  char line[STRING_LENGTH];
  char filename[STRING_LENGTH];
  FILE* fp;
  double pt[maxPerRow][maxPerRow][3];
  int readKnots = 0;
  int i,j,k;
  int pargc = 0;
  char **pargv = NULL;

  // there should be 1 argument, the file name
  if (argc < 2) {
    usage();
    exit(EXIT_FAILURE);
  }

  // the first parameter is the root file name
  strcpy(filename, argv[1]);

  // open the file and read the 16 points
  fp = fopen(filename, "r");

  if (!fp)
    exit(EXIT_FAILURE);

  // see if the user wants to enter knot values
  if (argc > 2) {
    if (strcasecmp(argv[2],"-k")==0) {
      // knot values at bottom of file
      readKnots = 1;
    }
  }

  for (i = 0; i < maxPerRow; ++i) {
    wt[i] = 1.0;
  }

  // first read number of points in u and v directions
  fgets(line, 250, fp);
  dpf_charsToArgcArgv(line, &pargc, &pargv);

  if (pargc > 1) {
    nptu = atoi(pargv[0]);
    nptv = atoi(pargv[1]);
  }
  else {
    usage();
    exit(EXIT_FAILURE);
  }

  delta_u = 1.0/((double)(nptu-1));
  delta_v = 1.0/((double)(nptv-1));
  // read the control point data
  i = 0;
  j = 0;
  k = 0;
  while (i < (nptu*nptv) && fgets(line, 250, fp)) {
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    printf("line: %s\n",line);
    if (pargc > 2) {
      pt[i][j][0] = (double)atof(pargv[0]);
      pt[i][j][1] = (double)atof(pargv[1]);
      pt[i][j][2] = (double)atof(pargv[2]);
      i = i + 1;
      if (i == nptu) {
	j = 0;
	j = j + 1;
      }
    }
  }

  // either read knot data or solve for knot values below
  if (readKnots == 1) {
    fgets(line, 250, fp);
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    printf("line for u: %s\n",line);
    printf("knot u vector:");
    for (i = 0; i < pargc; ++i) {
      knotu[i] = (double)atof(pargv[i]);
      printf("%f ",knotu[i]);
    }
    printf("\n");

    fgets(line, 250, fp);
    printf("knot v vector: ");
    dpf_charsToArgcArgv(line, &pargc, &pargv);
    printf("line for v and pargc: %d %s\n",pargc,line);
    for (i = 0; i < pargc; ++i) {
      knotv[i] = (double)atof(pargv[i]);
      printf("%f ",knotv[i]);
    }
    printf("\n");
  }
  else {
    // for now just assign knot values
    knotu[0] = 0;
    knotv[0] = 0;
    printf("knot u vector: %d ",knotu[0]);
    for (i = 1; i < (nptu+order); i++) {
      if ( (i > order-1) && (i < (nptu + 1)) )
	knotu[i] = knotu[i-1] + 1;
      else
	knotu[i] = knotu[i-1];
    }

    for (i = 1; i < (nptv+order); i++) {
      if ( (i > order-1) && (i < (nptv + 1)) )
	knotv[i] = knotv[i-1] + 1;
      else
	knotv[i] = knotv[i-1];
    }
  }

  // computes points and normals
  for (i = 0; i < nptu; ++i) {
    a = 1.0 - (double)i * delta_u;
    b = 1.0 - a;
    a = a*knotu[nptu+order-1];
    b = knotu[nptu+order-1] - a;
    printf("a and b: %f %F\n",a,b);
    // find the basis functions at this parameter
    rbasis(order, a, nptu, knotu, 
	   wt, basisu, basis0, basis1, i, k, d, e);
    for (j = 0; j < nptv; ++j) {
      c = 1.0 - (double)j * delta_v;
      d = 1.0 - c;
      // find the basis functions at this parameter
      rbasis(order, c, nptv, knotv, 
	     wt, basisv, basis0, basis1, i, k, d, e);
      

      // first get the coordinates at this point
      xv[i][j] = 0;

      yv[i][j] = 0;

      zv[i][j] = 0;

      // get the tangent vectors
      tanga[0] = 0;

      tanga[1] = 0;

      tanga[2] = 0;

      tangc[0] = 0;

      tangc[1] = 0;

      tangc[2] = 0;

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
    for (j = 0; j < nptv-1; ++j) {
  for (i = 0; i < nptv; ++i) {
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

void rbasis(int order, double t, int npts, double* knot, 
	    double* wt, double* basis, double* basis0,
	    double* basis1, int i, int k, double d, double e) {

  // calculate the first order nonrational basis functions Bi,1	*/
  printf("print first temp: ");
  for (i = 0; i<npts; i++) {
    if (( t >= knot[i]) && (t < knot[i+1]))
      basis0[i] = 1;
    else
      basis0[i] = 0;
    printf(" %f ",basis0[i]);
  }
  printf("\n");

  // calculate the Bi,2
  printf("print second temp: ");
  for (i = 0; i < npts; i++) {
    if (fabs(knot[i+1]-knot[i]) > .00000001)    
      // if the lower order basis function is zero  skip the calculation
      basis1[i] = (t-knot[i])/(knot[i+1]-knot[i]) * basis0[i];
    else
      basis1[i] = 0;
    if (fabs(knot[i+2]-knot[i+1]) > .00000001)
      basis1[i] = (knot[i+2]-t)/(knot[i+2]-knot[i+1]) * basis0[i+1];
    printf(" %f ",basis1[i]);
  }
  printf("\n");


  // calculate the Bi,3
  printf("print third temp: ");
  for (i = 0; i < npts; i++) {
    if (fabs(knot[i+2]-knot[i]) > .00000001)  
      // if the lower order basis function is zero  skip the calculation
      basis[i] = (t-knot[i+1])/(knot[i+2]-knot[i]) * basis1[i];
    else
      basis[i] = 0;
    if (fabs(knot[i+3]-knot[i+1]) > .00000001)
      basis[i] = (knot[i+3]-t)/(knot[i+3]-knot[i+1]) * basis1[i+1];
    printf(" %f ",basis1[i]);
  }
  printf("\n");  
}



