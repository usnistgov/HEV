

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rigidXform.h"


/*

These are some routines that I cooked up to deal with 4x4 matrices
that describe rigid motions in 3D.  Note that most of these routines
will not handle matrices that include scaling, skew, or ony other
non-rigid transformations.  Only rotation and translation are handled.

Note also that these matrices are built on the model that they
transform an (xyz) point by multiplying a 1x4 row vector [x, y, z, 1] 
by the 4x4 matrix with the point on the left and the matrix on the right.

This means that the rotational part of the matrix is in the upper left
3x3 part of the matrix and the translation is in the first three columns
of the fourth row of the matrix.  The matrices are indexed [row][col].

ALL ANGLES ARE IN DEGREES.


QUATERNIONS:


My quaternions are x, y, z, w; the real component is the last.

q = w + i*x + j*y + k*z

If the angle-axis representation of the rotation is  (a, (X,Y,Z))
then the quat is:

q = cos(a/2) + i ( X * sin(a/2)) + j (Y * sin(a/2)) + k ( Z * sin(a/2))

  w is cos(a/2)
  (x,y,z) is sin(a/2)*(X,Y,Z)


*/


#define   DEG_TO_RAD(a)      ((a)*M_PI/180.0)
#define   RAD_TO_DEG(a)      ((a)*180.0/M_PI)
#define   MAX(x,y)           (((x)>(y))?(x):(y))
#define   MIN(x,y)           (((x)<(y))?(x):(y))


int 
normalize3 (double v[3])
	{
	double len = sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	if (len == 0)
		{
		return -1;
		}

	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
	return 0;
	}

double dot3 (double a[3], double b[3])
	{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
	}

void 
crossProduct3 (double a[3], double b[3], double r[3])
	{
	r[0] = a[1]*b[2] - a[2]*b[1];
	r[1] = a[2]*b[0] - a[0]*b[2];
	r[2] = a[0]*b[1] - a[1]*b[0];
	}  // end of crossProduct3




static int
makeAxisRotMat (int axis, double angle, double m[4][4])
	{
	double s, c;

	identityMat (m);


	s = sin (DEG_TO_RAD(angle));
	c = cos (DEG_TO_RAD(angle));

	switch (axis)
		{
		case 'x':
		case 'X':
			m[1][1] = m[2][2] = c;
			m[1][2] = s;
			m[2][1] = -s;
			break;

		case 'y':
		case 'Y':
			m[0][0] = m[2][2] = c;
			m[0][2] = -s;
			m[2][0] = s;
			break;

		case 'z':
		case 'Z':
			m[0][0] = m[1][1] = c;
			m[0][1] = s;
			m[1][0] = -s;
			break;


		default:
			return -1;
		}

	
	return 0;
	}  // makeAxisRotMat




