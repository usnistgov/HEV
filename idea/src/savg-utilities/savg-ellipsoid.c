/*
 * 
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
 * savg-sphere				        *
 * Created by: Adele Peskin     		*
 * Modified by: Brandon M. Smith on 16 July '04 *
 * Created on: 4-13-04  			*
 * Current Version: 2.00			*
 *						*
 * Description:	                                *
 *    savg-sphere creates a sphere          	*
 * in savg format from a set of x,y,z points.   *			   
 * Its command line arguments are -n nval, and  *
 * -r rval,  where nval is the requested number *
 * of polygons, and rval is the                 *
 * radius of the sphere. At each level, the     *
 * edges of the current triangles are bisected  *
 * and each new point is pushed to the surface. *   
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifndef PI
#define PI 3.1415927
#endif

#define MAX 1000

/* Prints information on usage*/
void usage();
int findRows(int poly);
float printPoly(float* xp, float* yp, float* zp, 
		int n1, int n2, int n3,
		float aa, float bb, float cc);
double nearestPoint(double u, double v, double w, 
		    double a, double b, double c);
double nearestPoint2(double u, double v, double w, 
		    double a, double b, double c);
int main(argc, argv)
int argc;
char *argv[];
{
  int i,j;
  float r;	/* radius of sphere */
  int poly;     /* no. of polygons requested */
  int pts;	/* number of points in the input file */
  int rows;     /* rows of elements */
  float xp[MAX],yp[MAX],zp[MAX];
  float theta[MAX],phi[MAX];
  int conn[MAX][3];
  int cnt = 0;
  int nel = 0;
  int rp, sets, start, oldcount;
  float totalarea = 0.0;
  float totalarea2 = 0.0;
  float totalarea3 = 0.0;
  float area[MAX],uV[3],vV[3],nuv[3];
  float delta_theta, delta_phi;
  int first,first2,last,nquarter,nelquarter,n1,n2,n3;
  float theta2, phi2,zrow,zz,fract,ratio,dtheta,dphi;
  double a = 1.0;
  double b = 1.0;
  double c = 1.0;
  float p0[3],pend[3],rr,nearest,rch,tch;
  double mean,stdev;
/*
 * INITIALIZATION
 */

  poly = 128; /* default */
  r=1.0; /* default */
  pts = 6;
  rows = 2;
  
/*
 * COMMAND LINE PROCESSING
 */

  if (argc > 1) {
    // first argument is npoints
    rows = atoi(argv[1]);
    //printf("rows: %d\n",rows);
  }

  if (argc > 4) {
    a = atof(argv[2]);
    b = atof(argv[3]);
    c = atof(argv[4]);
  }
  else {
    usage();
    exit(EXIT_FAILURE);
  }

  // pts = number of points in each quarter ellipsoid
  pts = 0;
  cnt = 0;

  /* print points for theta: 0-2*PI; phi:0-PI/2 */
  /* calculate points and project each point to the sphere */
  delta_phi = (PI/2.0)/(rows);

  /* start with top point */
  theta[0] = 0.0;
  phi[0] = 0.0;
  xp[cnt] = a*sin(phi[0])*cos(theta[0]);
  yp[cnt] = b*sin(phi[0])*sin(theta[0]);
  zp[cnt] = c*cos(phi[0]);
  ++cnt;

  for (i = 1; i<(rows+1); ++i) {
    //find the first and last points on this line
    p0[0] = (a/rows) * (i-1);
    p0[1] = 0.0;
    pend[0] = 0.0;
    p0[1] = 0.0;
    pend[0] = 0.0;
    pend[1] = (b/rows) * (i-1);
    zp[cnt] = (i-1)/rows;
    //printf("p0,pend z  %f %f %f\n",p0[0],pend[1],zp[cnt]);
    for (j=0; j<=i; ++j) {
      fract = (1.0*j)/(1.0*i);
      //printf("fract: %f %d %d\n",fract,j,i);
      xp[cnt] = p0[0] - fract*(p0[0]-pend[0]);
      yp[cnt] = p0[1] - fract*(p0[1]-pend[1]);
      zp[cnt] = (1.0*i)/rows;
      zp[cnt] = c * (1.0-zp[cnt]);
      theta[cnt] = acos(zp[cnt]/c);
      phi[cnt] = fract*(PI/2.0);
      xp[cnt] = a*sin(theta[cnt])*cos(phi[cnt]);
      yp[cnt] = b*sin(theta[cnt])*sin(phi[cnt]);
      zp[cnt] = c*cos(theta[cnt]);
      //printf("point %f %f %f %d %d\n",xp[cnt],yp[cnt],zp[cnt],i,j);
      cnt = cnt + 1;
    }
  }
  nquarter = cnt;

  //now compute areas for each triangle and readjust
  first = 0;
  last = 0;
  for (i = 0; i < rows; ++i) {
    last = first;
    first = first+i+1;
    conn[nel][0] = first;
    conn[nel][1] = first+1;
    conn[nel][2] = last;
    //printf("conn: %d %d %d\n",conn[nel][0],conn[nel][1],conn[nel][2]);
    nel = nel + 1;
    sets = i;
    for (j = 0; j < sets; ++j) {
      conn[nel][0] = last + j;
      conn[nel][1] = first+1+j;
      conn[nel][2] = last+1+j;
      //printf("conn: %d %d %d\n",conn[nel][0],conn[nel][1],conn[nel][2]);
      nel = nel + 1;
      conn[nel][0] = first+1+j;
      conn[nel][1] = first+2+j;
      conn[nel][2] = last+1+j;
      //printf("conn: %d %d %d\n",conn[nel][0],conn[nel][1],conn[nel][2]);
      nel = nel + 1;
    }
  }
  nelquarter = nel;

  for (i=0; i<nel; ++i) {
    uV[0] = xp[conn[i][1]]-xp[conn[i][0]];
    uV[1] = yp[conn[i][1]]-yp[conn[i][0]];
    uV[2] = zp[conn[i][1]]-zp[conn[i][0]];
    vV[0] = xp[conn[i][2]]-xp[conn[i][0]];
    vV[1] = yp[conn[i][2]]-yp[conn[i][0]];
    vV[2] = zp[conn[i][2]]-zp[conn[i][0]];
    nuv[0] = (uV[1]) * (vV[2]) - (uV[2]) * (vV[1]);
    nuv[1] = (uV[2]) * (vV[0]) - (uV[0]) * (vV[2]);
    nuv[2] = (uV[0]) * (vV[1]) - (uV[1]) * (vV[0]);
    area[i] = 0.5*sqrt(nuv[0]*nuv[0] + nuv[1]*nuv[1] + nuv[2]*nuv[2]);
  }

  // edit row by row
  // find the total area in the top and bottom rows
  rch = 1.0/rows;
  tch = 0.0;
  if (rows>2) {
    totalarea2 = (area[1]+area[2]+area[3])/3.0;
    totalarea3 = 0.0;
    // compare the last row to the 2nd row     
    start = nel - (2*rows-1) - 1;
    for (j=0;j<2*rows-1;++j) {
      totalarea3 = totalarea3 + area[start+j];
    }
    totalarea3 = totalarea3/(2*rows-1);
    fract = totalarea3/totalarea2;
    if (fract<1.0) {
      ratio = fract*(rows-1)/(1.0-fract);
      tch = 1.0/(ratio*rows+rows-1.0);
      rch = ratio*tch;
      //printf("r,t: %f %f\n",rch,tch);
    }
    else {
      fract = 1.0/fract;
      ratio = fract*(rows-1)/(1.0-fract);
      tch = 1.0/(ratio*rows+rows-1.0);
      rch = ratio*tch;
      rch = rch + 4.0*tch;
      tch = -tch;
    }
  }
  
  // find new theta values
  cnt = 1;     
  dtheta = (PI/2.0)/rows;
  for (i=2; i<=(rows+1); ++i) {
    dphi = (PI/2.0)/(i-1);
    //printf("dphi for row: %d is: %f\n",i,dphi);
    for (j=0; j<i; ++j) {
      phi[cnt] = j*dphi;
      theta[cnt] = (PI/2.0)*rch*(i-1) + 
	           (PI/2.0)*tch*(i-2);
      xp[cnt] = a*sin(theta[cnt])*cos(phi[cnt]);
      yp[cnt] = b*sin(theta[cnt])*sin(phi[cnt]);
      zp[cnt] = c*cos(theta[cnt]);
      cnt = cnt + 1;
    }
  }
  
  //printf("nquarter: %d\n",nquarter);
  // now rotate each point 90 degrees to the points x<0, y>0, z>0
  for (j = 0; j < nquarter; ++j) {
    theta[cnt] = theta[j];
    phi[cnt] = phi[j] + PI/2.0;
    xp[cnt] = a*sin(theta[cnt])*cos(phi[cnt]);
    yp[cnt] = b*sin(theta[cnt])*sin(phi[cnt]);
    zp[cnt] = c*cos(theta[cnt]);
    cnt = cnt + 1;
  }
  for (j=0; j<nelquarter; ++j) {
    conn[nel][0] = conn[j][0]+nquarter;
    conn[nel][1] = conn[j][1]+nquarter;
    conn[nel][2] = conn[j][2]+nquarter;
    nel = nel + 1;
  }

  // rotate each point 180 degrees to the points x<0, y<0, z>0
  for (j = 0; j < nquarter; ++j) {
    theta[cnt] = theta[j];
    phi[cnt] = phi[j] + PI;
    xp[cnt] = a*sin(theta[cnt])*cos(phi[cnt]);
    yp[cnt] = b*sin(theta[cnt])*sin(phi[cnt]);
    zp[cnt] = c*cos(theta[cnt]);
    cnt = cnt + 1;
  }
  for (j=0; j<nelquarter; ++j) {
    conn[nel][0] = conn[j][0]+2*nquarter;
    conn[nel][1] = conn[j][1]+2*nquarter;
    conn[nel][2] = conn[j][2]+2*nquarter;
    nel = nel + 1;
  }
  
  // rotate each point 270 degrees to the points x<0, y<0, z>0
  for (j = 0; j < nquarter; ++j) {
    theta[cnt] = theta[j];
    phi[cnt] = phi[j] + 3.0*PI/2.0;
    xp[cnt] = a*sin(theta[cnt])*cos(phi[cnt]);
    yp[cnt] = b*sin(theta[cnt])*sin(phi[cnt]);
    zp[cnt] = c*cos(theta[cnt]);
    cnt = cnt + 1;
  }
  for (j=0; j<nelquarter; ++j) {
    conn[nel][0] = conn[j][0]+3*nquarter;
    conn[nel][1] = conn[j][1]+3*nquarter;
    conn[nel][2] = conn[j][2]+3*nquarter;
    nel = nel + 1;
  }
  
  oldcount = cnt;
  for (j = 0; j < oldcount; ++j) {
    xp[cnt] = xp[j];
    yp[cnt] = yp[j];
    zp[cnt] = -zp[j];
    cnt = cnt + 1;
  }
  for (j=0; j<4*nelquarter; ++j) {
    conn[nel][0] = conn[j][0]+4*nquarter;
    conn[nel][1] = conn[j][2]+4*nquarter;
    conn[nel][2] = conn[j][1]+4*nquarter;
    nel = nel + 1;
  }
  // print out the triangles
  first = 0;
  first2 = 1;
  for (i=0; i<nel; ++i) {
    n1 = conn[i][0];
    n2 = conn[i][1];
    n3 = conn[i][2];
    area[nel] = printPoly(xp, yp, zp, n1, n2, n3, a, b, c);
  }
  return 0;
}

