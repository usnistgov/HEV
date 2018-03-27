/************************************************
 * rotate.h					*
 * Created by: Jessica Chang and Harry Bullen	*
 * Created on: 7-11-02				*
 * Current Version: 1.00			*
 *						*
 * Description:					*
 *   Rotate takes 5 arguments, the first 3 of	*
 * which are the x, y, and z coordinates of the *
 * given point.  The fourth argument is the axis*
 * around which the point must rotate.  Finally,*
 * the fifth argument specifies the radians 	*
 * over which the point will rotate.  Rotate	*
 * returns the new rotated points as x, y, and 	*
 * z.						*
 ************************************************/

#ifndef ROTATE_H
#define ROTATE_H

#include <math.h>

#ifndef PI
#define PI 3.141592653589793238462643
#endif

enum ax {X_AXIS, Y_AXIS, Z_AXIS};

rotate(x,y,z,axis,rad)

double *x;
double *y;
double *z;
enum ax axis;
double rad;
{
	double newX, newY, newZ;
	double cosine, sine;
	cosine=cos(rad);
	sine=sin(rad);

	if (axis == X_AXIS)
	{
		newX = *x;
		newY = cosine*(*y)-sine*(*z);
		newZ = sine*(*y)+cosine*(*z);	 
	}

	else if (axis == Y_AXIS)
	{
		newX = cosine*(*x)+sine*(*z);
		newY = (*y);
		newZ = -sine*(*x)+cosine*(*z);
	}

	else if (axis == Z_AXIS)
	{
		newX = cosine*(*x)-sine*(*y);
		newY = sine*(*x)+cosine*(*y);
		newZ = (*z);
	}

	*x=newX;
	*y=newY;
	*z=newZ;

	return 0;
}

rotates(x,y,z,dr,dt,dp)

double *x;
double *y;
double *z;
double dt;
double dp;
double dr;
{
	double newX, newY, newZ;
	double r,theta,phi;
	double newr,newtheta,newphi;
	double xx,yy;
	xx = (*x);
	yy = (*y);
	printf("next point in rotates: %f %f %f\n",xx,yy,fabs(xx));
	// given the point, find r,theta,phi
	r = sqrt((*x)*(*x) + (*y)*(*y) + (*z)*(*z));

	if (r < 0.00001) {
	  printf("this point is the origin\n");
	  theta = 0.0;
	  phi = 0.0;
	}
	else {
	  // not the origin
	  printf("not the origin\n");
	  if (fabs(xx) > 0.00001) {
	    printf("x is greater than 0: %f\n",yy/xx);
	    theta = atan(yy/xx);
	    // put in correct quadrant
	    if ((*x) < 0.0000001) {
	      // not in region of arctan - fix
	      if (fabs(*y) < 0.000000001)
		theta = PI;
	      else if((*y) > 0.00)
		theta = (PI/2.0) - theta;
	      else
		// angle pi to 3pi/2
		theta = PI + (PI/2.0 - theta);
	    }
	    printf ("found theta %f\n",theta);
	  }
	  else {
	    // x is zero: the y axis: pi/2 or -pi/2
	    printf("x is zero: +- pi/2\n");
	    if ((*y) > -0.00001)
	      theta = PI/2.0;
	    else
	      theta = -PI/2.0;
	  }
	  if (fabs((*z)) > 0.00001) {
	    printf("z non-zero: \n");
	    phi = atan(sqrt((*x)*(*x) + (*y)*(*y))/(*z));
	    // put in correct quadrant
	    if ((*z) < 0.0) {
	      // not in region of arctan - fix
	      // angle pi/2 to pi
	      theta = (PI/4.0) - theta;
	    }
	  }
	  else {
	    printf("z is zero\n");
	    phi = PI/2.0;
	  }
	}
	printf("for point: %f %f %f\n",(*x),(*y),(*z));
	printf("oldr,t,p: %f %f %f\n",r,theta,phi);

	// update these values
	newr = r + dr;
	newtheta = theta + dt;
	newphi = phi + dp;

	// find new x,y,z
	printf("newr,t,p: %f %f %f\n",newr,newtheta,newphi);
	newX = newr * sin(newphi) * cos(newtheta);
	newY = newr * sin(newphi) * sin(newtheta);
	newZ = newr * cos(newphi);

	*x=newX;
	*y=newY;
	*z=newZ;

	return 0;
}

#endif /*#ifndef ROTATE_H */