double
quatNorm (double q[4])
	{
	return sqrt (q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
	}


int
normalizeQuat (double q[4])
	{
	double len;

	len = quatNorm (q);
	if (len == 0)
		{
		return -1;
		}

	q[0] /= len;
	q[1] /= len;
	q[2] /= len;
	q[3] /= len;
	
	return 0;
	}	// end of normalizeQuat


void
conjugateQuat (double q[4])
	{
	q[0] *= -1;
	q[1] *= -1;
	q[2] *= -1;
	}  // end of conjugateQuat


void
multQuat (double left[4], double right[4], double result[4])
	{

	result[0] = 
			+ left[1]*right[2] 
			- left[2]*right[1] 
			+ left[0]*right[3] 
			+ left[3]*right[0] 
			;

	result[1] = 
			+ left[2]*right[0] 
			- left[0]*right[2] 
			+ left[1]*right[3] 
			+ left[3]*right[1] 
			;

	result[2] = 
			+ left[0]*right[1] 
			- left[1]*right[0] 
			+ left[2]*right[3] 
			+ left[3]*right[2] 
			;

	result[3] = 
			+ left[3]*right[3] 
			- left[0]*right[0] 
			- left[1]*right[1] 
			- left[2]*right[2] 
			;

	}  // end of multQuat

void
quatToMat (double q[4], double m[4][4])
	{

	// transpose of matrix in spacecraft book
	// same as matrix in DIVERSE matrix.cpp




	// diagonals
	m[0][0] = 1.0   -   2 * (q[1]*q[1] + q[2]*q[2]);
	m[1][1] = 1.0   -   2 * (q[0]*q[0] + q[2]*q[2]);
	m[2][2] = 1.0   -   2 * (q[0]*q[0] + q[1]*q[1]);


	m[1][0] = 2 * (q[0]*q[1] - q[3]*q[2]);
	m[2][0] = 2 * (q[0]*q[2] + q[3]*q[1]);

	m[0][1] = 2 * (q[0]*q[1] + q[3]*q[2]);
	m[2][1] = 2 * (q[1]*q[2] - q[3]*q[0]);

	m[0][2] = 2 * (q[0]*q[2] - q[3]*q[1]);
	m[1][2] = 2 * (q[1]*q[2] + q[3]*q[0]);

	m[3][0] =
	m[3][1] =
	m[3][2] =
	m[0][3] =
	m[1][3] =
	m[2][3] = 0;
	m[3][3] = 1;

	}	// end of quatToMat

int
matToQuat (double m[4][4], double q[4])
	{



	// We calculate one of the four components directly
	// from the elements on the diagonal.  Then we use
	// this value and the off-diagonal elements of the
	// matrix to calculate the other components of the
	// quaternion.  
	//

	// The element that is calced directly from the diagonal elements
	// should not be too close to zero because it is used in the
	// denominator of subsequent calculations.
	// On the other hand, we don't want to make have the threshold
	// for rejection of the this be too high or we will do 
	// more (and unneccessary) sqrts.

	// All of this comes from algebra relating the elements of
	// of the matrix to the quaternion.  It assumes that the
	// matrix is pure rotation: no scaling, skew, or anything
	// else.

	q[3] = sqrt ( 1.0 + m[0][0] + m[1][1] + m[2][2] ) / 2;
	if (q[3] > 0.01)
	  {
	  q[0] = (m[1][2] - m[2][1]) / (4 * q[3]);
	  q[1] = (m[2][0] - m[0][2]) / (4 * q[3]);
	  q[2] = (m[0][1] - m[1][0]) / (4 * q[3]);
	  // printf ("Branch 1 q:  %f %f %f %f\n", q[0], q[1], q[2], q[3]);
	  }
	else
	  {
	  q[0] = sqrt (1 + m[0][0] - m[1][1] - m[2][2]) / 2;
	  if (q[0] > 0.01)
	    {
	    q[1] = (m[0][1] + m[1][0]) / (4*q[0]);
	    q[2] = (m[0][2] + m[2][0]) / (4*q[0]);
	    q[3] = (m[1][2] - m[2][1]) / (4*q[0]);
	    // printf ("Branch 2 q:  %f %f %f %f\n", q[0], q[1], q[2], q[3]);
	    }
	  else
	    {
	    q[1] = sqrt (1 - m[0][0] + m[1][1] - m[2][2]) / 2;
	    if (q[1] > 0.01)
	      {
	      q[0] = (m[0][1] + m[1][0]) / (4*q[1]);
	      q[2] = (m[1][2] + m[2][1]) / (4*q[1]);
	      q[3] = (m[2][0] - m[0][2]) / (4*q[1]);
	      // printf ("Branch 3 q:  %f %f %f %f\n", q[0], q[1], q[2], q[3]);
	      }
	    else
	      {
	      q[2] = sqrt (1 - m[0][0] - m[1][1] + m[2][2]) / 2;
	      q[0] = (m[0][2] + m[2][0]) / (4*q[2]);
	      q[1] = (m[1][2] + m[2][1]) / (4*q[2]);
	      q[3] = (m[0][1] - m[1][0]) / (4*q[2]);
	      // printf ("Branch 4 q:  %f %f %f %f\n", q[0], q[1], q[2], q[3]);
	      }
	    
	    }
	  }

	normalizeQuat (q);

	return 0;
	}	// end of matToQuat


#if 0
int
quatRotDiff (double q0[4], double q1[4], double diffq[4])
	{
	double tempq1[4];

	copyQuat (q1, tempq1);
	conjugateQuat (tempq1);
	multQuat (q0, tempq1, diffq);
	return 0;
	}
#endif




#if 1

void
matToEulerZXY (double rot[4][4], double euler[3])
	{
	double x, y, z, sinx, cosx;

	sinx = -rot[2][1];
	x = asin (-rot[2][1]);
	cosx = sqrt (1.0 - sinx*sinx);



	if ( (cosx > 1.0e-6)  ||  (cosx < -1.0e-6) )
		{
		y = atan2 (rot[2][0]/cosx, rot[2][2]/cosx);
		z = atan2 (rot[0][1]/cosx, rot[1][1]/cosx);
		}
	else
		{
		y = 0;
		z = atan2 (-rot[1][0], rot[0][0]);
		}

	euler[0] = RAD_TO_DEG (z);
	euler[1] = RAD_TO_DEG (x);
	euler[2] = RAD_TO_DEG (y);

	}    // matToEulerZXY





#endif

void
eulerToMat (double euler[3], int order[3], double rot[4][4])
	{
	double mat0[4][4];
	double mat1[4][4];
	double mat2[4][4];
	double tmpMat[4][4];
	char ax[3] = {'x', 'y', 'z'};

	makeAxisRotMat (ax[order[0]], euler[0], mat0);
	makeAxisRotMat (ax[order[1]], euler[1], mat1);
	makeAxisRotMat (ax[order[2]], euler[2], mat2);

	multMat (mat0, mat1, tmpMat);
	multMat (tmpMat, mat2, rot);

	}	// end of eulerToMat


void
axisAngleToMat (double angle, double a[3], double m[4][4])
	{
	double c, s, t;

	c = cos (DEG_TO_RAD(angle));
	s = sin (DEG_TO_RAD(angle));
	t = 1 - c;

	m[0][0] = c + t * a[0]*a[0];
	m[1][1] = c + t * a[1]*a[1];
	m[2][2] = c + t * a[2]*a[2];

	m[1][0] = t*a[0]*a[1] - a[2]*s;
	m[2][0] = t*a[0]*a[2] + a[1]*s;

	m[0][1] = t*a[0]*a[1] + a[2]*s;
	m[2][1] = t*a[1]*a[2] - a[0]*s;

	m[0][2] = t*a[0]*a[2] - a[1]*s;
	m[1][2] = t*a[1]*a[2] + a[0]*s;

	m[3][0] =
	m[3][1] =
	m[3][2] =
	m[0][3] =
	m[1][3] =
	m[2][3] = 0;
	m[3][3] = 1;

	}  // end of axisAngleToMat


void
matToAxisAngle (double m[4][4], double *angle, double axis[3])
	{
	double q[4];


	matToQuat (m, q);


	quatToAxisAngle (q, angle, axis);


	} // end of matToAxisAngle


int
quatToAxisAngle (double q[4], double *angle, double a[3])
	{
	double len;

	*angle = RAD_TO_DEG (2*acos (   MIN (1.0, MAX(q[3],-1.0))   ));

	len = sqrt (q[0]*q[0] + q[1]*q[1] + q[2]*q[2]);

	if (len == 0)
		{
		a[0] = 1;
		a[1] = 0;
		a[2] = 0;
		}
	else
		{
		a[0] = q[0]/len;
		a[1] = q[1]/len;
		a[2] = q[2]/len;
		}

	return 0;
	}  // end of quatToAxisAngle

int
axisAngleToQuat (double angle, double a[3], double q[4])
	{
	double s;


	// quat = 
	//  cos(a/2) + i ( X * sin(a/2)) + j (Y * sin(a/2)) + k ( Z * sin(a/2))

	q[3] = cos (DEG_TO_RAD (angle)/2.0);
	s = sin (DEG_TO_RAD (angle)/2.0);
	
	q[0] = s*a[0];
	q[1] = s*a[1];
	q[2] = s*a[2];

	return 0;
	} //  end of axisAngleToQuat



	
//////////////////////////////////

void identityMat (double m[4][4])
	{
	int i, j;

	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			m[i][j] = (i==j) ?  1 : 0;
			}
		}
	}	// end of identityMat