/* Prints information on usage*/
void usage()
{
  fprintf(stderr, "ERROR: Incorrect command line arguments\n");
  fprintf(stderr, "usage: savg-ellipsoid rows a b c\n");
  exit(1);
}

float printPoly(float* xp, float* yp, float* zp,
		int n1, int n2, int n3, 
		float aa, float bb, float cc) {
  float uV[3];
  float vV[3];
  float nuv[3];
  float area;
  float nxell,nyell,nzell,magnitude;

  printf("polygon\n");

  // compute normals at each point
  nxell = xp[n1]/(aa*aa);
  nyell = yp[n1]/(bb*bb);
  nzell = zp[n1]/(cc*cc);
  magnitude = sqrt(nxell*nxell + nyell*nyell + nzell*nzell);
  nxell = nxell/magnitude;
  nyell = nyell/magnitude;
  nzell = nzell/magnitude;
  printf("%f %f %f %f %f %f\n",xp[n1],yp[n1],zp[n1],nxell,nyell,nzell);

  nxell = xp[n2]/(aa*aa);
  nyell = yp[n2]/(bb*bb);
  nzell = zp[n2]/(cc*cc);
  magnitude = sqrt(nxell*nxell + nyell*nyell + nzell*nzell);
  nxell = nxell/magnitude;
  nyell = nyell/magnitude;
  nzell = nzell/magnitude;
  printf("%f %f %f %f %f %f\n",xp[n2],yp[n2],zp[n2],nxell,nyell,nzell);

  nxell = xp[n3]/(aa*aa);
  nyell = yp[n3]/(bb*bb);
  nzell = zp[n3]/(cc*cc);
  magnitude = sqrt(nxell*nxell + nyell*nyell + nzell*nzell);
  nxell = nxell/magnitude;
  nyell = nyell/magnitude;
  nzell = nzell/magnitude;
  printf("%f %f %f %f %f %f\n",xp[n3],yp[n3],zp[n3],nxell,nyell,nzell);

  uV[0] = xp[n2] - xp[n1];
  uV[1] = yp[n2] - yp[n1];
  uV[2] = zp[n2] - zp[n1];
  vV[0] = xp[n3] - xp[n1];
  vV[1] = yp[n3] - yp[n1];
  vV[2] = zp[n3] - zp[n1];
  nuv[0] = (uV[1]) * (vV[2]) - (uV[2]) * (vV[1]);
  nuv[1] = (uV[2]) * (vV[0]) - (uV[0]) * (vV[2]);
  nuv[2] = (uV[0]) * (vV[1]) - (uV[1]) * (vV[0]);
  area = 0.5*sqrt(nuv[0]*nuv[0] + nuv[1]*nuv[1] + nuv[2]*nuv[2]);
  //printf("area: %f\n",area);
  return area;
}
