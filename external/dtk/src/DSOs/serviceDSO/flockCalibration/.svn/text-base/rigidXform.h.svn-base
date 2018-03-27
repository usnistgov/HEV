#ifndef __RIGIDXFORM_H__
#define __RIGIDXFORM_H__


#ifdef __cplusplus
extern "C" {
#endif


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

My quaternions are x, y, z, w; the real component is the last.

*/

int normalize3 (double v[3]);
double dot3 (double a[3], double b[3]);
void crossProduct3 (double a[3], double b[3], double r[3]);


void identityMat (double m[4][4]);
void multMat (double left[4][4], double right[4][4], double result[4][4]);
void transposeMat (double src[4][4], double dest[4][4]);
void invertRotXlateMat (double src[4][4], double dest[4][4]);
void xformPointByMat (double ptIn[3], double m[4][4], double ptOut[3]);

void copyMat (double src[4][4], double dest[4][4]);


void matToEulerZXY (double rot[4][4], double euler[3]);
void eulerToMat (double euler[3], int order[3], double rot[4][4]);

void axisAngleToMat (double angle, double a[3], double m[4][4]);
void matToAxisAngle (double m[4][4], double *angle, double axis[3]);

void quatToMat (double q[4], double m[4][4]);
int  matToQuat (double m[4][4], double q[4]);

double quatNorm (double q[4]);
int  normalizeQuat (double q[4]);
void conjugateQuat (double q[4]);
void multQuat (double left[4], double right[4], double result[4]);
int  quatToAxisAngle (double q[4], double *angle, double a[3]);
int  axisAngleToQuat (double angle, double a[3], double q[4]);
void slerp (double q1[4], double q2[4], double qr[4], double t);


void printMat (char *label, double mat[4][4]);

#ifdef __cplusplus
}
#endif

#endif