void copyMat (double src[4][4], double dest[4][4])
	{
	memcpy (dest, src, 16 * sizeof (double));
	}    // end of copyMat



void transposeMat (double src[4][4], double dest[4][4])
	{
	int i, j;

	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			dest[i][j] = src[j][i];
			}
		}
	}    // end of transposeMat


void multMat (double left[4][4], double right[4][4], double result[4][4])
	{
	int i, j, k;

	for (i = 0; i < 4; i++)
		{
		for (j = 0; j < 4; j++)
			{
			result[i][j] = 0;
			for (k = 0; k < 4; k++)
				{
				result[i][j] += left[i][k] * right[k][j];
				}
			}
		}
	}


void xformPointByMat (double ptIn[3], double m[4][4], double ptOut[3])
	{
	ptOut[0]= ptIn[0]*m[0][0] + ptIn[1]*m[1][0] + ptIn[2]*m[2][0] + m[3][0];
	ptOut[1]= ptIn[0]*m[0][1] + ptIn[1]*m[1][1] + ptIn[2]*m[2][1] + m[3][1];
	ptOut[2]= ptIn[0]*m[0][2] + ptIn[1]*m[1][2] + ptIn[2]*m[2][2] + m[3][2];
	}



void invertRotXlateMat (double src[4][4], double dest[4][4])
	{
	// 
	
	double rot[4][4];
	double rotInv[4][4];
	double xlateInv[4][4];

	identityMat (xlateInv);

	xlateInv[3][0] = - src[3][0];
	xlateInv[3][1] = - src[3][1];
	xlateInv[3][2] = - src[3][2];

	copyMat (src, rot);
	rot[3][0] =
	rot[3][1] =
	rot[3][2] = 0;

	transposeMat (rot, rotInv);

	multMat (xlateInv, rotInv, dest);
	}	// end of invertRotXlateMat






void printMat (char *label, double mat[4][4])
        {
        int i, j;

        printf ("\n\n %s :\n", label);
        for (i = 0; i < 4; i++)
                {
                for (j = 0; j < 4; j++)
                        {
                        printf ("    %8.3f", mat[i][j]);
                        }
                printf ("\n");
                }

        printf ("\n\n");
        }


// ------------------------------------------------------


void slerp (double q1[4], double q2[4], double qr[4], double t)
	{
	double q3[4];
	int i;
	double sina, sinat, sinaomt, theta;
	double len;

	double dot = q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
	
	if (dot < 0)
		{
		dot = -dot;
		for (i=0;i<4;i++)
			{
				q3[i] = -q2[i];
			}
		}
	else
		{
		for (i=0;i<4;i++)
			{
			q3[i] = q2[i];
			}
		}


	if (dot > 0.999)
		{

		// If the two quats are close, just do linear interp
		// to avoid dividing by sin(thety) which is very close
		// to zero.

		len = 0;
		for (i=0;i<4;i++)
			{
			qr[i] = q1[i] + (q3[i]-q1[i])*t;
			len += qr[i]*qr[i];
			}

		// probably don't have to normalize, but what the heck.
		len = sqrt (len);
		qr[0] /= len;
		qr[1] /= len;
		qr[2] /= len;
		qr[3] /= len;

		}
	else
		{

		// Formula from Shoemake's paper

		theta = acos(dot);
		sina = sin(theta);
		sinat = sin(theta*t);
		sinaomt = sin(theta*(1-t));
		for (i=0;i<4;i++)
			{
			qr[i] = (q1[i]*sinaomt+q3[i]*sinat)/sina;
			}
		}

	}  // end of slerp


